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

#if !defined(_WIN32)

#include "network.h"
#include "ErrorHandler.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#if defined(_old_linux_)
#define hstrerror(x) sys_errlist[x]
#elif defined(sun)
#define hstrerror(x) "<network error>"
#endif

extern "C" {

void			nerror(const char* msg)
{
  if (msg)
    printError("%s: %s", msg, strerror(errno));
  else
    printError("%s", strerror(errno));
}

void			bzfherror(const char* msg)
{
  if (msg)
    printError("%s: %s", msg, hstrerror(h_errno));
  else
    printError("%s", hstrerror(h_errno));
}

int			getErrno()
{
  return errno;
}

}

int			BzfNetwork::setNonBlocking(int fd)
{
  int mode = fcntl(fd, F_GETFL, 0);
  if (mode == -1 || fcntl(fd, F_SETFL, mode | O_NDELAY) < 0)
    return -1;
  return 0;
}

#else /* defined(_WIN32) */

#include "network.h"
#include "ErrorHandler.h"
#include <stdio.h>
#include <string.h>

extern "C" {

int			inet_aton(const char* cp, struct in_addr* pin)
{
  unsigned long a = inet_addr(cp);
  if (a == (unsigned long)-1) {
    // could be an error or cp could be a broadcast address.
    // FIXME -- this check is a little simplistic.
    if (strcmp(cp, "255.255.255.255") != 0) return 0;
  }

  pin->s_addr = a;
  return 1;
}

void			nerror(const char* msg)
{
  if (msg)
    printError("%s: %d (0x%x)", msg, getErrno(), getErrno());
  else
    printError("%d (0x%x)", getErrno(), getErrno());
}

void			herror(const char* msg)
{
  if (msg)
    printError("%s: error code %d", msg, getErrno());
  else
    printError("error code %d", getErrno());
}

int			getErrno()
{
  return WSAGetLastError();
}

}

int			BzfNetwork::setNonBlocking(int fd)
{
  int on = 1;
  return ioctl(fd, FIONBIO, &on);
}

#endif /* defined(_WIN32) */
