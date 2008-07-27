/* Generic unistd.h */
/* $OpenLDAP: pkg/ldap/include/ac/unistd.h,v 1.35.2.3 2007/01/02 21:43:47 kurt Exp $ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 *
 * Copyright 1998-2007 The OpenLDAP Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */

#ifndef _AC_UNISTD_H
#define _AC_UNISTD_H

#if HAVE_SYS_TYPES_H
#	include <sys/types.h>
#endif

#if HAVE_UNISTD_H
#	include <unistd.h>
#endif

#if HAVE_PROCESS_H
#	include <process.h>
#endif

/* note: callers of crypt(3) should include <ac/crypt.h> */

#if defined(HAVE_GETPASSPHRASE)
LDAP_LIBC_F(char*)(getpassphrase)();

#elif defined(HAVE_GETPASS)
#define getpassphrase(p) getpass(p)
LDAP_LIBC_F(char*)(getpass)();

#else
#define NEED_GETPASSPHRASE 1
#define getpassphrase(p) lutil_getpass(p)
LDAP_LUTIL_F(char*)(lutil_getpass) LDAP_P((const char *getpass));
#endif

/* getopt() defines may be in separate include file */
#if HAVE_GETOPT_H
#	include <getopt.h>

#elif !defined(HAVE_GETOPT)
	/* no getopt, assume we need getopt-compat.h */
#	include <getopt-compat.h>

#else
	/* assume we need to declare these externs */
	LDAP_LIBC_V (char *) optarg;
	LDAP_LIBC_V (int) optind, opterr, optopt;
#endif

/* use lutil file locking */
#define ldap_lockf(x)	lutil_lockf(x)
#define ldap_unlockf(x)	lutil_unlockf(x)
#include <lutil_lockf.h>

/*
 * Windows: although sleep() will be resolved by both MSVC and Mingw GCC
 * linkers, the function is not declared in header files. This is
 * because Windows' version of the function is called _sleep(), and it
 * is declared in stdlib.h
 */

#ifdef _WIN32
#define sleep _sleep
#endif

#endif /* _AC_UNISTD_H */
