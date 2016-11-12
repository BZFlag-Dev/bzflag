/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
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
#include "MacGL.h"

// system headers
#include <AppKit/Appkit.h>

// common headers
#include "bzfSDL.h"

bool getCurrentMacOpenGLContext(CGLContextObj* cglContext)
{
#ifdef HAVE_SDL2
  NSOpenGLContext* highLevelContext = (NSOpenGLContext*) SDL_GL_GetCurrentContext();

  if (highLevelContext == NULL) {
    printf("Could not get current SDL OpenGL Context: %s\n", SDL_GetError());
    return false;
  }

  *cglContext = [highLevelContext CGLContextObj];
#else
  *cglContext = CGLGetCurrentContext();
#endif

  return true;
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
