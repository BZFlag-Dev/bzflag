dnl                   C O N F I G U R E . A C
dnl BZFlag Plugins
dnl Copyright (c) 1993 - 2008 Tim Riker
dnl
dnl This package is free software;  you can redistribute it and/or
dnl modify it under the terms of the license found in the file
dnl named COPYING that should have accompanied this file.
dnl
dnl THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
dnl IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
dnl
dnl ******************************************************************
dnl ***                    BZFlag Plugins                          ***
dnl ******************************************************************
dnl
dnl This file is organized into the following sections.  Please add new
dnl tests for functionality to the correct section.
dnl
dnl     0) information on the package
dnl     1) check command-line arguments
dnl     2) check programs
dnl     3) check libraries
dnl     4) check headers
dnl     5) check types/structures
dnl     6) check compiler characteristics
dnl     7) check functions
dnl     8) check system services
dnl     9) output a summary
dnl

dnl ***************************
dnl *** Package Information ***
dnl ***************************

dnl minimum version of autoconf required.  should coincide with
dnl setting in autogen.sh script.
AC_PREREQ(2.52)

AC_INIT(BZFlag Plugins, 14, [http://BZFlag.org], bzflag-plugins)
AC_CONFIG_SRCDIR(SAMPLE_PLUGIN/SAMPLE_PLUGIN.cpp)
AC_REVISION($Revision $)

AC_CONFIG_AUX_DIR([.])

# force locale setting to C so things like date output as expected
LC_ALL=C
# necessary for proper file creation on nfs volumes
umask 022

# override the default autoconf cflags/cxxflags if user has not
# modified them.
if test "x$CFLAGS" = "x" ; then
	# an unset CFLAGS var is set to "-O2 -g" during AC_PROG_CC, so
	# set it to something benign instead like whitespace
	CFLAGS=" "
fi
if test "x$CXXFLAGS" = "x" ; then
	# an unset CXXFLAGS var is set to "-O2 -g" during AC_PROG_CC, soe
	# set it to something benign instead like whitespace
	CXXFLAGS=" "
fi

# cannot override LD directly, so warn about that (configure sets it)
if test "x$LD" != "x" ; then
	AC_MSG_WARN([LD cannot be set directly yet it seems to be set ($LD)])
fi

AC_MSG_NOTICE([**************************])
AC_MSG_NOTICE([* --- BZFlag Plugins --- *])
AC_MSG_NOTICE([**************************])

# classify this machine
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# if this is not a checkout, disable dependency tracking for a faster
# build. it's not likely that the user is doing development from a
# source release. at least they really should not given the extra
# steps that are necessary for making a proper useful diff.
AC_MSG_CHECKING([whether dependency tracking should be enabled])
if test -d $srcdir/.svn ; then
	if test "x$enable_maintainer_mode" = "x" ; then
		enable_maintainer_mode="yes"
	fi
	if test "x$enable_dependency_tracking" = "x" ; then
		enable_dependency_tracking="yes"
	fi
	AC_MSG_RESULT([yes])
else
	if test "x$enable_maintainer_mode" = "x" ; then
		enable_maintainer_mode="no"
	fi
	if test "x$enable_dependency_tracking" = "x" ; then
		enable_dependency_tracking="no"
	fi
	AC_MSG_RESULT([no])
fi

AM_INIT_AUTOMAKE([1.6 dist-zip dist-bzip2])

# disabled static libraries in order to prevent double-compilation
AC_DISABLE_STATIC


dnl ***********************
dnl *** Check arguments ***
dnl ***********************

# no arguments required yet


dnl **************************
dnl *** Check for programs ***
dnl **************************

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_REQUIRE_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AM_PROG_MKDIR_P

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)


dnl ***************************
dnl *** Check for libraries ***
dnl ***************************

# no library checks needed so far


dnl *************************
dnl *** Check for headers ***
dnl *************************

# no header checks needed so far


dnl **********************************
dnl *** Check for types/structures ***
dnl **********************************

# no types/structures checks needed so far


dnl ******************************************
dnl *** Check for compiler characteristics ***
dnl ******************************************

dnl Make sure that we can actually compile
AC_MSG_CHECKING(compiler and flags for sanity)
AC_TRY_RUN([
#include <stdio.h>
int main(){exit(0);}
	],
	[	AC_MSG_RESULT(yes) ],
	[
		AC_MSG_RESULT(no)
		AC_MSG_NOTICE([}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}])
		AC_MSG_ERROR([*** compiler cannot create working executables, check config.log ***])
	]
)


dnl ***************************
dnl *** Check for functions ***
dnl ***************************

# no function checks needed so far


dnl *********************************
dnl *** Check for system services ***
dnl *********************************

# last thing to do before output is to check the main BZFlag include
# direcotry for the API header.
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/../include"


dnl **************************************
dnl *** Configure Makefiles and output ***
dnl **************************************

AC_CONFIG_FILES([
	HoldTheFlag/Makefile
	MSVC/Makefile
	MSVC/VC8/Makefile
	Makefile
	Phoenix/Makefile
	RogueGenocide/Makefile
	SAMPLE_PLUGIN/Makefile
	airspawn/Makefile
	bzfscron/Makefile
	chathistory/Makefile
	customflagsample/Makefile
	fastmap/Makefile
	flagStay/Makefile
	hiddenAdmin/Makefile
	httpTest/Makefile
	keepaway/Makefile
	killall/Makefile
	koth/Makefile
	logDetail/Makefile
	m4/Makefile
	mapchange/Makefile
	nagware/Makefile
	playHistoryTracker/Makefile
	plugin_utils/Makefile
	plugin_utils/VC8/Makefile
	python/Makefile
	rabidRabbit/Makefile
	recordmatch/Makefile
	regFlag/Makefile
	serverControl/Makefile
	serverSideBotSample/Makefile
	shockwaveDeath/Makefile
	soundTest/Makefile
	teamflagreset/Makefile
	thiefControl/Makefile
	timedctf/Makefile
	torBlock/Makefile
	unrealCTF/Makefile
	weaponArena/Makefile
	webstats/Makefile
	webReport/Makefile
	wwzones/Makefile
])

AC_OUTPUT


#################
# patch libtool #
#################
case $host_os in
    darwin*)
	for script in $ac_top_builddir $ac_abs_builddir $ac_builddir . ; do
	    if test "x$script" = "x" ; then
		libtoolscript="libtool"
	    else
		libtoolscript="${script}/libtool"
	    fi
	    if test -f ${libtoolscript} ; then
		if test -w ${libtoolscript} ; then
		    # remove any -all_load option.
		    # provokes libtool linker bug with noinst C++ libraries.
		    sed 's/-all_load.*convenience//g' < $libtoolscript > ${libtoolscript}.sed
		    if test ! "x`cat ${libtoolscript}`" = "x`cat ${libtoolscript}.sed`" ; then
			AC_MSG_RESULT([Found -all_load in libtool script, removing])
			cp ${libtoolscript}.sed ${libtoolscript}
		    fi
		    rm -f ${libtoolscript}.sed
		else
		    AC_MSG_WARN([libtool script exists but is not writable so not attempting to edit])
		fi
	    fi
	done
	;;
esac


dnl **********************
dnl *** Report Summary ***
dnl **********************

AC_MSG_RESULT([])
AC_MSG_RESULT([Plugins Configured for Compilation])
AC_MSG_RESULT([----------------------------------])
for i in ${srcdir}/* ; do
    if test -d $i && test -f $i/$i.cpp -o -f $i/$i.cxx ; then
       plugin="`basename $i`"
       AC_MSG_RESULT([  $plugin])
    fi
done
AC_MSG_RESULT([])
