# crystal.m4                                                   -*- Autoconf -*-
#==============================================================================
# Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

m4_define([cs_min_version_default], [0.99])

#------------------------------------------------------------------------------
# CS_PATH_CRYSTAL_CHECK([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                       [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS],
#                       [OPTIONAL-LIBS])
#	Checks for Crystal Space paths and libraries by consulting
#	cs-config. It first looks for cs-config in the paths mentioned by
#	$CRYSTAL, then in the paths mentioned by $PATH, and then in
#	/usr/local/crystalspace/bin.  Emits an error if it can not locate
#	cs-config, if the Crystal Space test program fails, or if the available
#	version number is unsuitable.  Exports the variables
#	CRYSTAL_CONFIG_TOOL, CRYSTAL_AVAILABLE, CRYSTAL_VERSION,
#	CRYSTAL_CFLAGS, CRYSTAL_LIBS, CRYSTAL_INCLUDE_DIR, and
#	CRYSTAL_AVAILABLE_LIBS.  If the check succeeds, then CRYSTAL_AVAILABLE
#	will be 'yes', and the other variables set to appropriate values. If it
#	fails, then CRYSTAL_AVAILABLE will be 'no', and the other variables
#	empty.  If REQUIRED-LIBS is specified, then it is a list of Crystal
#	Space libraries which must be present, and for which appropriate
#	compiler and linker flags will be reflected in CRYSTAL_CFLAGS and
#	CRYSTAL_LFLAGS. If OPTIONAL-LIBS is specified, then it is a list of
#	Crystal Space libraries for which appropriate compiler and linker flags
#	should be returned if the libraries are available.  It is not an error
#	for an optional library to be absent. The client can check
#	CRYSTAL_AVAILABLE_LIBS for a list of all libraries available for this
#	particular installation of Crystal Space.  The returned list is
#	independent of REQUIRED-LIBS and OPTIONAL-LIBS.  Use the results of the
#	check like this: CFLAGS="$CFLAGS $CRYSTAL_CFLAGS" and LDFLAGS="$LDFLAGS
#	$CRYSTAL_LIBS"
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CRYSTAL_CHECK],
[AC_ARG_WITH([cs-prefix],
    [AC_HELP_STRING([--with-cs-prefix=CRYSTAL_PREFIX],
	[specify location of Crystal Space installation; this is the \$prefix
	value used when installing the SDK])],
	[CRYSTAL="$withval"
	export CRYSTAL])
AC_ARG_VAR([CRYSTAL], [Prefix where Crystal Space is installed])
AC_ARG_ENABLE([cstest],
    [AC_HELP_STRING([--enable-cstest],
	[verify that the Crystal Space SDK is actually usable
	(default YES)])], [], [enable_cstest=yes])

# Try to find an installed cs-config.
cs_path=''
AS_IF([test -n "$CRYSTAL"],
    [my_IFS=$IFS; IFS=$PATH_SEPARATOR
    for cs_dir in $CRYSTAL; do
	AS_IF([test -n "$cs_path"], [cs_path="$cs_path$PATH_SEPARATOR"])
	cs_path="$cs_path$cs_dir$PATH_SEPARATOR$cs_dir/bin"
    done
    IFS=$my_IFS])

AS_IF([test -n "$cs_path"], [cs_path="$cs_path$PATH_SEPARATOR"])
cs_path="$cs_path$PATH$PATH_SEPARATOR/usr/local/crystalspace/bin"

AC_PATH_TOOL([CRYSTAL_CONFIG_TOOL], [cs-config], [], [$cs_path])

AS_IF([test -n "$CRYSTAL_CONFIG_TOOL"],
    [cfg="$CRYSTAL_CONFIG_TOOL"

    CS_CHECK_PROG_VERSION([Crystal Space], [$cfg --version],
	[m4_default([$1],[cs_min_version_default])], [9.9|.9],
	[cs_sdk=yes], [cs_sdk=no])

    AS_IF([test $cs_sdk = yes],
	[cs_liblist="$4"
	cs_optlibs=CS_TRIM([$5])
	AS_IF([test -n "$cs_optlibs"],
	    [cs_optlibs=`$cfg --available-libs $cs_optlibs`
	    cs_liblist="$cs_liblist $cs_optlibs"])
	CRYSTAL_VERSION=`$cfg --version $cs_liblist`
	CRYSTAL_CFLAGS=CS_RUN_PATH_NORMALIZE([$cfg --cxxflags $cs_liblist])
	CRYSTAL_LIBS=CS_RUN_PATH_NORMALIZE([$cfg --libs $cs_liblist])
	CRYSTAL_INCLUDE_DIR=CS_RUN_PATH_NORMALIZE(
	    [$cfg --includedir $cs_liblist])
	CRYSTAL_AVAILABLE_LIBS=`$cfg --available-libs`
	CRYSTAL_STATICDEPS=`$cfg --static-deps`
	AS_IF([test -z "$CRYSTAL_LIBS"], [cs_sdk=no])])],
    [cs_sdk=no])

AS_IF([test "$cs_sdk" = yes && test "$enable_cstest" = yes],
    [CS_CHECK_BUILD([if Crystal Space SDK is usable], [cs_cv_crystal_sdk],
	[AC_LANG_PROGRAM(
	    [#include <cssysdef.h>
	    #include <csutil/csstring.h>
	    csStaticVarCleanupFN csStaticVarCleanup;],
	    [csString s; s << "Crystal Space";])],
	[CS_CREATE_TUPLE([$CRYSTAL_CFLAGS],[],[$CRYSTAL_LIBS])], [C++],
	[], [cs_sdk=no])])

AS_IF([test "$cs_sdk" = yes],
   [CRYSTAL_AVAILABLE=yes
   $2],
   [CRYSTAL_AVAILABLE=no
   CRYSTAL_CFLAGS=''
   CRYSTAL_VERSION=''
   CRYSTAL_LIBS=''
   CRYSTAL_INCLUDE_DIR=''
   $3])
])


#------------------------------------------------------------------------------
# CS_PATH_CRYSTAL_HELPER([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                        [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS],
#                        [OPTIONAL-LIBS])
#	Deprecated: Backward compatibility wrapper for CS_PATH_CRYSTAL_CHECK().
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CRYSTAL_HELPER],
[CS_PATH_CRYSTAL_CHECK([$1],[$2],[$3],[$4],[$5])])


#------------------------------------------------------------------------------
# CS_PATH_CRYSTAL([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [REQUIRED-LIBS], [OPTIONAL-LIBS])
#	Convenience wrapper for CS_PATH_CRYSTAL_CHECK() which also invokes
#	AC_SUBST() for CRYSTAL_AVAILABLE, CRYSTAL_VERSION, CRYSTAL_CFLAGS,
#	CRYSTAL_LIBS, CRYSTAL_INCLUDE_DIR, and CRYSTAL_AVAILABLE_LIBS.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CRYSTAL],
[CS_PATH_CRYSTAL_CHECK([$1],[$2],[$3],[$4],[$5])
AC_SUBST([CRYSTAL_AVAILABLE])
AC_SUBST([CRYSTAL_VERSION])
AC_SUBST([CRYSTAL_CFLAGS])
AC_SUBST([CRYSTAL_LIBS])
AC_SUBST([CRYSTAL_INCLUDE_DIR])
AC_SUBST([CRYSTAL_AVAILABLE_LIBS])
AC_SUBST([CRYSTAL_STATICDEPS])])


#------------------------------------------------------------------------------
# CS_PATH_CRYSTAL_EMIT([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                      [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS], [OPTIONAL-LIBS],
#                      [EMITTER])
#	Convenience wrapper for CS_PATH_CRYSTAL_CHECK() which also emits
#	CRYSTAL_AVAILABLE, CRYSTAL_VERSION, CRYSTAL_CFLAGS, CRYSTAL_LIBS,
#	CRYSTAL_INCLUDE_DIR, and CRYSTAL_AVAILABLE_LIBS as the build properties
#	CRYSTAL.AVAILABLE, CRYSTAL.VERSION, CRYSTAL.CFLAGS, CRYSTAL.LIBS,
#	CRYSTAL.INCLUDE_DIR, and CRYSTAL.AVAILABLE_LIBS, respectively, using
#	EMITTER.  EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	property and value. If EMITTER is omitted, then
#	CS_EMIT_BUILD_PROPERTY()'s default emitter is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CRYSTAL_EMIT],
[CS_PATH_CRYSTAL_CHECK([$1],[$2],[$3],[$4],[$5])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.AVAILABLE],[$CRYSTAL_AVAILABLE],[$6])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.VERSION],[$CRYSTAL_VERSION],[$6])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.CFLAGS],[$CRYSTAL_CFLAGS],[$6])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.LFLAGS],[$CRYSTAL_LIBS],[$6])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.INCLUDE_DIR],[$CRYSTAL_INCLUDE_DIR],[$6])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.AVAILABLE_LIBS],[$CRYSTAL_AVAILABLE_LIBS],[$6])
_CS_PATH_CRYSTAL_EMIT([CRYSTAL.STATICDEPS],[$CRYSTAL_STATICDEPS],[$6])
])

AC_DEFUN([_CS_PATH_CRYSTAL_EMIT],
[CS_EMIT_BUILD_PROPERTY([$1],[$2],[],[],[$3])])


#------------------------------------------------------------------------------
# CS_PATH_CRYSTAL_JAM([MINIMUM-VERSION], [ACTION-IF-FOUND],
#                     [ACTION-IF-NOT-FOUND], [REQUIRED-LIBS], [OPTIONAL-LIBS])
#	Deprecated: Jam-specific backward compatibility wrapper for
#	CS_PATH_CRYSTAL_EMIT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_CRYSTAL_JAM],
[CS_PATH_CRYSTAL_EMIT([$1],[$2],[$3],[$4],[$5],[CS_JAMCONFIG_PROPERTY])])
