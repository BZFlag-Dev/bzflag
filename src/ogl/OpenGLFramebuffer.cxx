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

#include "common.h"

// interface header
#include "OpenGLFramebuffer.h"

// system headers
#include <map>
#include <string>

// common headers
#include "OpenGLGState.h"

#ifndef __APPLE__
// GL_ARB_framebuffer_object functions (these need to be loaded by the
// platform's OpenGL context initialization code)
PFNGLISRENDERBUFFERPROC glIsRenderbuffer = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;

PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample =
  NULL;

PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = NULL;

PFNGLISFRAMEBUFFERPROC glIsFramebuffer = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;

PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;

PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = NULL;

PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;

PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC
  glGetFramebufferAttachmentParameteriv = NULL;

PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = NULL;

PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
#endif // __APPLE__

void OpenGLFramebuffer::initFramebuffer() {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenRenderbuffers(1, &renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaLevel, GL_RGB,
				   width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			    GL_RENDERBUFFER, renderbuffer);

  glGenRenderbuffers(1, &depthRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaLevel,
				   GL_DEPTH24_STENCIL8,
				   width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			    GL_RENDERBUFFER, depthRenderbuffer);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Multisample framebuffer incomplete.\n");

  glBindFramebuffer(GL_FRAMEBUFFER, 0); // break the binding
}

void OpenGLFramebuffer::destroyFramebuffer() {
  glDeleteRenderbuffers(1, &renderbuffer);
  glDeleteRenderbuffers(1, &depthRenderbuffer);
  glDeleteFramebuffers(1, &framebuffer);
}

OpenGLFramebuffer::OpenGLFramebuffer() : contextActive(false), msaaLevel(1),
                                         width(1), height(1),
                                         renderbuffer(0), depthRenderbuffer(0),
					 framebuffer(0) {
  OpenGLGState::registerContextInitializer(freeContext, initContext,
					   (void*)this);
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
  OpenGLGState::unregisterContextInitializer(freeContext, initContext,
					     (void*)this);
}

void OpenGLFramebuffer::checkState(int newWidth, int newHeight,
				   int newMSAALevel) {
  if(width != newWidth || height != newHeight || msaaLevel != newMSAALevel) {
    width = newWidth;
    height = newHeight;
    msaaLevel = newMSAALevel;

    if(contextActive) {
      destroyFramebuffer();
      initFramebuffer();
    }
  }
}


void OpenGLFramebuffer::freeContext(void* self)
{
  if(OpenGLGState::getMaxSamples() == 1)
    return;

  if(! ((OpenGLFramebuffer*)self)->contextActive)
    return;

  ((OpenGLFramebuffer*)self)->destroyFramebuffer();
  ((OpenGLFramebuffer*)self)->contextActive = false;
}


void OpenGLFramebuffer::initContext(void* self)
{
  if(OpenGLGState::getMaxSamples() == 1)
    return;

  if(((OpenGLFramebuffer*)self)->contextActive)
    freeContext(self);

  ((OpenGLFramebuffer*)self)->initFramebuffer();
  ((OpenGLFramebuffer*)self)->contextActive = true;
}


/*
 * Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
