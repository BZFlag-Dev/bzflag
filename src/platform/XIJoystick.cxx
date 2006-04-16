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

/* XXX: FIXME: XIJoystick class as it stands is what happens when all the
 * XIJoystick stuff is moved from XWindow/XDisplay to here and cleaned up a bit.
 * It won't work until we can get a pointer to the current X display and window
 * as some functions seem to need them.
 * Will also have to add a callback or something because XInput sends buttonpresses
 * through the main event loop, e.g. to XDisplay.
 */

/* common headers */
#include "common.h"

#ifdef XIJOYSTICK

/* remove these lines once XIJoystick works */
#endif
#if 0
/* end removal */

/* interface header */
#include "XIJoystick.h"

/* system headers */
#include <stdlib.h>
#include <vector>
#include <string>

/* implementation headers */
#include "ErrorHandler.h"

static int	ioErrorHandler(Display*)
{
  abort();
  return 0;
}



XIJoystick::XIJoystick() : device(NULL),
			   devices(NULL),
			   buttonPressType(0),
			   buttonReleaseType(0)
{
  // XXX: FIXME: need pointer to display

  int dummy;
  if (XQueryExtension(display, "XInputExtension", &dummy, &dummy, &dummy)) {
    devices = XListInputDevices(display, &ndevices);
    XSetIOErrorHandler(ioErrorHandler);
  }
}

XIJoystick::~XIJoystick()
{
  if (device)
    XCloseDevice(display, device);
}

void	      XIJoystick::initJoystick(const char* joystickName)
{
  XAnyClassPtr c;
  XValuatorInfo *v = NULL;
  XDeviceInfo *d = devices;

  for (int i = 0; i < ndevices; i++) {
    if (!strcmp(d[i].name, joystickName)) {
      device = XOpenDevice(display, d[i].id);
      d = &d[i];
      c = d->inputclassinfo;
      for (int j = 0; j < d->num_classes; j++) {
	if (c->c_class == ValuatorClass) {
	  v = (XValuatorInfo*)c;
	  break;
	}
	c = (XAnyClassPtr)(((char*)c) + c->length);
      }
      break;
    }
  }

  if (v && v->num_axes >= 2) {
    int maxX = v->axes[0].max_value;
    int minX = v->axes[0].min_value;
    int maxY = v->axes[1].max_value;
    int minY = v->axes[1].min_value;
    scaleX = (2000 * 10000) / (maxX - minX);
    constX = 1000 + (2000 * maxX) / (minX - maxX);
    scaleY = (2000 * 10000) / (maxY - minY);
    constY = 1000 + (2000 * maxY) / (minY - maxY);
  } else {
    XCloseDevice(display, device);
    device = NULL;
  }

  int bPress = -1;
  int bRelease = -1;
  if (device) {
    XEventClass event_list[7];
    int cnt = 0, i;
    XInputClassInfo *ip;
    for (ip = device->classes, i = 0; i < d->num_classes; i++, ip++) {
      switch (ip->input_class) {
	case ButtonClass:
	  DeviceButtonPress(device, bPress, event_list[cnt]); cnt++;
	  DeviceButtonRelease(device, bRelease, event_list[cnt]); cnt++;
	  buttonPressType = bPress;
	  buttonReleaseType = bRelease;
	  break;
      }
    }
    // do we need a window name also?  probably...
    XSelectExtensionEvent(display, window, event_list, cnt);
    printError("Joystick initialized...");
  }
}

bool	      XIJoystick::joystick() const
{
  return (device != NULL);
}

void	      XIJoystick::getJoy(int& x, int& y)
{
  x = y = 0;

  if (!device) return;

  XDeviceState *state = XQueryDeviceState(display, device);
  if (!state) return;

  XInputClass *cls = state->data;
  for (int i = 0; i < state->num_classes; i++) {
    switch (cls->c_class) {
      case ValuatorClass:
	XValuatorState *val = (XValuatorState*)cls;
	if (val->num_valuators >= 2) {
	  x = val->valuators[0];
	  y = val->valuators[1];
	}
	break;
    }
    cls = (XInputClass *) ((char *) cls + cls->length);
  }

  XFreeDeviceState(state);

  x = (x * scaleX) / 10000 + constX;
  y = (y * scaleY) / 10000 + constY;

  // ballistic
  x = (x * abs(x)) / 1000;
  y = (y * abs(y)) / 1000;
}

unsigned long XIJoystick::getJoyButtons()
{
  /* XXX: FIXME: return joystick buttons.
   * These are reported to XDisplay as events, we'll
   * probably have to make XDisplay keep track of them
   * and query it for them, or make XDisplay call back
   * to another function in this class.
   */
}

void	      XIJoystick::getJoyDevices(std::vector<std::string> &list) const
{
  XAnyClassPtr c;
  XValuatorInfo *v = NULL;
  XDeviceInfo *d = devices;
  XDeviceInfo *xd = NULL;

  // look for input devices with 2 or more axes and add them to list
  for (int i = 0; i < ndevices; i++) {
    xd = &d[i];
    c = xd->inputclassinfo;
    for (int j = 0; j < xd->num_classes; j++) {
      if (c->c_class == ValuatorClass) {
	v = (XValuatorInfo*)c;
	if (v && v->num_axes >= 2)
	  list.push_back(sdt::string(xd.name));
      }
    }
  }

}

#endif //XIJOYSTICK

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
