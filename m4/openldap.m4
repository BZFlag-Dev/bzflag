dnl
dnl macros for configure.in to detect openldap
dnl $Id: openldap.m4,v 1.2 2006/03/13 19:16:11 mel Exp $
dnl

AC_DEFUN([AM_OPENLDAP_PATH],
[

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

AC_ARG_WITH(openldap-prefix,
            AC_HELP_STRING([--with-openldap-prefix=PFX],
                           [prefix where OPENLDAP is installed (optional)]),
     openldap_lib_prefix="$withval", openldap_lib_prefix="/usr")

dnl set OpenLDAP lib path if the checks succeeded
if test "$cmu_cv_openldap_api" = "yes"; then
	if test "$cmu_cv_openldap_compat" = "yes"; then
		OPENLDAP_LIBS="$openldap_lib_prefix/lib/libldap.so"
		OPENLDAP_CFLAGS=""
	fi
fi

AC_SUBST(OPENLDAP_CFLAGS)
AC_SUBST(OPENLDAP_LIBS)

])
