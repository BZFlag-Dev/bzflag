/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "BZDBCache.h"

bool verticalSyncAvailable() {
#if !defined HAVE_GLEW || (!defined HAVE_CGLGETCURRENTCONTEXT && defined __APPLE__)
  return false;
#elif defined WIN32
  if (wglSwapIntervalEXT) {
    return true;
  } else {
    return false;
  }
#elif defined HAVE_CGLGETCURRENTCONTEXT && defined __APPLE__ // !WIN32
  return true;
#else // !WIN32 && !__APPLE__
  if (glXSwapIntervalSGI) {
    return true;
  } else {
    return false;
  }
#endif
}

//---------------------------------------------------------------------------------
#if !defined HAVE_GLEW || (!defined HAVE_CGLGETCURRENTCONTEXT && defined __APPLE__)
void verticalSync() { return; }
#else
//---------------------------------------------------------------------------------


////////////
#ifdef WIN32
////////////


#include <GL/wglew.h>

static int oldVSync = -1; // -1 means "use the system setting"

void verticalSync() {
  if (BZDBCache::vsync == oldVSync) {
    return;
  }
  oldVSync = BZDBCache::vsync;

  // FIXME -- needs to be updated during context switches?
  if (wglSwapIntervalEXT) {
    wglSwapIntervalEXT(BZDBCache::vsync);
  }  
}


///////////////
#else // !WIN32
///////////////

////////////////
#ifdef __APPLE__
////////////////


static int oldVSync = -2; // -1 means "use the system setting", -2 is unitialized

void verticalSync() {
  if (BZDBCache::vsync == oldVSync) {
    return;
  }
  oldVSync = BZDBCache::vsync;

  CGLContextObj cglContext = CGLGetCurrentContext();
  GLint newSwapInterval = BZDBCache::vsync;
  
  if (newSwapInterval < 0) {
    newSwapInterval = 0;
  }
  // FIXME -- needs to be updated during context switches!
  // FiXME -- this seems to cause a bit flicker at the right top
  // in fullcreen mode while menu is opened (small horizontal lines)
  CGLSetParameter(cglContext, kCGLCPSwapInterval, &newSwapInterval); 
}

/////////////////
#else // !WIN32 && !__APPLE
/////////////////

#include <GL/glxew.h>

// NOTE: glXSwapIntervalSGI() does not let you turn off vsync

static int oldVSync = -1; // -1 means "use the system setting"

void verticalSync() {
  if (BZDBCache::vsync == oldVSync) {
    return;
  }
  oldVSync = BZDBCache::vsync;

  // FIXME -- needs to be updated during context switches?
  if (glXSwapIntervalSGI) {
    glXSwapIntervalSGI(BZDBCache::vsync);
  }  
}



/*
void verticalSync()
{
  if (GLXEW_SGI_video_sync) {
    const int vsync = BZDBCache::vsync;
    if (vsync > 0) {
      GLuint frameCount;
      if (glXGetVideoSyncSGI(&frameCount) == 0) {
        glXWaitVideoSyncSGI(vsync, (frameCount % vsync), &frameCount);
      }
    }
  }
  return;
}
*/

//////////////////
#endif // __APPLE
//////////////////


///////////////
#endif // WIN32
///////////////


//-----------------
#endif // HAVE_GLEW
//-----------------


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
