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

#ifndef __BZFGL_H__
#define __BZFGL_H__

/** this file contains headers necessary for opengl */

#ifdef __APPLE__
#  include <OpenGL/OpenGL.h>
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#endif

#ifndef GL_VERSION_1_1
# error OpenGL version 1.1 functionality is required
#endif

/* These will track glBegin/End pairs to make sure that they match */
#ifdef DEBUG
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

#define _GL_INVALID_ID 0xFFFFFFFF

/* disabling list and texture deletions can be useful for debugging
 * corrupted context states.  still don't have a "proper" context
 * manager, but things are working so it can be disabled.
 */
#if 0
#  ifndef glDeleteLists
#    define glDeleteLists(base,count) {}
#  endif
#  ifndef glDeleteTextures
#    define glDeleteTextures(count,textures) {}
#  endif
#endif

#endif /* __BZFGL_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
