/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* this config is just for Dev-C++ since it doesn't use automake*/

#ifndef _DEVCPP_CONFIG
#define _DEVCPP_CONFIG

#if defined(WIN32) && !defined(_WIN32)
#define _WIN32
#endif

/* Building regex */
#define BUILD_REGEX 1

/* Time Bomb expiration */
/* #undef TIME_BOMB */

/* Debug Rendering */
/* #undef DEBUG_RENDERING */

/* Enabling Robots */
#define ROBOT 1

/* Enabling Snapping */
#define SNAPPING 1

/* Enable score dumping for BZFS */
#define PRINTSCORE 1

/* Enable time limits for BZFS servers */
#define TIMELIMIT 1

/* On windows, strcasecmp is really stricmp */
#ifdef _WIN32
#define HAVE_STRICMP 1
#endif

/* Define to 1 if you have regex stuff available */
/* undef HAVE_REGEX_H */

#ifndef DEBUG
  #ifdef _DEBUG
    #define DEBUG 1
  #else
    #define DEBUG 0
  #endif
#endif

// define our OS
#ifndef BZ_BUILD_OS
  #if defined(__linux)
    #ifdef DEBUG
      #define BZ_BUILD_OS			"linux-DevC++-gccD"
    #else
      #define BZ_BUILD_OS			"linux-DevC++-gcc"
    #endif //DEBUG
  #else if defined(_WIN32)
    #ifdef DEBUG
      #define BZ_BUILD_OS			"W32-DevC++-MinGW32D"
    #else
      #define BZ_BUILD_OS			"W32-DevC++-MinGW32"
    #endif //DEBUG
  #endif //__linux
#endif

#ifdef _WIN32
  #ifndef WINVER
    #define WINVER 0x0400
  #endif
  #define _WIN32_WINNT 0x0400
  #include <Windows.h>
#endif //WIN32

#endif //_DEVCPP_CONFIG
