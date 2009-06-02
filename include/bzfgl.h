/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

// glew.h needs to come before gl.h, etc...
#include <GL/glew.h>

#ifdef __APPLE__
#  include <OpenGL/OpenGL.h>
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

/* These will track glBegin/End pairs to make sure that they match */
#ifdef DEBUG
#include <iostream>
#include <assert.h>
extern int __beginendCount;
#define glBegin(_value) {\
  if (__beginendCount==0) { \
    __beginendCount++;\
  } else {\
    std::cerr << "ERROR: glBegin called on " << __FILE__ << ':' << __LINE__ << " without calling glEnd before\n"; \
    assert(__beginendCount==0 && "glBegin called without glEnd"); \
  } \
  glBegin(_value);\
}
#define glEnd() {\
  if (__beginendCount==0) { \
    std::cerr << "ERROR: glEnd called on " << __FILE__ << ':' << __LINE__ << " without calling glBegin before\n"; \
    assert(__beginendCount!=0 && "glEnd called without glBegin"); \
  } else {\
    __beginendCount--;\
  } \
  glEnd();\
}
#endif


// glGenTextures() should never return 0
#define INVALID_GL_TEXTURE_ID ((GLuint) 0)

// glGenLists() will only return 0 for errors
#define INVALID_GL_LIST_ID ((GLuint) 0)


/* Protect us from ourselves. Warn when these
 * are called inside of the wrong context code
 * sections (freeing and initializing).
 */
// always swap these calls (context protection)
#ifndef UNSAFE_GL_CONTEXT
#  undef glCreateShader
#  undef glDeleteShader
#  undef glCreateProgram
#  undef glDeleteProgram
#  undef glGenQueries
#  undef glDeleteQueries
#  undef glGenFramebuffersEXT
#  undef glDeleteFramebuffersEXT
#  undef glGenRenderbuffersEXT
#  undef glDeleteRenderbuffersEXT
#  define glDeleteLists(base, count)		bzDeleteLists((base), (count))
#  define glDeleteTextures(count, textures)	bzDeleteTextures((count), (textures))
#  define glGenFramebuffersEXT(n, fbos)		bzGenFramebuffersEXT((n), (fbos))
#  define glDeleteFramebuffersEXT(n, fbos)	bzDeleteFramebuffersEXT((n), (fbos))
#  define glGenRenderbuffersEXT(n, rbos)	bzGenRenderbuffersEXT((n), (rbos))
#  define glDeleteRenderbuffersEXT(n, rbos)	bzDeleteRenderbuffersEXT((n), (rbos))
#  define glCreateShader(type)			bzCreateShader(type)
#  define glDeleteShader(id)			bzDeleteShader(id)
#  define glCreateProgram()			bzCreateProgram()
#  define glDeleteProgram(id)			bzDeleteProgram(id)
#  define glGenQueries(n, queries)		bzGenQueries((n), (queries))
#  define glDeleteQueries(n, queries)		bzDeleteQueries((n), (queries))
//#define DEBUG_GL_MATRIX_STACKS
#  ifdef DEBUG
#    define glNewList(list,mode)		bzNewList((list), (mode))
#    define glGenLists(count)			bzGenLists((count))
#    define glGenTextures(count, textures)	bzGenTextures((count), (textures))
#    ifdef DEBUG_GL_MATRIX_STACKS
#      define glPushMatrix()			bzPushMatrix()
#      define glPopMatrix()			bzPopMatrix()
#      define glMatrixMode(mode)			bzMatrixMode(mode)
#    endif // DEBUG_GL_MATRIX_STACKS
#  endif

// these are housed in OpenGLContext.cxx
extern void   bzNewList(GLuint list, GLenum mode);
extern GLuint bzGenLists(GLsizei count);
extern void   bzGenTextures(GLsizei count, GLuint *textures);
extern void   bzDeleteLists(GLuint base, GLsizei count);
extern void   bzDeleteTextures(GLsizei count, const GLuint *textures);
extern void   bzGenFramebuffersEXT(GLsizei n, GLuint* fbos);
extern void   bzDeleteFramebuffersEXT(GLsizei n, const GLuint* fbos);
extern void   bzGenRenderbuffersEXT(GLsizei n, GLuint* rbos);
extern void   bzDeleteRenderbuffersEXT(GLsizei n, const GLuint* rbos);
extern GLuint bzCreateShader(GLenum type);
extern void   bzDeleteShader(GLuint shader);
extern GLuint bzCreateProgram();
extern void   bzDeleteProgram(GLuint program);
extern void   bzGenQueries(GLsizei n, GLuint* queries);
extern void   bzDeleteQueries(GLsizei n, const GLuint* queries);
extern void   bzMatrixMode(GLenum mode);
extern void   bzPushMatrix();
extern void   bzPopMatrix();

#endif // UNSAFE_GL_CONTEXT

#endif /* __BZFGL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
