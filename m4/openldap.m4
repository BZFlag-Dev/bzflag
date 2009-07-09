dnl
dnl macros for configure.in to detect openldap
dnl $Id: openldap.m4,v 1.2 2006/03/13 19:16:11 mel Exp $
dnl

AC_DEFUN([AM_OPENLDAP_PATH],
[

AC_ARG_WITH(openldap-prefix,
            AC_HELP_STRING([--with-openldap-prefix=PFX],
                           [prefix where OPENLDAP is installed (optional)]),
	[
		OPENLDAP_CFLAGS="-I$withval/include"
		OPENLDAP_LIBS="-L$withval/lib -lldap"
	],
	[
		OEPNLDAP_CFLAGS=""
		OPENLDAP_LIBS="-lldap"
	])

openldap_all_ok="no"

ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
CPPFLAGS="$CPPFLAGS $OPENLDAP_CFLAGS"
LIBS="$OPENLDAP_LIBS $LIBS"

dnl
dnl Check for OpenLDAP header, library and basic functionality
AC_CACHE_CHECK([OpenLDAP header and library], [cmu_cv_openldap_hl], [
	AC_RUN_IFELSE([
		AC_LANG_SOURCE([[
#include <ldap.h>

int main() {
	LDAP *ld;
	return ldap_initialize(&ld, "ldap://127.0.0.1");
}
		]])],
		[cmu_cv_openldap_hl="yes"],
		[cmu_cv_openldap_hl="no"],
		[echo $ECHO_N "cross compiling; assuming OK... $ECHO_C"]
	)
])

if test "$cmu_cv_openldap_hl" = "yes"; then
	dnl
	dnl Check for OpenLDAP version compatility
	AC_CACHE_CHECK([OpenLDAP api], [cmu_cv_openldap_api],[
	    AC_COMPILE_IFELSE([ AC_LANG_SOURCE([[
#include <ldap.h>

#ifndef LDAP_API_FEATURE_X_OPENLDAP
#error not openldap
#endif
	]])],  [cmu_cv_openldap_api=yes], [cmu_cv_openldap_api=no])])

	if test "$cmu_cv_openldap_api" = "yes"; then
		dnl
		dnl Check for OpenLDAP version compatility
		AC_CACHE_CHECK([OpenLDAP version], [cmu_cv_openldap_compat],[
		    AC_RUN_IFELSE([ AC_LANG_SOURCE([[
#include <ldap.h>
#include <stdio.h>

// the defines are not required to be integer type (e.g patch = 40-beta)
// so scan integers from them

#define Q(x) #x
#define GET(x, y) int x; sscanf(Q(y), "%d", &x);

int main()
{
        GET(major, LDAP_VENDOR_VERSION_MAJOR);
        GET(minor, LDAP_VENDOR_VERSION_MINOR);
        GET(patch, LDAP_VENDOR_VERSION_PATCH);

        if((major == 2 && minor == 3 && patch >= 39) ||
           (major == 2 && minor >= 4))
                return 0;
        else
		return 1;
}
		]])],  [cmu_cv_openldap_compat=yes], [cmu_cv_openldap_compat=no])])

		if test "$cmu_cv_openldap_compat" = "yes"; then
			openldap_all_ok="yes"
		fi
	fi
		
fi

CPPFLAGS="$ac_save_CPPFLAGS"
LIBS="$ac_save_LIBS"

dnl if something went wrong, reset the flags
if test "$openldap_all_ok" != "yes"; then
	OPENLDAP_LIBS=""
	OPENLDAP_CFLAGS=""
fi

AC_SUBST(OPENLDAP_CFLAGS)
AC_SUBST(OPENLDAP_LIBS)

])
