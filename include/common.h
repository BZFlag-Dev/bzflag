/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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

#if (_WIN32)
	#pragma warning(disable: 4786)
#endif

#include <config.h>

#include <string>
#include <sstream>
#include <vector>
#include <stdarg.h>
#include <stdio.h>

// Might we be BSDish? sys/param.h has BSD defined if so
#if (defined(__unix__) || defined(unix) || defined(__APPLE__)) && !defined(USG)
#include <sys/param.h>
#endif

#if defined(_WIN32)
// turn off bogus `this used in base member initialization list'
#pragma warning(disable: 4355)
#pragma warning( 4 : 4786 )
#endif /* defined(_WIN32) */

#if !defined HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

#include <assert.h>
#include <stddef.h>

// some platforms don't have float versions of the math library
#if defined(_old_linux_) || defined(__APPLE__) || defined(sun)
#define	asinf		(float)asin
#define	atanf		(float)atan
#define	atan2f		(float)atan2
#define	cosf		(float)cos
#define	expf		(float)exp
#if !defined(__APPLE__)
#define	fabsf		(float)fabs
#define	floorf		(float)floor
#define	fmodf		(float)fmod
#endif
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


#if !defined(_WIN32) && !defined(__APPLE__)

#ifndef BSD
#ifndef __BEOS__
#include <values.h>
#else
#include <limits.h>
/* BeOS: FIXME */

#define MAXSHORT SHORT_MAX
#define MAXINT INT_MAX
#define MAXLONG LONG_MAX
#endif
#endif
#include <sys/types.h>

#if defined(__linux) || (defined(__sgi) && !defined(__INTTYPES_MAJOR))
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

typedef unsigned char	uint8_t;

#if defined( macintosh ) || defined( __BEOS__ )

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
  #ifdef __MWERKS__
    #include "strcasecmp.h"
  #endif

  #ifndef setenv
    #define setenv(a,b,c)
  #endif

  #ifndef putenv
    #define putenv(a)
  #endif
#endif /* defined( macintosh ) || defined( __BEOS__ ) */

#if defined(_WIN32)

#ifndef __MINGW32__
// missing float math functions
#define	hypotf		(float)hypot
#endif

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

class string_util {
  public:
    static std::string string_util::vformat(const char* fmt, va_list args) {
      // FIXME -- should prevent buffer overflow in all cases
      // not all platforms support vsnprintf so we'll use vsprintf and a
      // big temporary buffer and hope for the best.
      char buffer[8192];
      vsprintf(buffer, fmt, args);
      return std::string(buffer);
    }
    static std::string string_util::format(const char* fmt, ...) {
      va_list args;
      va_start(args, fmt);
      std::string result = vformat(fmt, args);
      va_end(args);
      return result;
    }
    // get a vector of strings from a string, using all of chars of the delims
    // string as separators 
    static std::vector<std::string> string_util::tokenize(std::string in, std::string delims){  
      std::vector<std::string> out;
      std::ostringstream outputString;
      std::istringstream inStream (in);

      std::string candidate;
      std::string testme;

      int character;
      while ( (character = inStream.get()) != EOF){
	testme = (char) character;
	if (testme.find_first_of(delims) != std::string::npos){
	  candidate = outputString.str();
	  outputString.str(""); // clear it
	  if (candidate != "")  out.push_back(candidate);
	} else {
	  outputString << (char)character;
	}
      }

      candidate = outputString.str();
      if (candidate != "")  out.push_back(candidate);
      return out;
    }
};

#endif // BZF_COMMON_H

// ex: shiftwidth=2 tabstop=8
