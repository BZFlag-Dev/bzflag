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
#include "XVisual.h"
#include "XWindow.h"
#include "LinuxMedia.h"
#ifdef USBJOYSTICK
  #include "USBJoystick.h"
#else
#include "LinuxJoystick.h"
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
				const char* name, const char*)
{
  XDisplay* display = new XDisplay(name, new LinuxDisplayMode);
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual*		LinuxPlatformFactory::createVisual(
				const BzfDisplay* display)
{
  return new XVisual((const XDisplay*)display);
}

BzfWindow*		LinuxPlatformFactory::createWindow(
				const BzfDisplay* display, BzfVisual* visual)
{
  return new XWindow((const XDisplay*)display, (XVisual*)visual);
}

BzfJoystick*		LinuxPlatformFactory::createJoystick()
{
#ifdef USBJOYSTICK
  // only works for USB joysticks under *BSD
  return new USBJoystick;
#else
  // no joystick
  return new BzfJoystick;
#endif
}

BzfMedia*		LinuxPlatformFactory::createMedia()
{
  return new LinuxMedia;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

