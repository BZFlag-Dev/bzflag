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

#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

/* so that socklen_t is defined in bzfrelay.c */
#define _BSD_SOCKLEN_T_
/* BZFlag version */
#define BVERSION 10900000
/* enable robots */
#define ROBOT

#undef TCP_NODELAY
#undef TCP_MAXSEG
/* temp compatibility define */
#define __unix__
