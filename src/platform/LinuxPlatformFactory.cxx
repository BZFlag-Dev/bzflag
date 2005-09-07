/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "LinuxPlatformFactory.h"
#include "LinuxDisplay.h"
#include "XVisual.h"
#include "XWindow.h"
#include "LinuxMedia.h"
#if defined(USBJOYSTICK)
  #include "USBJoystick.h"
#elif defined(XIJOYSTICK)
  #include "XIJoystick.h"
#else
  #include "EvdevJoystick.h"
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
#if defined(USBJOYSTICK)
  // only works for USB joysticks under *BSD
  return new USBJoystick;
#elif defined(XIJOYSTICK)
  // XInput Joystick
  return new XIJoystick;
#elif defined(HAVE_LINUX_INPUT_H)
  // Event device joystick
  return new EvdevJoystick;
#else
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

