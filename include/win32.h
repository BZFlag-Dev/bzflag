/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
 * common definitions specific to the _WIN32 platform
 */

#ifndef __WIN32_H__
#define	__WIN32_H__

#define _WINSOCKAPI_

#include <windows.h>
#include <stdio.h>
#include <float.h>

// missing constants
#ifndef MAXFLOAT
#define	MAXFLOAT	FLT_MAX
#endif

// Define this so that the math.h defines M_PI, M_SQRT2, and similar
#define _USE_MATH_DEFINES

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
#  pragma warning(disable: 4503)
#  pragma warning(disable: 4355)

// missing functions
#ifndef snprintf
#  define snprintf	_snprintf
#endif

#endif // _MSC_VER
#endif // __WIN32_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
