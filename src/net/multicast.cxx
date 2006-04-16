/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "multicast.h"

/* system implementation headers */
#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <string>

/* common implementation headers */
#include "ErrorHandler.h"


int			openBroadcast(int port, const char* service,
					struct sockaddr_in* addr)
{
#if defined(_WIN32)
  const BOOL optOn = TRUE;
#else
  const int optOn = 1;
#endif
  int fd;

  /* check parameters */
  if (!addr) {
    printError("openBroadcast: Must supply a return address structure!");
    return -1;
  }
  memset(addr, 0, sizeof(*addr));

  /* lookup service and check port */
  if (service) {
    struct servent *sp = getservbyname(service, "udp");
    if (!sp) {
      if (port <= 0) {
	std::vector<std::string> args;
	args.push_back(service);

	printError("openBroadcast: No udp service {1}", &args);
	return -1;
      }
    }
    else {
      port = sp->s_port;
    }
  }
  if (port <= 0) {
    std::vector<std::string> args;
    char buf[10];
    sprintf(buf,"%d", port);
    args.push_back(buf);
    printError("openBroadcast: Invalid port {1}", &args);
    return -1;
  }

  /* open socket */
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    nerror("openBroadcast: socket");
    return -1;
  }

  /* set address info */
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY);

#if defined(SO_REUSEPORT)
  /* set reuse port */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
				(SSOType)&optOn, sizeof(optOn)) < 0) {
    nerror("openBroadcast: setsockopt SO_REUSEPORT");
    close(fd);
    return -1;
  }
#elif defined(SO_REUSEADDR)
  /* set reuse address */
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(SSOType)&optOn, sizeof(optOn)) < 0) {
    nerror("openBroadcast: setsockopt SO_REUSEADDR");
    close(fd);
    return -1;
  }
#endif

  /* bind address */
  if (bind(fd, (const struct sockaddr*)addr, sizeof(*addr)) < 0) {
    nerror("openBroadcast: bind");
    close(fd);
    return -1;
  }

  /* make broadcast */
  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
				(SSOType)&optOn, sizeof(optOn)) < 0) {
    nerror("openBroadcast: setsockopt SO_BROADCAST");
    close(fd);
    return -1;
  }

  // address to send to is the broadcast address
  addr->sin_addr.s_addr = INADDR_BROADCAST;
  addr->sin_port = htons(port);

#if defined(__linux__)
  // linux doesn't seem to like INADDR_BROADCAST, but it's okay
  // with the broadcast address on the local network.  get the
  // broadcast address on the first interface that's not the
  // loopback interface.  (check the first numInterfaces
  // interfaces.)
  const int numInterfaces = 5;
  int i;
  struct ifconf conf;
  struct ifreq req[numInterfaces];

  // get the list of interface names
  conf.ifc_len = sizeof(req);
  conf.ifc_ifcu.ifcu_req = req;
  for (i = 0; i < numInterfaces; ++i)
    req[i].ifr_ifrn.ifrn_name[0] = 0;
  if (ioctl(fd, SIOCGIFCONF, &conf) < 0) {
    nerror("openBroadcast: getting interface list");
    close(fd);
    return(-1);
  }

  // get the broadcast address on each interface
  for (i = 0; i < numInterfaces; ++i) {
    // if no name then we're done
    if (req[i].ifr_ifrn.ifrn_name[0] == 0)
      break;

    // if we can't get the address then skip this interface
    if (ioctl(fd, SIOCGIFBRDADDR, req + i) < 0)
      continue;

    // if the address is the loopback broadcast address then skip it
    const sockaddr_in* ifbaddr = (const sockaddr_in*)
					&req[i].ifr_ifru.ifru_broadaddr;
    if (ntohl(ifbaddr->sin_addr.s_addr) == 0x7ffffffflu)
      continue;
    if (ifbaddr->sin_addr.s_addr == 0)
      continue;

    // got the broadcast address on the interface
    addr->sin_addr.s_addr = ifbaddr->sin_addr.s_addr;
    break;
  }

  // failed if no broadcast addresses found
  if (i == numInterfaces) {
    nerror("openBroadcast: getting broadcast address");
    close(fd);
    return(-1);
  }
#endif

  /* turn on non-blocking io */
  if (BzfNetwork::setNonBlocking(fd) < 0) {
    nerror("openBroadcast: setNonBlocking");
    close(fd);
    return(-1);
  }

  return fd;
}

int			closeBroadcast(int fd)
{
  if (fd == -1) return 0;
  return close(fd);
}

int			sendBroadcast(int fd, const void* buffer,
					int bufferLength,
					const struct sockaddr_in* addr)
{
  return sendto(fd, (const char*)buffer, bufferLength, 0,
				(const struct sockaddr*)addr, sizeof(*addr));
}

#if !defined(AddrLen)
#define AddrLen		int
#endif

#ifdef WIN32
/* This is a really really fugly hack to get around winsock sillyness
 * The newer versions of winsock have a socken_t typedef, and there
 * doesn't seem to be any way to tell the versions apart. However,
 * VC++ helps us out here by treating typedef as #define
 * If we've got a socklen_t typedefed, define HAVE_SOCKLEN_T to
 * avoid #define'ing it in common.h */

#ifndef socklen_t
	#define socklen_t int
#endif
#endif //WIN32

int			recvBroadcast(int fd, void* buffer, int bufferLength,
					 struct sockaddr_in* addr)
{
  struct sockaddr_in from;
  AddrLen fromLength = sizeof(from);

  int byteCount = recvfrom(fd, (char*)buffer, bufferLength, 0,
				(struct sockaddr*)&from, (socklen_t*) &fromLength);
  if (byteCount < 0) {
    if (getErrno() == EWOULDBLOCK) {
      return 0;
    }
    else {
      nerror("recvBroadcast");
      return -1;
    }
  }
  if (addr) *addr = from;
  return byteCount;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

