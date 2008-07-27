/* localize.h (i18n/l10n) */
/* $OpenLDAP: pkg/ldap/include/ac/localize.h,v 1.5.2.3 2007/01/02 21:43:47 kurt Exp $ */
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

#ifndef _AC_LOCALIZE_H
#define _AC_LOCALIZE_H

#ifdef LDAP_LOCALIZE

#	include <locale.h>
#	include <libintl.h>

	/* enable i18n/l10n */
#	define gettext_noop(s)		s
#	define _(s)					gettext(s)
#	define N_(s)				gettext_noop(s)
#	define ldap_pvt_setlocale(c,l)		((void) setlocale(c, l))
#	define ldap_pvt_textdomain(d)		((void) textdomain(d))
#	define ldap_pvt_bindtextdomain(p,d)	((void) bindtextdomain(p, d))

#else

	/* disable i18n/l10n */
#	define _(s)					s
#	define N_(s)				s
#	define ldap_pvt_setlocale(c,l)		((void) 0)
#	define ldap_pvt_textdomain(d)		((void) 0)
#	define ldap_pvt_bindtextdomain(p,d)	((void) 0)

#endif

#endif /* _AC_LOCALIZE_H */
