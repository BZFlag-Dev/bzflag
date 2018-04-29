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

#include "WinPlatformFactory.h"
#ifdef HAVE_SDL
#  include "SDL2Display.h"
#  include "SDL2Window.h"
#else
#  include "WinDisplay.h"
#  include "WinVisual.h"
#  include "WinWindow.h"
#endif
#include "DXJoystick.h"
#include "WinMedia.h"
#include "StateDatabase.h"

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new WinPlatformFactory;
  return instance;
}

#ifdef HAVE_SDL
SDLWindow*		WinPlatformFactory::sdlWindow = NULL;
#else
WinWindow*		WinPlatformFactory::winWindow = NULL;
#endif


WinPlatformFactory::WinPlatformFactory()
{
  // do nothing
}

WinPlatformFactory::~WinPlatformFactory()
{
  // do nothing
}

BzfDisplay *WinPlatformFactory::createDisplay(const char* name,
					      const char* videoFormat)
{
  BzfDisplay *display;
#ifdef HAVE_SDL
  SDLDisplay* sdlDisplay = new SDLDisplay();
  display		= sdlDisplay;
#else
  WinDisplay* winDisplay = new WinDisplay(name, videoFormat);
  display		= winDisplay;
#endif
  if (!display || !display->isValid()) {
    delete display;
    display = NULL;
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
  sdlWindow = new SDLWindow((const SDLDisplay*)display, (SDLVisual*)visual);
  return sdlWindow;
#else
  winWindow = new WinWindow((const WinDisplay*)display, (WinVisual*)visual);
  return winWindow;
#endif
}

BzfMedia*		WinPlatformFactory::createMedia()
{
  return new WinMedia();
}

BzfJoystick*		WinPlatformFactory::createJoystick()
{
  return new DXJoystick();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
