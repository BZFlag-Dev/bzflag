#ifndef __ARES_INET_NTOP_H
#define __ARES_INET_NTOP_H

/* $Id: inet_ntop.h,v 1.3 2007-11-19 15:47:01 bagder Exp $ */

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

#if defined(HAVE_INET_NTOP) && defined(HAVE_INET_NTOP_IPV6)
#define ares_inet_ntop(w,x,y,z) inet_ntop(w,x,y,z)
#else
const char *ares_inet_ntop(int af, const void *src, char *dst, size_t size);
#endif

#endif /* __ARES_INET_NTOP_H */
