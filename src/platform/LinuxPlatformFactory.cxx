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

#include "LinuxPlatformFactory.h"
#include "LinuxDisplay.h"
#ifdef HAVE_SDL
#include "SDLMedia.h"
#include "SDLDisplay.h"
#else
#include "XVisual.h"
#include "XWindow.h"
#include "LinuxMedia.h"
#endif

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new LinuxPlatformFactory;
  return instance;
}

LinuxPlatformFactory::LinuxPlatformFactory()
{
  // do nothing
}

LinuxPlatformFactory::~LinuxPlatformFactory()
{
  // do nothing
}

BzfDisplay*		LinuxPlatformFactory::createDisplay(
#ifdef HAVE_SDL
				const char* , const char*)
#else
				const char* name, const char*)
#endif
{
#ifdef HAVE_SDL
  SDLDisplay* display = new SDLDisplay();
#else
  XDisplay* display = new XDisplay(name, new LinuxDisplayMode);
#endif
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual*		LinuxPlatformFactory::createVisual(
				const BzfDisplay* display)
{
#ifdef HAVE_SDL
  return new SDLVisual((const SDLDisplay*)display);
#else
  return new XVisual((const XDisplay*)display);
#endif
}

BzfWindow*		LinuxPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
#ifdef HAVE_SDL
  return new SDLWindow((const SDLDisplay*)display, (SDLVisual*)visual);
#else
  return new XWindow((const XDisplay*)display, (XVisual*)visual);
#endif
}

BzfMedia*		LinuxPlatformFactory::createMedia()
{
#ifdef HAVE_SDL
  return new SDLMedia;
#else
  return new LinuxMedia;
#endif
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

