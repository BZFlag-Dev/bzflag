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

/* common headers */
#include "common.h"

/* interface header */
#include "WinJoystick.h"

/* system headers */
#include <mmsystem.h>
#include <vector>
#include <string>

/* implementation headers */
#include "ErrorHandler.h"
#include "TextUtils.h"

WinJoystick::WinJoystick() : inited(false)
{
}

WinJoystick::~WinJoystick()
{
}

void	      WinJoystick::initJoystick(const char* joystickName)
{
  inited = false;

  if (!strcmp(joystickName, "off") || !strcmp(joystickName, "")) {
    return;
  }

  if (strlen(joystickName) < 11) {
    printError("Invalid joystick name.");
    return;
  }

  if (joystickName[10] == '1')
    JoystickID = JOYSTICKID1;
  else if (joystickName[10] == '2')
    JoystickID = JOYSTICKID2;

  JOYINFO joyInfo;
  JOYCAPS joyCaps;
  if ((joyGetPos(JoystickID, &joyInfo) != JOYERR_NOERROR) ||
      (joyGetDevCaps(JoystickID, &joyCaps, sizeof(joyCaps)) != JOYERR_NOERROR)) {
    printError("Unable to initialize joystick.  Perhaps it is not plugged in?");
    return;
  }

  xMin = (float)joyCaps.wXmin;
  xMax = (float)joyCaps.wXmax;
  yMin = (float)joyCaps.wYmin;
  yMax = (float)joyCaps.wYmax;

  inited = true;
}

bool	      WinJoystick::joystick() const
{
  return inited;
}

void	      WinJoystick::getJoy(int& x, int& y)
{
  if (!inited)
    return;

  JOYINFOEX joyInfo;
  // we're only interested in position
  joyInfo.dwFlags = JOY_RETURNX | JOY_RETURNY | JOY_USEDEADZONE;
  joyInfo.dwSize = sizeof(joyInfo);

  // check for errors
  if (joyGetPosEx(JoystickID, &joyInfo) != JOYERR_NOERROR) {
    printError("Could not get extended info from joystick");
    return;
  }

  // adjust X and Y to scale
  x = (int)((joyInfo.dwXpos / xMax) * 2000 - 1000);
  y = (int)((joyInfo.dwYpos / yMax) * 2000 - 1000);

  // ballistic
  x = (x * abs(x)) / 1000;
  y = (y * abs(y)) / 1000;

}

unsigned long WinJoystick::getJoyButtons()
{
  if (!inited)
    return 0;

  JOYINFOEX joyInfo;
  // we're only interested in buttons
  joyInfo.dwFlags = JOY_RETURNBUTTONS;
  joyInfo.dwSize = sizeof(joyInfo);

  // check for errors
  if (joyGetPosEx(JoystickID, &joyInfo) != JOYERR_NOERROR) {
    printError("Could not get extended info from joystick");
    return 0;
  }

  unsigned long retbuts = joyInfo.dwButtons;
  unsigned long buttons = 0;
  if (retbuts & JOY_BUTTON1)  buttons = buttons | 0x00001;
  if (retbuts & JOY_BUTTON2)  buttons = buttons | 0x00002;
  if (retbuts & JOY_BUTTON3)  buttons = buttons | 0x00004;
  if (retbuts & JOY_BUTTON4)  buttons = buttons | 0x00008;
  if (retbuts & JOY_BUTTON5)  buttons = buttons | 0x00010;
  if (retbuts & JOY_BUTTON6)  buttons = buttons | 0x00020;
  if (retbuts & JOY_BUTTON7)  buttons = buttons | 0x00040;
  if (retbuts & JOY_BUTTON8)  buttons = buttons | 0x00080;
  if (retbuts & JOY_BUTTON9)  buttons = buttons | 0x00100;
  if (retbuts & JOY_BUTTON10) buttons = buttons | 0x00200;

  return buttons;
}

void	      WinJoystick::getJoyDevices(std::vector<std::string> &list) const
{
  list.clear();
  if (joyGetNumDevs() != 0) {
    // we have at least one joystick driver, get the name of both joystick IDs if they exist.
    JOYCAPS joyCaps;
    if (joyGetDevCaps(JOYSTICKID1, &joyCaps, sizeof(joyCaps)) == JOYERR_NOERROR) {
      list.push_back(TextUtils::format("Joystick 1 (%s)", joyCaps.szPname));
    }
    if (joyGetDevCaps(JOYSTICKID2, &joyCaps, sizeof(joyCaps)) == JOYERR_NOERROR) {
      list.push_back(TextUtils::format("Joystick 2 (%s)", joyCaps.szPname));
    }
  }
}



// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
