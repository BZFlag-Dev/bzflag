# Process this file with autoconf to produce a configure script.

# version number: aabbccddd, aa=major, bb=minor, cc=revision, ddd=build.
# increase build number for every potentially released build.
# must change this in MSDev project, too.
AC_INIT(bzflag, 10707001)
VERSIONID=1.7g1
AC_SUBST(VERSIONID)

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(include/config.h)
AC_CONFIG_SRCDIR(src/bzflag/bzflag.cxx)

#Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AC_CANONICAL_HOST
AM_CONDITIONAL(LINUX, test x$host_os = xlinux-gnu)
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
AM_CONDITIONAL(MACOS, test x$host_os = xmacos)

case $host_os in
     cygwin|mingw32|windows**) win32=true;;
     *) win32=false;;
esac
AM_CONDITIONAL(WIN32, $win32)

case $host_os in
   linux-gnu)
	case $host_vendor in
	     pc)  CONFIG='linux-i386';;
	     ppc) CONFIG='linux-ppc';;
	     *)   CONFIG='linux';;
	esac;;
   solaris*)
	if $sun_cc; then
	   CONFIG=solaris-cc;
	elif $sun_gcc; then
	   CONFIG=solaris-gcc;
	else
	   CONFIG=solaris;
	fi;;
   irix*)
	if test $host_cpu = mips64; then
	   CONFIG=irix-mips3;
	else
	   CONFIG=irix-mips2;
	fi;;
   macos)
	CONFIG=macosx;;
   cygwin|mingw32|windows*)
	CONFIG=config-win32;;
esac
AC_SUBST(CONFIG)

AC_PATH_XTRA

CPPFLAGS="$CPPFLAGS $X_CFLAGS"
LDFLAGS="$X_LIBS"
case $host_os in
     solaris*)	LIBS="-lresolv -lsocket -lnsl";;
esac;

AC_CHECK_LIB(m, sqrtf)

if test !$X_DISPLAY_MISSING; then
if test "x$EXTRA_LIBS" != x; then
   LIBS="$EXTRA_LIBS $LIBS";
fi
   LIBS="-lX11 $LIBS";
if test "x$X_PRE_LIBS" != x; then
   LIBS="$X_PRE_LIBS $LIBS";
fi
fi

AC_CHECK_LIB(Xext, XextAddDisplay)

if test $host_os = irix; then
   LIBS="-lXsgivc $LIBS";
elif test $host_os = linux-gnu; then
   AC_CHECK_LIB(Xxf86vm, XF86VidModeGetAllModeLines)
   AC_CHECK_LIB(Xi, XListInputDevices)
fi

AC_CHECK_LIB(GL, glBegin)
AC_CHECK_LIB(GLU, gluScaleImage)

case $host_os in
     solaris*)
	LDFLAGS="$LDFLAGS -L/usr/demo/SOUND/lib";
	LIBS="-laudio $LIBS";;
     irix)
	if test ! -f $${ROOT}/usr/lib/libaudio.so; then
# on irix 6.2 and earlier we have libaudio.a and on later irix versions we have
# libaudio.so.  we need to include `-B static' before -laudio if we need to
# link against libaudio.a.  this macro decides at link time if it's necessary
# or not based on whether libaudio.so exists.
	       LIBS="-B static -laudio -B dynamic $LIBS";
	else
	       LIBS="-laudio -B dynamic $LIBS";
	fi;;
esac

case $host_os in
     macos)
	LDFLAGS="$LDFLAGS -framework Carbon -framework OpenGL -framework AGL";;
     irix)
	if test x$host_cpu = xmips64; then
	   LDFLAGS="$LDFLAGS -Wl,-wall -L$(ROOT)/usr/lib32"
	else
	   LDFLAGS="$LDFLAGS -Wl,-wall -L$(ROOT)/usr/lib"
	fi;;
esac

INSTALL_DATA_DIR="\\\"\${pkglibdir}\\\""
AC_SUBST(INSTALL_DATA_DIR)

CPPFLAGS="-DVERSION=$PACKAGE_VERSION"

AC_ARG_ENABLE(timebomb, AC_HELP_STRING([--enable-timebomb], [build exp Date]))
if test x$enable_timebomb = xyes; then
   CPPFLAGS="$CPPFLAGS -DTIME_BOMB=\"3/13/1998\"";
fi

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Turn on debugging]))
AM_CONDITIONAL(DEBUG, test x$enable_debug = xyes)

if test x$enable_debug = xyes; then
   CPPFLAGS="$CPPFLAGS -DDEBUG_RENDERING";
fi

#
# comment to disable robot tanks
#
AC_ARG_ENABLE(robots, AC_HELP_STRING([--enable-robots], [Enable Robots]), [],
				    [enable_robots=yes])
if test x$enable_robots = xyes; then
   CPPFLAGS="$CPPFLAGS -DROBOT"
fi

CPPFLAGS="$CPPFLAGS -DINSTALL_DATA_DIR=\"$INSTALL_DATA_DIR\""

case $host_os in
     linux-gnu)
	CPPFLAGS="$CPPFLAGS -I/usr/include -D_BSD_SOURCE -D_POSIX_SOURCE";
	CPPFLAGS="$CPPFLAGS -DHALF_RATE_AUDIO -DXF86VIDMODE_EXT -DXIJOYSTICK";
	CPPFLAGS="$CPPFLAGS -Wall -W"
	CFLAGS="$CFLAGS -ansi";
	CXXFLAGS="$CXXFLAGS -fno-exceptions -fno-rtti";
	case $host_vendor in
	     pc)  CPPFLAGS="$CPPFLAGS -mcpu=i486";;
	     ppc) CPPFLAGS="$CPPFLAGS -mcpu=603";;
	esac;;
     solaris*)
	CPPFLAGS="$CPPFLAGS -DSUN_OGL_NO_VERTEX_MACROS";
	if test sun_wspro; then
	   CPPFLAGS="$CPPFLAGS -KPIC"
	elif test sun_cc; then
	   CPPFLAGS="$CPPFLAGS -fast"
	elif test sun_gcc; then
	   CXXFLAGS="$CXXFLAGS -fno-exceptions -fno-rtti"
	fi;;
     macos)
	CPPFLAGS="$CPPFLAGS -D_MACOSX_ -D_BSD_SOCKLEN_T_=int -I/usr/include";
	CPPFLAGS="$CPPFLAGS -Wall -W";
	CFLAGS="$CFLAGS -ansi";
	CXXFLAGS="$CXXFLAGS -fno-exceptions -fno-rtti";;
     cygwin|mingw32|windows**)
	CPPFLAGS="$CPPFLAGS /D \"WIN32\" /D \"_MBCS\"";
	CPPFLAGS="$CPPFLAGS /I \"$(top_srcdir)/include\" /nologo /W3";;
     irix)
	if test x$host_cpu != xmips64; then
	   CPPFLAGS="$CPPFLAGS -Dbool=boolean -Dtrue=True -Dfalse=False"
	fi;
	CPPFLAGS="$CPPFLAGS -I$(ROOT)/usr/include -fullwarn";
	if test x$host_cpu = xmips64; then
	   CPPFLAGS="CPPFLAGS -n32 -mips3";
	else
	   CPPFLAGS="CPPFLAGS -o32 -mips2";
	fi;
	CFLAGS="${CFLAGS} -xansi";;
esac

# /FD generates dependencies;  don't know where though
# /Fd names pdb file   /Fd"$(INTDIR)\\"
# /Fo names obj file   /Fo"$(INTDIR)\\"
# /D "_WINDOWS" when building windows app
# /D "_CONSOLE" when building console app
# /D "_LIB"     when building libraries
# /c to compile only
if test x$enable_debug = xyes; then
   case $host_os in
     linux-gnu)
	CFLAGS="$CFLAGS -g";
	CPPFLAGS="$CPPFLAGS -DDEBUG -Werror";
	CXXFLAGS="$CXXFLAGS -g -Wno-deprecated";;
     solaris*)
	CFLAGS="$CFLAGS -g";
	CXXFLAGS="$CXXFLAGS -g";
	CPPFLAGS="$CPPFLAGS -DDEBUG";;
     macos)
	CFLAGS="$CFLAGS -g";
	CXXFLAGS="$CXXFLAGS -g";
	CPPFLAGS="$CPPFLAGS -DDEBUG";;
     cygwin|mingw32|windows**)
	CPPFLAGS="$CPPFLAGS /MDd /Gm /ZI /Od /GZ /D \"DEBUG\" /D \"_DEBUG\"";;
     irix)
	CFLAGS="$CFLAGS -g";
	CXXFLAGS="$CXXFLAGS -g";
	CPPFLAGS="$CPPFLAGS -DDEBUG";;
    esac;
else
    case $host_os in
	linux-gnu)
		CFLAGS="$CFLAGS -O2";
		CXXFLAGS="$CXXFLAGS -O2";
		CPPFLAGS="$CPPFLAGS -DNDEBUG";
		CXXFLAGS="$CXXFLAGS -ffast-math -fomit-frame-pointer";
		CXXFLAGS="$CXXFLAGS -fexpensive-optimizations";;
	 solaris*)
		CFLAGS="$CFLAGS -O";
		CXXFLAGS="$CXXFLAGS -O";
		CPPFLAGS="$CPPFLAGS -DNDEBUG";;
	 macos)
		CFLAGS="$CFLAGS -O2";
		CXXFLAGS="$CXXFLAGS -O2";
		CPPFLAGS="$CPPFLAGS -DNDEBUG";
		CXXFLAGS="$CXXFLAGS -ffast-math -fomit-frame-pointer";
		CXXFLAGS="$CXXFLAGS -fexpensive-optimizations";;
	 cygwin|mingw32|windows**)
		CPPFLAGS="$CPPFLAGS /MT /O2 /D \"NDEBUG\"";;
	 irix)
		if test x$host_cpu = xmips64; then
		   CFLAGS="$CFLAGS -O3";
		   CXXFLAGS="$CXXFLAGS -O3";
		   CPPFLAGS="$CPPFLAGS -DNDEBUG -OPT:Olimit=0:space=ON";
		   CPPFLAGS="$CPPFLAGS -TENV:X=1 -CG:unique_exit=on";
		   CPPFLAGS="$CPPFLAGS -OPT:IEEE_arithmetic=1:roundoff=0:"
		   CPPFLAGS="${CPPFLAGS}wrap_around_unsafe_opt=off";
		else
		   CFLAGS="$CFLAGS -O2";
		   CXXFLAGS="$CXXFLAGS -O2";
		   CPPFLAGS="$CPPFLAGS -DNDEBUG";
		fi;;
     esac;
fi;


if test $host_os = irix; then
   if test $host_cpu = mips64; then
# warnings switched off:
#   1174 -- variable declared but never referenced
#   1209 -- controlling expression is constant
#   1233 -- explicit type is missing
#   1314 -- value copied to temporary, reference to temporary used
#   1375 -- destructor is not virtual
#   1376 -- no accessible constructor
#   1681 -- functions don't match -- virtual function override intended?
	CFLAGS="$CFLAGS -diag_suppress 1009,1014,1107,1110,1116,1164,1174";
	CFLAGS="${CFLAGS},1185,1188,1204,1230,1233";
	CXXFLAGS="$CXXFLAGS -diag_suppress 1174,1209,1233,1314,1375,1681";
   else
	CXXFLAGS="$CXXFLAGS -woff 3482,3577";
   fi;
fi;

LIBDIR="\$(top_srcdir)/lib"
AC_SUBST(LIBDIR)

AC_CONFIG_FILES(Makefile
                src/Makefile
                src/net/Makefile
                src/common/Makefile
                src/obstacle/Makefile
                src/platform/Makefile
                src/ogl/Makefile
                src/scene/Makefile
                src/geometry/Makefile
                src/bzflag/Makefile
                src/bzfs/Makefile
                src/bzfls/Makefile
                src/bzfrelay/Makefile
                man/Makefile
                package/Makefile
		data/Makefile
		bzwtransform/Makefile
		debian/Makefile
		misc/Makefile
		win32/Makefile
		doc/Makefile
		include/Makefile
		package/linux/Makefile
		package/rpm/Makefile
		package/irix/Makefile
		package/mac/Makefile
		package/mac/notes/Makefile
		package/mac/platform/Makefile
		package/mac/src/Makefile
		package/win32/Makefile
		package/win32/include/Makefile
		package/win32/nsis/Makefile
		package/win32/src/Makefile
		package/win32/src/bzfinst/Makefile
		package/win32/src/bzfuinst/Makefile
		package/win32/src/makedb/Makefile
		src/geometry/models/Makefile
		src/geometry/models/hitank/Makefile
		src/geometry/models/lowtank/Makefile
		src/geometry/models/medtank/Makefile
		src/platform/MacBZFlag.pbproj/Makefile)
AC_OUTPUT