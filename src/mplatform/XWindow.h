/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* XWindow:
 *	OpenGL X window.
 */

#ifndef BZF_XWINDOW_H
#define BZF_XWINDOW_H

#include "BzfWindow.h"
#include "XDisplay.h"
#include <X11/Xlib.h>
#include <GL/glx.h>

#ifdef XIJOYSTICK
#include <X11/extensions/XInput.h>
#endif

#ifdef USBJOYSTICK
#ifdef __cplusplus
/* Argh! usb.h has a structure with a member "class". We don't use it, so
 * let's just move it out of the way
 */
#define class CLASS
extern "C" {
#endif
#ifdef __FreeBSD__
#include <libusb.h>
#else
#include <usb.h>
#endif
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>
#ifdef __cplusplus
#undef class
}
#endif
#endif

class XVisual;

class XWindow : public BzfWindow {
public:
	XWindow(const XDisplay*, XVisual*);
	~XWindow();

	bool				isValid() const;

	void				showWindow(bool);

	// set the cached window size.  returns true if the size changed.
	// note that this does *not* reshape the window, it just records
	// the window's shape.
	bool				updateShape(int x, int y, int w, int h);

	// these report the last shape set via updateShape().  they do not
	// query the server for the current size because of how slow that is.
	void				getPosition(int& x, int& y);
	void				getSize(int& width, int& height) const;

	void				setTitle(const char*);
	void				setPosition(int x, int y);
	void				setSize(int width, int height);
	void				setMinSize(int width, int height);
	void				setFullscreen();

	void				warpMouse(int x, int y);
	void				getMouse(int& x, int& y) const;
	void				grabMouse();
	void				ungrabMouse();
	void				showMouse();
	void				hideMouse();

	void				setGamma(float);
	float				getGamma() const;
	bool				hasGammaControl() const;

	void				makeCurrent();
	void				swapBuffers();
	void				makeContext();
	void				freeContext();

#if defined(USBJOYSTICK) || defined(XIJOYSTICK)
    void				initJoystick(const char* joystickName);
    bool            	joystick() const;
    void				getJoystick(float& x, float& y) const;
    unsigned long		getJoyButtons() const;
#endif

    // other X stuff
    static XWindow*		lookupWindow(Window);

    static void			reactivateAll();
    static void			deactivateAll();

private:
	void				loadColormap();
	unsigned short		getIntensityValue(float i) const;
	static float		pixelField(int i, int bits, int offset);
	static void			countBits(unsigned long mask, int& num, int& offset);

private:
	XDisplay::Rep*		display;
	Window				window;
	Colormap			colormap;
	GLXContext			context;
	bool				noWM;
	bool				defaultColormap;
	XWindow*			prev;
	XWindow*			next;
	XVisualInfo			visual;
	unsigned long*		colormapPixels;
	float				gammaVal;
	XSizeHints			sizeHints;
	static XWindow*		first;

	// shadow state to avoid queries to X server
	int					xWindow, yWindow;
	int					wWindow, hWindow;

#ifdef XIJOYSTICK
    XDevice*			device;
    float				xZero, xScale;
    float				yZero, yScale;
#endif
};

#endif // BZF_XWINDOW_H
