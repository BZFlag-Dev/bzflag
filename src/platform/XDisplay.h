/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* XDisplay:
 *	Encapsulates an X windows display
 */

#ifndef BZF_XDISPLAY_H
#define	BZF_XDISPLAY_H

#include "BzfDisplay.h"
#include <X11/Xlib.h>

#ifdef XIJOYSTICK
#include <X11/extensions/XInput.h>
#endif

class BzfString;
class BzfKeyEvent;
class XDisplayMode;

class XDisplay : public BzfDisplay {
  public:
			XDisplay(const char* displayName,
				XDisplayMode* adoptedVideoModeChanger = NULL);
			~XDisplay();

    boolean		isValid() const;
    boolean		isEventPending() const;
    boolean		getEvent(BzfEvent&) const;

    // for other X stuff
    class Rep {
      public:
			Rep(const char*);
			~Rep();

	void		ref();
	void		unref();

	Display*	getDisplay() const { return display; }
	int		getScreen() const { return screen; }
	Window		getRootWindow() const;

#ifdef XIJOYSTICK
	XDeviceInfo*	getDevices() const { return devices; }
	int		getNDevices() const { return ndevices; }
	void		setButtonPressType(int& type) {
			  buttonPressType = type;
			}
	void		setButtonReleaseType(int& type) {
			  buttonReleaseType = type;
			}
	int		getButtonPressType() const
				{ return buttonPressType; }
	int		getButtonReleaseType() const
				{ return buttonReleaseType; }
	int		mapButton(int button) const;
#endif

      private:
	int		refCount;
	Display*	display;
	int		screen;
#ifdef XIJOYSTICK
	XDeviceInfo*	devices;
	int		ndevices;
	int		buttonPressType;
	int		buttonReleaseType;
#endif
    };
    Rep*		getRep() const { return rep; }

  private:
    // not implemented
			XDisplay(const XDisplay&);
    XDisplay&		operator=(const XDisplay&);

    boolean		getKey(const XEvent&, BzfKeyEvent&) const;

    boolean		doSetResolution(int);
    boolean		doSetDefaultResolution();

  private:
    Rep*		rep;
    XDisplayMode*	mode;
};

class XDisplayMode {
  public:
    typedef XDisplay::ResInfo ResInfo;

			XDisplayMode();
    virtual		~XDisplayMode();

    // override to return the available display modes, how many there
    // are (num), and which one is the current mode (current).  return
    // NULL if mode switching isn't available (in which case num and
    // current are ignored).
    virtual ResInfo**	init(XDisplay* owner, int& num, int& current);

    // set the display mode to modeNumber (an index into the list
    // returned by init().  return True iff successful.
    virtual boolean	set(int modeNumber);

    // similar to set() except mode is to be treated as the default
    // mode.  default implementation calls set().
    virtual boolean	setDefault(int modeNumber);
};

#endif // BZF_XDISPLAY_H
// ex: shiftwidth=2 tabstop=8
