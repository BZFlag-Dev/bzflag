/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * common definitions
 */

#ifndef BZF_COMMON_H
#define	BZF_COMMON_H

// Might we be BSDish? sys/param.h has BSD defined if so
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#if defined(_WIN32)
// turn off bogus `this used in base member initialization list'
#pragma warning(disable: 4355)
#endif /* defined(_WIN32) */

#include <assert.h>
#include <stddef.h>

#if defined(_MACOSX_)
	#include <CoreServices/CoreServices.h>
        #undef TCP_NODELAY
        #undef TCP_MAXSEG
#endif

// Normally, user types are capitalized but X11 already defines Bool
// _and_ Boolean.  We don't want to include any X header files here
// so we have to use lowercase.  Way to go, X.
//
// Oh jeez, windows defines boolean and BOOLEAN!  *sigh*  alright,
// I'll match the type that windows uses which is unsigned char
// (I was using int).
//
// note -- this isn't bool because this predates common availability
// of compilers that support bool.
typedef	unsigned char	boolean;

// Boolean constants.  Again X messes us up.  At least this time
// X uses #define so we can undo its evil effects.  If Xlib.h is
// included after this file, we'll get the defines and not the
// booleans.  We could include Xlib.h here to avoid this but that
// would reduce the portability of this file, and besides it's huge.
#ifdef False
#undef False
#endif
#ifdef True
#undef True
#endif
const boolean		False = 0;
const boolean		True = !False;

// some platforms don't have float versions of the math library
#if defined(_old_linux_) || defined(_MACOSX_) || defined(sun)
#define	asinf		(float)asin
#define	atanf		(float)atan
#define	atan2f		(float)atan2
#define	cosf		(float)cos
#define	expf		(float)exp
#define	fabsf		(float)fabs
#define	floorf		(float)floor
#define	fmodf		(float)fmod
#define	hypotf		(float)hypot
#define	logf		(float)log
#define	powf		(float)pow
#define	sinf		(float)sin
#define	sqrtf		(float)sqrt
#define	tanf		(float)tan
#endif

// random number stuff
#include <stdlib.h>
#define bzfrand()	((double)rand() / ((double)RAND_MAX + 1.0))
#define bzfsrand(_s)	srand(_s)


#if !defined(_WIN32) & !defined(macintosh)

#ifndef BSD
#include <values.h>
#endif
#include <sys/types.h>

#if defined(__linux) || defined(_MACOSX_) || (defined(__sgi) && !defined(__INTTYPES_MAJOR))
typedef u_int16_t	uint16_t;
typedef u_int32_t	uint32_t;
#endif
#if defined(sun)
typedef signed short	int16_t;
typedef ushort_t	uint16_t;
typedef signed int	int32_t;
typedef uint_t		uint32_t;
#endif

#endif

#if defined( macintosh )

// missing constants

  #ifndef MAXFLOAT
    #define	MAXFLOAT	3.402823466e+38f
  #endif

  #ifndef M_PI
    #define	M_PI		  3.14159265358979323846f
  #endif

  #ifndef M_SQRT1_2
    #define	M_SQRT1_2	0.70710678118654752440f
  #endif

// need some integer types
  #include <inttypes.h>

// my own strcasecmp, missing in MSL
// maybe this should be in BzfString.h ?
  #ifdef __MWERKS__
    #include "strcasecmp.h"
  #endif

  #ifndef setenv
    #define setenv(a,b,c)
  #endif

  #ifndef putenv
    #define putenv(a)
  #endif
#endif /* defined( macintosh ) */

#if defined(_WIN32)

// missing float math functions
#define	hypotf		(float)hypot

// missing constants
#ifndef MAXFLOAT
#define	MAXFLOAT	3.402823466e+38f
#endif
#ifndef M_PI
#define	M_PI		3.14159265358979323846f
#endif
#ifndef M_SQRT1_2
#define	M_SQRT1_2	0.70710678118654752440f
#endif

// missing types

typedef signed short	int16_t;
typedef unsigned short	uint16_t;
typedef signed int	int32_t;
typedef unsigned int	uint32_t;

#endif /* !defined(_WIN32) */

#endif // BZF_COMMON_H

#ifdef False
#undef False
#endif
#ifdef True
#undef True
#endif
// ex: shiftwidth=2 tabstop=8
