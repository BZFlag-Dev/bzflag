/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * include gl.h.  this isn't as simple as including GL/gl.h
 * on win32 because it depends on platform specific identifiers.
 * that's very rude.
 */

#ifndef BZF_GL_H

#if defined(_WIN32)
// windows wants to include winsock but we can't allow that.  we
// include winsock2.h (in network.h) and the two don't play together.
#define _WINSOCKAPI_

// including windows.h slows down compiles a *lot*.  should
// replace this with only the necessary definitions.
#include <windows.h>
#endif

#if defined(_MACOSX_)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#endif

