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

#include "SDLPlatformFactory.h"
#include "SDLMedia.h"
#include "SDLDisplay.h"
#include "SDLJoystick.h"

PlatformFactory* PlatformFactory::getInstance()
{
  if (!instance)
    instance = new SdlPlatformFactory;
  return instance;
}

SdlPlatformFactory::SdlPlatformFactory()
{
  // do nothing
}

SdlPlatformFactory::~SdlPlatformFactory()
{
  // do nothing
}

BzfDisplay* SdlPlatformFactory::createDisplay(const char*, const char*)
{
  SDLDisplay* display = new SDLDisplay();
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual* SdlPlatformFactory::createVisual(const BzfDisplay* display)
{
  return new SDLVisual((const SDLDisplay*)display);
}

BzfWindow* SdlPlatformFactory::createWindow(const BzfDisplay* display,
					    BzfVisual* visual)
{
  return new SDLWindow((const SDLDisplay*)display, (SDLVisual*)visual);
}

BzfMedia* SdlPlatformFactory::createMedia()
{
  return new SDLMedia;
}

BzfJoystick* SdlPlatformFactory::createJoystick()
{
  return new SDLJoystick;
}
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

