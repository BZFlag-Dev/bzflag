/*
 * USB Joystick support for {Net,Free}BSD.
 *
 * Copyright 2001, Nick Sayer
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * This file was "inspired" by the joy_usb.c file that is part of the
 * xmame project. To the extent that code was copied from that file,
 * it is the author's opinion that distribution under the GPL of the
 * derivative work is allowed.
 *
 */

#ifdef USBJOYSTICK
#include "XWindow.h"
#include <X11/Intrinsic.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

#define MAX_AXIS 3

class usb_joystick {
public:
	usb_joystick(const char *name);
	~usb_joystick();
	void poll();
	int num_axis;
	int axis[MAX_AXIS];
	int axis_scale[MAX_AXIS];
	int axis_const[MAX_AXIS];
	unsigned long buttons;
	boolean status;
private:
	int fd;
	struct hid_item *hids;
	char *data_buf;
	int data_buf_size;
	int data_buf_offset;
};

usb_joystick *stick;

usb_joystick::usb_joystick(const char *name)
{
	report_desc_t rd;
	hid_data *d;
	hid_item h;
	int report_id;

	status = FALSE;
	hids = NULL;
	num_axis = 0;

	if ((fd = open(name, O_RDONLY | O_NONBLOCK))<0)
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
			(h.kind == hid_collection &&
			page == HUP_GENERIC_DESKTOP &&
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
				axis_const[which_axis] = 1000 + (2000*h.logical_maximum)/(h.logical_minimum-h.logical_maximum);
				axis_scale[which_axis] = (2000*10000)/(h.logical_maximum-h.logical_minimum);
				axis[which_axis] = (h.logical_minimum +
					h.logical_maximum) / 2;
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

	status = TRUE;
}
usb_joystick::~usb_joystick() {
	close(fd);
}

void usb_joystick::poll() {
	int len;

/*
 * The device will buffer a lot of deltas. This can lead to a lot of
 * latency. To avoid this, we will empty the buffer every time.
 * It's possible the device may report only changed entries, so we
 * must process all of the frames to avoid dropping buttons (for example).
 */
	while ((len = read(fd, data_buf, data_buf_size)) == data_buf_size) {

		struct hid_item *h;

		for (h = hids ; h; h = h->next) {
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

void XWindow::initJoystick(const char *joystickName)
{
	stick= new usb_joystick(joystickName);
}

boolean XWindow::joystick() const
{
	return (stick!=NULL)?stick->status:FALSE;
}

void XWindow::getJoy(int &x, int &y) const
{
	stick->poll();
	x = (stick->axis[0]*stick->axis_scale[0])/10000 + stick->axis_const[0];
	y = (stick->axis[1]*stick->axis_scale[1])/10000 + stick->axis_const[1];
}

unsigned long XWindow::getJoyButtons() const
{
	stick->poll();
	return stick->buttons;
}

#endif

// ex: shiftwidth=2 tabstop=8
