/* Generic stdarg.h */
/* $OpenLDAP: pkg/ldap/include/ac/stdarg.h,v 1.17.2.3 2007/01/02 21:43:47 kurt Exp $ */
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

#ifndef _AC_STDARG_H
#define _AC_STDARG_H 1

/* require STDC variable argument support */

#include <stdarg.h>

#ifndef HAVE_STDARG
#	define HAVE_STDARG 1
#endif

#endif /* _AC_STDARG_H */
