/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BzfEvent:
 *	Abstract, platform independent base for OpenGL windows.
 */

#ifndef BZF_EVENT_H
#define	BZF_EVENT_H

#include "common.h"

class BzfWindow;

class BzfQuitEvent {
  public:
};

class BzfMotionEvent {
  public:
    int			x;
    int			y;
};

class BzfResizeEvent {
  public:
    int			width;
    int			height;
};

class BzfMapEvent {
  public:
};

class BzfUnmapEvent {
  public:
};

class BzfKeyEvent {
  public:
    enum Button {
			NoButton = 0,
			Pause,
			Home,
			End,
			Left,
			Right,
			Up,
			Down,
			PageUp,
			PageDown,
			Insert,
			Delete,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			LeftMouse,
			MiddleMouse,
			RightMouse
    };
    enum {
			ShiftKey = 1,
			ControlKey = 2,
			AltKey = 4
    };

    char		ascii;
    int			button;
    int			shift;
};

class BzfEvent {
  public:
    enum Type {
			Quit,
			Redraw,
			Resize,
			Map,
			Unmap,
			MouseMove,
			KeyUp,
			KeyDown
    };

    Type		type;
    BzfWindow*		window;
    union {
      public:
	BzfQuitEvent	quit;
	BzfResizeEvent	resize;
	BzfMotionEvent	mouseMove;
	BzfMapEvent	map;
	BzfUnmapEvent	unmap;
	BzfKeyEvent	keyUp;
	BzfKeyEvent	keyDown;
    };
};

#endif /* BZF_EVENT_H */
