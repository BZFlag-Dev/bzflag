/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string.h>
#include <sys/types.h>
#if !defined(_WIN32)
#include <unistd.h>
#include <setjmp.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include "Address.h"
#include "bzsignal.h"
#include "ErrorHandler.h"
#include "Pack.h"

#if defined(sun)
  // Solaris...
  extern "C" int inet_aton(const char *, struct in_addr *);
#endif

//
// Address
//

#if !defined( GUSI_20 ) && !defined( WIN32 )
  Address			Address::localAddress("");
#endif

Address::Address()
{
  InAddr tempAddr;

  memset(&tempAddr, 0, sizeof(addr));
  tempAddr.s_addr = htonl(INADDR_ANY);
  addr.push_back(tempAddr);
}

Address::Address(const std::string& name)
{
  Address a = getHostAddress((const char*)(name.length() ? name.c_str() : NULL));
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

bool			Address::isAny() const
{
  return addr[0].s_addr == htonl(INADDR_ANY);
}

std::string		Address::getDotNotation() const
{
  return std::string(inet_ntoa(addr[0]));
}

uint8_t		        Address::getIPVersion() const {
  return 4;
}

#if !defined(_WIN32)
static jmp_buf alarmEnv;
static void		onAlarm(int)
{
  // jump back to setjmp.  this handles the race condition where we
  // set the alarm but gethostbyname() wouldn't have been called
  // until after the alarm goes off (resulting in an indeterminate
  // wait).
  longjmp(alarmEnv, 1);
}
#endif

Address			Address::getHostAddress(const char* hname)
{
  Address a;
  InAddr tempAddr;
  int j;

  struct hostent* hent;
  if (!hname) {				// local address
    char hostname[MAXHOSTNAMELEN+1];
    if (gethostname(hostname, sizeof(hostname)) >= 0)
      hent = gethostbyname(hostname);
    else
      return a;
  }
  else if (inet_aton(hname, &tempAddr) != 0) {
    a.addr.push_back(tempAddr);
    return a;
  }
  else {				// non-local address
#if !defined(_WIN32)
    // set alarm to avoid waiting too long
    SIG_PF oldAlarm = bzSignal(SIGALRM, SIG_PF(onAlarm));
    if (oldAlarm != SIG_ERR) {
      if (setjmp(alarmEnv) != 0) {
	// alarm went off
	hent = NULL;
	printError("Looking up host name: timeout");
	return a;
      }

      // wait up to this many seconds
      alarm(8);
    }
#endif

    hent = gethostbyname(hname);

#if !defined(_WIN32)
    if (oldAlarm != SIG_ERR) {
      alarm(0);
      bzSignal(SIGALRM, oldAlarm);
    }
#endif
  }

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
#if !defined(_WIN32)
  // set alarm to avoid waiting too long
  SIG_PF oldAlarm = bzSignal(SIGALRM, SIG_PF(onAlarm));
  if (oldAlarm != SIG_ERR) {
    if (setjmp(alarmEnv) != 0) {
      // alarm went off
      return std::string(inet_ntoa(addr));
    }

    // wait up to this many seconds
    alarm(8);
  }
#endif

  int addrLen = sizeof(addr);
  struct hostent* hent = gethostbyaddr((char*)&addr, addrLen, AF_INET);

#if !defined(_WIN32)
  if (oldAlarm != SIG_ERR) {
    alarm(0);
    bzSignal(SIGALRM, oldAlarm);
  }
#endif

  if (!hent) {
    // can't lookup name -- return in standard dot notation
    return std::string(inet_ntoa(addr));
  }
  return std::string(hent->h_name);
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
  unsigned char* buf = (unsigned char*)_buf;
  buf = (unsigned char*)nboPackUByte(_buf, 4);
  // everything in InAddr  is already in network byte order
  int32_t hostaddr = int32_t(addr[0].s_addr);
  ::memcpy(buf, &hostaddr, sizeof(int32_t));	buf += sizeof(int32_t);
  return (void*)buf;
}

void*			Address::unpack(void* _buf)
{
  unsigned char* buf = (unsigned char*)_buf;
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
  return (void*)buf;
}

//
// ServerId
//

void*			ServerId::pack(void* _buf) const
{
  // everything in PlayerId is already in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr = int32_t(serverHost.s_addr);
  ::memcpy(buf, &hostaddr, sizeof(int32_t));	buf += sizeof(int32_t);
  ::memcpy(buf, &port, sizeof(int16_t));	buf += sizeof(int16_t);
  ::memcpy(buf, &number, sizeof(int16_t));	buf += sizeof(int16_t);
  return (void*)buf;
}

void*			ServerId::unpack(void* _buf)
{
  // everything in ServerId should be stored in network byte order
  unsigned char* buf = (unsigned char*)_buf;
  int32_t hostaddr;
  ::memcpy(&hostaddr, buf, sizeof(int32_t));	buf += sizeof(int32_t);
  ::memcpy(&port, buf, sizeof(int16_t));	buf += sizeof(int16_t);
  ::memcpy(&number, buf, sizeof(int16_t));	buf += sizeof(int16_t);
  serverHost.s_addr = u_long(hostaddr);
  return (void*)buf;
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

