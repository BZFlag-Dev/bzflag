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

/* Currently builds with SDL */
#define HAVE_SDL 1

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
  #if defined(__linux) //Dev-C++ for Linux is not usable (8/2004), but for the future
    #ifdef DEBUG
      #define BZ_BUILD_OS			"linux-DevC++-gccD"
    #else
      #define BZ_BUILD_OS			"linux-DevC++-gcc"
    #endif //DEBUG
  #elif defined(_WIN32)
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
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0400
  #endif
  #include <Windows.h>
#endif //WIN32

#endif //_DEVCPP_CONFIG
