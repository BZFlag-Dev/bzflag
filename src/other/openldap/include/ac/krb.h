/* Generic krb.h */
/* $OpenLDAP: pkg/ldap/include/ac/krb.h,v 1.15.2.3 2007/01/02 21:43:47 kurt Exp $ */
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

/* Kerberos IV */

#ifndef _AC_KRB_H
#define _AC_KRB_H

#if defined( HAVE_KRB4 )

#if defined( HAVE_KERBEROSIV_KRB_H )
#include <kerberosIV/krb.h>
#elif defined( HAVE_KRB_H )
#include <krb.h>
#endif

#if defined( HAVE_KERBEROSIV_DES_H )
#include <kerberosIV/des.h>
#elif defined( HAVE_DES_H )
#include <des.h>
#endif

#endif /* HAVE_KRB4 */
#endif /* _AC_KRB_H */
