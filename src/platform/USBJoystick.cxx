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
#ifndef BSD
#error Native USB Joystick support requires BSD. On other platforms please enable SDL or XIJOYSTICK.
#endif

#include <vector>
#include <string>

// Moved bodily from XWindow.h - hope it still works :)

/* Argh! usb.h has a structure with a member "class". We don't use it, so
 * let's just move it out of the way
 */
#define class CLASS

__BEGIN_DECLS

#ifdef __FreeBSD__
#  include <libusb.h>
#else
#  include <usb.h>
#endif
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>

__END_DECLS

#undef class

#include "USBJoystick.h"


USBJoystick::USBJoystick() : status(false)
{
}

USBJoystick::~USBJoystick()
{
  if (status == true)
    close(fd);
}

void USBJoystick::initJoystick(const char *name)
{
  report_desc_t rd;
  hid_data *d;
  hid_item h;
  int report_id;

  status = false;
  hids = NULL;
  num_axis = 0;

  if ((fd = open(name, O_RDONLY | O_NONBLOCK)) < 0)
    return;

  if ((rd = hid_get_report_desc(fd)) == 0) {
    close(fd);
    return;
  }

  data_buf_size = hid_report_size(rd, hid_input, &report_id);
  if ((data_buf = (char *)malloc(data_buf_size)) == NULL) {
    hid_dispose_report_desc(rd);
  }
  data_buf_offset = (report_id != 0);

  int is_joystick = 0;
  int interesting_hid = FALSE;
  for (d = hid_start_parse(rd, 1 << hid_input); hid_get_item(d, &h); ) {
    int page = HID_PAGE(h.usage);
    int usage = HID_USAGE(h.usage);
    is_joystick = is_joystick ||
      (h.kind == hid_collection && page == HUP_GENERIC_DESKTOP &&
      (usage == HUG_JOYSTICK || usage == HUG_GAME_PAD));

    if (h.kind != hid_input)
      continue;

    if (!is_joystick)
      continue;

    interesting_hid = TRUE;
    if (page == HUP_GENERIC_DESKTOP) {
      int which_axis;
      switch (usage) {
	case HUG_X:
	case HUG_RX: which_axis = 0; break;
	case HUG_Y:
	case HUG_RY: which_axis = 1; break;
	case HUG_Z:
	case HUG_RZ: which_axis = 2; break;
	default: interesting_hid = FALSE;
      }
      if (interesting_hid) {
	axis_const[which_axis] = 1000 + (2000 * h.logical_maximum) /
	  (h.logical_minimum - h.logical_maximum);
	axis_scale[which_axis] = (2000 * 10000) /
	  (h.logical_maximum - h.logical_minimum);
	axis[which_axis] = (h.logical_minimum + h.logical_maximum) / 2;
	if (num_axis < (which_axis + 1))
	  num_axis = which_axis + 1;
      }
    }
    if (interesting_hid) {
      struct hid_item *newhid = new struct hid_item;
      if (newhid == NULL) {
	close(fd);
	return;
      }
      *newhid = h;
      newhid->next = hids;
      hids = newhid;
    }
  }
  hid_end_parse(d);

  status = true;
}

void USBJoystick::poll()
{
  int len;

/*
* The device will buffer a lot of deltas. This can lead to a lot of
* latency. To avoid this, we will empty the buffer every time.
* It's possible the device may report only changed entries, so we
* must process all of the frames to avoid dropping buttons (for example).
*/
  while ((len = read(fd, data_buf, data_buf_size)) == data_buf_size) {

    struct hid_item *h;

    for (h = hids; h; h = h->next) {
      int d = hid_get_data(data_buf + data_buf_offset, h);
      int page = HID_PAGE(h->usage);
      int usage = HID_USAGE(h->usage);

      int which_axis;
      if (page == HUP_GENERIC_DESKTOP) {
	switch (usage) {
	  case HUG_X:
	  case HUG_RX: which_axis = 0; break;
	  case HUG_Y:
	  case HUG_RY: which_axis = 1; break;
	  case HUG_Z:
	  case HUG_RZ: which_axis = 2; break;
	}
	axis[which_axis] = d;
      } else if (page == HUP_BUTTON) {
	buttons &= ~ (1 << (usage - 1));
	buttons |= (d == h->logical_maximum)?1 << (usage - 1):0;
      }
    }
  }

}

bool USBJoystick::joystick() const
{
  return status;
}

void USBJoystick::getJoy(int &x, int &y)
{
  if (status) {
    poll();
    x = (axis[0] * axis_scale[0]) / 10000 + axis_const[0];
    y = (axis[1] * axis_scale[1]) / 10000 + axis_const[1];
  } else {
    x = y = 0;
  }
}

unsigned long USBJoystick::getJoyButtons()
{
  if (status) {
    poll();
    return buttons;
  } else {
    return 0;
  }
}

void USBJoystick::getJoyDevices(std::vector<std::string> &list) const
{
  //FIXME: Not implemented
  list.clear();
}

#endif


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
