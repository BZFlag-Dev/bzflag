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
		OPENLDAP_CFLAGS="-I$withval_prefix/include"
		OPENLDAP_LIBS="-L$withval_prefix/lib -lldap"
	],
	[
		OEPNLDAP_CFLAGS=""
		OPENLDAP_LIBS="-lldap"
	])



ac_save_CPPFLAGS="$CPPFLAGS"
ac_save_LIBS="$LIBS"
CPPFLAGS="$CPPFLAGS $OPENLDAP_CFLAGS"
LIBS="$OPENLDAP_LIBS $LIBS"

dnl
dnl Check for OpenLDAP version compatility
AC_CACHE_CHECK([OpenLDAP api], [cmu_cv_openldap_api],[
    AC_EGREP_CPP(__openldap_api,[
#include <ldap.h>

#ifdef LDAP_API_FEATURE_X_OPENLDAP
char *__openldap_api = LDAP_API_FEATURE_X_OPENLDAP;
#endif
],      [cmu_cv_openldap_api=yes], [cmu_cv_openldap_api=no])])

dnl
dnl Check for OpenLDAP version compatility
AC_CACHE_CHECK([OpenLDAP version], [cmu_cv_openldap_compat],[
    AC_EGREP_CPP(__openldap_compat,[
#include <ldap.h>

/* Require 2.3.39+ */
#if LDAP_VENDOR_VERSION_MAJOR == 2  && LDAP_VENDOR_VERSION_MINOR == 3 && LDAP_VENDOR_VERSION_PATCH >= 39
char *__openldap_compat = "2.3.39 or better okay";
#elif LDAP_VENDOR_VERSION_MAJOR == 2  && LDAP_VENDOR_VERSION_MINOR >= 4
char *__openldap_compat = "2.4.0 or better okay";
#endif
],      [cmu_cv_openldap_compat=yes], [cmu_cv_openldap_compat=no])])

dnl make sure it links too if the initial tests succeeded
if test "$cmu_cv_openldap_api" = "yes"; then
	if test "$cmu_cv_openldap_compat" = "yes"; then
		AC_RUN_IFELSE([
			AC_LANG_SOURCE([[
#include <ldap.h>

int main() {
	LDAP *ld;
	return ldap_initialize(&ld, "ldap://127.0.0.1");
}
				]])
			],
			[openldap_all_ok="yes"],
			[],
			[echo $ECHO_N "cross compiling; assuming OK... $ECHO_C"]
		)
	fi
fi

CPPFLAGS="$ac_save_CPPFLAGS"
LIBS="$ac_save_LIBS"

dnl if something failed, reset the flags
if test "$openldap_all_ok" != "yes"; then
	OPENLDAP_LIBS=""
	OPENLDAP_CFLAGS=""
fi

AC_SUBST(OPENLDAP_CFLAGS)
AC_SUBST(OPENLDAP_LIBS)

])
