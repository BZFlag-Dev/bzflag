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

/*
 * common definitions specific to the _WIN32 platform
 */

#ifndef __WIN32_H__
#define	__WIN32_H__

#ifdef __MINGW32__
#include <windows.h>
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

#ifndef int16_t
typedef signed short	int16_t;
#endif

#ifndef uint16_t
typedef unsigned short	uint16_t;
#endif

#ifndef int32_t
typedef signed int	int32_t;
#endif

#ifndef uint32_t
typedef unsigned int	uint32_t;
#endif

/* stuff specific to visual studio */
#if (_MSC_VER)
// turn off bogus `this used in base member initialization list'
#  pragma warning(disable: 4786)
#  pragma warning(disable: 4503)
#  pragma warning(disable: 4355)

// missing functions
#  define snprintf	_snprintf

#  define PATH_MAX	MAX_PATH

#endif // _MSC_VER
#endif // __WIN32_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
