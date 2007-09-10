/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDLJoystick:
 *	Encapsulates an SDL joystick
 */

#ifndef BZF_SDLJOY_H
#define	BZF_SDLJOY_H

#include "BzfJoystick.h"
#include "bzfSDL.h"

class SDLJoystick : public BzfJoystick {
  public:
		SDLJoystick();
		~SDLJoystick();

    void	initJoystick(const char* joystickName);
    bool	joystick() const;
    void	getJoy(int& x, int& y);
    unsigned long getJoyButtons();
    void	getJoyDevices(std::vector<std::string> &list) const;
    void	getJoyDeviceAxes(std::vector<std::string> &list) const;
    void	setXAxis(const std::string axis);
    void	setYAxis(const std::string axis);

  private:
    SDL_Joystick		*joystickID;
    int			 joystickButtons;
    int			 xAxis;
    int			 yAxis;
};

#endif // BZF_SDLJOY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
