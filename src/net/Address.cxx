/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface header
#include "Address.h"

// system headers
#include <string.h>
#include <string>
#include <sys/types.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <setjmp.h>
#endif

// common implementation headers
#include "bzsignal.h"
#include "ErrorHandler.h"

// local implementation headers
#include "Pack.h"

#if defined(sun)
// Solaris...
extern "C" int inet_aton(const char *, struct in_addr *);
#endif

// RFC 1035 limits the length of a fully qualified domain name to 255
// octets, so calling sysconf(_SC_HOST_NAME_MAX) to find some other
// limit and/or dynamically expanding the gethostname() buffer on
// demand are complex solutions to a problem that does not exist in
// the environments for which BZFlag is designed.  The value chosen
// here provides a substantial margin in case the RFC 1035 limit is
// raised in the future.
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  511
#endif

// helper function, might want to live someplace else
static char iptextbuf[INET6_ADDRSTRLEN];
char *sockaddr2iptext(const struct sockaddr *sa)
{
    switch(sa->sa_family)
    {
    case AF_INET:
        inet_ntop(AF_INET, &(((const struct sockaddr_in *)sa)->sin_addr), iptextbuf, INET6_ADDRSTRLEN);
        break;

    case AF_INET6:
        inet_ntop(AF_INET6, &(((const struct sockaddr_in6 *)sa)->sin6_addr), iptextbuf, INET6_ADDRSTRLEN);
        break;

    default:
        strncpy(iptextbuf, "Unknown AF", 11);
    }

    return iptextbuf;
}

// address + []:port
static char iptextport[INET6_ADDRSTRLEN + 8];
char *sockaddr2iptextport(const struct sockaddr *sa)
{
    switch(sa->sa_family)
    {
    case AF_INET:
        sprintf(iptextport, "%s:%u", sockaddr2iptext(sa), ntohs(((const struct sockaddr_in *)sa)->sin_port));
        break;

    case AF_INET6:
        sprintf(iptextport, "[%s]:%u", sockaddr2iptext(sa), ntohs(((const struct sockaddr_in6 *)sa)->sin6_port));
        break;

    default:
        sprintf(iptextport, "Unknown AF:%i", sa->sa_family);
    }

    return iptextport;
}

//
// Address
//

Address::Address()
{
    memset(&addr, 0, sizeof(addr));
}

Address::Address(const std::string &_iptextport)
{
    struct addrinfo hints;
    struct addrinfo *result;

    // FIXME: handle ip with no : and assume port 0 for banlists, etc. ?
    // FIXME: handle brackets like [2001:db8::1]:5154 ?
    // FIXME: what to do with 2001:db8::1:5154 or 2001:db8:::5154 ?

    std::size_t found = _iptextport.find_last_of(":");
    std::string tryiptext = _iptextport.substr(0, found);
    std::string port = _iptextport.substr(found + 1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    // FIXME: make this non-blocking
    int s = getaddrinfo(tryiptext.c_str(), port.c_str(), &hints, &result);
    if (s == 0)
    {
        // FIXME: try other entries
        memcpy(&addr, (const void *)result->ai_addr, sizeof(addr));
        freeaddrinfo(result);
        logDebugMessage(3, "Address() got: %s from %s\n",
                        getIpTextPort().c_str(), _iptextport.c_str());
        return;
    }
    logDebugMessage(1, "Address() failure: %s\n",_iptextport.c_str());
    memset(&addr, 0, sizeof(addr));
    return;
}

Address::Address(const Address *address)
{
    memcpy(&addr, (const void *)&address->addr, sizeof(addr));
    iptext = address->iptext;
    iptextport = address->iptextport;
}

Address::Address(const Address &address)
{
    memcpy(&addr, (const void *)&address.addr, sizeof(addr));
    iptext = address.iptext;
    iptextport = address.iptextport;
}

Address::Address(const struct sockaddr_in6 *_addr)
{
    memcpy(&addr, (const void *)_addr, sizeof(addr));
    logDebugMessage(5, "Address() got: %s\n", getIpTextPort().c_str());
}

Address::~Address()
{
    // do nothing
}

Address&        Address::operator=(const Address& address)
{
    memcpy(&addr, (const void *)&address.addr, sizeof(addr));
    return *this;
}

bool            Address::operator==(const Address& address) const
{
    // compare address ONLY, ignore port
    if (addr.sin6_family == AF_INET6 && address.addr.sin6_family == AF_INET6)
        return !memcmp(&addr.sin6_addr, &address.addr.sin6_addr, sizeof(addr.sin6_addr));
    if (addr.sin6_family == AF_INET && address.addr.sin6_family == AF_INET)
    {
        const sockaddr_in *ip4a = (const sockaddr_in *)&addr;
        const sockaddr_in *ip4b = (const sockaddr_in *)&address.addr;
        return ip4a->sin_addr.s_addr == ip4b->sin_addr.s_addr;
    }
    if (addr.sin6_family == AF_INET6 &&
            address.addr.sin6_family == AF_INET &&
            isMapped())
    {
        // addr is mapped ipv4 in ipv6
        const sockaddr_in *ip4b = (const sockaddr_in *)&address.addr;
        return addr.sin6_addr.__in6_u.__u6_addr32[3] == ip4b->sin_addr.s_addr;
    }
    if (addr.sin6_family == AF_INET &&
            address.addr.sin6_family == AF_INET6 &&
            address.isMapped())
    {
        // addrress.addr is mapped ipv4 in ipv6
        const sockaddr_in *ip4a = (const sockaddr_in *)&addr;
        return ip4a->sin_addr.s_addr == address.addr.sin6_addr.__in6_u.__u6_addr32[3];
    }
    logDebugMessage(4,"Address== mixed family: %s\n",
                    sockaddr2iptextport((const struct sockaddr *)&address.addr));
    logDebugMessage(4,"\twith: %s\n",
                    sockaddr2iptextport((const struct sockaddr *)&addr));
    return false;
}

bool            Address::operator!=(const Address& address) const
{
    return !operator==(address);
}

bool            Address::operator<(Address const& address) const
{
    return memcmp(&addr, &address.addr, sizeof(addr)) < 0;
}

/* IPv4 mapped into different IPv6/96 address blocks */
bool            Address::isMapped() const
{
    switch(addr.sin6_family)
    {
    case AF_INET6:
        // IPv4 mapped into ::ffff:a.b.c.d space
        if (addr.sin6_addr.__in6_u.__u6_addr32[0] == 0 &&
                addr.sin6_addr.__in6_u.__u6_addr32[1] == 0 &&
                addr.sin6_addr.__in6_u.__u6_addr32[2] == htonl(0xffff))
            return true;
        // NAT64 common space 64:ff9b::8.8.8.8
        else if (addr.sin6_addr.__in6_u.__u6_addr32[0] == htonl(0x0064ff9b) &&
                 addr.sin6_addr.__in6_u.__u6_addr32[1] == 0 &&
                 addr.sin6_addr.__in6_u.__u6_addr32[2] == htonl(0xffff))
            return true;
        return false;
    default:
        return false;
    }
}

bool            Address::isAny() const
{
    switch(addr.sin6_family)
    {
    case AF_INET:
        return ((const struct sockaddr_in*)&addr)->sin_addr.s_addr == htonl(INADDR_ANY);

    case AF_INET6:
        return (addr.sin6_addr.__in6_u.__u6_addr32[0] == 0 &&
                addr.sin6_addr.__in6_u.__u6_addr32[1] == 0 &&
                addr.sin6_addr.__in6_u.__u6_addr32[2] == 0 &&
                addr.sin6_addr.__in6_u.__u6_addr32[3] == 0);

    default:
        return false;
    }
}

bool            Address::isPrivate() const
{
    uint32_t ip4 = ((const struct sockaddr_in *)&addr)->sin_addr.s_addr;

    switch(addr.sin6_family)
    {
    case AF_INET:

        // 127.0.0.0/8
        if ((ip4 & htonl(0xff000000u)) == htonl(0x7f000000u))
            return true;
        // 10.0.0.0/8
        if ((ip4 & htonl(0xff000000u)) == htonl(0x0a000000u))
            return true;
        // 172.16.0.0/12
        if ((ip4 & htonl(0xfff00000u)) == htonl(0xac100000u))
            return true;
        // 192.168.0.0/16
        if ((ip4 & htonl(0xffff0000u)) == htonl(0xc0a80000u))
            return true;
        return false;

    case AF_INET6:
        // FIXME: add IPv6
        // fc00::/7
        if ((addr.sin6_addr.__in6_u.__u6_addr32[0] &
            htonl(0xfe000000u)) ==
            htonl(0xfc000000u))
            return true;
        // fe80::/10
        if ((addr.sin6_addr.__in6_u.__u6_addr32[0] &
            htonl(0xffc00000u)) ==
            htonl(0xfe800000u))
            return true;
        return false;

    default:
        return false;
    }
}

sockaddr *Address::getAddr()
{
    return (sockaddr*)&addr;
}

sockaddr_in *Address::getAddr_in()
{
    return (sockaddr_in*)&addr;
}

sockaddr_in6 *Address::getAddr_in6()
{
    return &addr;
}

in_port_t Address::getNPort() const
{
    return addr.sin6_port;
}

std::string     Address::getIpText()
{
    if (iptext.size() == 0)
        iptext = sockaddr2iptext((const struct sockaddr *)&addr);
    return iptext.c_str();
}

std::string     Address::getIpTextPort()
{
    if (iptextport.size() == 0)
        iptextport = sockaddr2iptextport((const struct sockaddr *)&addr);
    return iptextport.c_str();
}

uint8_t         Address::getIPVersion() const
{
    return addr.sin6_family;
}

static const struct hostent* bz_gethostbyname(const std::string &name)
{
    // FIXME: convert to non-blocking ares call
    const struct hostent* hent = NULL;

    if (name.length() > 0)
        hent = gethostbyname(name.c_str());
    else
    {
        // use our own host name if none is specified
        char hostname[MAXHOSTNAMELEN+1];
        if (gethostname(hostname, MAXHOSTNAMELEN) >= 0)
        {
            // ensure null termination regardless of
            // gethostname() implementation details
            hostname[MAXHOSTNAMELEN] = '\0';
            hent = gethostbyname(hostname);
        }
    }
    return hent;
}

Address Address::getHostAddress(const std::string &hname)
{
    Address a = new Address();

    // FIXME convert fron iptext without dns lookup
    //if (hname.length() > 0 && inet_aton(hname.c_str(), &a.addr.sin_addr.s_addr != 0)
    //    return a;

    const struct hostent* hent = bz_gethostbyname(hname);
    if (!hent)
    {
        herror("Looking up host name");
        return a;
    }

    // FIXME: we only look at the first IP
    //::memcpy(&addr, hent->h_addr_list[0], sizeof(addr));
    return a;
}

const std::string Address::getHostName(const std::string &hostname) // const
{
    const struct hostent* hent = bz_gethostbyname(hostname);
    if (!hent)
        return std::string();
    return std::string(hent->h_name);
}

void*           Address::pack(void* _buf) const
{
    unsigned char* buf = (unsigned char*)_buf;

    // ipv4 pointer to simplfy code
    const struct sockaddr_in *addr_in = (const struct sockaddr_in *)&addr;

    buf = (unsigned char*)nboPackUByte(_buf, addr.sin6_family);
    // should already in network byte order
    switch(addr.sin6_family)
    {
    case AF_INET:
        ::memcpy(buf, &addr_in->sin_addr.s_addr, sizeof(in_addr_t));
        buf += sizeof(in_addr_t);
        ::memcpy(buf, &addr_in->sin_port, sizeof(in_port_t));
        buf += sizeof(in_port_t);
        break;

    case AF_INET6:
        ::memcpy(buf, &addr.sin6_addr, sizeof(in6_addr));
        buf += sizeof(in6_addr);
        ::memcpy(buf, &addr.sin6_port, sizeof(in_port_t));
        buf += sizeof(in_port_t);
        break;

    default:
        logDebugMessage(0,"Address(): unknown family %u\n", addr.sin6_family);
        exit(EXIT_FAILURE);
    }
    return (void*)buf;
}

const void*     Address::unpack(const void* _buf)
{
    // ipv4 pointer to simplfy code
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;

    const unsigned char* buf = (const unsigned char*)_buf;
    memset(&addr, 0, sizeof(addr));
    // FIXME - parse first byte to see if it's IPv4 or IPv6
    u_int8_t family;
    buf = (const unsigned char*)nboUnpackUByte(buf, family);
    addr.sin6_family = family;
    switch(addr.sin6_family)
    {
    case AF_INET:
        ::memcpy(&(addr_in->sin_addr), buf, sizeof(in_addr_t));
        buf += sizeof(in_addr_t);
        ::memcpy(&addr_in->sin_port, buf, sizeof(in_port_t));
        buf += sizeof(in_port_t);
        break;

    case AF_INET6:
        ::memcpy(&addr.sin6_addr, buf, sizeof(in6_addr));
        buf += sizeof(in6_addr);
        ::memcpy(&addr.sin6_port, buf, sizeof(in_port_t));
        buf += sizeof(in_port_t);
        break;

    default:
        logDebugMessage(0, "Address(): unknown family %u\n", addr.sin6_family);
        exit(EXIT_FAILURE);
    }

    // should be stored in network byte order
    return buf;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
