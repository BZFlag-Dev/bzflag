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

#include "SGIPlatformFactory.h"
#include "SGIDisplay.h"
#include "XVisual.h"
#include "XWindow.h"
#include "SGIMedia.h"

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new SGIPlatformFactory;
  return instance;
}

SGIPlatformFactory::SGIPlatformFactory()
{
  // do nothing
}

SGIPlatformFactory::~SGIPlatformFactory()
{
  // do nothing
}

BzfDisplay*		SGIPlatformFactory::createDisplay(
				const char* name, const char*)
{
  XDisplay* display = new XDisplay(name, new SGIDisplayMode);
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual*		SGIPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  return new XVisual((const XDisplay*)display);
}

BzfWindow*		SGIPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  return new XWindow((const XDisplay*)display, (XVisual*)visual);
}

BzfMedia*		SGIPlatformFactory::createMedia()
{
  return new SGIMedia;
}
