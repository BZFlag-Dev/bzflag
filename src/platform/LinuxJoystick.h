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

/* LinuxJoystick:
 *	Encapsulates a void joystick
 */

#ifndef BZF_LINUXJOY_H
#define	BZF_LINUXJOY_H

#include "BzfJoystick.h"

class LinuxJoystick : public BzfJoystick {
  public:
		LinuxJoystick() {};
		~LinuxJoystick() {};

    void	initJoystick(const char* joystickName) {};
    bool	joystick() const {return false;};
    void	getJoy(int& x, int& y) const {x = 0; y = 0;};
    unsigned long getJoyButtons() const {return 0;};
    void        getJoyDevices(std::vector<std::string> &list) const {};
};

#endif // BZF_LINUXJOY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
