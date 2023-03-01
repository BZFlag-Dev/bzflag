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
        strncpy(iptextport, "Unknown AF", 11);
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

    iptextport = _iptextport;

    std::size_t found = iptextport.find_last_of(":");
    iptext = iptextport.substr(0, found);
    std::string port = iptextport.substr(found + 1);

    memset(&hints, 0, sizeof(hints));
    //FIXME convert _iptext and fill addr
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    int s = getaddrinfo(iptext.c_str(), port.c_str(), &hints, &result);
    if (s == 0) {
        // assume first entry as we used AI_NUMERICSERV
        memcpy(&addr, (const void *)result->ai_addr, sizeof(addr));
        freeaddrinfo(result);
        return;
    }
    // FIXME: DNS lookup needed
    printf("Need DNS support in Address()\n");
    exit(EXIT_FAILURE);
}

Address::Address(const Address &address) {
    memcpy(&addr, (const void *)&address.addr, sizeof(addr));
    iptext = address.iptext;
    iptextport = address.iptextport;
}

Address::Address(const struct sockaddr_in *_addr)
{
    memcpy(&addr, (const void *)_addr, sizeof(addr));
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
    return memcmp(&addr, &address.addr, sizeof(addr));
}

bool            Address::operator!=(const Address& address) const
{
    return memcmp(&addr, &address.addr, sizeof(addr));
}

bool            Address::operator<(Address const& address) const
{
    return memcmp(&addr, &address.addr, sizeof(addr)) < 0;
}
bool            Address::isAny() const
{
    // FIXME: add IPv6
    return addr.sin_addr.s_addr == htonl(INADDR_ANY);
}

bool            Address::isPrivate() const
{
    // FIXME: add IPv6
    // 127.0.0.0/8
    if ((addr.sin_addr.s_addr & htonl(0xff000000u)) == htonl(0x7f000000u))
        return(true);
    // 10.0.0.0/8
    if ((addr.sin_addr.s_addr & htonl(0xff000000u)) == htonl(0x0a000000u))
        return(true);
    // 172.16.0.0/12
    if ((addr.sin_addr.s_addr & htonl(0xfff00000u)) == htonl(0xac100000u))
        return(true);
    // 192.168.0.0/16
    if ((addr.sin_addr.s_addr & htonl(0xffff0000u)) == htonl(0xc0a80000u))
        return(true);
    return(false);
}
struct sockaddr *Address::getAddr()
{
    return (struct sockaddr*)&addr;
}

struct sockaddr_in *Address::getAddr_in()
{
    return &addr;
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
    return addr.sin_family;
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
    Address a;

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

std::string     Address::getHostByAddress(InAddr addr)
{
    int addrLen = sizeof(addr);
    struct hostent* hent = gethostbyaddr((char*)&addr, addrLen, AF_INET);

    if (!hent)
    {
        // can't lookup name -- return in standard dot notation
        return std::string(inet_ntoa(addr));
    }
    return std::string(hent->h_name);
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
    buf = (unsigned char*)nboPackUByte(_buf, addr.sin_family);
    // should already in network byte order
    switch(addr.sin_family) {
        case AF_INET:
            ::memcpy(buf, &addr.sin_addr.s_addr, sizeof(in_addr_t));
            buf += sizeof(in_addr_t);
            ::memcpy(buf, &addr.sin_port, sizeof(in_port_t));
            buf += sizeof(in_port_t);
            break;

        case AF_INET6:
            ::memcpy(buf, &addr.sin_addr, sizeof(in6_addr));
            buf += sizeof(in6_addr);
            ::memcpy(buf, &addr.sin_port, sizeof(in_port_t));
            buf += sizeof(in_port_t);

            break;

        default:
            exit(-1);
    }
    return (void*)buf;
}

const void*     Address::unpack(const void* _buf)
{
    const unsigned char* buf = (const unsigned char*)_buf;
    memset(&addr, 0, sizeof(addr));
    // FIXME - parse first byte to see if it's IPv4 or IPv6
    u_int8_t family;
    buf = (const unsigned char*)nboUnpackUByte(buf, family);
    addr.sin_family = family;
    switch(addr.sin_family) {
        case AF_INET:
            ::memcpy(&addr.sin_addr.s_addr, buf, sizeof(in_addr_t));
            buf += sizeof(in_addr_t);
            ::memcpy(&addr.sin_port, buf, sizeof(in_port_t));
            buf += sizeof(in_port_t);
            break;

        case AF_INET6:
            ::memcpy(&addr.sin_addr.s_addr, buf, sizeof(in6_addr));
            buf += sizeof(in6_addr);
            ::memcpy(&addr.sin_port, buf, sizeof(in_port_t));
            buf += sizeof(in_port_t);
            break;

        default:
            exit(-1);
    }

    // should be stored in network byte order
    return buf;
}

//
// ServerId
//

void*           ServerId::pack(void* _buf) const
{
    // already in network byte order
    unsigned char* buf = (unsigned char*)_buf;
    assert(addr.sin_family == AF_INET);
    ::memcpy(buf, &addr.sin_addr.s_addr, sizeof(int32_t));
    buf += sizeof(int32_t);
    ::memcpy(buf, &addr.sin_port, sizeof(int16_t));
    buf += sizeof(int16_t);
    ::memcpy(buf, &number, sizeof(int16_t));
    buf += sizeof(int16_t);
    return (void*)buf;
}

const void*     ServerId::unpack(const void* _buf)
{
    // everything in ServerId should be stored in network byte order
    const unsigned char* buf = (const unsigned char*)_buf;
    addr.sin_family = AF_INET;
    ::memcpy(&addr.sin_addr.s_addr, buf, sizeof(int32_t));
    buf += sizeof(int32_t);
    ::memcpy(&addr.sin_port, buf, sizeof(int16_t));
    buf += sizeof(int16_t);
    ::memcpy(&number, buf, sizeof(int16_t));
    buf += sizeof(int16_t);
    return buf;
}

bool            ServerId::operator==(const ServerId& id) const
{
    return addr.sin_addr.s_addr == id.addr.sin_addr.s_addr &&
           addr.sin_port == id.addr.sin_port &&
           number == id.number;
}

bool            ServerId::operator!=(const ServerId& id) const
{
    return addr.sin_addr.s_addr != id.addr.sin_addr.s_addr ||
           addr.sin_port != id.addr.sin_port || number != id.number;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
