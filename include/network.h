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

/*
 * includes platform specific network files and adds missing stuff
 *
 * unfortunately this can include far more than necessary
 */

#ifndef	BZF_NETWORK_H
#define	BZF_NETWORK_H

#include "common.h"
#include "BzfString.h"

#if !defined(_WIN32)

#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifndef GUSI_20
  #include <sys/param.h>
#endif
#include <net/if.h>
#include <netinet/in.h>
#if defined(__linux__)
/* these are defined in both socket.h and tcp.h without ifdef guards. */
#undef TCP_NODELAY
#undef TCP_MAXSEG
#endif
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#if defined(_old_linux_) || (!defined(__linux__) && !defined(sun) && !defined(__FreeBSD__) && !defined(macintosh))
#include <bstring.h>
#endif

// add our own def block
#if defined (macintosh)
  #ifdef GUSI_20
    #define getsockname(a,b,c)       getsockname(a,b,(unsigned int *)c)
    #define accept(a,b,c)            accept(a,b,(unsigned int *)c)
    #define recvfrom(a,b,c,d,e,f)    recvfrom(a, (void*)b, (unsigned long)c, d, e, (unsigned int*)f)

    #define MAXHOSTNAMELEN 255

    #define O_NDELAY O_NONBLOCK

    #define hstrerror(x) "hstrerror is broken"
  #endif
#endif

#if defined(__linux__) && !defined(_old_linux_)
#define AddrLen		unsigned int

/* setsockopt incorrectly prototypes the 4th arg without const. */
#define SSOType		void*
#endif

#if defined(__FreeBSD__) && !defined(_MACOSX_)
#define AddrLen		socklen_t
#endif

#if defined(sun)
/* setsockopt prototypes the 4th arg as const char*. */
#define SSOType		const char*

/* connect prototypes the 2nd arg without const */
#define CNCTType	struct sockaddr
#endif

extern "C" {

#define herror(_x)	bzfherror(_x)

void			nerror(const char* msg);
void			bzfherror(const char* msg);
int			getErrno();

}

#else /* !defined(_WIN32) */

#include <winsock2.h>
#include <ws2tcpip.h>

// turn off == signed/unsigned mismatch.  would rather not, but FD_SET
// is broken.
#pragma warning(disable: 4018)

#define	MAXHOSTNAMELEN	64

#define	EWOULDBLOCK	WSAEWOULDBLOCK
#define	ECONNRESET	WSAECONNRESET
#define	EBADMSG		WSAECONNRESET	/* not defined by windows */

/* setsockopt prototypes the 4th arg as const char*. */
#define SSOType		const char*

#define	close(__x)	closesocket(__x)
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
#define AddrLen		int
#endif

#if !defined(SSOType)
#define SSOType		const void*
#endif
#if !defined(CNCTType)
#define CNCTType	const struct sockaddr
#endif

#if !defined(INADDR_NONE)
#define INADDR_NONE	((in_addr_t)0xffffffff)
#endif

class BzfNetwork {
  public:
    static int		setNonBlocking(int fd);
    static boolean	dereferenceURLs(BzfStringAList& list,
				int max, BzfStringAList& failedList);
    static boolean	parseURL(const BzfString& url,
				BzfString& protocol,
				BzfString& hostname,
				int& port,
				BzfString& pathname);

  private:
    static BzfString	dereferenceHTTP(const BzfString& hostname, int port,
				const BzfString& pathname);
    static BzfString	dereferenceFile(const BzfString& pathname);
    static void		insertLines(BzfStringAList& list,
				int index, const BzfString& data);
};

#endif // BZF_NETWORK_H
// ex: shiftwidth=2 tabstop=8
