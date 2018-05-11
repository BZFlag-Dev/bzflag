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

/* OpenGLFramebuffer:
 *	Encapsulates an OpenGL framebuffer object for multisampled rendering.
 */

#ifndef OPENGLFRAMEBUFFER_H
#define OPENGLFRAMEBUFFER_H

#include "common.h"

// common headers
#include "bzfgl.h"

class OpenGLFramebuffer {
private:
  bool contextActive;
  int msaaLevel, width, height;
  GLuint renderbuffer, depthRenderbuffer, framebuffer;

  void initFramebuffer();
  void destroyFramebuffer();

public:
  OpenGLFramebuffer();
  ~OpenGLFramebuffer();

  void checkState(int, int, int);

  GLint getFramebuffer() { return framebuffer; }

  static void freeContext(void*);
  static void initContext(void*);
};

#endif // OPENGLFRAMEBUFFER_H


/*
 * Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
