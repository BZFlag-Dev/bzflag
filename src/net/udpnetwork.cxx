/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "udpnetwork.h"
#include "ErrorHandler.h"


int			openUDPNetwork(int port, struct sockaddr_in* addr)
{
#if defined(_WIN32)
  const BOOL optOn = TRUE;
  BOOL opt = optOn;
#else
  const int optOn = 1;
  int opt = optOn;
#endif
  int fd;

  /* check parameters */
  if (!addr) {
    printError("openUDPnetwork: Must supply an address structure!");
    return -1;
  }
  memset(addr, 0, sizeof(*addr));

  /* open socket */
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    nerror("openUDPnetwork: socket");
    return -1;
  }

  /* set address info */
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY);
  addr->sin_port = htons(port);

#if defined(SO_REUSEPORT)
  /* set reuse port */
  opt = optOn;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
				(SSOType)&opt, sizeof(opt)) < 0) {
    nerror("openUDPnetwork: setsockopt SO_REUSEPORT");
    close(fd);
    return -1;
  }
#elif defined(SO_REUSEADDR)
  /* set reuse address */
  opt = optOn;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
				(SSOType)&opt, sizeof(opt)) < 0) {
    nerror("openUDPnetwork: setsockopt SO_REUSEADDR");
    close(fd);
    return -1;
  }
#endif

  /* bind address */
  if (bind(fd, (const struct sockaddr*)addr, sizeof(*addr)) < 0) {
    nerror("openUDPnetwork: bind");
    close(fd);
    return -1;
  }

  return fd;
}


int			closeUDPNetwork(int fd)
{
  if (fd == -1) return 0;
  return close(fd);
}


int			sendUDPNetwork(int fd, const void* buffer,
					int bufferLength,
					const struct sockaddr_in* addr)
{
  return sendto(fd, (const char*)buffer, bufferLength, 0,
				(const struct sockaddr*)addr, sizeof(*addr));
}

#if !defined(AddrLen)
#define AddrLen		int
#endif

int			recvUDPNetwork(int fd, void* buffer, int bufferLength,
					 struct sockaddr_in* addr)
{
  struct sockaddr_in from;
  AddrLen fromLength = sizeof(from);

  int byteCount = recvfrom(fd, (char*)buffer, bufferLength, 0,
				(struct sockaddr*)&from, &fromLength);
  if (byteCount < 0) {
    if (getErrno() == EWOULDBLOCK) {
      return 0;
    }
    else {
      nerror("recvUDPNetwork");
      return -1;
    }
  }
  if (addr) *addr = from;
  return byteCount;
}

// ex: shiftwidth=2 tabstop=8
