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

/*
 * includes platform specific network files and adds missing stuff
 *
 * unfortunately this can include far more than necessary
 */

#ifndef	BZF_NETWORK_H
#define	BZF_NETWORK_H

#if defined(_MSC_VER)
	#pragma warning(disable: 4786)
#endif

#include "common.h"
#include <string>
#include <vector>

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
#if defined(_old_linux_) || (!defined(__linux__) && !defined(sun) && !defined(__FreeBSD__) && !defined(__APPLE__) && !defined(macintosh) && !defined(__BEOS__))
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

#if defined(__FreeBSD__) || defined(sun) || defined(__NetBSD__)
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

/* BeOS net_server has closesocket(), which _must_ be used in place of close() */
#if defined(__BEOS__) && (IPPROTO_TCP != 6)
#define close(__x) closesocket(__x)
#endif

#else /* !defined(_WIN32) */

// turn off == signed/unsigned mismatch.  would rather not, but FD_SET
// is broken.
#ifdef _MSC_VER
#pragma warning(disable: 4018)
#endif

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
    static int		setBlocking(int fd);
    static bool	dereferenceURLs(std::vector<std::string>& list,
				unsigned int max, std::vector<std::string>& failedList);
    static bool	parseURL(const std::string& url,
				std::string& protocol,
				std::string& hostname,
				int& port,
				std::string& pathname);

  private:
    static std::string	dereferenceHTTP(const std::string& hostname, int port,
				const std::string& pathname);
    static std::string	dereferenceFile(const std::string& pathname);
    static void		insertLines(std::vector<std::string>& list,
				int index, const std::string& data);
};

#endif // BZF_NETWORK_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

