/* Generic alloca.h */
/* $OpenLDAP: pkg/ldap/include/ac/alloca.h,v 1.16.2.3 2007/01/02 21:43:47 kurt Exp $ */
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

#ifndef _AC_ALLOCA_H
#define _AC_ALLOCA_H

/*
 * use of alloca is disallowed as it is machine dependent
 */
#error  "alloca() not supported, use malloc()"

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
extern char *(alloca)();
#   endif
#  endif
# endif
#endif


#endif /* _AC_ALLOCA_H */
