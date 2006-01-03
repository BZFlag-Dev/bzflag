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

/* WinJoystick:
 *	Encapsulates a Windows MultiMedia (non-DirectInput) joystick
 */

#ifndef BZF_WINJOY_H
#define	BZF_WINJOY_H

#include "BzfJoystick.h"
#include <vector>
#include <string>

class WinJoystick : public BzfJoystick {
  public:
		WinJoystick();
		~WinJoystick();

    void	initJoystick(const char* joystickName);
    bool	joystick() const;
    void	getJoy(int& x, int& y);
    unsigned long getJoyButtons();
    void	getJoyDevices(std::vector<std::string> &list) const;
    void	getJoyDeviceAxes(std::vector<std::string> &list) const;
    void	setXAxis(const std::string axis);
    void	setYAxis(const std::string axis);

  private:
    unsigned int JoystickID;
    bool	inited;

    struct AxisInfo {
      std::string name;
      bool exists;
      DWORD requestFlag;
      float min;
      float max;
    };

    std::vector<AxisInfo> axes;

    int xIndex;
    int yIndex;

};

#endif // BZF_WINJOY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
