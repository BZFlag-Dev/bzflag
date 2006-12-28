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

#ifdef _MSC_VER
#pragma warning(4:4800)
#endif

WinJoystick::WinJoystick() : inited(false)
{
}

WinJoystick::~WinJoystick()
{
}

void	      WinJoystick::initJoystick(const char* joystickName)
{
  inited = false;

  if (!strcasecmp(joystickName, "off") || !strcmp(joystickName, "")) {
    return;
  }

  if (strlen(joystickName) < 11) {
    printError("Invalid joystick name.");
    return;
  }

  if (joystickName[9] == '1')
    JoystickID = JOYSTICKID1;
  else if (joystickName[9] == '2')
    JoystickID = JOYSTICKID2;

  JOYINFO joyInfo;
  JOYCAPS joyCaps;
  MMRESULT result1 = joyGetPos(JoystickID, &joyInfo);
  MMRESULT result2 = joyGetDevCaps(JoystickID, &joyCaps, sizeof(joyCaps));
  if ((result1 != JOYERR_NOERROR) || (result2 != JOYERR_NOERROR)) {
    printError("Unable to initialize joystick.  Perhaps it is not plugged in?");
    return;
  }

  axes.clear();

  AxisInfo x;
  x.name = "X";
  x.exists = true;
  x.requestFlag = JOY_RETURNX;
  x.min = (float)joyCaps.wXmin;
  x.max = (float)joyCaps.wXmax;
  axes.push_back(x);

  AxisInfo y;
  y.name = "Y";
  y.exists = true;
  y.requestFlag = JOY_RETURNY;
  y.min = (float)joyCaps.wYmin;
  y.max = (float)joyCaps.wYmax;
  axes.push_back(y);

  AxisInfo z;
  z.name = "Z";
  z.exists = (bool)(joyCaps.wCaps & JOYCAPS_HASZ);
  z.requestFlag = JOY_RETURNZ;
  z.min = (float)joyCaps.wZmin;
  z.max = (float)joyCaps.wZmax;
  axes.push_back(z);

  AxisInfo r;
  r.name = "R";
  r.exists = (bool)(joyCaps.wCaps & JOYCAPS_HASR);
  r.requestFlag = JOY_RETURNR;
  r.min = (float)joyCaps.wRmin;
  r.max = (float)joyCaps.wRmax;
  axes.push_back(r);

  AxisInfo u;
  u.name = "U";
  u.exists = (bool)(joyCaps.wCaps & JOYCAPS_HASU);
  u.requestFlag = JOY_RETURNU;
  u.min = (float)joyCaps.wUmin;
  u.max = (float)joyCaps.wUmax;
  axes.push_back(u);

  AxisInfo v;
  v.name = "V";
  v.exists = (bool)(joyCaps.wCaps & JOYCAPS_HASV);
  v.requestFlag = JOY_RETURNV;
  v.min = (float)joyCaps.wVmin;
  v.max = (float)joyCaps.wVmax;
  axes.push_back(v);

  xIndex = 0;
  yIndex = 1;

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
  joyInfo.dwFlags = axes[xIndex].requestFlag | axes[yIndex].requestFlag;
  joyInfo.dwSize = sizeof(joyInfo);

  // check for errors
  MMRESULT result = joyGetPosEx(JoystickID, &joyInfo);
  if (result != JOYERR_NOERROR) {
    printError("Could not get extended info from joystick");
    return;
  }

  // get the information we asked for out of the structure
  DWORD xRet = 0;
  DWORD yRet = 0;
  switch (axes[xIndex].requestFlag) {
    case JOY_RETURNX: xRet = joyInfo.dwXpos; break;
    case JOY_RETURNY: xRet = joyInfo.dwYpos; break;
    case JOY_RETURNZ: xRet = joyInfo.dwZpos; break;
    case JOY_RETURNR: xRet = joyInfo.dwRpos; break;
    case JOY_RETURNU: xRet = joyInfo.dwUpos; break;
    case JOY_RETURNV: xRet = joyInfo.dwVpos; break;
    default: printError("Failed to get Joystick X axis information"); break;
  }
  switch (axes[yIndex].requestFlag) {
    case JOY_RETURNX: yRet = joyInfo.dwXpos; break;
    case JOY_RETURNY: yRet = joyInfo.dwYpos; break;
    case JOY_RETURNZ: yRet = joyInfo.dwZpos; break;
    case JOY_RETURNR: yRet = joyInfo.dwRpos; break;
    case JOY_RETURNU: yRet = joyInfo.dwUpos; break;
    case JOY_RETURNV: yRet = joyInfo.dwVpos; break;
    default: printError("Failed to get Joystick Y axis information"); break;
  }

  // adjust X and Y to scale
  x = (int)((xRet / axes[xIndex].max) * 2000 - 1000);
  y = (int)((yRet / axes[yIndex].max) * 2000 - 1000);

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
  if (retbuts & JOY_BUTTON1)  buttons = buttons | 0x00000001;
  if (retbuts & JOY_BUTTON2)  buttons = buttons | 0x00000002;
  if (retbuts & JOY_BUTTON3)  buttons = buttons | 0x00000004;
  if (retbuts & JOY_BUTTON4)  buttons = buttons | 0x00000008;
  if (retbuts & JOY_BUTTON5)  buttons = buttons | 0x00000010;
  if (retbuts & JOY_BUTTON6)  buttons = buttons | 0x00000020;
  if (retbuts & JOY_BUTTON7)  buttons = buttons | 0x00000040;
  if (retbuts & JOY_BUTTON8)  buttons = buttons | 0x00000080;
  if (retbuts & JOY_BUTTON9)  buttons = buttons | 0x00000100;
  if (retbuts & JOY_BUTTON10) buttons = buttons | 0x00000200;
  if (retbuts & JOY_BUTTON11) buttons = buttons | 0x00000400;
  if (retbuts & JOY_BUTTON12) buttons = buttons | 0x00000800;
  if (retbuts & JOY_BUTTON13) buttons = buttons | 0x00001000;
  if (retbuts & JOY_BUTTON14) buttons = buttons | 0x00002000;
  if (retbuts & JOY_BUTTON15) buttons = buttons | 0x00004000;
  if (retbuts & JOY_BUTTON16) buttons = buttons | 0x00008000;
  if (retbuts & JOY_BUTTON17) buttons = buttons | 0x00010000;
  if (retbuts & JOY_BUTTON18) buttons = buttons | 0x00020000;
  if (retbuts & JOY_BUTTON19) buttons = buttons | 0x00040000;
  if (retbuts & JOY_BUTTON20) buttons = buttons | 0x00080000;
  if (retbuts & JOY_BUTTON21) buttons = buttons | 0x00100000;
  if (retbuts & JOY_BUTTON22) buttons = buttons | 0x00200000;
  if (retbuts & JOY_BUTTON23) buttons = buttons | 0x00400000;
  if (retbuts & JOY_BUTTON24) buttons = buttons | 0x00800000;
  if (retbuts & JOY_BUTTON25) buttons = buttons | 0x01000000;
  if (retbuts & JOY_BUTTON26) buttons = buttons | 0x02000000;
  if (retbuts & JOY_BUTTON27) buttons = buttons | 0x04000000;
  if (retbuts & JOY_BUTTON28) buttons = buttons | 0x08000000;
  if (retbuts & JOY_BUTTON29) buttons = buttons | 0x10000000;
  if (retbuts & JOY_BUTTON30) buttons = buttons | 0x20000000;
  if (retbuts & JOY_BUTTON31) buttons = buttons | 0x40000000;
  if (retbuts & JOY_BUTTON32) buttons = buttons | 0x80000000;
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

void	      WinJoystick::getJoyDeviceAxes(std::vector<std::string> &list) const
{
  list.clear();
  if (!inited)
    return;

  for (unsigned int i = 0; i < axes.size(); ++i) {
    if (axes[i].exists)
      list.push_back(axes[i].name);
  }
}

void	      WinJoystick::setXAxis(const std::string axis)
{
  if (!inited)
    return;

  for (unsigned int i = 0; i < axes.size(); ++i) {
    if (axes[i].exists) {
      if (axes[i].name == axis) {
	xIndex = i;
	break;
      }
    }
  }
}

void	      WinJoystick::setYAxis(const std::string axis)
{
  if (!inited)
    return;

  for (unsigned int i = 0; i < axes.size(); ++i) {
    if (axes[i].exists) {
      if (axes[i].name == axis) {
	yIndex = i;
	break;
      }
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
