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
#include <cstdint>
#include <cstdio>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

#undef TCP_NODELAY
#undef TCP_MAXSEG
#define __unix__
