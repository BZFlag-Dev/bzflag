#                        I S N A N . M 4
# BZFlag
# Copyright (c) 1993 - 2006 Tim Riker
#
# This package is free software; you can redistribute it and/or modify
# it under the terms of the license found in the file named COPYING
# that should have accompanied this file.
#
# THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
###
# BZ_FUNC_ISNAN
# checks whether the isnan() function can be found, takes no arguments

AC_DEFUN([BZ_FUNC_ISNAN], [
AC_LANG_PUSH([C++])

AC_MSG_CHECKING([for isnan])

dnl see if we have std::isnan
bz_isnan_works=no
AC_TRY_COMPILE([
#ifdef HAVE_CMATH
#  include <cmath>
#endif
], [
float f = 0.0f;
std::isnan(f);
], [bz_isnan_works=yes])

if test "x$bz_isnan_works" = "xyes" ; then
	AC_DEFINE([HAVE_STD__ISNAN], [1], [Define to 1 if `std::isnan' is available])
else
	dnl try again using just math.h, but include cmath in case it clobbers
	AC_TRY_COMPILE([
#include <cmath>
#include <math.h>
], [
float f = 0.0f;
isnan(f);
], [bz_isnan_works=yes])
	if test "x$bz_isnan_works" = "xyes" ; then
		AC_DEFINE([HAVE_ISNAN], [1], [Define to 1 if `isnan' is available])
	fi
fi

AC_MSG_RESULT([$bz_isnan_works])

AC_LANG_POP([C++])
])

# Local Variables:
# mode: m4
# tab-width: 8
# standard-indent: 4
# indent-tabs-mode: t
# End:
# ex: shiftwidth=4 tabstop=8
