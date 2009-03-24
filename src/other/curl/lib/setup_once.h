#ifndef __SETUP_ONCE_H
#define __SETUP_ONCE_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: setup_once.h,v 1.35 2008-08-27 00:25:03 yangtse Exp $
 ***************************************************************************/


/********************************************************************
 *                              NOTICE                              *
 *                             ========                             *
 *                                                                  *
 *  Content of header files lib/setup_once.h and ares/setup_once.h  *
 *  must be kept in sync. Modify the other one if you change this.  *
 *                                                                  *
 ********************************************************************/


/*
 * Inclusion of common header files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif
#else
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#endif

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif


/*
 * Definition of timeval struct for platforms that don't have it.
 */

#ifndef HAVE_STRUCT_TIMEVAL
struct timeval {
 long tv_sec;
 long tv_usec;
};
#endif


/*
 * If we have the MSG_NOSIGNAL define, make sure we use
 * it as the fourth argument of function send()
 */

#ifdef HAVE_MSG_NOSIGNAL
#define SEND_4TH_ARG MSG_NOSIGNAL
#else
#define SEND_4TH_ARG 0
#endif


/*
 * Windows build targets have socklen_t definition in
 * ws2tcpip.h but some versions of ws2tcpip.h do not
 * have the definition. It seems that when the socklen_t
 * definition is missing from ws2tcpip.h the definition
 * for INET_ADDRSTRLEN is also missing, and that when one
 * definition is present the other one also is available.
 */

#if defined(WIN32) && !defined(HAVE_CONFIG_H)
#  if ( defined(_MSC_VER) && !defined(INET_ADDRSTRLEN) ) || \
      (!defined(_MSC_VER) && !defined(HAVE_WS2TCPIP_H) )
#    define socklen_t int
#  endif
#endif


#if defined(__minix)
/* Minix doesn't support recv on TCP sockets */
#define sread(x,y,z) (ssize_t)read((RECV_TYPE_ARG1)(x), \
                                   (RECV_TYPE_ARG2)(y), \
                                   (RECV_TYPE_ARG3)(z))

#elif defined(HAVE_RECV)
/*
 * The definitions for the return type and arguments types
 * of functions recv() and send() belong and come from the
 * configuration file. Do not define them in any other place.
 *
 * HAVE_RECV is defined if you have a function named recv()
 * which is used to read incoming data from sockets. If your
 * function has another name then don't define HAVE_RECV.
 *
 * If HAVE_RECV is defined then RECV_TYPE_ARG1, RECV_TYPE_ARG2,
 * RECV_TYPE_ARG3, RECV_TYPE_ARG4 and RECV_TYPE_RETV must also
 * be defined.
 *
 * HAVE_SEND is defined if you have a function named send()
 * which is used to write outgoing data on a connected socket.
 * If yours has another name then don't define HAVE_SEND.
 *
 * If HAVE_SEND is defined then SEND_TYPE_ARG1, SEND_QUAL_ARG2,
 * SEND_TYPE_ARG2, SEND_TYPE_ARG3, SEND_TYPE_ARG4 and
 * SEND_TYPE_RETV must also be defined.
 */

#if !defined(RECV_TYPE_ARG1) || \
    !defined(RECV_TYPE_ARG2) || \
    !defined(RECV_TYPE_ARG3) || \
    !defined(RECV_TYPE_ARG4) || \
    !defined(RECV_TYPE_RETV)
  /* */
  Error Missing_definition_of_return_and_arguments_types_of_recv
  /* */
#else
#define sread(x,y,z) (ssize_t)recv((RECV_TYPE_ARG1)(x), \
                                   (RECV_TYPE_ARG2)(y), \
                                   (RECV_TYPE_ARG3)(z), \
                                   (RECV_TYPE_ARG4)(0))
#endif
#else /* HAVE_RECV */
#ifndef sread
  /* */
  Error Missing_definition_of_macro_sread
  /* */
#endif
#endif /* HAVE_RECV */


#if defined(__minix)
/* Minix doesn't support send on TCP sockets */
#define swrite(x,y,z) (ssize_t)write((SEND_TYPE_ARG1)(x), \
                                    (SEND_TYPE_ARG2)(y), \
                                    (SEND_TYPE_ARG3)(z))

#elif defined(HAVE_SEND)
#if !defined(SEND_TYPE_ARG1) || \
    !defined(SEND_QUAL_ARG2) || \
    !defined(SEND_TYPE_ARG2) || \
    !defined(SEND_TYPE_ARG3) || \
    !defined(SEND_TYPE_ARG4) || \
    !defined(SEND_TYPE_RETV)
  /* */
  Error Missing_definition_of_return_and_arguments_types_of_send
  /* */
#else
#define swrite(x,y,z) (ssize_t)send((SEND_TYPE_ARG1)(x), \
                                    (SEND_TYPE_ARG2)(y), \
                                    (SEND_TYPE_ARG3)(z), \
                                    (SEND_TYPE_ARG4)(SEND_4TH_ARG))
#endif
#else /* HAVE_SEND */
#ifndef swrite
  /* */
  Error Missing_definition_of_macro_swrite
  /* */
#endif
#endif /* HAVE_SEND */


#if 0
#if defined(HAVE_RECVFROM)
/*
 * Currently recvfrom is only used on udp sockets.
 */
#if !defined(RECVFROM_TYPE_ARG1) || \
    !defined(RECVFROM_TYPE_ARG2) || \
    !defined(RECVFROM_TYPE_ARG3) || \
    !defined(RECVFROM_TYPE_ARG4) || \
    !defined(RECVFROM_TYPE_ARG5) || \
    !defined(RECVFROM_TYPE_ARG6) || \
    !defined(RECVFROM_TYPE_RETV)
  /* */
  Error Missing_definition_of_return_and_arguments_types_of_recvfrom
  /* */
#else
#define sreadfrom(s,b,bl,f,fl) (ssize_t)recvfrom((RECVFROM_TYPE_ARG1)  (s),  \
                                                 (RECVFROM_TYPE_ARG2 *)(b),  \
                                                 (RECVFROM_TYPE_ARG3)  (bl), \
                                                 (RECVFROM_TYPE_ARG4)  (0),  \
                                                 (RECVFROM_TYPE_ARG5 *)(f),  \
                                                 (RECVFROM_TYPE_ARG6 *)(fl))
#endif
#else /* HAVE_RECVFROM */
#ifndef sreadfrom
  /* */
  Error Missing_definition_of_macro_sreadfrom
  /* */
#endif
#endif /* HAVE_RECVFROM */


#ifdef RECVFROM_TYPE_ARG6_IS_VOID
#  define RECVFROM_ARG6_T int
#else
#  define RECVFROM_ARG6_T RECVFROM_TYPE_ARG6
#endif
#endif /* if 0 */


/*
 * Uppercase macro versions of ANSI/ISO is*() functions/macros which
 * avoid negative number inputs with argument byte codes > 127.
 */

#define ISSPACE(x)  (isspace((int)  ((unsigned char)x)))
#define ISDIGIT(x)  (isdigit((int)  ((unsigned char)x)))
#define ISALNUM(x)  (isalnum((int)  ((unsigned char)x)))
#define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#define ISGRAPH(x)  (isgraph((int)  ((unsigned char)x)))
#define ISALPHA(x)  (isalpha((int)  ((unsigned char)x)))
#define ISPRINT(x)  (isprint((int)  ((unsigned char)x)))
#define ISUPPER(x)  (isupper((int)  ((unsigned char)x)))
#define ISLOWER(x)  (islower((int)  ((unsigned char)x)))

#define ISBLANK(x)  (int)((((unsigned char)x) == ' ') || \
                          (((unsigned char)x) == '\t'))


/*
 * Typedef to 'unsigned char' if bool is not an available 'typedefed' type.
 */

#ifndef HAVE_BOOL_T
typedef unsigned char bool;
#define HAVE_BOOL_T
#endif


/*
 * Default definition of uppercase TRUE and FALSE.
 */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/*
 * Typedef to 'int' if sig_atomic_t is not an available 'typedefed' type.
 */

#ifndef HAVE_SIG_ATOMIC_T
typedef int sig_atomic_t;
#define HAVE_SIG_ATOMIC_T
#endif


/*
 * Convenience SIG_ATOMIC_T definition
 */

#ifdef HAVE_SIG_ATOMIC_T_VOLATILE
#define SIG_ATOMIC_T static sig_atomic_t
#else
#define SIG_ATOMIC_T static volatile sig_atomic_t
#endif


/*
 * Default return type for signal handlers.
 */

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif


/*
 * Macro used to include code only in debug builds.
 */

#ifdef CURLDEBUG
#define DEBUGF(x) x
#else
#define DEBUGF(x) do { } while (0)
#endif


/*
 * Macro used to include assertion code only in debug builds.
 */

#if defined(CURLDEBUG) && defined(HAVE_ASSERT_H)
#define DEBUGASSERT(x) assert(x)
#else
#define DEBUGASSERT(x) do { } while (0)
#endif


/*
 * Macro SOCKERRNO / SET_SOCKERRNO() returns / sets the *socket-related* errno
 * (or equivalent) on this platform to hide platform details to code using it.
 */

#ifdef USE_WINSOCK
#define SOCKERRNO         ((int)WSAGetLastError())
#define SET_SOCKERRNO(x)  (WSASetLastError((int)(x)))
#else
#define SOCKERRNO         (errno)
#define SET_SOCKERRNO(x)  (errno = (x))
#endif


/*
 * Macro ERRNO / SET_ERRNO() returns / sets the NOT *socket-related* errno
 * (or equivalent) on this platform to hide platform details to code using it.
 */

#ifdef WIN32
#define ERRNO         ((int)GetLastError())
#define SET_ERRNO(x)  (SetLastError((DWORD)(x)))
#else
#define ERRNO         (errno)
#define SET_ERRNO(x)  (errno = (x))
#endif


/*
 * Portable error number symbolic names defined to Winsock error codes.
 */

#ifdef USE_WINSOCK
#undef  EBADF            /* override definition in errno.h */
#define EBADF            WSAEBADF
#undef  EINTR            /* override definition in errno.h */
#define EINTR            WSAEINTR
#undef  EINVAL           /* override definition in errno.h */
#define EINVAL           WSAEINVAL
#define EWOULDBLOCK      WSAEWOULDBLOCK
#define EINPROGRESS      WSAEINPROGRESS
#define EALREADY         WSAEALREADY
#define ENOTSOCK         WSAENOTSOCK
#define EDESTADDRREQ     WSAEDESTADDRREQ
#define EMSGSIZE         WSAEMSGSIZE
#define EPROTOTYPE       WSAEPROTOTYPE
#define ENOPROTOOPT      WSAENOPROTOOPT
#define EPROTONOSUPPORT  WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT  WSAESOCKTNOSUPPORT
#define EOPNOTSUPP       WSAEOPNOTSUPP
#define EPFNOSUPPORT     WSAEPFNOSUPPORT
#define EAFNOSUPPORT     WSAEAFNOSUPPORT
#define EADDRINUSE       WSAEADDRINUSE
#define EADDRNOTAVAIL    WSAEADDRNOTAVAIL
#define ENETDOWN         WSAENETDOWN
#define ENETUNREACH      WSAENETUNREACH
#define ENETRESET        WSAENETRESET
#define ECONNABORTED     WSAECONNABORTED
#define ECONNRESET       WSAECONNRESET
#define ENOBUFS          WSAENOBUFS
#define EISCONN          WSAEISCONN
#define ENOTCONN         WSAENOTCONN
#define ESHUTDOWN        WSAESHUTDOWN
#define ETOOMANYREFS     WSAETOOMANYREFS
#define ETIMEDOUT        WSAETIMEDOUT
#define ECONNREFUSED     WSAECONNREFUSED
#define ELOOP            WSAELOOP
#ifndef ENAMETOOLONG     /* possible previous definition in errno.h */
#define ENAMETOOLONG     WSAENAMETOOLONG
#endif
#define EHOSTDOWN        WSAEHOSTDOWN
#define EHOSTUNREACH     WSAEHOSTUNREACH
#ifndef ENOTEMPTY        /* possible previous definition in errno.h */
#define ENOTEMPTY        WSAENOTEMPTY
#endif
#define EPROCLIM         WSAEPROCLIM
#define EUSERS           WSAEUSERS
#define EDQUOT           WSAEDQUOT
#define ESTALE           WSAESTALE
#define EREMOTE          WSAEREMOTE
#endif


/*
 *  Actually use __32_getpwuid() on 64-bit VMS builds for getpwuid()
 */

#if defined(VMS) && \
    defined(__INITIAL_POINTER_SIZE) && (__INITIAL_POINTER_SIZE == 64)
#define getpwuid __32_getpwuid
#endif


/*
 * Macro argv_item_t hides platform details to code using it.
 */

#ifdef VMS
#define argv_item_t  __char_ptr32
#else
#define argv_item_t  char *
#endif


/*
 * We use this ZERO_NULL to avoid picky compiler warnings,
 * when assigning a NULL pointer to a function pointer var.
 */

#define ZERO_NULL 0


#if defined (__LP64__) && defined(__hpux) && !defined(_XOPEN_SOURCE_EXTENDED)
#include <sys/socket.h>
/* HP-UX has this oddity where it features a few functions that don't work
   with socklen_t so we need to convert to ints

   This is due to socklen_t being a 64bit int under 64bit ABI, but the
   pre-xopen (default) interfaces require an int, which is 32bits.

   Therefore, Anytime socklen_t is passed by pointer, the libc function
   truncates the 64bit socklen_t value by treating it as a 32bit value.


   Note that some socket calls are allowed to have a NULL pointer for
   the socklen arg.
*/

inline static int Curl_hp_getsockname(int s, struct sockaddr *name,
                                      socklen_t *namelen)
{
  int rc;
  if(namelen) {
     int len = *namelen;
     rc = getsockname(s, name, &len);
     *namelen = len;
   }
  else
     rc = getsockname(s, name, 0);
  return rc;
}

inline static int Curl_hp_getsockopt(int  s, int level, int optname,
                                     void *optval, socklen_t *optlen)
{
  int rc;
  if(optlen) {
    int len = *optlen;
    rc = getsockopt(s, level, optname, optval, &len);
    *optlen = len;
  }
  else
    rc = getsockopt(s, level, optname, optval, 0);
  return rc;
}

inline static int Curl_hp_accept(int sockfd, struct sockaddr *addr,
                                 socklen_t *addrlen)
{
  int rc;
  if(addrlen) {
     int len = *addrlen;
     rc = accept(sockfd, addr, &len);
     *addrlen = len;
  }
  else
     rc = accept(sockfd, addr, 0);
  return rc;
}


inline static ssize_t Curl_hp_recvfrom(int s, void *buf, size_t len, int flags,
                                       struct sockaddr *from,
                                       socklen_t *fromlen)
{
  ssize_t rc;
  if(fromlen) {
    int fromlen32 = *fromlen;
    rc = recvfrom(s, buf, len, flags, from, &fromlen32);
    *fromlen = fromlen32;
  }
  else {
    rc = recvfrom(s, buf, len, flags, from, 0);
  }
  return rc;
}

#define getsockname(a,b,c) Curl_hp_getsockname((a),(b),(c))
#define getsockopt(a,b,c,d,e) Curl_hp_getsockopt((a),(b),(c),(d),(e))
#define accept(a,b,c) Curl_hp_accept((a),(b),(c))
#define recvfrom(a,b,c,d,e,f) Curl_hp_recvfrom((a),(b),(c),(d),(e),(f))

#endif /* HPUX work-around */


#endif /* __SETUP_ONCE_H */

