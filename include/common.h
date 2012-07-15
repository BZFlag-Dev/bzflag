/* bzflag
 * Copyright (c) 1993-2012 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * common definitions
 */

#ifndef BZF_COMMON_H
#define	BZF_COMMON_H

/* this should always be the very FIRST header */

#ifdef _DEVCPP /* the Dev-C++ build is acting very stubborn; this is (hopefully) -temporary- */
# include_next "config.h"
#else
# include "config.h"
#endif

#ifdef _WIN32
#  undef NOMINMAX
#  define NOMINMAX 1
#  include "win32.h"
#endif

#include <stdio.h>
#include <stdlib.h> /* needed for bzfrand */
#include <math.h>
#ifdef __cplusplus
#  include <cmath>
#endif


extern int debugLevel;


/* Provide a means to conveniently test the version of the GNU
 * compiler.  Use it like this:
 *
 * #if GCC_PREREQ(2,8)
 * ... code requiring gcc 2.8 or later ...
 * #endif
 *
 * WARNING: THIS MACRO IS CONSIDERED PRIVATE AND SHOULD NOT BE USED
 * OUTSIDE OF THIS HEADER FILE.  DO NOT RELY ON IT.
 */
#ifndef GCC_PREREQ
#  if defined __GNUC__
#    define GCC_PREREQ(major, minor) __GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor))
#  else
#    define GCC_PREREQ(major, minor) 0
#  endif
#else
#  warning "GCC_PREREQ is already defined.  See the common.h header."
#endif


/**
 * UNUSED provides a common mechanism for declaring unused parameters.
 * Use it like this:
 *
 * int
 * my_function(int argc, char **UNUSED(argv))
 * {
 *   ...
 * }
 *
 */
#ifndef UNUSED
#  if GCC_PREREQ(2, 5)
     /* GCC-style */
#    define UNUSED(parameter) (parameter) __attribute__((unused))
#  else
     /* MSVC/C++ */
#    ifdef __cplusplus
#      if defined(NDEBUG)
#	define UNUSED(parameter) /* parameter */
#      else /* some of them are asserted */
#	 define UNUSED(parameter) (parameter)
#      endif
#    else
#      if defined(_MSC_VER)
	 /* disable reporting an "unreferenced formal parameter" */
#	pragma warning( disable : 4100 )
#      endif
#      define UNUSED(parameter) (parameter)
#    endif
#  endif
#else
#  undef UNUSED
#  define UNUSED(parameter) (parameter)
#  warning "UNUSED was previously defined.  Parameter declaration behavior is unknown, see common.h"
#endif


/* near zero by some epsilon convenience define since relying on
 * the floating point unit for proper equivalence is not safe
 */
#define NEAR_ZERO(_value,_epsilon)  ( ((_value) > -_epsilon) && ((_value) < _epsilon) )

/* (radians <--> degrees) conversion values */
#define DEG2RAD 0.0174532925199432957692369076848861271344287189
#define RAD2DEG 57.29577951308232087679815481410517033240547247
#define DEG2RADf ((float)DEG2RAD)
#define RAD2DEGf ((float)RAD2DEG)


/* seven places of precision is pretty safe, so something less precise */
#ifdef FLT_EPSILON
#  define ZERO_TOLERANCE FLT_EPSILON
#else
#  define ZERO_TOLERANCE 1.0e-06f
#endif

/* Might we be BSDish? sys/param.h has BSD defined if so */
#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif

#ifdef HAVE__STRICMP
#  define strcasecmp _stricmp
#endif
#ifdef HAVE__STRNICMP
#  define strncasecmp _strnicmp
#endif
#ifndef HAVE_VSNPRINTF
#  ifdef HAVE__VSNPRINTF
#    define vsnprintf _vsnprintf
#  else
#    define vsnprintf(buf, size, fmt, list) vsprintf(buf, fmt, list)
#  endif
#endif

/* some platforms don't have float versions of the math library */
#ifndef HAVE_ASINF
#  define	asinf		(float)asin
#endif
#ifndef HAVE_ATAN2F
#  define	atan2f		(float)atan2
#endif
#ifndef HAVE_ATANF
#  define	atanf		(float)atan
#endif
#ifndef HAVE_COSF
#  define	cosf		(float)cos
#endif
#ifndef HAVE_EXPF
#  define	expf		(float)exp
#endif
#ifndef HAVE_FABSF
#  define	fabsf		(float)fabs
#endif
#ifndef HAVE_FLOORF
#  define	floorf		(float)floor
#endif
#ifndef HAVE_FMODF
#  define	fmodf		(float)fmod
#endif
#ifndef HAVE_HYPOTF
#  define	hypotf		(float)hypot
#endif
#ifndef HAVE_LOGF
#  define	logf		(float)log
#endif
#ifndef HAVE_LOG10F
#  define	log10f		(float)log10
#endif
#ifndef HAVE_POWF
#  define	powf		(float)pow
#endif
#ifndef HAVE_SINF
#  define	sinf		(float)sin
#endif
#ifndef HAVE_SQRTF
#  define	sqrtf		(float)sqrt
#endif
#ifndef HAVE_TANF
#  define	tanf		(float)tan
#endif


/* random number stuff */
#define bzfrand()	((double)rand() / ((double)RAND_MAX + 1.0))
#define bzfsrand(_s)	srand(_s)

#ifndef __BEOS__
#  ifdef HAVE_VALUES_H
#    include <values.h>
#  endif
#else
#  include <limits.h>
/* BeOS: FIXME */
#  define MAXSHORT SHORT_MAX
#  define MAXINT INT_MAX
#  define MAXLONG LONG_MAX
#endif /* __BEOS__ */

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

/* need some integer types */
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#else
#  if defined(__linux) || (defined(__sgi) && !defined(__INTTYPES_MAJOR))
typedef u_int16_t	uint16_t;
typedef u_int32_t	uint32_t;
#  endif
#  if defined(sun)
typedef signed short	int16_t;
typedef ushort_t	uint16_t;
typedef signed int	int32_t;
typedef uint_t		uint32_t;
#  endif
typedef unsigned char	uint8_t;
#endif


/* missing constants */

#ifndef MAXFLOAT
#  define	MAXFLOAT	3.402823466e+38f
#endif

#ifndef M_PI
#  define	M_PI		3.14159265358979323846f
#endif

#ifndef M_SQRT1_2
#  define	M_SQRT1_2	0.70710678118654752440f
#endif


#ifdef __BEOS__
#  ifndef setenv
#    define setenv(a,b,c)
#  endif
#  ifndef putenv
#    define putenv(a)
#  endif
#endif /* __BEOS__ */

#ifdef countof
#  undef countof
#endif
#define countof(__x)   (sizeof(__x) / sizeof(__x[0]))


#ifdef HAVE_STD__ISNAN
#  ifdef isnan
#    undef isnan
#  endif
#  define isnan std::isnan
#elif defined(HAVE__ISNAN)
#  ifdef isnan
#    undef isnan
#  endif
#  define isnan _isnan
#else
#  ifndef HAVE_ISNAN
#    ifdef __cplusplus
#      ifdef isnan
#	undef isnan
#      endif
       template<typename Tp>
       inline int isnan(Tp f)
       {
	 return (f!=f);
       }
#    else
#      define isnan(f) ((f) != (f))
#    endif /* __cplusplus */
#  endif /* HAVE_ISNAN */
#endif /* HAVE_STD__ISNAN */


#ifndef HAVE_STD__MAX
#  ifdef __cplusplus
#    ifdef max
#      undef max
#    endif
     namespace std
     {
       template<typename comparable>
       inline const comparable& max(const comparable& a, const comparable& b)
       {
	 return  a < b ? b : a;
       }
     }
#  else
#    ifdef max
#      undef max
#    endif
#    define max(a,b) a < b ? b : a
#  endif /* __cplusplus */
#endif /* HAVE_STD__MAX */

#ifndef HAVE_STD__MIN
#  ifdef __cpluscplus
#    ifdef min
#      undef min
#    endif
     namespace std
     {
       template<typename comparable>
       inline const comparable& min(const comparable& a, const comparable& b)
       {
	 return b < a ? b : a;
       }
     }
#  else
#    ifdef min
#      undef min
#    endif
#    define min(a,b) b < a ? b : a
#  endif /* __cplusplus */
#endif /* HAVE_STD_MIN */

#ifdef BUILD_REGEX
#  include "bzregex.h"
#elif defined(HAVE_REGEX_H)
#  include <regex.h>
#else
#  define regex_t void
#endif  /* BUILD_REGEX */

#endif /* BZF_COMMON_H */


/* Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
