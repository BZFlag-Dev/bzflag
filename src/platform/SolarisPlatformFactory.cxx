/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "SolarisPlatformFactory.h"
#include "XDisplay.h"
#include "XVisual.h"
#include "XWindow.h"
#include "SolarisMedia.h"

PlatformFactory*	PlatformFactory::getInstance()
{
  if (!instance) instance = new SolarisPlatformFactory;
  return instance;
}

SolarisPlatformFactory::SolarisPlatformFactory()
{
  // do nothing
}

SolarisPlatformFactory::~SolarisPlatformFactory()
{
  // do nothing
}

BzfDisplay*		SolarisPlatformFactory::createDisplay(
				const char* name, const char*)
{
  XDisplay* display = new XDisplay(name);
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual*		SolarisPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  return new XVisual((const XDisplay*)display);
}

BzfWindow*		SolarisPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  return new XWindow((const XDisplay*)display, (XVisual*)visual);
}

BzfMedia*		SolarisPlatformFactory::createMedia()
{
  return new SolarisMedia;
}
