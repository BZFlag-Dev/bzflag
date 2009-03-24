#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# $Id: curl-compilers.m4,v 1.46 2008-11-18 20:13:56 yangtse Exp $
#***************************************************************************

# File version for 'aclocal' use. Keep it a single number.
# serial 46


dnl CURL_CHECK_COMPILER
dnl -------------------------------------------------
dnl Verify if the C compiler being used is known.

AC_DEFUN([CURL_CHECK_COMPILER], [
  #
  compiler_id="unknown"
  compiler_num="0"
  #
  flags_dbg_all="unknown"
  flags_dbg_yes="unknown"
  flags_dbg_off="unknown"
  flags_opt_all="unknown"
  flags_opt_yes="unknown"
  flags_opt_off="unknown"
  #
  CURL_CHECK_COMPILER_DEC_C
  CURL_CHECK_COMPILER_HPUX_C
  CURL_CHECK_COMPILER_IBM_C
  CURL_CHECK_COMPILER_INTEL_C
  CURL_CHECK_COMPILER_GNU_C
  CURL_CHECK_COMPILER_LCC
  CURL_CHECK_COMPILER_SGI_MIPSPRO_C
  CURL_CHECK_COMPILER_SGI_MIPS_C
  CURL_CHECK_COMPILER_SUNPRO_C
  CURL_CHECK_COMPILER_TINY_C
  CURL_CHECK_COMPILER_WATCOM_C
  #
  if test "$compiler_id" = "unknown"; then
  cat <<_EOF 1>&2
***
*** Warning: This configure script does not have information about the
*** compiler you are using, relative to the flags required to enable or
*** disable generation of debug info, optimization options or warnings.
***
*** Whatever settings are present in CFLAGS will be used for this run.
***
*** If you wish to help the cURL project to better support your compiler
*** you can report this and the required info on the libcurl development
*** mailing list: http://cool.haxx.se/mailman/listinfo/curl-library/
***
_EOF
  fi
])


dnl CURL_CHECK_COMPILER_DEC_C
dnl -------------------------------------------------
dnl Verify if compiler being used is DEC C.

AC_DEFUN([CURL_CHECK_COMPILER_DEC_C], [
  AC_MSG_CHECKING([if compiler is DEC/Compaq/HP C])
  CURL_CHECK_DEF([__DECC], [], [silent])
  CURL_CHECK_DEF([__DECC_VER], [], [silent])
  if test "$curl_cv_have_def___DECC" = "yes" &&
    test "$curl_cv_have_def___DECC_VER" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_id="DEC_C"
    flags_dbg_all="-g -g0 -g1 -g2 -g3"
    flags_dbg_yes="-g2"
    flags_dbg_off="-g0"
    flags_opt_all="-O -O0 -O1 -O2 -O3 -O4"
    flags_opt_yes="-O1"
    flags_opt_off="-O0"
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_GNU_C
dnl -------------------------------------------------
dnl Verify if compiler being used is GNU C.

AC_DEFUN([CURL_CHECK_COMPILER_GNU_C], [
  AC_REQUIRE([CURL_CHECK_COMPILER_INTEL_C])dnl
  AC_MSG_CHECKING([if compiler is GNU C])
  CURL_CHECK_DEF([__GNUC__], [], [silent])
  if test "$curl_cv_have_def___GNUC__" = "yes" &&
    test "$compiler_id" = "unknown"; then
    AC_MSG_RESULT([yes])
    compiler_id="GNU_C"
    gccver=`$CC -dumpversion`
    gccvhi=`echo $gccver | cut -d . -f1`
    gccvlo=`echo $gccver | cut -d . -f2`
    compiler_num=`(expr $gccvhi "*" 100 + $gccvlo) 2>/dev/null`
    flags_dbg_all="-g -g0 -g1 -g2 -g3"
    flags_dbg_all="$flags_dbg_all -ggdb"
    flags_dbg_all="$flags_dbg_all -gstabs"
    flags_dbg_all="$flags_dbg_all -gstabs+"
    flags_dbg_all="$flags_dbg_all -gcoff"
    flags_dbg_all="$flags_dbg_all -gxcoff"
    flags_dbg_all="$flags_dbg_all -gdwarf-2"
    flags_dbg_all="$flags_dbg_all -gvms"
    flags_dbg_yes="-g"
    flags_dbg_off="-g0"
    flags_opt_all="-O -O0 -O1 -O2 -O3 -Os"
    flags_opt_yes="-O2"
    flags_opt_off="-O0"
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_HPUX_C
dnl -------------------------------------------------
dnl Verify if compiler being used is HP-UX C.

AC_DEFUN([CURL_CHECK_COMPILER_HPUX_C], [
  AC_MSG_CHECKING([if compiler is HP-UX C])
  CURL_CHECK_DEF([__HP_cc], [], [silent])
  if test "$curl_cv_have_def___HP_cc" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_id="HP_UX_C"
    flags_dbg_all="-g -s"
    flags_dbg_yes="-g"
    flags_dbg_off="-s"
    flags_opt_all="-O +O0 +O1 +O2 +O3 +O4"
    flags_opt_yes="+O2"
    flags_opt_off="+O0"
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_IBM_C
dnl -------------------------------------------------
dnl Verify if compiler being used is IBM C.

AC_DEFUN([CURL_CHECK_COMPILER_IBM_C], [
  AC_MSG_CHECKING([if compiler is IBM C])
  CURL_CHECK_DEF([__IBMC__], [], [silent])
  if test "$curl_cv_have_def___IBMC__" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_id="IBM_C"
    flags_dbg_all="-g -g0 -g1 -g2 -g3"
    flags_dbg_yes="-g"
    flags_dbg_off=""
    flags_opt_all="-O -O0 -O1 -O2 -O3 -O4 -O5"
    flags_opt_all="$flags_opt_all -qnooptimize"
    flags_opt_all="$flags_opt_all -qoptimize=0"
    flags_opt_all="$flags_opt_all -qoptimize=1"
    flags_opt_all="$flags_opt_all -qoptimize=2"
    flags_opt_all="$flags_opt_all -qoptimize=3"
    flags_opt_all="$flags_opt_all -qoptimize=4"
    flags_opt_all="$flags_opt_all -qoptimize=5"
    flags_opt_yes="-O2"
    flags_opt_off="-qnooptimize"
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_INTEL_C
dnl -------------------------------------------------
dnl Verify if compiler being used is Intel C.

AC_DEFUN([CURL_CHECK_COMPILER_INTEL_C], [
  AC_BEFORE([$0],[CURL_CHECK_COMPILER_GNU_C])dnl
  AC_MSG_CHECKING([if compiler is Intel C])
  CURL_CHECK_DEF([__INTEL_COMPILER], [], [silent])
  if test "$curl_cv_have_def___INTEL_COMPILER" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_num="$curl_cv_def___INTEL_COMPILER"
    CURL_CHECK_DEF([__i386__], [], [silent])
    CURL_CHECK_DEF([__unix__], [], [silent])
    if test "$curl_cv_have_def___unix__" = "yes"; then
      compiler_id="INTEL_UNIX_C"
      flags_dbg_all="-g -g0"
      flags_dbg_yes="-g"
      flags_dbg_off="-g0"
      flags_opt_all="-O -O0 -O1 -O2 -O3 -Os"
      flags_opt_yes="-O2"
      flags_opt_off="-O0"
      dnl icc 9.1 optimization on IA32 triggers SIGSEGV
      if test "$curl_cv_have_def___i386__" = "yes" &&
        test "$compiler_num" -eq "910"; then
        INTEL_UNIX_C_OPT_SIGSEGV="yes"
      else
        INTEL_UNIX_C_OPT_SIGSEGV="no"
      fi
    else
      compiler_id="INTEL_WINDOWS_C"
      flags_dbg_all="/ZI /Zi /zI /zi /ZD /Zd /zD /zd /Z7 /z7 /Oy /Oy-"
      flags_dbg_all="$flags_dbg_all /debug"
      flags_dbg_all="$flags_dbg_all /debug:none"
      flags_dbg_all="$flags_dbg_all /debug:minimal"
      flags_dbg_all="$flags_dbg_all /debug:partial"
      flags_dbg_all="$flags_dbg_all /debug:full"
      flags_dbg_all="$flags_dbg_all /debug:semantic_stepping"
      flags_dbg_all="$flags_dbg_all /debug:extended"
      flags_dbg_yes="/Zi /Oy-"
      flags_dbg_off="/debug:none /Oy-"
      flags_opt_all="/O /O0 /O1 /O2 /O3 /Od /Og /Og- /Oi /Oi-"
      flags_opt_yes="/O2"
      flags_opt_off="/Od"
    fi
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_LCC
dnl -------------------------------------------------
dnl Verify if compiler being used is LCC.

AC_DEFUN([CURL_CHECK_COMPILER_LCC], [
  AC_MSG_CHECKING([if compiler is LCC])
  CURL_CHECK_DEF([__LCC__], [], [silent])
  if test "$curl_cv_have_def___LCC__" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_id="LCC"
    flags_dbg_all="-g"
    flags_dbg_yes="-g"
    flags_dbg_off=""
    flags_opt_all=""
    flags_opt_yes=""
    flags_opt_off=""
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_SGI_MIPS_C
dnl -------------------------------------------------
dnl Verify if compiler being used is SGI MIPS C.

AC_DEFUN([CURL_CHECK_COMPILER_SGI_MIPS_C], [
  AC_REQUIRE([CURL_CHECK_COMPILER_SGI_MIPSPRO_C])dnl
  AC_MSG_CHECKING([if compiler is SGI MIPS C])
  CURL_CHECK_DEF([__GNUC__], [], [silent])
  CURL_CHECK_DEF([__sgi], [], [silent])
  if test "$curl_cv_have_def___GNUC__" = "no" &&
    test "$curl_cv_have_def___sgi" = "yes" &&
    test "$compiler_id" = "unknown"; then
    AC_MSG_RESULT([yes])
    compiler_id="SGI_MIPS_C"
    flags_dbg_all="-g -g0 -g1 -g2 -g3"
    flags_dbg_yes="-g"
    flags_dbg_off="-g0"
    flags_opt_all="-O -O0 -O1 -O2 -O3 -Ofast"
    flags_opt_yes="-O2"
    flags_opt_off="-O0"
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_SGI_MIPSPRO_C
dnl -------------------------------------------------
dnl Verify if compiler being used is SGI MIPSpro C.

AC_DEFUN([CURL_CHECK_COMPILER_SGI_MIPSPRO_C], [
  AC_BEFORE([$0],[CURL_CHECK_COMPILER_SGI_MIPS_C])dnl
  AC_MSG_CHECKING([if compiler is SGI MIPSpro C])
  CURL_CHECK_DEF([__GNUC__], [], [silent])
  CURL_CHECK_DEF([_COMPILER_VERSION], [], [silent])
  CURL_CHECK_DEF([_SGI_COMPILER_VERSION], [], [silent])
  if test "$curl_cv_have_def___GNUC__" = "no" &&
    (test "$curl_cv_have_def__SGI_COMPILER_VERSION" = "yes" ||
     test "$curl_cv_have_def__COMPILER_VERSION" = "yes"); then
    AC_MSG_RESULT([yes])
    compiler_id="SGI_MIPSPRO_C"
    flags_dbg_all="-g -g0 -g1 -g2 -g3"
    flags_dbg_yes="-g"
    flags_dbg_off="-g0"
    flags_opt_all="-O -O0 -O1 -O2 -O3 -Ofast"
    flags_opt_yes="-O2"
    flags_opt_off="-O0"
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_SUNPRO_C
dnl -------------------------------------------------
dnl Verify if compiler being used is SunPro C.

AC_DEFUN([CURL_CHECK_COMPILER_SUNPRO_C], [
  AC_MSG_CHECKING([if compiler is SunPro C])
  CURL_CHECK_DEF([__SUNPRO_C], [], [silent])
  if test "$curl_cv_have_def___SUNPRO_C" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_id="SUNPRO_C"
    flags_dbg_all="-g -s"
    flags_dbg_yes="-g"
    flags_dbg_off="-s"
    flags_opt_all="-O -xO -xO1 -xO2 -xO3 -xO4 -xO5"
    flags_opt_yes="-xO2"
    flags_opt_off=""
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_TINY_C
dnl -------------------------------------------------
dnl Verify if compiler being used is Tiny C.

AC_DEFUN([CURL_CHECK_COMPILER_TINY_C], [
  AC_MSG_CHECKING([if compiler is Tiny C])
  CURL_CHECK_DEF([__TINYC__], [], [silent])
  if test "$curl_cv_have_def___TINYC__" = "yes"; then
    AC_MSG_RESULT([yes])
    compiler_id="TINY_C"
    flags_dbg_all="-g -b"
    flags_dbg_yes="-g"
    flags_dbg_off=""
    flags_opt_all=""
    flags_opt_yes=""
    flags_opt_off=""
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CHECK_COMPILER_WATCOM_C
dnl -------------------------------------------------
dnl Verify if compiler being used is Watcom C.

AC_DEFUN([CURL_CHECK_COMPILER_WATCOM_C], [
  AC_MSG_CHECKING([if compiler is Watcom C])
  CURL_CHECK_DEF([__WATCOMC__], [], [silent])
  if test "$curl_cv_have_def___WATCOMC__" = "yes"; then
    AC_MSG_RESULT([yes])
    CURL_CHECK_DEF([__UNIX__], [], [silent])
    if test "$curl_cv_have_def___UNIX__" = "yes"; then
      compiler_id="WATCOM_UNIX_C"
      flags_dbg_all="-g1 -g1+ -g2 -g3"
      flags_dbg_yes="-g2"
      flags_dbg_off=""
      flags_opt_all="-O0 -O1 -O2 -O3"
      flags_opt_yes="-O2"
      flags_opt_off="-O0"
    else
      compiler_id="WATCOM_WINDOWS_C"
      flags_dbg_all=""
      flags_dbg_yes=""
      flags_dbg_off=""
      flags_opt_all=""
      flags_opt_yes=""
      flags_opt_off=""
    fi
  else
    AC_MSG_RESULT([no])
  fi
])


dnl CURL_CONVERT_INCLUDE_TO_ISYSTEM
dnl -------------------------------------------------
dnl Changes standard include paths present in CFLAGS
dnl and CPPFLAGS into isystem include paths. This is
dnl done to prevent GNUC from generating warnings on
dnl headers from these locations, even though this is
dnl not reliable on ancient GNUC versions.

AC_DEFUN([CURL_CONVERT_INCLUDE_TO_ISYSTEM], [
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  tmp_has_include="no"
  tmp_chg_FLAGS="$CFLAGS"
  for word1 in $tmp_chg_FLAGS; do
    case "$word1" in
      -I*)
        tmp_has_include="yes"
        ;;
    esac
  done
  if test "$tmp_has_include" = "yes"; then
    tmp_chg_FLAGS=`echo "$tmp_chg_FLAGS" | "$SED" 's/^-I/ -isystem /g'`
    tmp_chg_FLAGS=`echo "$tmp_chg_FLAGS" | "$SED" 's/ -I/ -isystem /g'`
    CFLAGS="$tmp_chg_FLAGS"
    squeeze CFLAGS
  fi
  tmp_has_include="no"
  tmp_chg_FLAGS="$CPPFLAGS"
  for word1 in $tmp_chg_FLAGS; do
    case "$word1" in
      -I*)
        tmp_has_include="yes"
        ;;
    esac
  done
  if test "$tmp_has_include" = "yes"; then
    tmp_chg_FLAGS=`echo "$tmp_chg_FLAGS" | "$SED" 's/^-I/ -isystem /g'`
    tmp_chg_FLAGS=`echo "$tmp_chg_FLAGS" | "$SED" 's/ -I/ -isystem /g'`
    CPPFLAGS="$tmp_chg_FLAGS"
    squeeze CPPFLAGS
  fi
])


dnl CURL_COMPILER_WORKS_IFELSE ([ACTION-IF-WORKS], [ACTION-IF-NOT-WORKS])
dnl -------------------------------------------------
dnl Verify if the C compiler seems to work with the
dnl settings that are 'active' at the time the test
dnl is performed.

AC_DEFUN([CURL_COMPILER_WORKS_IFELSE], [
  dnl compilation capability verification
  tmp_compiler_works="unknown"
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([[
    ]],[[
      int i = 1;
      return i;
    ]])
  ],[
    tmp_compiler_works="yes"
  ],[
    tmp_compiler_works="no"
    echo " " >&6
    sed 's/^/cc-fail: /' conftest.err >&6
    echo " " >&6
  ])
  dnl linking capability verification
  if test "$tmp_compiler_works" = "yes"; then
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[
      ]],[[
        int i = 1;
        return i;
      ]])
    ],[
      tmp_compiler_works="yes"
    ],[
      tmp_compiler_works="no"
      echo " " >&6
      sed 's/^/link-fail: /' conftest.err >&6
      echo " " >&6
    ])
  fi
  dnl only do runtime verification when not cross-compiling
  if test "x$cross_compiling" != "xyes" &&
    test "$tmp_compiler_works" = "yes"; then
    AC_RUN_IFELSE([
      AC_LANG_PROGRAM([[
#       ifdef __STDC__
#         include <stdlib.h>
#       endif
      ]],[[
        int i = 0;
        exit(i);
      ]])
    ],[
      tmp_compiler_works="yes"
    ],[
      tmp_compiler_works="no"
      echo " " >&6
      echo "run-fail: test program exited with status $ac_status" >&6
      echo " " >&6
    ])
  fi
  dnl branch upon test result
  if test "$tmp_compiler_works" = "yes"; then
  ifelse($1,,:,[$1])
  ifelse($2,,,[else
    $2])
  fi
])


dnl CURL_SET_COMPILER_BASIC_OPTS
dnl -------------------------------------------------
dnl Sets compiler specific options/flags which do not
dnl depend on configure's debug, optimize or warnings
dnl options.

AC_DEFUN([CURL_SET_COMPILER_BASIC_OPTS], [
  AC_REQUIRE([CURL_CHECK_COMPILER])dnl
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  #
  if test "$compiler_id" != "unknown"; then
    #
    if test "$compiler_id" = "GNU_C"; then
      CURL_CONVERT_INCLUDE_TO_ISYSTEM
    fi
    #
    tmp_save_CPPFLAGS="$CPPFLAGS"
    tmp_save_CFLAGS="$CFLAGS"
    tmp_CPPFLAGS=""
    tmp_CFLAGS=""
    #
    case "$compiler_id" in
        #
      DEC_C)
        #
        dnl Select strict ANSI C compiler mode
        tmp_CFLAGS="$tmp_CFLAGS -std1"
        dnl Turn off optimizer ANSI C aliasing rules
        tmp_CFLAGS="$tmp_CFLAGS -noansi_alias"
        dnl Generate warnings for missing function prototypes
        tmp_CFLAGS="$tmp_CFLAGS -warnprotos"
        dnl Change some warnings into fatal errors
        tmp_CFLAGS="$tmp_CFLAGS -msg_fatal toofewargs,toomanyargs"
        ;;
        #
      GNU_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      HP_UX_C)
        #
        dnl Disallow run-time dereferencing of null pointers
        tmp_CFLAGS="$tmp_CFLAGS -z"
        dnl Disable some remarks
        dnl #4227: padding struct with n bytes to align member
        dnl #4255: padding size of struct with n bytes to alignment boundary
        tmp_CFLAGS="$tmp_CFLAGS +W 4227,4255"
        ;;
        #
      IBM_C)
        #
        dnl Ensure that compiler optimizations are always thread-safe.
        tmp_CFLAGS="$tmp_CFLAGS -qthreaded"
        dnl Disable type based strict aliasing optimizations, using worst
        dnl case aliasing assumptions when compiling. Type based aliasing
        dnl would restrict the lvalues that could be safely used to access
        dnl a data object.
        tmp_CFLAGS="$tmp_CFLAGS -qnoansialias"
        dnl Force compiler to stop after the compilation phase, without
        dnl generating an object code file when compilation has errors.
        tmp_CFLAGS="$tmp_CFLAGS -qhalt=e"
        ;;
        #
      INTEL_UNIX_C)
        #
        dnl On unix this compiler uses gcc's header files, so
        dnl we select ANSI C89 dialect plus GNU extensions.
        tmp_CFLAGS="$tmp_CFLAGS -std=gnu89"
        dnl Change some warnings into errors
        dnl #140: too many arguments in function call
        dnl #147: declaration is incompatible with 'previous one'
        dnl #165: too few arguments in function call
        dnl #266: function declared implicitly
        tmp_CPPFLAGS="$tmp_CPPFLAGS -we 140,147,165,266"
        dnl Disable some remarks
        dnl #279: controlling expression is constant
        dnl #981: operands are evaluated in unspecified order
        dnl #1469: "cc" clobber ignored
        tmp_CPPFLAGS="$tmp_CPPFLAGS -wd 279,981,1469"
        ;;
        #
      INTEL_WINDOWS_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      LCC)
        #
        dnl Disallow run-time dereferencing of null pointers
        tmp_CFLAGS="$tmp_CFLAGS -n"
        ;;
        #
      SGI_MIPS_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      SGI_MIPSPRO_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      SUNPRO_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      TINY_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      WATCOM_UNIX_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      WATCOM_WINDOWS_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
    esac
    #
    squeeze tmp_CPPFLAGS
    squeeze tmp_CFLAGS
    #
    if test ! -z "$tmp_CFLAGS" || test ! -z "$tmp_CPPFLAGS"; then
      AC_MSG_CHECKING([if compiler accepts some basic options])
      CPPFLAGS="$tmp_save_CPPFLAGS $tmp_CPPFLAGS"
      CFLAGS="$tmp_save_CFLAGS $tmp_CFLAGS"
      squeeze CPPFLAGS
      squeeze CFLAGS
      CURL_COMPILER_WORKS_IFELSE([
        AC_MSG_RESULT([yes])
        AC_MSG_NOTICE([compiler options added: $tmp_CFLAGS $tmp_CPPFLAGS])
      ],[
        AC_MSG_RESULT([no])
        AC_MSG_WARN([compiler options rejected: $tmp_CFLAGS $tmp_CPPFLAGS])
        dnl restore initial settings
        CPPFLAGS="$tmp_save_CPPFLAGS"
        CFLAGS="$tmp_save_CFLAGS"
      ])
    fi
    #
  fi
])


dnl CURL_SET_COMPILER_DEBUG_OPTS
dnl -------------------------------------------------
dnl Sets compiler specific options/flags which depend
dnl on configure's debug option.

AC_DEFUN([CURL_SET_COMPILER_DEBUG_OPTS], [
  AC_REQUIRE([CURL_CHECK_OPTION_DEBUG])dnl
  AC_REQUIRE([CURL_CHECK_COMPILER])dnl
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  #
  if test "$compiler_id" != "unknown"; then
    #
    tmp_save_CFLAGS="$CFLAGS"
    tmp_save_CPPFLAGS="$CPPFLAGS"
    #
    tmp_options=""
    tmp_CFLAGS="$CFLAGS"
    tmp_CPPFLAGS="$CPPFLAGS"
    CURL_VAR_STRIP([tmp_CFLAGS],[$flags_dbg_all])
    CURL_VAR_STRIP([tmp_CPPFLAGS],[$flags_dbg_all])
    #
    if test "$want_debug" = "yes"; then
      AC_MSG_CHECKING([if compiler accepts debug enabling options])
      tmp_options="$flags_dbg_yes"
    fi
    if test "$want_debug" = "no"; then
      AC_MSG_CHECKING([if compiler accepts debug disabling options])
      tmp_options="$flags_dbg_off"
    fi
    #
    CPPFLAGS="$tmp_CPPFLAGS"
    CFLAGS="$tmp_CFLAGS $tmp_options"
    squeeze CPPFLAGS
    squeeze CFLAGS
    CURL_COMPILER_WORKS_IFELSE([
      AC_MSG_RESULT([yes])
      AC_MSG_NOTICE([compiler options added: $tmp_options])
    ],[
      AC_MSG_RESULT([no])
      AC_MSG_WARN([compiler options rejected: $tmp_options])
      dnl restore initial settings
      CPPFLAGS="$tmp_save_CPPFLAGS"
      CFLAGS="$tmp_save_CFLAGS"
    ])
    #
  fi
])


dnl CURL_SET_COMPILER_OPTIMIZE_OPTS
dnl -------------------------------------------------
dnl Sets compiler specific options/flags which depend
dnl on configure's optimize option.

AC_DEFUN([CURL_SET_COMPILER_OPTIMIZE_OPTS], [
  AC_REQUIRE([CURL_CHECK_OPTION_OPTIMIZE])dnl
  AC_REQUIRE([CURL_CHECK_COMPILER])dnl
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  #
  if test "$compiler_id" != "unknown"; then
    #
    tmp_save_CFLAGS="$CFLAGS"
    tmp_save_CPPFLAGS="$CPPFLAGS"
    #
    tmp_options=""
    tmp_CFLAGS="$CFLAGS"
    tmp_CPPFLAGS="$CPPFLAGS"
    honor_optimize_option="yes"
    #
    dnl If optimization request setting has not been explicitly specified,
    dnl it has been derived from the debug setting and initially assumed.
    dnl This initially assumed optimizer setting will finally be ignored
    dnl if CFLAGS or CPPFLAGS already hold optimizer flags. This implies
    dnl that an initially assumed optimizer setting might not be honored.
    #
    if test "$want_optimize" = "assume_no" ||
       test "$want_optimize" = "assume_yes"; then
      AC_MSG_CHECKING([if compiler optimizer assumed setting might be used])
      CURL_VAR_MATCH_IFELSE([tmp_CFLAGS],[$flags_opt_all],[
        honor_optimize_option="no"
      ])
      CURL_VAR_MATCH_IFELSE([tmp_CPPFLAGS],[$flags_opt_all],[
        honor_optimize_option="no"
      ])
      AC_MSG_RESULT([$honor_optimize_option])
      if test "$honor_optimize_option" = "yes"; then
        if test "$want_optimize" = "assume_yes"; then
          want_optimize="yes"
        fi
        if test "$want_optimize" = "assume_no"; then
          want_optimize="no"
        fi
      fi
    fi
    #
    if test "$honor_optimize_option" = "yes"; then
      CURL_VAR_STRIP([tmp_CFLAGS],[$flags_opt_all])
      CURL_VAR_STRIP([tmp_CPPFLAGS],[$flags_opt_all])
      if test "$want_optimize" = "yes"; then
        AC_MSG_CHECKING([if compiler accepts optimizer enabling options])
        tmp_options="$flags_opt_yes"
      fi
      if test "$want_optimize" = "no"; then
        AC_MSG_CHECKING([if compiler accepts optimizer disabling options])
        tmp_options="$flags_opt_off"
      fi
      CPPFLAGS="$tmp_CPPFLAGS"
      CFLAGS="$tmp_CFLAGS $tmp_options"
      squeeze CPPFLAGS
      squeeze CFLAGS
      CURL_COMPILER_WORKS_IFELSE([
        AC_MSG_RESULT([yes])
        AC_MSG_NOTICE([compiler options added: $tmp_options])
      ],[
        AC_MSG_RESULT([no])
        AC_MSG_WARN([compiler options rejected: $tmp_options])
        dnl restore initial settings
        CPPFLAGS="$tmp_save_CPPFLAGS"
        CFLAGS="$tmp_save_CFLAGS"
      ])
    fi
    #
  fi
])


dnl CURL_SET_COMPILER_WARNING_OPTS
dnl -------------------------------------------------
dnl Sets compiler options/flags which depend on
dnl configure's warnings given option.

AC_DEFUN([CURL_SET_COMPILER_WARNING_OPTS], [
  AC_REQUIRE([CURL_CHECK_OPTION_WARNINGS])dnl
  AC_REQUIRE([CURL_CHECK_COMPILER])dnl
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  #
  if test "$compiler_id" != "unknown"; then
    #
    tmp_save_CPPFLAGS="$CPPFLAGS"
    tmp_save_CFLAGS="$CFLAGS"
    tmp_CPPFLAGS=""
    tmp_CFLAGS=""
    #
    case "$compiler_id" in
        #
      DEC_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Select a higher warning level than default level2
          tmp_CFLAGS="$tmp_CFLAGS -msg_enable level3"
        fi
        ;;
        #
      GNU_C)
        #
        if test "$want_warnings" = "yes"; then
          #
          dnl Do not enable -pedantic when cross-compiling with a gcc older
          dnl than 3.0, to avoid warnings from third party system headers.
          if test "x$cross_compiling" != "xyes" ||
            test "$compiler_num" -ge "300"; then
            tmp_CFLAGS="$tmp_CFLAGS -pedantic"
          fi
          #
          dnl Set of options we believe *ALL* gcc versions support:
          tmp_CFLAGS="$tmp_CFLAGS -Wall -W"
          #
          dnl Only gcc 1.4 or later
          if test "$compiler_num" -ge "104"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wpointer-arith -Wwrite-strings"
            dnl If not cross-compiling with a gcc older than 3.0
            if test "x$cross_compiling" != "xyes" ||
              test "$compiler_num" -ge "300"; then
              tmp_CFLAGS="$tmp_CFLAGS -Wunused -Wshadow"
            fi
          fi
          #
          dnl Only gcc 2.7 or later
          if test "$compiler_num" -ge "207"; then
            tmp_CFLAGS="$tmp_CFLAGS -Winline -Wnested-externs"
            dnl If not cross-compiling with a gcc older than 3.0
            if test "x$cross_compiling" != "xyes" ||
              test "$compiler_num" -ge "300"; then
              tmp_CFLAGS="$tmp_CFLAGS -Wmissing-declarations"
              tmp_CFLAGS="$tmp_CFLAGS -Wmissing-prototypes"
            fi
          fi
          #
          dnl Only gcc 2.95 or later
          if test "$compiler_num" -ge "295"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wno-long-long"
          fi
          #
          dnl Only gcc 2.96 or later
          if test "$compiler_num" -ge "296"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wfloat-equal"
            tmp_CFLAGS="$tmp_CFLAGS -Wno-multichar -Wsign-compare"
            dnl -Wundef used only if gcc is 2.96 or later since we get
            dnl lots of "`_POSIX_C_SOURCE' is not defined" in system
            dnl headers with gcc 2.95.4 on FreeBSD 4.9
            tmp_CFLAGS="$tmp_CFLAGS -Wundef"
          fi
          #
          dnl Only gcc 2.97 or later
          if test "$compiler_num" -ge "297"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wno-format-nonliteral"
          fi
          #
          dnl Only gcc 3.0 or later
          if test "$compiler_num" -ge "300"; then
            dnl -Wunreachable-code seems totally unreliable on my gcc 3.3.2 on
            dnl on i686-Linux as it gives us heaps with false positives.
            dnl Also, on gcc 4.0.X it is totally unbearable and complains all
            dnl over making it unusable for generic purposes. Let's not use it.
            tmp_CFLAGS="$tmp_CFLAGS"
          fi
          #
          dnl Only gcc 3.3 or later
          if test "$compiler_num" -ge "303"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wendif-labels -Wstrict-prototypes"
          fi
          #
          dnl Only gcc 3.4 or later
          if test "$compiler_num" -ge "304"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wdeclaration-after-statement"
          fi
          #
          dnl Only gcc 4.3 or later
          if test "$compiler_num" -ge "403"; then
            tmp_CFLAGS="$tmp_CFLAGS -Wtype-limits -Wold-style-declaration"
            tmp_CFLAGS="$tmp_CFLAGS -Wmissing-parameter-type -Wempty-body"
            tmp_CFLAGS="$tmp_CFLAGS -Wclobbered -Wignored-qualifiers"
          fi
          #
        fi
        #
        dnl Do not issue warnings for code in system include paths.
        if test "$compiler_num" -ge "300"; then
          tmp_CFLAGS="$tmp_CFLAGS -Wno-system-headers"
        else
          dnl When cross-compiling with a gcc older than 3.0, disable
          dnl some warnings triggered on third party system headers.
          if test "x$cross_compiling" = "xyes"; then
            if test "$compiler_num" -ge "104"; then
              dnl gcc 1.4 or later
              tmp_CFLAGS="$tmp_CFLAGS -Wno-unused -Wno-shadow"
            fi
            if test "$compiler_num" -ge "207"; then
              dnl gcc 2.7 or later
              tmp_CFLAGS="$tmp_CFLAGS -Wno-missing-declarations"
              tmp_CFLAGS="$tmp_CFLAGS -Wno-missing-prototypes"
            fi
          fi
        fi
        ;;
        #
      HP_UX_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Issue all warnings
          dnl tmp_CFLAGS="$tmp_CFLAGS +w1"
          dnl Due to the HP-UX socklen_t issue it is insane to use the +w1
          dnl warning level. Until the issue is somehow fixed we will just
          dnl use the +w2 warning level.
          tmp_CFLAGS="$tmp_CFLAGS +w2"
        fi
        ;;
        #
      IBM_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      INTEL_UNIX_C)
        #
        if test "$want_warnings" = "yes"; then
          if test "$compiler_num" -gt "600"; then
            dnl Show errors, warnings, and remarks
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wall -w2"
            dnl Perform extra compile-time code checking
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wcheck"
            dnl Warn on nested comments
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wcomment"
            dnl Show warnings relative to deprecated features
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wdeprecated"
            dnl Enable warnings for missing prototypes
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wmissing-prototypes"
            dnl Enable warnings for 64-bit portability issues
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wp64"
            dnl Enable warnings for questionable pointer arithmetic
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wpointer-arith"
            dnl Check for function return typw issues
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wreturn-type"
            dnl Warn on variable declarations hiding a previous one
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wshadow"
            dnl Warn when a variable is used before initialized
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wuninitialized"
            dnl Warn if a declared function is not used
            tmp_CPPFLAGS="$tmp_CPPFLAGS -Wunused-function"
          fi
        fi
        dnl Disable using EBP register in optimizations
        tmp_CFLAGS="$tmp_CFLAGS -fno-omit-frame-pointer"
        dnl Disable use of ANSI C aliasing rules in optimizations
        tmp_CFLAGS="$tmp_CFLAGS -no-ansi-alias"
        dnl Value-safe optimizations on floating-point data
        tmp_CFLAGS="$tmp_CFLAGS -fp-model precise"
        dnl Only icc 10.0 or later
        if test "$compiler_num" -ge "1000"; then
          dnl Disable vectorizer diagnostic information
          tmp_CFLAGS="$tmp_CFLAGS -vec-report0"
        fi
        dnl Disable some optimizations to debug icc 9.1 SIGSEGV
        if test "$INTEL_UNIX_C_OPT_SIGSEGV" = "yes"; then
          dnl Disable interprocedural optimizations
          tmp_CFLAGS="$tmp_CFLAGS -no-ip -no-ipo"
          dnl Separate functions for the linker
          tmp_CFLAGS="$tmp_CFLAGS -ffunction-sections"
          dnl Disable inlining of user-defined functions
          tmp_CFLAGS="$tmp_CFLAGS -Ob0"
          dnl Disable inline expansion of intrinsic functions
          tmp_CFLAGS="$tmp_CFLAGS -fno-builtin"
          dnl Disable inlining of functions
          tmp_CFLAGS="$tmp_CFLAGS -fno-inline"
          dnl Disable some IPO for single file optimizations
          tmp_CFLAGS="$tmp_CFLAGS -fno-inline-functions"
          dnl Disable inlining of standard library functions
          tmp_CFLAGS="$tmp_CFLAGS -nolib-inline"
          dnl Disable full and partial inlining when IPO
          tmp_CFLAGS="$tmp_CFLAGS -ip-no-inlining"
          dnl Enable floating-point stack integrity checks
          tmp_CFLAGS="$tmp_CFLAGS -fpstkchk"
          dnl Enable run-time detection of buffer overruns.
          tmp_CFLAGS="$tmp_CFLAGS -fstack-security-check"
          dnl Assume aliasing in the program.
          tmp_CFLAGS="$tmp_CFLAGS -falias"
          dnl Assume that arguments may be aliased.
          tmp_CFLAGS="$tmp_CFLAGS -alias-args"
          dnl Assume aliasing within functions
          tmp_CFLAGS="$tmp_CFLAGS -ffnalias"
          dnl Disable prefetch insertion optimization
          tmp_CFLAGS="$tmp_CFLAGS -no-prefetch"
          dnl Disable loop unrolling optimization
          tmp_CFLAGS="$tmp_CFLAGS -unroll0"
        fi
        ;;
        #
      INTEL_WINDOWS_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
      LCC)
        #
        if test "$want_warnings" = "yes"; then
          dnl Highest warning level is double -A, next is single -A.
          dnl Due to the big number of warnings these trigger on third
          dnl party header files it is impractical for us to use any of
          dnl them here. If you want them simply define it in CPPFLAGS.
          tmp_CFLAGS="$tmp_CFLAGS"
        fi
        ;;
        #
      SGI_MIPS_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Perform stricter semantic and lint-like checks
          tmp_CFLAGS="$tmp_CFLAGS -fullwarn"
        fi
        ;;
        #
      SGI_MIPSPRO_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Perform stricter semantic and lint-like checks
          tmp_CFLAGS="$tmp_CFLAGS -fullwarn"
          dnl Disable some remarks
          dnl #1209: controlling expression is constant
          tmp_CFLAGS="$tmp_CFLAGS -woff 1209"
        fi
        ;;
        #
      SUNPRO_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Perform stricter semantic and lint-like checks
          tmp_CFLAGS="$tmp_CFLAGS -v"
        fi
        ;;
        #
      TINY_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Activate all warnings
          tmp_CFLAGS="$tmp_CFLAGS -Wall"
          dnl Make string constants be of type const char *
          tmp_CFLAGS="$tmp_CFLAGS -Wwrite-strings"
          dnl Warn use of unsupported GCC features ignored by TCC
          tmp_CFLAGS="$tmp_CFLAGS -Wunsupported"
        fi
        ;;
        #
      WATCOM_UNIX_C)
        #
        if test "$want_warnings" = "yes"; then
          dnl Issue all warnings
          tmp_CFLAGS="$tmp_CFLAGS -Wall -Wextra"
        fi
        ;;
        #
      WATCOM_WINDOWS_C)
        #
        dnl Placeholder
        tmp_CFLAGS="$tmp_CFLAGS"
        ;;
        #
    esac
    #
    squeeze tmp_CPPFLAGS
    squeeze tmp_CFLAGS
    #
    if test ! -z "$tmp_CFLAGS" || test ! -z "$tmp_CPPFLAGS"; then
      AC_MSG_CHECKING([if compiler accepts strict warning options])
      CPPFLAGS="$tmp_save_CPPFLAGS $tmp_CPPFLAGS"
      CFLAGS="$tmp_save_CFLAGS $tmp_CFLAGS"
      squeeze CPPFLAGS
      squeeze CFLAGS
      CURL_COMPILER_WORKS_IFELSE([
        AC_MSG_RESULT([yes])
        AC_MSG_NOTICE([compiler options added: $tmp_CFLAGS $tmp_CPPFLAGS])
      ],[
        AC_MSG_RESULT([no])
        AC_MSG_WARN([compiler options rejected: $tmp_CFLAGS $tmp_CPPFLAGS])
        dnl restore initial settings
        CPPFLAGS="$tmp_save_CPPFLAGS"
        CFLAGS="$tmp_save_CFLAGS"
      ])
    fi
    #
  fi
])


dnl CURL_SHFUNC_SQUEEZE
dnl -------------------------------------------------
dnl Declares a shell function squeeze() which removes
dnl redundant whitespace out of a shell variable.

AC_DEFUN([CURL_SHFUNC_SQUEEZE], [
squeeze() {
  _sqz_result=""
  eval _sqz_input=\[$][$]1
  for _sqz_token in $_sqz_input; do
    if test -z "$_sqz_result"; then
      _sqz_result="$_sqz_token"
    else
      _sqz_result="$_sqz_result $_sqz_token"
    fi
  done
  eval [$]1=\$_sqz_result
  return 0
}
])


dnl CURL_PROCESS_DEBUG_BUILD_OPTS
dnl -------------------------------------------------
dnl Settings which depend on configure's debug given
dnl option, and further configure the build process.
dnl Don't use this macro for compiler dependant stuff.

AC_DEFUN([CURL_PROCESS_DEBUG_BUILD_OPTS], [
  AC_REQUIRE([CURL_CHECK_OPTION_DEBUG])dnl
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  AC_BEFORE([$0],[AC_PROG_LIBTOOL])dnl
  #
  if test "$want_debug" = "yes"; then
    CPPFLAGS="$CPPFLAGS -DCURLDEBUG"
    squeeze CPPFLAGS
  fi
  #
])


dnl CURL_CHECK_PROG_CC
dnl -------------------------------------------------
dnl Check for compiler program, preventing CFLAGS and
dnl CPPFLAGS from being unexpectedly changed.

AC_DEFUN([CURL_CHECK_PROG_CC], [
  ac_save_CFLAGS="$CFLAGS"
  ac_save_CPPFLAGS="$CPPFLAGS"
  AC_PROG_CC
  CFLAGS="$ac_save_CFLAGS"
  CPPFLAGS="$ac_save_CPPFLAGS"
])


dnl CURL_VAR_MATCH (VARNAME, VALUE)
dnl -------------------------------------------------
dnl Verifies if shell variable VARNAME contains VALUE.
dnl Contents of variable VARNAME and VALUE are handled
dnl as whitespace separated lists of words. If at least
dnl one word of VALUE is present in VARNAME the match
dnl is considered positive, otherwise false.

AC_DEFUN([CURL_VAR_MATCH], [
  ac_var_match_word="no"
  for word1 in $[$1]; do
    for word2 in [$2]; do
      if test "$word1" = "$word2"; then
        ac_var_match_word="yes"
      fi
    done
  done
])


dnl CURL_VAR_MATCH_IFELSE (VARNAME, VALUE,
dnl                        [ACTION-IF-MATCH], [ACTION-IF-NOT-MATCH])
dnl -------------------------------------------------
dnl This performs a CURL_VAR_MATCH check and executes
dnl first branch if the match is positive, otherwise
dnl the second branch is executed.

AC_DEFUN([CURL_VAR_MATCH_IFELSE], [
  CURL_VAR_MATCH([$1],[$2])
  if test "$ac_var_match_word" = "yes"; then
  ifelse($3,,:,[$3])
  ifelse($4,,,[else
    $4])
  fi
])


dnl CURL_VAR_STRIP (VARNAME, VALUE)
dnl -------------------------------------------------
dnl Contents of variable VARNAME and VALUE are handled
dnl as whitespace separated lists of words. Each word
dnl from VALUE is removed from VARNAME when present.

AC_DEFUN([CURL_VAR_STRIP], [
  AC_REQUIRE([CURL_SHFUNC_SQUEEZE])dnl
  ac_var_stripped=""
  for word1 in $[$1]; do
    ac_var_strip_word="no"
    for word2 in [$2]; do
      if test "$word1" = "$word2"; then
        ac_var_strip_word="yes"
      fi
    done
    if test "$ac_var_strip_word" = "no"; then
      ac_var_stripped="$ac_var_stripped $word1"
    fi
  done
  dnl squeeze whitespace out of result
  [$1]="$ac_var_stripped"
  squeeze [$1]
])

