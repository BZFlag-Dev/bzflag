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

#include "XDisplay.h"
#include "XWindow.h"
#include "BzfEvent.h"
#include <string.h>
#include <X11/keysym.h>

#ifdef XIJOYSTICK
#include <stdlib.h>
static int	ioErrorHandler(Display*)
{
  abort();
  return 0;
}
#endif

//
// XDisplay::Rep
//

XDisplay::Rep::Rep(const char* displayName) :
				refCount(1),
				display(NULL),
				screen(0)
#ifdef XIJOYSTICK
				,devices(NULL)
#endif
{
  // open display
  display = XOpenDisplay(displayName);
  if (!display) return;

#ifdef XIJOYSTICK
  int dummy;
  if (XQueryExtension(display, "XInputExtension", &dummy, &dummy, &dummy)) {
    devices = XListInputDevices(display, &ndevices);
    XSetIOErrorHandler(ioErrorHandler);
  }
#endif

  // other initialization
  screen = DefaultScreen(display);
}

XDisplay::Rep::~Rep()
{
#ifdef XIJOYSTICK
  if (devices)
    XFreeDeviceList(devices);
#endif
  if (display) XCloseDisplay(display);
}

void			XDisplay::Rep::ref()
{
  refCount++;
}

void			XDisplay::Rep::unref()
{
  if (--refCount <= 0) delete this;
}

Window			XDisplay::Rep::getRootWindow() const
{
  return display ? RootWindow(display, screen) : None;
}

#ifdef XIJOYSTICK
int			XDisplay::Rep::mapButton(int button) const
{
  static const int map[] = { BzfKeyEvent::LeftMouse,
				BzfKeyEvent::MiddleMouse,
				BzfKeyEvent::RightMouse,
				BzfKeyEvent::F1,
				BzfKeyEvent::F2,
				BzfKeyEvent::F3,
				BzfKeyEvent::F4,
				BzfKeyEvent::F5,
				BzfKeyEvent::F6,
				BzfKeyEvent::F7,
				BzfKeyEvent::F8,
				BzfKeyEvent::F9
			};
  if (button < 1 || button > 12)
    return BzfKeyEvent::NoButton;
  return map[button];
}
#endif

//
// XDisplay
//

XDisplay::XDisplay(const char* displayName, XDisplayMode* _mode) :
				rep(NULL),
				mode(_mode)
{
  // open display
  rep = new Rep(displayName);

  // make default mode changer if one wasn't supplied
  if (!mode)
    mode = new XDisplayMode;

  if (rep->getDisplay()) {
    // get resolutions
    int numModes, currentMode;
    ResInfo** resInfo = mode->init(this, numModes, currentMode);

    // if no modes then make default
    if (!resInfo) {
      resInfo = new ResInfo*[1];
      resInfo[0] = new ResInfo("default",
			DisplayWidth(rep->getDisplay(), rep->getScreen()),
			DisplayHeight(rep->getDisplay(), rep->getScreen()), 0);
      numModes = 1;
      currentMode = 0;
    }

    // register modes
    initResolutions(resInfo, numModes, currentMode);
  }
}

XDisplay::~XDisplay()
{
  setDefaultResolution();
  delete mode;
  rep->unref();
}

boolean			XDisplay::isValid() const
{
  return rep->getDisplay() != NULL;
}

boolean			XDisplay::isEventPending() const
{
  return (XPending(rep->getDisplay()) != 0);
}

boolean			XDisplay::getEvent(BzfEvent& event) const
{
  XEvent xevent;
  XNextEvent(rep->getDisplay(), &xevent);

  switch (xevent.type) {
    case Expose:
    case ConfigureNotify:
    case MotionNotify:
    case MapNotify:
    case UnmapNotify:
    case ButtonPress:
    case ButtonRelease:
    case KeyPress:
    case KeyRelease:
    case ClientMessage:
      event.window = XWindow::lookupWindow(xevent.xexpose.window);
      if (!event.window) return False;
      break;

    default:
#ifdef XIJOYSTICK
      if ((xevent.type != rep->getButtonPressType()) &&
	  (xevent.type != rep->getButtonReleaseType()))
#endif
	return False;
  }

  switch (xevent.type) {
    case Expose:
      if (xevent.xexpose.count != 0) return False;
      event.type = BzfEvent::Redraw;
      break;

    case ConfigureNotify: {
/* attempt to filter out non-size changes, but getSize() returns the
 * current size so it always matches the size in the event.
      int width, height;
      event.window->getSize(width, height);
      if (width == xevent.xconfigure.width &&
	  height == xevent.xconfigure.height)
	return False;
*/
      event.type = BzfEvent::Resize;
      event.resize.width = xevent.xconfigure.width;
      event.resize.height = xevent.xconfigure.height;
      break;
    }

    case MotionNotify:
      event.type = BzfEvent::MouseMove;
      event.mouseMove.x = xevent.xmotion.x;
      event.mouseMove.y = xevent.xmotion.y;
      break;

    case MapNotify:
      event.type = BzfEvent::Map;
      break;

    case UnmapNotify:
      event.type = BzfEvent::Unmap;
      break;

    case ButtonPress:
      event.type = BzfEvent::KeyDown;
      event.keyDown.ascii = 0;
      event.keyDown.shift = 0;
      switch (xevent.xbutton.button) {
	case Button1: event.keyDown.button = BzfKeyEvent::LeftMouse; break;
	case Button2: event.keyDown.button = BzfKeyEvent::MiddleMouse; break;
	case Button3: event.keyDown.button = BzfKeyEvent::RightMouse; break;
	default:      return False;
      }
      break;

    case ButtonRelease:
      event.type = BzfEvent::KeyUp;
      event.keyUp.ascii = 0;
      event.keyUp.shift = 0;
      switch (xevent.xbutton.button) {
	case Button1: event.keyUp.button = BzfKeyEvent::LeftMouse; break;
	case Button2: event.keyUp.button = BzfKeyEvent::MiddleMouse; break;
	case Button3: event.keyUp.button = BzfKeyEvent::RightMouse; break;
	default:      return False;
      }
      break;

    case KeyPress:
      event.type = BzfEvent::KeyDown;
      if (!getKey(xevent, event.keyDown)) return False;
      break;

    case KeyRelease:
      event.type = BzfEvent::KeyUp;
      if (!getKey(xevent, event.keyUp)) return False;
      break;

    case ClientMessage: {
      XClientMessageEvent* cme = (XClientMessageEvent*)&xevent;
      if (cme->format == 32) {
	if ((Atom)cme->data.l[0] == XInternAtom(rep->getDisplay(),
					"WM_DELETE_WINDOW", True)) {
	  event.type = BzfEvent::Quit;
	  break;
	}
      }
      return False;
    }

#ifdef XIJOYSTICK
    default:
      if (xevent.type == rep->getButtonPressType()) {
	XDeviceButtonEvent *button = (XDeviceButtonEvent*) &xevent;
	event.type = BzfEvent::KeyDown;
	event.keyDown.ascii = 0;
	event.keyDown.shift = 0;
	event.keyDown.button = rep->mapButton(button->button);
	if (event.keyDown.button == BzfKeyEvent::NoButton)
  	return False;
      } else if (xevent.type == rep->getButtonReleaseType()) {
	XDeviceButtonEvent *button = (XDeviceButtonEvent*) &xevent;
	event.type = BzfEvent::KeyUp;
	event.keyUp.ascii = 0;
	event.keyUp.shift = 0;
	event.keyUp.button = rep->mapButton(button->button);
	if (event.keyUp.button == BzfKeyEvent::NoButton)
  	return False;
      }
#endif
  }

  return True;
}

boolean			XDisplay::getKey(const XEvent& xevent,
						BzfKeyEvent& key) const
{
  char buf[3];
  KeySym keysym;
  if (XLookupString((XKeyEvent*)&xevent.xkey, buf, 1, &keysym, NULL) == 1) {
    key.ascii = buf[0];
    key.button = BzfKeyEvent::NoButton;

    if (keysym == XK_Delete) {
      key.ascii = 0;
      key.button = BzfKeyEvent::Delete;
    }
  }
  else {
    key.ascii = 0;
    switch (keysym) {
      case XK_Pause:	key.button = BzfKeyEvent::Pause; break;
      case XK_Home:	key.button = BzfKeyEvent::Home; break;
      case XK_End:	key.button = BzfKeyEvent::End; break;
      case XK_Left:	key.button = BzfKeyEvent::Left; break;
      case XK_Right:	key.button = BzfKeyEvent::Right; break;
      case XK_Up:	key.button = BzfKeyEvent::Up; break;
      case XK_Down:	key.button = BzfKeyEvent::Down; break;
      case XK_Page_Up:	key.button = BzfKeyEvent::PageUp; break;
      case XK_Page_Down: key.button = BzfKeyEvent::PageDown; break;
      case XK_Insert:	key.button = BzfKeyEvent::Insert; break;
      case XK_Delete:	key.button = BzfKeyEvent::Delete; break;
      case XK_F1:	key.button = BzfKeyEvent::F1; break;
      case XK_F2:	key.button = BzfKeyEvent::F2; break;
      case XK_F3:	key.button = BzfKeyEvent::F3; break;
      case XK_F4:	key.button = BzfKeyEvent::F4; break;
      case XK_F5:	key.button = BzfKeyEvent::F5; break;
      case XK_F6:	key.button = BzfKeyEvent::F6; break;
      case XK_F7:	key.button = BzfKeyEvent::F7; break;
      case XK_F8:	key.button = BzfKeyEvent::F8; break;
      case XK_F9:	key.button = BzfKeyEvent::F9; break;
      case XK_F10:	key.button = BzfKeyEvent::F10; break;
      case XK_F11:	key.button = BzfKeyEvent::F11; break;
      case XK_F12:	key.button = BzfKeyEvent::F12; break;
      default:		return False;
    }
  }

  key.shift = 0;
  if (xevent.xkey.state & ShiftMask) key.shift |= BzfKeyEvent::ShiftKey;
  if (xevent.xkey.state & ControlMask) key.shift |= BzfKeyEvent::ControlKey;
  if (xevent.xkey.state & Mod1Mask) key.shift |= BzfKeyEvent::AltKey;
  return True;
}

boolean			XDisplay::doSetResolution(int modeIndex)
{
  return mode->set(modeIndex);
}

boolean			XDisplay::doSetDefaultResolution()
{
  return mode->setDefault(getDefaultResolution());
}

//
// XDisplayMode
//

XDisplayMode::XDisplayMode()
{
  // do nothing
}

XDisplayMode::~XDisplayMode()
{
  // do nothing
}

XDisplayMode::ResInfo**	XDisplayMode::init(XDisplay*, int&, int&)
{
  // no switching
  return NULL;
}

boolean			XDisplayMode::set(int)
{
  // no switching
  return False;
}

boolean			XDisplayMode::setDefault(int mode)
{
  return set(mode);
}
