/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "WinPlatformFactory.h"
#include "WinDisplay.h"
#include "WinVisual.h"
#include "WinWindow.h"
#include "WinMedia.h"

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new WinPlatformFactory;
  return instance;
}

WinWindow*		WinPlatformFactory::window = NULL;

WinPlatformFactory::WinPlatformFactory()
{
  // do nothing
}

WinPlatformFactory::~WinPlatformFactory()
{
  // do nothing
}

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

BzfVisual*		WinPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  return new WinVisual((const WinDisplay*)display);
}

BzfWindow*		WinPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  window = new WinWindow((const WinDisplay*)display, (WinVisual*)visual);
  return window;
}

BzfMedia*		WinPlatformFactory::createMedia()
{
  return new WinMedia(window);
}
