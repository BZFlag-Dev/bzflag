/* $Id: ares_ipv6.h,v 1.7 2007-11-19 15:47:01 bagder Exp $ */

/* Copyright (C) 2005 by Dominick Meglio
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef ARES_IPV6_H
#define ARES_IPV6_H

#ifndef HAVE_PF_INET6
#define PF_INET6 AF_INET6
#endif

#ifndef s6_addr
struct in6_addr {
  union {
    unsigned char _S6_u8[16];
  } _S6_un;
};
#define s6_addr _S6_un._S6_u8
#endif

#ifndef HAVE_STRUCT_SOCKADDR_IN6
struct sockaddr_in6
{
  unsigned short  sin6_family;
  unsigned short  sin6_port;
  unsigned long   sin6_flowinfo;
  struct in6_addr sin6_addr;
  unsigned int    sin6_scope_id;
};
#endif

#ifndef HAVE_STRUCT_ADDRINFO
struct addrinfo 
{
  int              ai_flags;
  int              ai_family;
  int              ai_socktype;
  int              ai_protocol;
  socklen_t        ai_addrlen;   /* Follow rfc3493 struct addrinfo */
  char            *ai_canonname;
  struct sockaddr *ai_addr;
  struct addrinfo *ai_next;
};
#endif

#ifndef NS_IN6ADDRSZ
#if SIZEOF_STRUCT_IN6_ADDR == 0
/* We cannot have it set to zero, so we pick a fixed value here */
#define NS_IN6ADDRSZ 16
#else
#define NS_IN6ADDRSZ SIZEOF_STRUCT_IN6_ADDR
#endif
#endif

#ifndef NS_INADDRSZ
#define NS_INADDRSZ SIZEOF_STRUCT_IN_ADDR
#endif

#ifndef NS_INT16SZ
#define NS_INT16SZ 2
#endif

#ifndef IF_NAMESIZE
#ifdef IFNAMSIZ
#define IF_NAMESIZE IFNAMSIZ
#else
#define IF_NAMESIZE 256
#endif
#endif

#endif /* ARES_IPV6_H */
