/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "Address.h"
#include <string.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <signal.h>
#endif

#if !defined(inet_aton)
  // Solaris...
  extern "C" int inet_aton(const char *, struct in_addr *);
#endif

//
// Address
//

Address			Address::localAddress("");

Address::Address()
{
  memset(&addr, 0, sizeof(addr));
  addr.s_addr = htonl(INADDR_ANY);
}

Address::Address(const BzfString& name)
{
  memset(&addr, 0, sizeof(addr));
  Address a = getHostAddress((const char*)(name.isNull() ?
					NULL : (const char*)name));
  addr = a.addr;
}

Address::Address(const Address& address) : addr(address.addr)
{
  // do nothing
}

Address::Address(const InAddr& _addr)
{
  addr = _addr;
}

Address::Address(const struct sockaddr_in& _addr)
{
  addr = _addr.sin_addr;
}

Address::~Address()
{
  // do nothing
}

Address&		Address::operator=(const Address& address)
{
  addr = address.addr;
  return *this;
}

Address::operator InAddr() const
{
  return addr;
}

boolean			Address::operator==(const Address& address) const
{
  return addr.s_addr == address.addr.s_addr;
}

boolean			Address::operator!=(const Address& address) const
{
  return addr.s_addr != address.addr.s_addr;
}

boolean			Address::isAny() const
{
  return addr.s_addr == htonl(INADDR_ANY);
}

BzfString		Address::getDotNotation() const
{
  return BzfString(inet_ntoa(addr));
}

Address			Address::getHostAddress(const char* hname)
{
  Address a;

  struct hostent* hent;
  if (!hname) {				// local address
    char hostname[MAXHOSTNAMELEN+1];
    if (gethostname(hostname, sizeof(hostname)) >= 0)
      hent = gethostbyname(hostname);
    else
      return a;
  }
  else if (inet_aton(hname, &a.addr) != 0) {
    return a;
  }
  else {				// non-local address
    hent = gethostbyname(hname);
  }

  if (!hent) {
    herror("Address::getHostAddress");
    return a;
  }

  ::memcpy(&a.addr, hent->h_addr_list[0], sizeof(a.addr));
  return a;
}

#if !defined(_WIN32)
static void		timeout(int)
{
  // FIXME -- small chance here of alarm going off again before we can
  // ignore it.  should be using BSD signals.
  signal(SIGALRM, SIG_IGN);
  alarm(0);
}
#endif

BzfString		Address::getHostByAddress(InAddr addr)
{
  int addrLen = sizeof(addr);
#if !defined(_WIN32)
  signal(SIGALRM, SIG_PF(timeout));
  alarm(2);
#endif
  struct hostent* hent = gethostbyaddr((char*)&addr, addrLen, AF_INET);
#if !defined(_WIN32)
  alarm(0);
  signal(SIGALRM, SIG_IGN);
#endif // !defined(_WIN32)
  if (!hent) {
    // can't lookup name -- return in standard dot notation
    return BzfString(inet_ntoa(addr));
  }
  return BzfString(hent->h_name);
}

const char*		Address::getHostName(const char* hostname) // const
{
  char myname[MAXHOSTNAMELEN+1];
  const char* name = hostname;
  if (!name)
    if (gethostname(myname, sizeof(myname)) >= 0)
      name = myname;
  if (!name) return NULL;
  struct hostent* hent = gethostbyname(name);
  if (!hent) return NULL;
  return hent->h_name;
}

void*			Address::pack(void* _buf) const
{
  // everything in Address is already in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr = int32_t(addr.s_addr);
  ::memcpy(buf, &hostaddr, sizeof(int32_t));	buf += sizeof(int32_t);
  return (void*)buf;
}

void*			Address::unpack(void* _buf)
{
  // everything in Address should be stored in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr;
  ::memcpy(&hostaddr, buf, sizeof(int32_t));	buf += sizeof(int32_t);
  addr.s_addr = u_long(hostaddr);
  return (void*)buf;
}

//
// PlayerId
//

void*			PlayerId::pack(void* _buf) const
{
  // everything in PlayerId is already in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr = int32_t(serverHost.s_addr);
  ::memcpy(buf, &hostaddr, sizeof(int32_t));	buf += sizeof(int32_t);
  ::memcpy(buf, &port, sizeof(int16_t));	buf += sizeof(int16_t);
  ::memcpy(buf, &number, sizeof(int16_t));	buf += sizeof(int16_t);
  return (void*)buf;
}

void*			PlayerId::unpack(void* _buf)
{
  // everything in PlayerId should be stored in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr;
  ::memcpy(&hostaddr, buf, sizeof(int32_t));	buf += sizeof(int32_t);
  ::memcpy(&port, buf, sizeof(int16_t));	buf += sizeof(int16_t);
  ::memcpy(&number, buf, sizeof(int16_t));	buf += sizeof(int16_t);
  serverHost.s_addr = u_long(hostaddr);
  return (void*)buf;
}

boolean			PlayerId::operator==(const PlayerId& id) const
{
  return serverHost.s_addr == id.serverHost.s_addr &&
			port == id.port && number == id.number;
}

boolean			PlayerId::operator!=(const PlayerId& id) const
{
  return serverHost.s_addr != id.serverHost.s_addr ||
			port != id.port || number != id.number;
}
