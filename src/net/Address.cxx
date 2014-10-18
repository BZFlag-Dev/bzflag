/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
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
#define	MAXHOSTNAMELEN	511
#endif

//
// Address
//

Address::Address()
{
  InAddr tempAddr;

  memset(&tempAddr, 0, sizeof(tempAddr));
  tempAddr.s_addr = htonl(INADDR_ANY);
  addr.push_back(tempAddr);
}

Address::Address(const std::string& name)
{
  Address a = getHostAddress(name);
  addr.push_back(a.addr[0]);
}

Address::Address(const Address& address) : addr(address.addr)
{
  // do nothing
}

Address::Address(const InAddr& _addr)
{
  addr.push_back(_addr);
}

Address::Address(const struct sockaddr_in& _addr)
{
  addr.push_back(_addr.sin_addr);
}

Address::~Address()
{
  // do nothing
}

Address&		Address::operator=(const Address& address)
{
  addr.clear();
  addr.push_back(address.addr[0]);
  return *this;
}

Address::operator InAddr() const
{
  return addr[0];
}

bool			Address::operator==(const Address& address) const
{
  return addr[0].s_addr == address.addr[0].s_addr;
}

bool			Address::operator!=(const Address& address) const
{
  return addr[0].s_addr != address.addr[0].s_addr;
}

bool			Address::operator<(Address const& address) const
{
  return addr[0].s_addr < address.addr[0].s_addr;
}
bool			Address::isAny() const
{
  return addr[0].s_addr == htonl(INADDR_ANY);
}

bool			Address::isPrivate() const
{
  // 127.0.0.0/8
  if ((addr[0].s_addr & htonl(0xff000000u)) == htonl(0x7f000000u))
    return(true);
  // 10.0.0.0/8
  if ((addr[0].s_addr & htonl(0xff000000u)) == htonl(0x0a000000u))
    return(true);
  // 172.16.0.0/12
  if ((addr[0].s_addr & htonl(0xfff00000u)) == htonl(0xac100000u))
    return(true);
  // 192.168.0.0/16
  if ((addr[0].s_addr & htonl(0xffff0000u)) == htonl(0xc0a80000u))
    return(true);
  return(false);
}

std::string		Address::getDotNotation() const
{
  return std::string(inet_ntoa(addr[0]));
}

uint8_t			Address::getIPVersion() const {
  return 4;
}

static const struct hostent* bz_gethostbyname(const std::string &name)
{
  const struct hostent* hent = NULL;

  if (name.length() > 0)
    hent = gethostbyname(name.c_str());
  else {
    // use our own host name if none is specified
    char hostname[MAXHOSTNAMELEN+1];
    if (gethostname(hostname, MAXHOSTNAMELEN) >= 0) {
      // ensure null termination regardless of
      // gethostname() implementation details
      hostname[MAXHOSTNAMELEN] = '\0';
      hent = gethostbyname(hostname);
    }
  }
  return hent;
}

Address	Address::getHostAddress(const std::string &hname)
{
  Address a;
  InAddr tempAddr;
  int j;

  if (hname.length() > 0 && inet_aton(hname.c_str(), &tempAddr) != 0) {
    a.addr.clear();
    a.addr.push_back(tempAddr);
    return a;
  }

  const struct hostent* hent = bz_gethostbyname(hname);
  if (!hent) {
    herror("Looking up host name");
    return a;
  }

  a.addr.clear();
  for (j=0; hent->h_addr_list[j] != NULL; j++){
    ::memcpy(&tempAddr, hent->h_addr_list[j], sizeof(tempAddr));
    a.addr.push_back(tempAddr);
  }
  return a;
}

std::string		Address::getHostByAddress(InAddr addr)
{
  int addrLen = sizeof(addr);
  struct hostent* hent = gethostbyaddr((char*)&addr, addrLen, AF_INET);

  if (!hent) {
    // can't lookup name -- return in standard dot notation
    return std::string(inet_ntoa(addr));
  }
  return std::string(hent->h_name);
}

const std::string Address::getHostName(const std::string &hostname) // const
{
  const struct hostent* hent = bz_gethostbyname(hostname);
  if (!hent) {
    return std::string();
  }
  return std::string(hent->h_name);
}

void*			Address::pack(void* _buf) const
{
  unsigned char* buf = (unsigned char*)_buf;
  buf = (unsigned char*)nboPackUByte(_buf, 4);
  // everything in InAddr  is already in network byte order
  int32_t hostaddr = int32_t(addr[0].s_addr);
  ::memcpy(buf, &hostaddr, sizeof(int32_t));	buf += sizeof(int32_t);
  return (void*)buf;
}

const void*		Address::unpack(const void* _buf)
{
  const unsigned char* buf = (const unsigned char*)_buf;
  InAddr tempAddr;
  // FIXME - should actually parse the first byte to see if it's IPv4 or
  // IPv6
  ++buf;
  // everything in InAddr should be stored in network byte order
  int32_t hostaddr;
  ::memcpy(&hostaddr, buf, sizeof(int32_t));	buf += sizeof(int32_t);
  tempAddr.s_addr = u_long(hostaddr);
  addr.clear();
  addr.push_back(tempAddr);
  return buf;
}

//
// ServerId
//

void*			ServerId::pack(void* _buf) const
{
  // everything in ServerId is already in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr = int32_t(serverHost.s_addr);
  ::memcpy(buf, &hostaddr, sizeof(int32_t));	buf += sizeof(int32_t);
  ::memcpy(buf, &port, sizeof(int16_t));	buf += sizeof(int16_t);
  ::memcpy(buf, &number, sizeof(int16_t));	buf += sizeof(int16_t);
  return (void*)buf;
}

const void*		ServerId::unpack(const void* _buf)
{
  // everything in ServerId should be stored in network byte order
  const unsigned char* buf = (const unsigned char*)_buf;
  int32_t hostaddr;
  ::memcpy(&hostaddr, buf, sizeof(int32_t));	buf += sizeof(int32_t);
  ::memcpy(&port, buf, sizeof(int16_t));	buf += sizeof(int16_t);
  ::memcpy(&number, buf, sizeof(int16_t));	buf += sizeof(int16_t);
  serverHost.s_addr = u_long(hostaddr);
  return buf;
}

bool			ServerId::operator==(const ServerId& id) const
{
  return serverHost.s_addr == id.serverHost.s_addr &&
			port == id.port && number == id.number;
}

bool			ServerId::operator!=(const ServerId& id) const
{
  return serverHost.s_addr != id.serverHost.s_addr ||
			port != id.port || number != id.number;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
