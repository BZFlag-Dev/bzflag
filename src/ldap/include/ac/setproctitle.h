/* Generic setproctitle.h */
/* $OpenLDAP: pkg/ldap/include/ac/setproctitle.h,v 1.19.2.3 2007/01/02 21:43:47 kurt Exp $ */
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

#ifndef _AC_SETPROCTITLE_H
#define _AC_SETPROCTITLE_H

#ifdef LDAP_PROCTITLE

#if defined( HAVE_LIBUTIL_H )
#	include <libutil.h>
#else
	/* use lutil version */
	LDAP_LUTIL_F (void) (setproctitle) LDAP_P((const char *fmt, ...)) \
		LDAP_GCCATTR((format(printf, 1, 2)));
	LDAP_LUTIL_V (int) Argc;
	LDAP_LUTIL_V (char) **Argv;
#endif

#endif /* LDAP_PROCTITLE */
#endif /* _AC_SETPROCTITLE_H */
