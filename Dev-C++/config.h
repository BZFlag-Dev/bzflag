/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* this config is just for Dev-C++ since it doesn't use automake */

#ifndef _DEVCPP_CONFIG
#define _DEVCPP_CONFIG

#define NOMINMAX 1

#if defined(WIN32) && !defined(_WIN32)
#   define _WIN32
#endif
//__GNUC_PATCHLEVEL__ was introduced with GCC 3.x, so ifndef, this thing is old!
#ifndef __GNUC_PATCHLEVEL__
#   warning It is recommended that you update MinGW32 to the latest version of GCC.
#endif

/* Currently builds with SDL */
//#ifndef BZFLAG_CXX
#define HAVE_SDL 1
//#endif

#define _USE_BZ_API 1

/* Building regex */
#define BUILD_REGEX 1

/* Time Bomb expiration */
/* #undef TIME_BOMB */

/* Debug Rendering */
/* #undef DEBUG_RENDERING */

/* Uses Kerberos for authentication */
//#define HAVE_KRB5 1 //define if you have this library; it is not pre-packaged with mingw32

/* Enabling Robots */
#define ROBOT 1

/* Enabling Motion-Freezing */
#define FREEZING 1

/* Enabling Snapping */
#define SNAPPING 1

/* Enable score dumping for BZFS */
//#define PRINTSCORE 1 //already defined in bzfs.h (10/26/2004)

/* find a matching function for count() */
#define HAVE_STD__COUNT 1

/* all that floating point math nonsense; GCC already has all of this crap */
#define HAVE_ASINF 1
#define HAVE_ATAN2F 1
#define HAVE_ATANF 1
#define HAVE_COSF 1
#define HAVE_EXPF 1
#define HAVE_FABSF 1
#define HAVE_FLOORF 1
#define HAVE_FMODF 1
#define HAVE_HYPOTF 1
#define HAVE_LOGF 1
#define HAVE_POWF 1
#define HAVE_SINF 1
#define HAVE_SQRTF 1
#define HAVE_TANF 1

#define HAVE_STD__MIN 1
#define HAVE_STD__MAX 1

/* Define to 1 if you have regex stuff available */
/* undef HAVE_REGEX_H */

#if !defined(DEBUG) && defined(_DEBUG)
#   define DEBUG 1
#endif

// define our OS
#ifndef BZ_BUILD_OS
  #if defined(__linux) //Dev-C++ for Linux is not usable (8/2004), but for the future
    #ifdef DEBUG
      #define BZ_BUILD_OS			"linux-DevC++-gccD"
    #else
      #define BZ_BUILD_OS			"linux-DevC++-gcc"
    #endif //DEBUG
  #elif defined(_WIN32)
    #ifdef __MINGW32__
	#ifdef DEBUG
	    #define BZ_BUILD_OS			"W32-DevC++-MinGW32D"
	#else
	    #define BZ_BUILD_OS			"W32-DevC++-MinGW32"
	#endif
    #elif defined(__CYGWIN__)
	#ifdef DEBUG
	    #define BZ_BUILD_OS			"W32-DevC++-CygwinD"
	#else
	    #define BZ_BUILD_OS			"W32-DevC++-Cygwin"
	#endif
    #else
	#ifdef DEBUG
	    #define BZ_BUILD_OS			"W32-DevC++D"
	#else
	    #define BZ_BUILD_OS			"W32-DevC++"
	#endif
    #endif
  #endif //__linux
#endif

#ifdef _WIN32
  #ifndef WINVER
    #define WINVER 0x0400
  #endif
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0400
  #endif
  #include <Windows.h>
#endif //WIN32

#include <math.h>

/* Define to 1 if you have the `WaitForSingleObject' function. */
#define HAVE_WAITFORSINGLEOBJECT 1

/* Define to 1 if you have the `Sleep' function. */
#define HAVE_SLEEP 1

/* Define to 1 if you have the `wglGetCurrentContext' function. */
#define HAVE_WGLGETCURRENTCONTEXT 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

#endif
