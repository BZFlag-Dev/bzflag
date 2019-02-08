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

#ifndef __BZFGL_H__
#define __BZFGL_H__

/** this file contains headers necessary for opengl */

#include "common.h"

#include <GL/glew.h>

#ifndef GL_VERSION_1_5
# error OpenGL version 1.5 functionality is required
#endif


// glGenTextures() should never return 0
#define INVALID_GL_TEXTURE_ID ((GLuint) 0)


/* Protect us from ourselves. Warn when these
 * are called inside of the wrong context code
 * sections (freeing and initializing).
 */
//#define DEBUG_GL_MATRIX_STACKS
#ifdef DEBUG
#  define glGenTextures(count, textures)    bzGenTextures((count), (textures))
#  ifdef DEBUG_GL_MATRIX_STACKS
#    define glPushMatrix()          bzPushMatrix()
#    define glPopMatrix()           bzPopMatrix()
#    define glMatrixMode(mode)          bzMatrixMode(mode)
#  endif // DEBUG_GL_MATRIX_STACKS
#endif
// always swap these calls (context protection)
#define glDeleteTextures(count, textures)   bzDeleteTextures((count), (textures))

// these are housed at the end of OpenGLGState.cxx, for now
extern void   bzGenTextures(GLsizei count, GLuint *textures);
extern void   bzDeleteTextures(GLsizei count, const GLuint *textures);
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
