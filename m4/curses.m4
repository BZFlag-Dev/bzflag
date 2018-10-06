# This macro tries to find curses, and defines HAVE_CURSES_H or HAVE_NCURSES_H
# if any of those headers are found. It also defines CURSES_LIB.
AC_DEFUN([MP_WITH_CURSES], [
    mp_save_LIBS="$LIBS"
    CURSES_LIB=""

    ifdef([PKG_CHECK_MODULES], [
        PKG_CHECK_MODULES([ncurses], [ncurses],
        [CURSES_LIB="$ncurses_LIBS"]
        AC_DEFINE(HAVE_NCURSES_H, , [Use the header file ncurses.h])
        AC_SUBST(CURSES_LIB),
        [_CHECK_CURSES])
    ],
    [_CHECK_CURSES])
    LIBS="$mp_save_LIBS"
])dnl

AC_DEFUN([_CHECK_CURSES], [
    AC_CACHE_CHECK([for working ncurses], mp_cv_ncurses,
        [LIBS="$LIBS -lncurses"
        AC_LINK_IFELSE([
        AC_LANG_PROGRAM([[#include <ncurses.h>
        ]], [[chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr();
        ]])], [mp_cv_ncurses=yes], [mp_cv_ncurses=no])])
        if test "$mp_cv_ncurses" = yes ; then
            AC_DEFINE(HAVE_NCURSES_H, , [Use the header file ncurses.h])
            CURSES_LIB="-lncurses"
            AC_SUBST(CURSES_LIB)
        fi
])dnl
