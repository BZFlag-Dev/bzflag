# This macro tries to find curses, and defines HAVE_CURSES_H or HAVE_NCURSES_H
# if any of those headers are found. It also defines CURSES_LIB.
AC_DEFUN([MP_WITH_CURSES],
  [AC_ARG_WITH(xcurses, [  --with-xcurses          Force the use of XCurses over ncurses],,)
AC_ARG_WITH(curses, [  --with-curses           Force the use of curses over ncurses],,)
    mp_save_LIBS="$LIBS"
   CURSES_LIB=""
   if test "$with_curses" != yes -a "$with_xcurses" != yes
   then
     AC_CACHE_CHECK([for working ncurses], mp_cv_ncurses,
       [LIBS="$LIBS -lncurses"
	AC_LINK_IFELSE([
	AC_LANG_PROGRAM([[#include <ncurses.h>
	]], [[chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr();
	]])], [mp_cv_ncurses=yes], [mp_cv_ncurses=no])])
     if test "$mp_cv_ncurses" = yes
     then
       AC_DEFINE(HAVE_NCURSES_H, , [Use the header file ncurses.h])
       CURSES_LIB="-lncurses"
     fi
   fi
   if test ! "$CURSES_LIB" -a "$with_xcurses" != yes
   then
     AC_CACHE_CHECK([for working curses], mp_cv_curses,
       [LIBS="$mp_save_LIBS -lcurses"
	AC_LINK_IFELSE([
	AC_LANG_PROGRAM([[#include <curses.h>
	]], [[chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr();
	]])], [mp_cv_curses=yes], [mp_cv_curses=no])])
     if test "$mp_cv_curses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H, , [Use the header file curses.h])
       CURSES_LIB="-lcurses"
     fi
   fi
   if test ! "$CURSES_LIB" -a "$with_xcurses" != yes
   then
     AC_CACHE_CHECK([for working PDcurses], mp_cv_pdcurses,
       [LIBS="$mp_save_LIBS -lpdcurses"
	AC_LINK_IFELSE([
	AC_LANG_PROGRAM([[#include <curses.h>
	]], [[chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr();
	]])], [mp_cv_pdcurses=yes], [mp_cv_pdcurses=no])])
     if test "$mp_cv_pdcurses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H, , [Use the header file curses.h])
       CURSES_LIB="-lpdcurses"
     fi
   fi
   if test ! "$CURSES_LIB" -a "$with_curses" != yes
   then
     xcurses_deplibs="-L$x_libraries -lXaw -lXmu -lXt -lX11 -lSM -lICE -lXext"
     AC_CACHE_CHECK([for working XCurses], mp_cv_xcurses,
       [LIBS="$mp_save_LIBS -lXCurses $xcurses_deplibs"
	AC_LINK_IFELSE([
	AC_LANG_PROGRAM([[#include <xcurses.h>
	]], [[chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr();
	]])], [mp_cv_xcurses=yes], [mp_cv_xcurses=no])])
     if test "$mp_cv_xcurses" = yes
     then
       AC_DEFINE(HAVE_XCURSES_H, , [Use the header file xcurses.h])
       CURSES_LIB="-lXCurses $xcurses_deplibs"
     fi
   fi
   LIBS="$mp_save_LIBS"
])dnl
# Configure paths for SDL
# Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_SDL([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for SDL, and define SDL_CFLAGS and SDL_LIBS
dnl
AC_DEFUN([AM_PATH_SDL],
[dnl
dnl Get the cflags and libraries from the sdl-config script
dnl
AC_ARG_WITH(sdl-prefix,[  --with-sdl-prefix=PFX   Prefix where SDL is installed (optional)],
            sdl_prefix="$withval", sdl_prefix="")
AC_ARG_WITH(sdl-exec-prefix,[  --with-sdl-exec-prefix=PFX Exec prefix where SDL is installed (optional)],
            sdl_exec_prefix="$withval", sdl_exec_prefix="")
AC_ARG_ENABLE(sdltest, [  --disable-sdltest       Do not try to compile and run a test SDL program],
		    , enable_sdltest=yes)

  if test x$sdl_exec_prefix != x ; then
     sdl_args="$sdl_args --exec-prefix=$sdl_exec_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_exec_prefix/bin/sdl-config
     fi
  fi
  if test x$sdl_prefix != x ; then
     sdl_args="$sdl_args --prefix=$sdl_prefix"
     if test x${SDL_CONFIG+set} != xset ; then
        SDL_CONFIG=$sdl_prefix/bin/sdl-config
     fi
  fi

  AC_REQUIRE([AC_CANONICAL_TARGET])
  PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
  min_sdl_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)
  no_sdl=""
  if test "$SDL_CONFIG" = "no" ; then
    no_sdl=yes
  else
    SDL_CFLAGS=`$SDL_CONFIG $sdlconf_args --cflags`
    SDL_LIBS=`$SDL_CONFIG $sdlconf_args --libs`

    sdl_major_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    sdl_minor_version=`$SDL_CONFIG $sdl_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    sdl_micro_version=`$SDL_CONFIG $sdl_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_sdltest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $SDL_CFLAGS"
      CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
      LIBS="$LIBS $SDL_LIBS"
dnl
dnl Now check if the installed SDL is sufficiently new. (Also sanity
dnl checks the results of sdl-config to some extent
dnl
      rm -f conf.sdltest
      AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;

  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;

  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

]])],[],[no_sdl=yes],[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_sdl" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])
  else
     AC_MSG_RESULT(no)
     if test "$SDL_CONFIG" = "no" ; then
       echo "*** The sdl-config script installed by SDL could not be found"
       echo "*** If SDL was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the SDL_CONFIG environment variable to the"
       echo "*** full path to sdl-config."
     else
       if test -f conf.sdltest ; then
        :
       else
          echo "*** Could not run SDL test program, checking why..."
          CFLAGS="$CFLAGS $SDL_CFLAGS"
          CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
          LIBS="$LIBS $SDL_LIBS"
	  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include "SDL.h"

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
]], [[
return 0;
]])],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding SDL or finding the wrong"
          echo "*** version of SDL. If it is not finding SDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means SDL was incorrectly installed"
          echo "*** or that you have moved SDL since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl-config script: $SDL_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     SDL_CFLAGS=""
     SDL_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(SDL_CFLAGS)
  AC_SUBST(SDL_LIBS)
  rm -f conf.sdltest
])

dnl MY_CURL
dnl -------
dnl set my_cv_curl_vers to the version of libcurl or NONE
dnl if libcurl is not found or is too old

AC_DEFUN([MY_CURL],[
 AC_CACHE_VAL(my_cv_curl_vers,[
 my_cv_curl_vers=NONE
 dnl check is the plain-text version of the required version
 check="7.10.0"
 dnl check_hex must be UPPERCASE if any hex letters are present
 check_hex="070A00"

 AC_MSG_CHECKING([for curl >= $check])

 if eval curl-config --version >/dev/null >/dev/null; then
   ver=`curl-config --version | sed -e "s/libcurl //g"`
   hex_ver=`curl-config --vernum | tr 'a-f' 'A-F'`
   ok=`echo "ibase=16; if($hex_ver>=$check_hex) $hex_ver else 0" | bc`

   if test x$ok != x0; then
     my_cv_curl_vers="$ver"
     AC_MSG_RESULT([$my_cv_curl_vers])
   else
     AC_MSG_RESULT(FAILED)
     AC_MSG_WARN([$ver is too old. Need version $check or higher.])
   fi
 else
   AC_MSG_RESULT(FAILED)
   AC_MSG_WARN([curl-config was not found])
 fi
 ])
])
