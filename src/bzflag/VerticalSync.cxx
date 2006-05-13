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


/*********************/
#if defined(HAVE_GLEW)
/*********************/


/////////////////
#if defined(unix)
/////////////////


#include <GL/glxew.h>

void verticalSync()
{
  if (GLXEW_SGI_video_sync) {
    const int vsync = BZDB.evalInt("vsync");
    if (vsync > 0) {
      GLuint frameCount;
      if (glXGetVideoSyncSGI(&frameCount) == 0) {
        glXWaitVideoSyncSGI(vsync, frameCount % vsync, &frameCount);
      }
    }
  }
  return;
}


/////////////////////
#elif defined(_WIN32)
/////////////////////


#include <GL/wglew.h>

void verticalSync()
{
  if (WGLEW_EXT_swap_control) {
    const int vsync = BZDB.evalInt("vsync");
    if (vsync >= 0) {
      const int current = wglGetSwapIntervalEXT();
      if (vsync != current) {
        wglSwapIntervalEXT(vsync);
      }
    }
  }
  return;
}


///////////////////////////
#else // not unix or _WIN32
///////////////////////////


void verticalSync()
{
  return;
}


///////////////////////
#endif // unix / _WIN32
///////////////////////


/*****************/
#else // HAVE_GLEW
/*****************/


void verticalSync()
{
  return;
}


/******************/
#endif // HAVE_GLEW
/******************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
