dnl                   C O N F I G U R E . A C
dnl BZFlag
dnl Copyright (c) 1993 - 2006 Tim Riker
dnl
dnl This package is free software;  you can redistribute it and/or
dnl modify it under the terms of the license found in the file
dnl named COPYING that should have accompanied this file.
dnl
dnl THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
dnl IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
dnl

AC_INIT
AC_CONFIG_SRCDIR(src/bzflag/bzflag.cxx)
AC_REVISION($Revision $)

AC_CONFIG_AUX_DIR(misc)

# sed magic to get version numbers from src/date/buildDate.cxx
MAJOR_VERSION=`sed -e 's/#.*define.*BZ_MAJOR_VERSION[^0-9]*\(.*\)/\1/' -e t -e d < \$srcdir/src/date/buildDate.cxx`
MINOR_VERSION=`sed -e 's/#.*define.*BZ_MINOR_VERSION[^0-9]*\(.*\)/\1/' -e t -e d < \$srcdir/src/date/buildDate.cxx`
REV=`sed -e 's/#.*define.*BZ_REV[^0-9]*\(.*\)/\1/' -e t -e d < \$srcdir/src/date/buildDate.cxx`

CONF_DATE=`date -u +%Y%m%d`
BZFLAG_DATE=`date -u +%Y-%m-%d`
BZFLAG_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$REV"
echo "BZFlag-$BZFLAG_VERSION.$CONF_DATE"
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE(bzflag, $BZFLAG_VERSION)
AM_CONFIG_HEADER(include/config.h)

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(BZFLAG_VERSION)
AC_SUBST(BZFLAG_DATE)

# Let the C++ code know about OS and user
AC_DEFINE_UNQUOTED(BZ_BUILD_OS, "$host_os", [BZFlag System Environment])
AC_CHECK_PROG(HASWHOAMI, whoami, yes, no)
if test $HASWHOAMI = yes; then
  AC_DEFINE_UNQUOTED(BZ_BUILD_USER, "`whoami`", [User who compiled BZFlag])
else
  AC_DEFINE_UNQUOTED(BZ_BUILD_USER, "anonymous", [User who compiled BZFlag])
fi

# check for kerberos option
AC_ARG_WITH(kerberos, [  --with-kerberos	  with Kerberos support])

# check for regex option (on means build our own)
AC_ARG_WITH(regex, [  --with-regex		  Use bzflag regex lib])
if test x$with_regex != xno ; then
   AC_DEFINE(BUILD_REGEX, 1, [Building regex])
fi
AM_CONDITIONAL(BUILD_REGEX, test "x$with_regex" != "xno")

# check for SDL option (enabled by default now)
AC_ARG_WITH(SDL, [  --without-SDL		  do not use Simple DirectMedia Layer])
if test x$with_SDL != xno; then
  with_SDL=yes
fi

dnl FIXME: need to move the rest of the ARGs up here too


# cannot override LD directly, so warn (configure will override it)
if test "x$LD" != "x" ; then
	AC_MSG_WARN([LD cannot be set directly yet it seems to be set ($LD)])
fi

#Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX

AC_ARG_ENABLE(ccachetest, [  --disable-ccachetest    don't try to use ccache])
if test x$enable_ccachetest != xno; then
  AC_PATH_PROG(CCACHE, ccache)
fi
if test "$CCACHE"
then
  CC="$CCACHE $CC"
  CXX="$CCACHE $CXX"
fi

AC_CHECK_TOOL(WINDRES, windres, :)

dnl On SGI systems, figure out if we are using MIPSPro compiler
using_mipspro=no
if test "$build_vendor" = "sgi" ; then
    AC_MSG_CHECKING([whether we are using the MIPSPro compiler])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],
	[[#if defined(sgi) && !defined(__GNUC__) && defined(_COMPILER_VERSION)
this line should cause a compile failure when building with the MIPSpro compiler;
#endif
int main() { return 0; }
    ]])],[using_mipspro=no],[using_mipspro=yes])
    AC_MSG_RESULT([$using_mipspro])
fi

AC_PROG_LN_S
AC_PROG_RANLIB

# disabled shared libraries in order to prevent double-compilation
# FIXME: necessary until the flags are used properly.
AC_DISABLE_SHARED

if test x$enable_shared != xno ; then
   AC_DEFINE(_USE_BZ_API, 1, [Enable plugins])
fi
AM_CONDITIONAL(BUILD_PLUGINS, test "x$enable_shared" != "xno")

# libtool shouldn't be generated until after LD is set.
# AC_PROG_LIBTOOL goes poof in libtool 1.9, but that's not very
# widespread at all.
# LT_INIT
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)


AC_CHECK_PROG(HASPOD2MAN, pod2man, yes, no)
if test $HASPOD2MAN = no; then
  AC_MSG_WARN([some man pages will not be generated])
fi
AM_CONDITIONAL(HASPOD2MAN, test $HASPOD2MAN = yes)
AC_SUBST(AR)
AC_CHECK_PROG(AR, ar, ar)
AC_SUBST(AR_FLAGS)

AC_CHECK_LIB(z, compress, BUILDZLIB=no, BUILDZLIB=yes)
AM_CONDITIONAL(BUILDZLIB, test $BUILDZLIB = yes)

  #checking for libcurl library
  MY_CURL
  if test $my_cv_curl_vers != NONE; then
    CURL_CFLAGS=`curl-config --cflags`
    CURL_LIBS=`curl-config --libs`
    if test "x$build_vendor" = "xapple" ; then
      if test -f /usr/bin/libcurl.a ; then
        # prefer to link static
        CURL_LIBS=/usr/bin/libcurl.a
      else
        # mac os x curl-config outputs erroneous -arch i386 in libs
        CURL_LIBS="`echo $CURL_LIBS | sed 's/-arch i386//'`"
      fi
    fi
  else
    AC_MSG_ERROR([libcurl is required. You must install it. \
See http://curl.haxx.se/])
  fi
  AC_SUBST(CURL_CFLAGS)

LIBS="$CURL_LIBS $LIBS"

# Testing if kerberos 5 is in the system
KRB5_CONFIG=no
if test "x$with_kerberos" = "xyes" ; then
  AC_PATH_PROG(KRB5_CONFIG, krb5-config, no)
  if test "x$KRB5_CONFIG" != "xno" ; then
    KRB5_CFLAGS=`$KRB5_CONFIG --cflags`
    KRB5_LIBS=`$KRB5_CONFIG --libs`
  fi
fi
AC_SUBST(KRB5_CFLAGS)
AC_SUBST(KRB5_LIBS)


# Just for kicks
AC_MSG_CHECKING([for artificial intelligence for bz's robots])
AC_MSG_RESULT([not found])

#Checks for headers
AC_HEADER_STDC
AC_LANG(C++)
AC_CHECK_HEADERS( \
	cstdlib \
	cstdio \
	cstring \
)
if test "x$with_kerberos" = "xyes" ; then
  AC_CHECK_HEADERS(\
	krb5.h\
	Kerberos/Kerberos.h \
  )
fi
AC_LANG(C)
AC_CHECK_HEADERS( \
	GL/glx.h \
	SDL/SDL.h \
	X11/Xlib.h \
	bstring.h \
	arpa/nameser_compat.h \
	linux/input.h \
	stdint.h \
	sys/param.h \
	unistd.h \
	values.h \
	dsound.h \
)
# nameser_compat.h used by src/c-ares

#Checks for types
#AM_CHECK_TYPES(socklen_t)
#AC_CHECK_TYPES(socklen_t)
AC_C_BIGENDIAN
AC_C_CONST

AC_MSG_CHECKING([for socklen_t type])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
		#include <sys/socket.h>]],
 [[socklen_t len = 42; return 0;]])],
 [ac_cv_type_socklen_t=yes],[ac_cv_type_socklen_t=no])
if test $ac_cv_type_socklen_t != yes; then
    AC_DEFINE(socklen_t, int, [if socklen_t is not defined, provide something useful])
else
    AC_DEFINE(HAVE_SOCKLEN_T, 1, [if socklen_t is defined, make note of it])
fi
AC_MSG_RESULT([$ac_cv_type_socklen_t])

# Check for working FF_RUMBLE support in linux/input.h
AC_MSG_CHECKING([for linux rumble joystick support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <linux/input.h>]],
	[[struct ff_effect x; x.u.rumble.weak_magnitude = 42; return 0;]])],
	[ac_ff_effect_rumble=yes],[ac_ff_effect_rumble=no])
if test x$ac_ff_effect_rumble = xyes; then
    AC_DEFINE(HAVE_FF_EFFECT_RUMBLE, 1, [we appear to have working support for rumble force feedback effects])
fi
AC_MSG_RESULT([$ac_ff_effect_rumble])

# Check for working directional FF support in linux/input.h
AC_MSG_CHECKING([for linux force feedback joystick support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <linux/input.h>]],
	[[struct ff_effect x; x.direction = 0x4000; return 0;]])],
	[ac_ff_effect_directional=yes],[ac_ff_effect_directional=no])
if test x$ac_ff_effect_directional = xyes; then
    AC_DEFINE(HAVE_FF_EFFECT_DIRECTIONAL, 1, [we appear to have working support for directional force feedback effects])
fi
AC_MSG_RESULT([$ac_ff_effect_directional]) 

case $host_os in
     linux*|kfreebsd*|freebsd*|netbsd*|dragonfly*) linux=true;;
     *) linux=false;;
esac
AM_CONDITIONAL(LINUX, $linux)
AM_CONDITIONAL(BEOS, test x$host_os = xbeos)
AM_CONDITIONAL(PC,    test x$host_vendor = xpc)
AM_CONDITIONAL(PPC,   test x$host_vendor = xppc)

case $host_os in
       solaris*) solaris=true;;
       *) solaris=false;;
     esac
AM_CONDITIONAL(SOLARIS, $solaris)
# Write code to detect platform specialization
sun_wspro=false
sun_cc=false
sun_gcc=false

case $host_os in
     irix*) irix=true;;
     *) irix=false;;
esac
AM_CONDITIONAL(IRIX, $irix)
AM_CONDITIONAL(BUGGY_MIPSPRO, test "x$using_mipspro" = "xyes")

case $host_os in
     cygwin|mingw32*|windows**) win32=true;;
     *) win32=false;;
esac
AM_CONDITIONAL(WIN32, $win32)

case $host_os in
   macos|darwin*) macos=true;;
   *) macos=false;;
esac

AM_CONDITIONAL(MACOS, $macos)

#CONFIG is only used for irix packaging
case $host_os in
   irix*)
	CONFIG=irix-mips3;
        AC_SUBST(CONFIG)
        ;;
esac

AC_PATH_XTRA
CPPFLAGS="$CPPFLAGS $X_CFLAGS" 
LDFLAGS="$LDFLAGS $X_LIBS"

case $host_os in
  solaris*)
    GLIBS="-lresolv -laudio $GLIBS"
    LIBS="$LIBS -lsocket -lnsl -lresolv"
    LDFLAGS="$LDFLAGS -L/usr/demo/SOUND/lib"
    ;;
  irix*)
    GLIBS="-lXsgivc -lX11 -laudio $GLIBS"
    ;;
  beos)
    GLIBS=" -lmedia -lgame $GLIBS"
    LIBS="-lbe"
    ;;
  macos|darwin*)
    ;;
  *)
    ;;
esac;

# Check for the math lib
AC_CHECK_LIB(m, sqrtf)

# for BeOS - old network stack don't have those libs ( move it in the case switch ?)
AC_CHECK_LIB([socket], [socket])
AC_CHECK_LIB([socket], [gethostent], [], AC_CHECK_LIB([bind], [gethostent]))

# check for various IEEE and c99 standard math functions in libm
AC_CHECK_LIB([m], [acosf], [AC_DEFINE([HAVE_ACOSF],[1],[libm includes acosf])])
AC_CHECK_LIB([m], [asinf], [AC_DEFINE([HAVE_ASINF],[1],[libm includes asinf])])
AC_CHECK_LIB([m], [atan2f], [AC_DEFINE([HAVE_ATAN2F],[1],[libm includes atan2f])])
AC_CHECK_LIB([m], [atanf], [AC_DEFINE([HAVE_ATANF],[1],[libm includes atanf])])
AC_CHECK_LIB([m], [cosf], [AC_DEFINE([HAVE_COSF],[1],[libm includes cosf])])
AC_CHECK_LIB([m], [expf], [AC_DEFINE([HAVE_EXPF],[1],[libm includes expf])])
AC_CHECK_LIB([m], [fabsf], [AC_DEFINE([HAVE_FABSF],[1],[libm includes fabsf])])
AC_CHECK_LIB([m], [floorf], [AC_DEFINE([HAVE_FLOORF],[1],[libm includes floorf])])
AC_CHECK_LIB([m], [fmodf], [AC_DEFINE([HAVE_FMODF],[1],[libm includes fmodf])])
AC_CHECK_LIB([m], [hypotf], [AC_DEFINE([HAVE_HYPOTF],[1],[libm includes hypotf])])
AC_CHECK_LIB([m], [logf], [AC_DEFINE([HAVE_LOGF],[1],[libm includes logf])])
AC_CHECK_LIB([m], [log10f], [AC_DEFINE([HAVE_LOG10F],[1],[libm includes log10f])])
AC_CHECK_LIB([m], [powf], [AC_DEFINE([HAVE_POWF],[1],[libm includes powf])])
AC_CHECK_LIB([m], [sinf], [AC_DEFINE([HAVE_SINF],[1],[libm includes sinf])])
AC_CHECK_LIB([m], [sqrtf], [AC_DEFINE([HAVE_SQRTF],[1],[libm includes sqrtf])])
AC_CHECK_LIB([m], [tanf], [AC_DEFINE([HAVE_TANF],[1],[libm includes tanf])])

# avoid using X11 in BeOS and Mac OS X
case $host_os in
  macos|darwin*)
    ;;
  beos*)
    ;;
  *)
    if test x$no_x = x; then
      GLIBS="$X_PRE_LIBS -lX11 $EXTRA_LIBS $GLIBS"
    fi

    AC_CHECK_LIB(Xext, XextAddDisplay, [GLIBS="-lXext $GLIBS"], [], $GLIBS)

    if test x$host_os = xlinux-gnu -o x$host_os = xlinux ; then
      AC_CHECK_LIB(Xxf86vm, XF86VidModeGetAllModeLines, [GLIBS="-lXxf86vm $GLIBS"], [], $GLIBS)
      AC_CHECK_LIB(Xi, XListInputDevices, [GLIBS="-lXi $GLIBS"], [], $GLIBS)
    fi
    ;;
esac

AC_LANG(C++)

AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <regex.h>
]], [[
regcomp(0, 0, 0);
regexec(0, 0, 0, 0, 0);
]])],[AC_DEFINE(HAVE_REGEX_H, 1,
		[Define to 1 if you have regex stuff available])],[])

ac_cv_search_glBegin=no
ac_cv_search_gluScaleImage=no
ac_func_search_save_LIBS=$LIBS

LIBS="-framework OpenGL $GLIBS $ac_func_search_save_LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <OpenGL/gl.h>]],
				[[glBegin(GL_POINTS)]])],
	       [ac_cv_search_glBegin="-framework OpenGL"],[])
for ac_lib in opengl32 GL GL2; do
  LIBS="-l$ac_lib $GLIBS $ac_func_search_save_LIBS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <GL/gl.h>]],
				  [[glBegin(GL_POINTS)]])],
		 [ac_cv_search_glBegin="-l$ac_lib"
                  break],[])
done
if test "$ac_cv_search_glBegin" != no; then
  GLIBS="$ac_cv_search_glBegin $GLIBS"
  if test "$ac_cv_search_glBegin" = -lGL2; then
    AC_DEFINE(BEOS_USE_GL2, 1, [Use new GL Kit for BeOS])
  fi

  LIBS="-framework OpenGL $ac_func_search_save_LIBS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <OpenGL/glu.h>]],
			[[gluScaleImage(GL_RED,0,0,GL_INT,0,0,0,GL_INT,0)]])],
		 [ac_cv_search_gluScaleImage="-framework OpenGL"],[])
  for ac_lib in glu32 GL GLU; do
    LIBS="-l$ac_lib $GLIBS $ac_func_search_save_LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <GL/glu.h>]],
			[[gluScaleImage(GL_RED,0,0,GL_INT,0,0,0,GL_INT,0)]])],
		   [ac_cv_search_gluScaleImage="-l$ac_lib"
		    break],[])
  done
  if test "$ac_cv_search_gluScaleImage" != no; then
    GLIBS="$ac_cv_search_gluScaleImage $GLIBS"
  fi
fi

LIBS=$ac_func_search_save_LIBS
AC_SUBST(GLIBS)

# Check for the GLEW library (GL Extension Wrangler)
AC_MSG_CHECKING([for GLEW library])
tmpLIBS=$LIBS
GLEW_LIBS="-lGLEW"
LIBS=$GLEW_LIBS
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <GL/glew.h>]],
                                [[glewInit()]])],
               [glew_check="yes"], [glew_check="no"])
if test $glew_check = "yes"; then
  AC_DEFINE(HAVE_GLEW, 1, [GLEW library is available])
else
  GLEW_LIBS=""
fi
AC_MSG_RESULT([$glew_check])
AC_SUBST(GLEW_LIBS)
LIBS=$tmpLIBS

case $host_os in
  mingw32*)

  AC_CHECK_LIB(ws2_32, closesocket)
  AC_CHECK_LIB(winmm, timeGetTime)

  GLIBS="-mwindows -lgdi32 -ldxguid -ldsound -ldinput $GLIBS"
  # MingW does some funny stuff with DirectX.
  # Test for a working DirectInput.
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
  #define DIRECTINPUT_VERSION 0x0700
  #include <dinput.h>
  ]], [[
  IDirectInput7* directInput;
  HINSTANCE hinst = GetModuleHandle(NULL);
  HRESULT success = DirectInputCreateEx(hinst, DIRECTINPUT_VERSION,
                    IID_IDirectInput7, (void**)&directInput, NULL);
  ]])],[],[AC_DEFINE(BROKEN_DINPUT, 1, [Define to 1 if your DirectInput headers and libraries are broken or mismatched.])])
  ;;
esac

# Remove ogg/vorbis dependencies until we actually need them.
#
# AC_CHECK_LIB(ogg, ogg_stream_init, [ALIBS="-logg $ALIBS"], [], $ALIBS)
# AC_CHECK_LIB(vorbis, vorbis_info_init, [ALIBS="-lvorbis $ALIBS"], [], $ALIBS)
# AC_CHECK_LIB(vorbisfile, ov_open, [ALIBS="-lvorbisfile $ALIBS"], [], $ALIBS)
# AC_SUBST(ALIBS)

case $host_os in
     macos|darwin*)
	if test x$with_SDL = xyes; then
	    LIBS="$LIBS -framework Carbon -framework Cocoa -framework OpenGL"
	else
	    LIBS="$LIBS -framework Carbon -framework AGL"
	fi
	;;
     irix)
	if test x$host_cpu = xmips64; then
	   LDFLAGS="$LDFLAGS -L$(ROOT)/usr/lib32"
	else
	   LDFLAGS="$LDFLAGS -L$(ROOT)/usr/lib"
	fi
	;;
esac
if test $prefix = NONE; then
	prefix=$ac_default_prefix
fi
INSTALL_DATA_DIR=`eval echo $datadir/bzflag`
# newer (read stupid) versions of autoconf need to get evaled twice
INSTALL_DATA_DIR=`eval echo $INSTALL_DATA_DIR`
AC_SUBST(INSTALL_DATA_DIR)
desktopdir=`eval echo $datadir/applications`
AC_SUBST(desktopdir)

AC_ARG_ENABLE(threads, [  --enable-threads        build with thread])
if test x$enable_threads = xyes; then
   AC_DEFINE(USE_THREADS, 1, [Use threads])
   CPPFLAGS="$CPPFLAGS -D_REENTRANT"
   LDFLAGS="$LDFLAGS -lpthread"
fi

AC_CHECK_LIB(cares, ares_init, BUILDCARES=no, BUILDCARES=yes)
AM_CONDITIONAL(BUILDCARES, test $BUILDCARES = yes)
if test "x$BUILDCARES" = "xyes" ; then
      AC_MSG_WARN([using an internal c-ares lib.])
      AC_MSG_WARN([Consider installing an updated library at system level])
      AC_MSG_WARN([link is http://daniel.haxx.se/projects/c-ares/])
fi


MP_WITH_CURSES
# do we want bzadmin?
AC_ARG_ENABLE(bzadmin, [  --disable-bzadmin       do not build text client])
if test x$enable_bzadmin != xno; then
    if test "x$CURSES_LIB" = x; then
      AC_MSG_WARN([could not find a curses library, will build bzadmin without curses])
    fi
    AM_CONDITIONAL(BZADMIN_INCLUDED, true)
else
  AM_CONDITIONAL(BZADMIN_INCLUDED, false)
fi
AM_CONDITIONAL(HAVE_CURSES, test "x$CURSES_LIB" != x)
AC_SUBST(CURSES_LIB)


AC_ARG_ENABLE(timebomb, [  --enable-timebomb       build exp Date])
if test x$enable_timebomb = xyes; then
   AC_DEFINE(TIME_BOMB, "3/13/1998", [Time Bomb expiration])
fi

savedLIBS=$LIBS
LIBS="$GLIBS $LIBS"
AC_LANG(C)
if test x$with_SDL = xyes; then

  AC_MSG_CHECKING([for SDL framework])
  savedLDFLAGS="$LDFLAGS"
  LDFLAGS="$LDFLAGS -framework SDL"
  framework_SDL_flag_works=yes
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <SDL/SDL_version.h>]], [[]])],
		 [],[framework_SDL_flag_works=no ])
  AC_MSG_RESULT([$framework_SDL_flag_works])
  if test x$framework_SDL_flag_works = xno ; then
    LDFLAGS="$savedLDFLAGS"
    with_SDL=no
  fi

  if test x$with_SDL = xno ; then
    AM_PATH_SDL(1.2.5, with_SDL=yes, with_SDL=no)
    if test x$with_SDL = xno; then
      AC_MSG_WARN([could not find an appropriate SDL version. BZflag will build without SDL])
    fi
  fi
fi
AC_LANG(C++)
LIBS=$savedLIBS
if test x$with_SDL = xyes; then
  AC_DEFINE(HAVE_SDL, 1, [Using SDL])
fi
AM_CONDITIONAL(HAVE_SDL, test x$with_SDL = xyes)

# Check for the SDL_image library
AC_MSG_CHECKING([for SDL_image library])
tmpLIBS=$LIBS
SDL_IMAGE_LIBS="-lSDL_image"
LIBS=$SDL_IMAGE_LIBS
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <SDL/SDL_image.h>]],
                                [[IMG_Load(NULL)]])],
               [sdl_image_check="yes"], [sdl_image_check="no"])
if test $sdl_image_check = "yes"; then
  AC_DEFINE(HAVE_SDL_IMAGE, 1, [SDL_image library is available])
else
  SDL_IMAGE_LIBS=""
fi
AC_MSG_RESULT([$sdl_image_check])
AC_SUBST(SDL_IMAGE_LIBS)
LIBS=$tmpLIBS

AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging])
AM_CONDITIONAL(DEBUG, test x$enable_debug = xyes)
if test x$enable_debug = xyes; then
   AC_DEFINE(DEBUG_RENDERING, 1, [Debug Rendering])
   AC_ARG_ENABLE(profiling, [  --enable-profiling      turn on profiling])
fi

AC_ARG_ENABLE(client, [  --disable-client        servers-only build])
if test x$enable_client != xno; then
  if test "$ac_cv_search_gluScaleImage" = no; then
    AC_MSG_WARN(
    [Client build has been requested, but GL is not fully available (missing gl.h)
     ... disabling client generation])
    AM_CONDITIONAL(CLIENT_INCLUDED, false)
    enable_client=no
  else
    AM_CONDITIONAL(CLIENT_INCLUDED, true)
  fi
else
  AM_CONDITIONAL(CLIENT_INCLUDED, false)
fi

AC_ARG_ENABLE(robots, [  --disable-robots        disable robots])
if test x$enable_robots != xno; then
  AC_DEFINE(ROBOT, 1, [Enabling Robots])
fi

AC_ARG_ENABLE(snapping, [  --disable-snapping      disable snapping])
if test x$enable_snapping != xno; then
  AC_DEFINE(SNAPPING, 1, [Enabling Snapping])
fi

AC_DEFINE_UNQUOTED(INSTALL_DATA_DIR, "$INSTALL_DATA_DIR",
[Data file directory])
case $host_os in
     linux*)
	AC_DEFINE(HALF_RATE_AUDIO, 1, [Half rate Audio])
	AC_DEFINE(XF86VIDMODE_EXT, 1, [XFree86 Video Mode Extension])
	# FIXME see XIJoystick.cxx
	#if test x$no_sdl = xyes; then
	#AC_DEFINE(XIJOYSTICK, 1, [XInput Joystick Handling])
	#fi
	#CPPFLAGS="$CPPFLAGS -Wall -W -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings -Wconversion -Wredundant-decls"
	CPPFLAGS="$CPPFLAGS -Wall -W"
	case $host_vendor in
	     pc)
	       if expr `$CC -dumpversion` \>= "3.4" > /dev/null ; then
	         # silly hack, should be a proper configure compiler option test
	         if test x$host_cpu = xx86_64; then
		   CPPFLAGS="$CPPFLAGS -mtune=k8";
		 fi
	         CPPFLAGS="$CPPFLAGS -mtune=$host_cpu";
	       else
	         # silly hack, should be a proper configure compiler option test
	         if test x$host_cpu = xx86_64; then
		   CPPFLAGS="$CPPFLAGS -mcpu=k8";
		 fi
	         CPPFLAGS="$CPPFLAGS -mcpu=$host_cpu";
	       fi;;
	     ppc) CPPFLAGS="$CPPFLAGS -mcpu=$host_cpu";;
	esac;;
     cygwin)
	AC_DEFINE(HALF_RATE_AUDIO, 1, [Half rate Audio])
	AC_DEFINE(XF86VIDMODE_EXT, 1, [XFree86 Video Mode Extension])
	#if test x$no_sdl = xyes; then
	#AC_DEFINE(XIJOYSTICK, 1, [XInput Joystick Handling])
	#fi
	CPPFLAGS="$CPPFLAGS -Wall -W -mcpu=$host_cpu";;
     mingw32*)
	WINPORT=windows_port.o
	AC_SUBST(WINPORT)
	AC_DEFINE(HALF_RATE_AUDIO, 1, [Half rate Audio])
	CPPFLAGS="$CPPFLAGS -Wall -W"
	if expr `$CC -dumpversion` \>= "3.4" > /dev/null ; then
	  CPPFLAGS="$CPPFLAGS -mtune=$host_cpu";
	else
	  CPPFLAGS="$CPPFLAGS -mcpu=$host_cpu";
	fi;;
     solaris*)
	AC_DEFINE(ETC_INET, [], [hosts is in /etc/inet/])
	AC_DEFINE(SUN_OGL_NO_VERTEX_MACROS, [], [Sun OpenGL No Macro Vertex])
	if $sun_wspro; then
	   CPPFLAGS="$CPPFLAGS -KPIC"
	elif $sun_cc; then
	   CPPFLAGS="$CPPFLAGS -fast"
	elif $sun_gcc; then
	   CPPFLAGS="$CPPFLAGS -fpermissive"
	fi;;
     macos|darwin*)
	CPPFLAGS="$CPPFLAGS -pipe -pedantic -fno-exceptions -W -Wall -Wundef -Wno-import";
	CFLAGS="$CFLAGS -pipe -pedantic -fno-exceptions -W -Wall -Wundef -Wno-import";;
     irix)
	CPPFLAGS="$CPPFLAGS -Wall";
	if test x$host_cpu = xmips64; then
	   CPPFLAGS="CPPFLAGS -march=mips3";
	else
	   CPPFLAGS="CPPFLAGS -march=mips2";
	fi;;
     beos*)
	AC_DEFINE(HAVE_DEFINED_TOLOWER, 1, [tolower and toupper are not functions])
	;;
esac

# We use buildDate.cxx's compilation date instead
# CXXFLAGS="$CXXFLAGS -DBZ_BUILD_DATE=$CONF_DATE"

if test x$enable_debug = xyes; then
   CFLAGS="$CFLAGS -O0";
   CXXFLAGS="$CXXFLAGS -O0";
   case $host_os in
     linux*|cygwin|mingw32*)
	CPPFLAGS="$CPPFLAGS -Werror -Wshadow";
	CXXFLAGS="$CXXFLAGS -Wno-deprecated -fsigned-char";
	if test x$enable_profiling = xyes; then
	  CFLAGS="$CFLAGS -pg";
	  CXXFLAGS="$CXXFLAGS -pg";
	fi;;
     macos|darwin*)
	if test x$enable_profiling = xyes; then
	  CFLAGS="$CFLAGS -pg";
	  CXXFLAGS="$CXXFLAGS -pg";
	fi;;
     beos)
	CPPFLAGS="$CPPFLAGS -Werror";;
    esac;
    AC_DEFINE(DEBUG, 1, [Debugging enabled])
else
    case $host_os in
	 linux*|cygwin|mingw32*)
		CXXFLAGS="$CXXFLAGS -ffast-math -fno-exceptions -fsigned-char";;
	 solaris*)
		CFLAGS="$CFLAGS -O";
		CXXFLAGS="$CXXFLAGS -O";;
	 macos|darwin*)
		CFLAGS="$CFLAGS -O3";
		CXXFLAGS="$CXXFLAGS -O3";
		CXXFLAGS="$CXXFLAGS -ffast-math -fomit-frame-pointer";
		CXXFLAGS="$CXXFLAGS -fexpensive-optimizations -fno-exceptions";
		if test x$enable_profiling = xyes; then
			CFLAGS="$CFLAGS -pg";
			CXXFLAGS="$CXXFLAGS -pg";
		fi;;
	 irix)
		if test x$host_cpu = xmips64; then
		   CFLAGS="$CFLAGS -O3";
		   CXXFLAGS="$CXXFLAGS -O3";
		   CPPFLAGS="$CPPFLAGS -ffast-math -fomit-frame-pointer";
		   CPPFLAGS="$CPPFLAGS -fexpensive-optimizations -fno-exceptions";
		else
		   CFLAGS="$CFLAGS -O2";
		   CXXFLAGS="$CXXFLAGS -O2";
		fi;;
	 beos)
		CFLAGS="$CFLAGS -O2";
		CXXFLAGS="$CXXFLAGS -O2";
		CXXFLAGS="$CXXFLAGS -ffast-math -fomit-frame-pointer -fno-exceptions";;
     esac;
     AC_DEFINE(NDEBUG, 1, [Debugging disabled])
fi;

dnl check for -search_paths_first linker flag when making dynamic libraries
search_paths_first_flag="-Wl,-search_paths_first -mdynamic-no-pic"
AC_MSG_CHECKING([if the compiler understands $search_paths_first_flag])
search_paths_first_flag_works=yes
PRELDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $search_paths_first_flag"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[],
	       [search_paths_first_flag_works=no])
AC_MSG_RESULT($search_paths_first_flag_works)
if test "x$search_paths_first_flag_works" = "xno" ; then
        LDFLAGS="$PRELDFLAGS"
fi


# Check for functions
AC_CHECK_FUNCS(\
	CGLGetCurrentContext \
	Sleep \
	WaitForSingleObject \
	_stricmp \
	_strnicmp \
	_vsnprintf \
	atexit \
	getrlimit \
	glXGetCurrentContext \
	select \
	snooze \
	usleep \
	vsnprintf \
	wglGetCurrentContext \
)

dnl test for isnan() presence
BZ_FUNC_ISNAN

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <algorithm>
]], [[
int i = std::min(0, 1); i = i;
]])],[AC_DEFINE(HAVE_STD__MIN, 1, [Define to 1 if you have a conforming std::min])],[])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <algorithm>
]], [[
int i = std::max(0, 1); i = i;
]])],[AC_DEFINE(HAVE_STD__MAX, 1, [Define to 1 if you have a conforming std::max])],[])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <algorithm>
]], [[
char array[] = "test";
int i = std::count(array, array + 5, 't'); i = i;
]])],[AC_DEFINE(HAVE_STD__COUNT, 1, [Define to 1 if you have a conforming
std::count, otherwise old version of count template is assumed])],[])


if test "x$with_kerberos" = "xyes" ; then
  have_krb5=no
  AC_MSG_CHECKING([for Kerberos functionality])
  if test "x$KRB5_CONFIG" != "xno" ; then
    have_krb5=yes
  elif test "x$ac_cv_header_krb5_h" = "xyes" ; then
    have_krb5=yes
  elif test "x$ac_cv_header_Kerberos_Kerberos_h" = "xyes" ; then
    have_krb5=yes
  fi
  if test "x$have_krb5" = "xyes" ; then
    AC_DEFINE(HAVE_KRB5, 1, [Using Kerberos 5])
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([Kerberos build support was requested, but Kerberos support was not found])
  fi
fi
AM_CONDITIONAL(HAVE_KRB5, test x$have_krb5 = xyes)

LIBDIR="\$(top_srcdir)/lib"
AC_SUBST(LIBDIR)


AC_CONFIG_FILES(bzflag.info
	BZFlag.xcode/Makefile
	Dev-C++/Makefile
	Makefile
	bzflag.lsm
	bzflag.spec
	data/Makefile
	data/fonts/Makefile
	data/l10n/Makefile
	debian/Makefile
	doc/Makefile
	doc/faq/Makefile
	doc/guide/Commands/Makefile
	doc/guide/Flags/Makefile
	doc/guide/Geometry/Makefile
	doc/guide/Installation/Makefile
	doc/guide/Makefile
	doc/guide/Tactics/Makefile
	doc/howto/Makefile
	doc/man/Makefile
	include/Makefile
	m4/Makefile
	man/Makefile
	man/bzadmin.6
	man/bzflag.6
	man/bzfs.6
	man/bzw.5
	misc/Makefile
	package/Makefile
	package/irix/Makefile
	package/linux/Makefile
	package/rpm/Makefile
	package/win32/Makefile
	package/win32/nsis/Makefile
	plugins/Makefile
	plugins/RogueGenocide/Makefile
	plugins/SAMPLE_PLUGIN/Makefile
	plugins/airspawn/Makefile
	plugins/chathistory/Makefile
	plugins/flagStay/Makefile
	plugins/hiddenAdmin/Makefile
	plugins/torBlock/Makefile
	plugins/keepaway/Makefile
	plugins/killall/Makefile
	plugins/logDetail/Makefile
	plugins/playHistoryTracker/Makefile
	plugins/python/Makefile
	plugins/recordmatch/Makefile
	plugins/shockwaveDeath/Makefile
	plugins/teamflagreset/Makefile
	plugins/timedctf/Makefile
	plugins/vocaliser/Makefile
        plugins/wwzones/Makefile
	plugins/HoldTheFlag/Makefile
	plugins/nagware/Makefile
	plugins/doc/Makefile
	plugins/vc71/Makefile
	src/3D/Makefile
	src/Makefile
	src/bzadmin/Makefile
	src/bzflag/Makefile
	src/bzfs/Makefile
	src/c-ares/Makefile
	src/common/Makefile
	src/date/Makefile
	src/game/Makefile
	src/geometry/Makefile
	src/geometry/models/Makefile
	src/geometry/models/tank/Makefile
	src/mediafile/Makefile
	src/net/Makefile
	src/obstacle/Makefile
	src/ogl/Makefile
	src/platform/MacOSX/BZFlag-Info.plist
	src/platform/MacOSX/Makefile
	src/platform/Makefile
	src/regex/Makefile
	src/scene/Makefile
	src/zlib/Makefile
	tools/Makefile
	tools/TextTool-W32/Makefile
	tools/TextTool-W32/res/Makefile
	tools/bzwtransform/Makefile
	tools/modeltool/Makefile
	tools/modeltool/vc71/Makefile
	win32/Makefile
	win32/VC71/Makefile
	win32/VC8/Makefile)
AC_OUTPUT

# use AC_MSG_RESULT instead of AC_MSG_NOTIFY to support old autoconf

echo "CFLAGS are $CFLAGS"
echo "LDFLAGS are $LDFLAGS"
echo "CPPFLAGS are $CPPFLAGS"
echo "LIBS are $LIBS"

AC_MSG_RESULT([])
AC_MSG_RESULT([BZFlag-$BZFLAG_VERSION.$CONF_DATE with the following:])
if test "x$CURSES_LIB" = x; then
       AC_MSG_RESULT([     no curses!])
else
       AC_MSG_RESULT([     curses: $CURSES_LIB])
fi
if test x$with_SDL = xno; then
       AC_MSG_RESULT([     no SDL!])
else
       AC_MSG_RESULT([     SDL])
fi
if test "x$enable_bzadmin" = "xno"; then
       AC_MSG_RESULT([     no bzadmin client binary!])
else
       AC_MSG_RESULT([     bzadmin])
fi
if test "x$enable_client" = "xno"; then
       AC_MSG_RESULT([     no bzflag client binary!])
else
       AC_MSG_RESULT([     bzflag client])
fi
