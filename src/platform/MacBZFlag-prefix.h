/* bzflag
 * Copyright 1993-2003, Chris Schoeneman, Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <cstdio>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

/* so that socklen_t is defined in bzfrelay.c */
#define _BSD_SOCKLEN_T_
/* BZFlag version */
#define VERSION	10707001
/* enable robots (but they don't work?)
#define ROBOT
/* We are MacOS X */
#define _MACOSX_

#undef TCP_NODELAY
#undef TCP_MAXSEG
#define __unix__
