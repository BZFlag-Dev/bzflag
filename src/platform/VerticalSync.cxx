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


#if defined(__APPLE__)
	#  if defined(HAVE_CGLGETCURRENTCONTEXT)
	bool verticalSyncAvailable() { return true; }
	#  else
	bool verticalSyncAvailable() { return false; }
	#  endif
#elif defined(_WIN32) // WIN32
	#  include <GL/wglew.h>
	bool verticalSyncAvailable() { return (wglSwapIntervalEXT != NULL); }
#else // GLX
	#  include <GL/glxew.h>
	bool verticalSyncAvailable() { return (glXSwapIntervalSGI != NULL); }
#endif


//////////////////////
#if defined(__APPLE__)
//////////////////////

#  if !defined(HAVE_CGLGETCURRENTCONTEXT)

void verticalSync() { return; }

#  else

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

#  endif // !defined HAVE_CGLGETCURRENTCONTEXT

//////////////////////////////
#elif defined(_WIN32) // WIN32
//////////////////////////////

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


////////////
#else // GLX
////////////

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


//////
#endif
//////


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
