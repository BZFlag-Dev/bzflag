/*
 * USB Joystick support for {Net,Free}BSD.
 *
 * Copyright 2001, Nick Sayer
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * This file was "inspired" by the joy_usb.c file that is part of the
 * xmame project. To the extent that code was copied from that file,
 * it is the author's opinion that distribution under the GPL of the
 * derivative work is allowed.
 *
 */

#include "common.h"

#ifdef USBJOYSTICK
#include "BzfJoystick.h"
#include <X11/Intrinsic.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>

#include <stdio.h>

#define MAX_AXIS 3

class USBJoystick : public BzfJoystick {
public:
	USBJoystick();
	~USBJoystick();
    void	initJoystick(const char* joystickName);
    bool	joystick() const;
    void	getJoy(int& x, int& y);
    unsigned long getJoyButtons();
    void	getJoyDevices(std::vector<std::string> &list) const;

private:
    void    poll();
    int	    num_axis;
    int	    axis[MAX_AXIS];
    int	    axis_scale[MAX_AXIS];
    int	    axis_const[MAX_AXIS];
    unsigned long buttons;
    bool    status;
    int	    fd;
    struct  hid_item *hids;
    char    *data_buf;
    int	    data_buf_size;
    int	    data_buf_offset;
};

#endif //USBJOYSTICK


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
