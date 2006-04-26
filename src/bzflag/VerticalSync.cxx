/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "VerticalSync.h"

// common headers
#include "bzfgl.h"
#include "StateDatabase.h"


#ifndef HAVE_GLEW
void verticalSync() { return; }
#else

#ifndef unix
void verticalSync() { return; }
#else

#include <GL/glxew.h>

void verticalSync()
{
  if (GLXEW_SGI_video_sync) {
    const GLuint skips = BZDB.evalInt("vsync");
    if (skips > 0) {
      GLuint frameCount;
      if (glXGetVideoSyncSGI(&frameCount) == 0) {
        const int div = skips + 1;
        const int rem = (frameCount + skips) % div;
        glXWaitVideoSyncSGI(div, rem, &frameCount);
      }
    }
  }
  return;
}


#endif // unix
#endif // HAVE_GLEW


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
