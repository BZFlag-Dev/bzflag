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

		bool			isValid() const;

		void			getPosition(float& x, float& y) const;
		unsigned long	getButtons() const;

	private:
		void			poll();

	private:
		bool			status;
		int						fd;
		struct hid_item*	hids;
		char*			data_buf;
		int						data_buf_size;
		int						data_buf_offset;
		int						num_axis;

		int						axis[MAX_AXIS];
		int						axisZero[MAX_AXIS];
		int						axisScale[MAX_AXIS];
		unsigned long	buttons;
};

static usb_joystick*		stick = NULL;

usb_joystick::usb_joystick(const char *name) : data_buf(NULL)
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
		fd = -1;
		return;
	}

	data_buf_size = hid_report_size(rd, hid_input, &report_id);
	if ((data_buf = new char[data_buf_size]) == NULL) {
		hid_dispose_report_desc(rd);
		close(fd);
		fd = -1;
		return;
	}
	data_buf_offset = (report_id != 0);

	int is_joystick = 0;
	int interesting_hid = false;
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

		interesting_hid = true;
		if (page == HUP_GENERIC_DESKTOP) {
			int which_axis;
			switch (usage) {
				case HUG_X:
				case HUG_RX: which_axis = 0; break;
				case HUG_Y:
				case HUG_RY: which_axis = 1; break;
				case HUG_Z:
				case HUG_RZ: which_axis = 2; break;
				default: interesting_hid = false;
			}
			if (interesting_hid) {
				axisZero[which_axis]  = 0.5f * (h.logical_maximum + h.logical_minimum);
				axisScale[which_axis] = 2.0f / (h.logical_maximum - h.logical_minimum);
				axis[which_axis]      = axisZero[which_axis];
				if (num_axis < (which_axis + 1))
					num_axis = which_axis + 1;
			}
		}
		if (interesting_hid) {
			struct hid_item *newhid = new struct hid_item;
			if (newhid == NULL) {
				close(fd);
				fd = -1;
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

usb_joystick::~usb_joystick()
{
	if (fd != -1)
		close(fd);
	delete[] data_buf;
	while (hids != NULL) {
		struct hid_item* next = hids->next;
		delete hids;
		hids = next;
	}
}

bool					usb_joystick::isValid() const
{
	return status;
}

void					usb_joystick::getPosition(float& x, float& y) const
{
	poll();
	x = axisScale[0] * (stick->axis[0] + axisZero[0]);
	y = axisScale[1] * (stick->axis[1] + axisZero[1]);
}

unsigned long			usb_joystick::getButtons() const
{
	poll();
	return buttons;
}

void					usb_joystick::poll()
{
/*
 * The device will buffer a lot of deltas. This can lead to a lot of
 * latency. To avoid this, we will empty the buffer every time.
 * It's possible the device may report only changed entries, so we
 * must process all of the frames to avoid dropping buttons (for example).
 */

	int len;
	while ((len = read(fd, data_buf, data_buf_size)) == data_buf_size) {
		struct hid_item *h;

		for (h = hids ; h != NULL; h = h->next) {
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
			}
			else if (page == HUP_BUTTON) {
				buttons &= ~ (1 << (usage - 1));
				buttons |= (d == h->logical_maximum) ? (1 << (usage - 1)) : 0;
			}
		}
	}
}

void					XWindow::initJoystick(const char *joystickName)
{
	stick = new usb_joystick(joystickName);
	if (!stick->isValid()) {
		delete stick;
		stick = NULL;
	}
}

bool					XWindow::joystick() const
{
	return (stick != NULL);
}

void					XWindow::getJoystick(float &x, float &y) const
{
	assert(stick != NULL);
	stick->getPosition(x, y);
}

unsigned long			XWindow::getJoyButtons() const
{
	assert(stick != NULL);
	return stick->getButtons();
}

#endif

// ex: shiftwidth=4 tabstop=4
