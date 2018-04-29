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

// interface header
#include "BzfJoystick.h"

// system headers
#include <string.h>
#include <vector>
#include <string>

// common implementation headers
#include "ErrorHandler.h"

BzfJoystick::BzfJoystick()
{
}

BzfJoystick::~BzfJoystick()
{
}

void			BzfJoystick::initJoystick(const char* joystickName)
{
  if (strcmp(joystickName, "off") && strcmp(joystickName, "")) {
    std::vector<std::string> args;
    args.push_back(joystickName);
    printError("Joystick '{1}' not supported...", &args);
  }
}

int		     BzfJoystick::getNumHats()
{
  return 0;
}

void		    BzfJoystick::getJoyHat(int UNUSED(hat), float &hatX, float &hatY)
{
  hatX = hatY = 0;
}

unsigned long		BzfJoystick::getJoyButtons()
{
  return 0;
}

bool			BzfJoystick::joystick() const
{
  return false;
}

void			BzfJoystick::getJoy(int& x, int& y)
{
  x = y = 0;
}

void		    BzfJoystick::getJoyDevices(std::vector<std::string>
						 &list) const
{
  list.clear();
}

void		    BzfJoystick::getJoyDeviceAxes(std::vector<std::string>
						  &list) const
{
  list.clear();
  list.push_back("default");
}

bool		    BzfJoystick::ffHasRumble() const
{
  return false;
}

void		    BzfJoystick::ffRumble(int, float, float, float, float)
{
}

bool		    BzfJoystick::ffHasDirectional() const
{
  return false;
}

void		    BzfJoystick::ffDirectionalConstant(int, float, float, float, float, float)
{
}

void		    BzfJoystick::ffDirectionalPeriodic(int, float, float, float, float, float, float, PeriodicType)
{
}

void		    BzfJoystick::ffDirectionalResistance(float, float, float, ResistanceType)
{
}

void		    BzfJoystick::setXAxis(const std::string &)
{
}

void		    BzfJoystick::setYAxis(const std::string &)
{
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
