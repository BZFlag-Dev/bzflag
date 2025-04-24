/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __BZFGL_H__
#define __BZFGL_H__

/** this file contains headers necessary for opengl */

#include "common.h"

/* GLAD2 include */
#include <glad/gl.h>
/* We use glu too */
#include <GL/glu.h>

// glGenTextures() should never return 0
#define INVALID_GL_TEXTURE_ID ((GLuint) 0)

// glGenLists() will only return 0 for errors
#define INVALID_GL_LIST_ID ((GLuint) 0)


/* Protect us from ourselves. Warn when these
 * are called inside of the wrong context code
 * sections (freeing and initializing).
 */
//#define DEBUG_GL_MATRIX_STACKS
#ifdef DEBUG
#  ifdef DEBUG_GL_MATRIX_STACKS
#    define glPushMatrix()          bzPushMatrix()
#    define glPopMatrix()           bzPopMatrix()
#    define glMatrixMode(mode)          bzMatrixMode(mode)
#  endif // DEBUG_GL_MATRIX_STACKS
#endif

// these are housed at the end of OpenGLGState.cxx, for now
extern void   bzPushMatrix();
extern void   bzPopMatrix();
extern void   bzMatrixMode(GLenum mode);


#endif /* __BZFGL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
