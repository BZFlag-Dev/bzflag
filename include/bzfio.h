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
 * Include c++ iostream.
 */

#ifndef BZF_MYIO
#define BZF_MYIO

/* egcs headers on linux define NULL as (void*)0.  that's a no no in C++. */
#if defined(NULL)
#undef NULL
#endif
#define NULL 0

/* insert any other broken OS conditionals here */
#if !defined(_WIN32)
#endif
#include <iostream>

#endif


/*
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */

