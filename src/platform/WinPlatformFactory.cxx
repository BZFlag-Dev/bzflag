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

#include "WinPlatformFactory.h"
#ifdef HAVE_SDL
#include "SDLMedia.h"
#include "SDLDisplay.h"
#include "SDLJoystick.h"
#endif
#include "DXJoystick.h"
#ifdef HAVE_DSOUND_H
#include "WinMedia.h"
#endif
#include "WinDisplay.h"
#include "WinVisual.h"
#include "WinWindow.h"
#include "WinJoystick.h"
#include "StateDatabase.h"

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new WinPlatformFactory;
  return instance;
}

#ifdef HAVE_SDL
SDLWindow*		WinPlatformFactory::sdlWindow = NULL;
#endif
WinWindow*		WinPlatformFactory::winWindow = NULL;

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
  bool useNative = true;
#ifdef HAVE_SDL
  if (BZDB.isSet("SDLVideo") && BZDB.isTrue("SDLVideo"))
    useNative = false;
#endif

  BzfDisplay *display;
  if (useNative) {
    WinDisplay* winDisplay = new WinDisplay(name, videoFormat);
    display		= winDisplay;
  } else {
#ifdef HAVE_SDL
    SDLDisplay* sdlDisplay = new SDLDisplay();
    display		= sdlDisplay;
#else
    display		= NULL;
#endif
  }
  if (!display || !display->isValid()) {
    delete display;
    display = NULL;
  }
  return display;
}

BzfVisual*		WinPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  bool useNative = true;
#ifdef HAVE_SDL
  if (BZDB.isSet("SDLVideo") && BZDB.isTrue("SDLVideo"))
    useNative = false;
#endif

  if (useNative)
    return new WinVisual((const WinDisplay*)display);
  else
#ifdef HAVE_SDL
    return new SDLVisual((const SDLDisplay*)display);
#else
    return NULL;
#endif
}

BzfWindow*		WinPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  bool useNative = true;
#ifdef HAVE_SDL
  if (BZDB.isSet("SDLVideo") && BZDB.isTrue("SDLVideo"))
    useNative = false;
#endif

  if (useNative) {
    winWindow = new WinWindow();
    return winWindow;
  } else {
#ifdef HAVE_SDL
    sdlWindow = new SDLWindow((const SDLDisplay*)display, (SDLVisual*)visual);
    return sdlWindow;
#else
    return NULL;
#endif
  }
}

BzfMedia*		WinPlatformFactory::createMedia()
{
  bool useNative = true;
#ifndef HAVE_DSOUND_H
  useNative = false;
#endif
#ifdef HAVE_SDL
  if (BZDB.isSet("SDLAudio") && BZDB.isTrue("SDLAudio"))
    useNative = false;
#endif

  if (useNative) {
#ifdef HAVE_DSOUND_H
    return new WinMedia(winWindow);
#else
    return NULL;
#endif
  } else {
#ifdef HAVE_SDL
    return new SDLMedia();
#else
    return NULL;
#endif
  }
}

BzfJoystick*		WinPlatformFactory::createJoystick()
{
  bool useNative = true;
#ifdef HAVE_SDL
  if (BZDB.isSet("SDLJoystick") && BZDB.isTrue("SDLJoystick"))
    useNative = false;
#endif
  if (useNative) {
#if defined(USE_DINPUT)
    return new DXJoystick();
#else
    return new WinJoystick();
#endif
  } else {
#if defined(HAVE_SDL)
    return new SDLJoystick();
#else
    return NULL;
#endif
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

