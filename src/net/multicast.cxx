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
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <string>
#include "common.h"
#include "multicast.h"
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

int			openMulticast(const Address& address, int port,
					const char* service, int ttl,
					const char* net_interface,
					const char* mode,
					struct sockaddr_in* addr)
{
  // Multicast is broken
  return -1;

  struct ip_mreq mreq;
  struct in_addr ifaddr;
  int fd;
#if defined(_WIN32)
  const BOOL optOn = TRUE;
#else
  const int optOn = 1;
#endif

  /* check parameters */
  if (!addr) {
    printError("openMulticast: Must supply a return address structure!");
    return -1;
  }
  memset(addr, 0, sizeof(*addr));

  /* check mode */
  if (!mode || (mode[0] != 'r' && mode[0] != 'w')) {
    printError("openMulticast: Invalid mode");
    return -1;
  }

  /* lookup service and check port */
  if (service) {
    struct servent *sp = getservbyname(service, "udp");
    if (!sp) {
      if (port <= 0) {
	std::vector<std::string> args;
	args.push_back(service);
	printError("openMulticast: No udp service {1}", &args);
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
    sprintf(buf, "%d", port);
    args.push_back(buf);
    printError("openMulticast: Invalid port {1}", &args);
    return -1;
  }

  /* check group address */
  mreq.imr_multiaddr = InAddr(address);
  if (!IN_MULTICAST(ntohl(mreq.imr_multiaddr.s_addr))) {
    printError("openMulticast: Invalid multicast address");
    return -1;
  }

  /* open socket */
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    nerror("openMulticast: socket");
    return -1;
  }

  /* check interface */
  if (net_interface && net_interface[0]) {
    ifaddr.s_addr = inet_addr(net_interface);
    if (ifaddr.s_addr == (unsigned long)-1) {
      struct hostent* hp = gethostbyname(net_interface);
      if (!hp) {
	std::vector<std::string> args;
	args.push_back(net_interface);
	printError("openMulticast: Can't get address of {1}", &args);
	close(fd);
	return -1;
      }
      memcpy(&ifaddr, hp->h_addr, hp->h_length);
    }
    mreq.imr_interface = ifaddr;
  }
  else {
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

#if defined(__linux__)
    // linux doesn't seem to like INADDR_ANY as the interface.
    // find the first non-loopback interface and use that.
    // (only check the first numInterfaces interfaces.)
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
      nerror("openMulticast: getting interface list");
      close(fd);
      return(-1);
    }

    // get the address of each interface
    for (i = 0; i < numInterfaces; ++i) {
      // if no name then we're done
      if (req[i].ifr_ifrn.ifrn_name[0] == 0)
	break;

      // if we can't get the address then skip this interface
      if (ioctl(fd, SIOCGIFADDR, req + i) < 0)
	continue;

      // if the address is the loopback address then skip it
      const sockaddr_in* ifaddr = (const sockaddr_in*)
					&req[i].ifr_ifru.ifru_addr;
      if (ntohl(ifaddr->sin_addr.s_addr) == INADDR_LOOPBACK)
	continue;

      // got the address on the interface
      if (!net_interface || !net_interface[0])
	mreq.imr_interface.s_addr = ifaddr->sin_addr.s_addr;
      break;
    }
#endif
  }

  /* set address info */
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = mreq.imr_multiaddr.s_addr;
  addr->sin_port = htons(port);

  /* set options on socket */
  if (mode[0] == 'w') {
#if defined(_WIN32) || defined(__linux__) || defined(GUSI_20)
    /* windows requires socket to be bound before using multicast sockopts */
    /* linux requires that we bind to an interface's address to send */
    struct sockaddr_in tmpAddr;
    memset(&tmpAddr, 0, sizeof(tmpAddr));
    tmpAddr.sin_family = AF_INET;
    tmpAddr.sin_addr.s_addr = mreq.imr_interface.s_addr;
    tmpAddr.sin_port = 0;
    if (bind(fd, (const struct sockaddr*)&tmpAddr, sizeof(tmpAddr)) < 0) {
      nerror("openMulticast: bind");
      close(fd);
      return -1;
    }
#endif

    /* set ttl */
    if (setMulticastTTL(fd, ttl) < 0) {
      nerror("WARNING: openMulticast: setsockopt IP_MULTICAST_TTL");
      close(fd);
      return -1;
    }

    if (net_interface && net_interface[0] &&
		setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,
		(SSOType)&ifaddr, sizeof(ifaddr)) < 0) {
      nerror("WARNING: openMulticast: Can't multicast on interface");
      close(fd);
      return -1;
    }
  }

  else {
#if defined(SO_REUSEPORT)
    /* set reuse port */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
				(SSOType)&optOn, sizeof(optOn)) < 0) {
      nerror("WARNING: openMulticast: setsockopt SO_REUSEPORT");
      close(fd);
      return -1;
    }
#endif
#if defined(SO_REUSEADDR)
    /* set reuse address */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(SSOType)&optOn, sizeof(optOn)) < 0) {
      nerror("WARNING: openMulticast: setsockopt SO_REUSEADDR");
      close(fd);
      return -1;
    }
#endif

    /* bind address */
#if defined(_WIN32)
    /* unlike linux, win32 requires that we bind to the ANY address */
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    if (bind(fd, (const struct sockaddr*)addr, sizeof(*addr)) < 0) {
      nerror("openMulticast: bind");
      close(fd);
      return -1;
    }

    /* join multicast group */
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				(SSOType)&mreq, sizeof(mreq)) < 0) {
      nerror("WARNING: openMulticast: Can't join multicast group");
      close(fd);
      return -1;
    }
  }

  /* turn on non-blocking io */
  if (BzfNetwork::setNonBlocking(fd) < 0) {
    nerror("openBroadcast: setNonBlocking");
    close(fd);
    return(-1);
  }

  return fd;
}

int			closeMulticast(int fd)
{
  if (fd == -1) return 0;
  return close(fd);
}

int			setMulticastTTL(int fd, int ttl)
{
#if defined(_WIN32)
  unsigned int opt = (unsigned int)ttl;
#else /* defined(_WIN32) */
  unsigned char opt = (unsigned char)ttl;
#endif /* defined(_WIN32) */
  return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL,
				(SSOType)&opt, sizeof(opt));
}

int			sendMulticast(int fd, const void* buffer,
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

int			recvMulticast(int fd, void* buffer, int bufferLength,
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
      nerror("recvMulticast");
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

