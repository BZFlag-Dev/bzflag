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

#include "WinPlatformFactory.h"
#ifdef HAVE_SDL
#include "SDLDisplay.h"
#include "SDLMedia.h"
#else
#include "WinDisplay.h"
#include "WinVisual.h"
#include "WinWindow.h"
#include "WinMedia.h"
#endif

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new WinPlatformFactory;
  return instance;
}

#ifdef HAVE_SDL
SDLWindow*		WinPlatformFactory::window = NULL;
#else
WinWindow*		WinPlatformFactory::window = NULL;
#endif

WinPlatformFactory::WinPlatformFactory()
{
  // do nothing
}

WinPlatformFactory::~WinPlatformFactory()
{
  // do nothing
}

BzfDisplay*		WinPlatformFactory::createDisplay(
#ifdef HAVE_SDL
				const char*, const char*)
{
  SDLDisplay* display = new SDLDisplay();
#else
				const char* name, const char* videoFormat)
{
  WinDisplay* display = new WinDisplay(name, videoFormat);
#endif
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual*		WinPlatformFactory::createVisual(
				const BzfDisplay* display)
{
#ifdef HAVE_SDL
  return new SDLVisual((const SDLDisplay*)display);
#else
  return new WinVisual((const WinDisplay*)display);
#endif
}

BzfWindow*		WinPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
#ifdef HAVE_SDL
  window = new SDLWindow((const SDLDisplay*)display, (SDLVisual*)visual);
#else
  window = new WinWindow((const WinDisplay*)display, (WinVisual*)visual);
#endif
  return window;
}

BzfMedia*		WinPlatformFactory::createMedia()
{
#ifdef HAVE_SDL
  return new SDLMedia();
#else
  return new WinMedia(window);
#endif
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

