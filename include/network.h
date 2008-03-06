/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * includes platform specific network files and adds missing stuff
 *
 * unfortunately this can include far more than necessary
 */

#ifndef	BZF_NETWORK_H
#define	BZF_NETWORK_H

#include "common.h"

#ifdef _WIN32
# if defined(_MSC_VER)
#   pragma warning(disable: 4786)
# endif
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

/* system headers */
#include <vector>
#include <string>

#if !defined(_WIN32)

#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#ifndef GUSI_20
#  include <sys/param.h>
#endif

#include <net/if.h>
#include <netinet/in.h>

#if defined(__linux__)
/* these are defined in both socket.h and tcp.h without ifdef guards. */
#  undef TCP_NODELAY
#  undef TCP_MAXSEG
#endif

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_BSTRING_H
#  include <bstring.h>
#endif

#if defined(__linux__) && !defined(_old_linux_)
#  define AddrLen		unsigned int
/* setsockopt incorrectly prototypes the 4th arg without const. */
#  define SSOType		void*
#elif defined(BSD) || defined(sun) || defined(__GLIBC__)
#  define AddrLen		socklen_t
#elif defined (__APPLE__)
#  include <AvailabilityMacros.h>
#  if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
#    define AddrLen		socklen_t
#  endif
#endif

#if defined(sun)
/* setsockopt prototypes the 4th arg as const char*. */
#  define SSOType		const char*
/* connect prototypes the 2nd arg without const */
#  define CNCTType	struct sockaddr
#endif

extern "C" {

#define herror(_x)	bzfherror(_x)

  void			nerror(const char* msg);
  void			bzfherror(const char* msg);
  int			getErrno();

}

/* BeOS net_server has closesocket(), which _must_ be used in place of close() */
#if defined(__BEOS__) && (IPPROTO_TCP != 6)
#  define close(__x) closesocket(__x)
#endif

#else /* !defined(_WIN32) */

#define	MAXHOSTNAMELEN	64

#define EINPROGRESS	WSAEWOULDBLOCK
#define	EWOULDBLOCK	WSAEWOULDBLOCK
#define	ECONNRESET	WSAECONNRESET
#define	EBADMSG		WSAECONNRESET	/* not defined by windows */

/* setsockopt prototypes the 4th arg as const char*. */
#define SSOType		const char*

inline int close(SOCKET s) { return closesocket(s); }
#define	ioctl(__fd, __req, __arg) \
			ioctlsocket(__fd, __req, (u_long*)__arg)
#define	gethostbyaddr(__addr, __len, __type) \
			gethostbyaddr((const char*)__addr, __len, __type)

extern "C" {

  int			inet_aton(const char* cp, struct in_addr* pin);
  void			nerror(const char* msg);
  void			herror(const char* msg);
  int			getErrno();

}

#endif /* !defined(_WIN32) */

#if !defined(AddrLen)
#  define AddrLen		int
#endif

#if !defined(SSOType)
#  define SSOType		const void*
#endif
#if !defined(CNCTType)
#  define CNCTType	const struct sockaddr
#endif

#if !defined(INADDR_NONE)
#  define INADDR_NONE	((in_addr_t)0xffffffff)
#endif

class BzfNetwork {
public:
  static int		setNonBlocking(int fd);
  static int		setBlocking(int fd);
  static bool	parseURL(const std::string& url,
			 std::string& protocol,
			 std::string& hostname,
			 int& port,
			 std::string& pathname);
};

#endif // BZF_NETWORK_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
