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
#include "SDLMedia.h"
#include "SDLDisplay.h"
#include "SDLJoystick.h"
#else
#include "WinMedia.h"
#include "WinDisplay.h"
#include "WinVisual.h"
#include "WinWindow.h"
#include "WinJoystick.h"
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

#ifdef HAVE_SDL
BzfDisplay*		WinPlatformFactory::createDisplay(
				const char*, const char*)
{
  SDLDisplay* display = new SDLDisplay();
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}
#else
BzfDisplay*		WinPlatformFactory::createDisplay(
				const char* name, const char* videoFormat)
{
  WinDisplay* display = new WinDisplay(name, videoFormat);
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}
#endif

#ifdef HAVE_SDL
BzfVisual*		WinPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  return new SDLVisual((const SDLDisplay*)display);
}
#else
BzfVisual*		WinPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  return new WinVisual((const WinDisplay*)display);
}
#endif

#ifdef HAVE_SDL
BzfWindow*		WinPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  window = new SDLWindow((const SDLDisplay*)display, (SDLVisual*)visual);
  return window;
}
#else
BzfWindow*		WinPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  window = new WinWindow((const WinDisplay*)display, (WinVisual*)visual);
  return window;
}
#endif

BzfMedia*		WinPlatformFactory::createMedia()
{
#ifdef HAVE_SDL
  return new SDLMedia();
#else
  return new WinMedia(window);
#endif
}

BzfJoystick*		WinPlatformFactory::createJoystick()
{
#ifdef HAVE_SDL
  return new SDLJoystick();
#else
  return new WinJoystick();
#endif
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

