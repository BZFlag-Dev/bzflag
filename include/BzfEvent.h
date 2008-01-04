/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

			/* Arrows + Home/End pad */
			Home,
			End,
			Left,
			Right,
			Up,
			Down,
			PageUp,
			PageDown,
			Insert,
			/* Ascii character */
			Backspace,
			Delete,
			/* Numeric keypad */
			Kp0,
			Kp1,
			Kp2,
			Kp3,
			Kp4,
			Kp5,
			Kp6,
			Kp7,
			Kp8,
			Kp9,
			Kp_Period,
			Kp_Divide,
			Kp_Multiply,
			Kp_Minus,
			Kp_Plus,
			Kp_Enter,
			Kp_Equals,
			/* Function keys */
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
			/* Miscellaneous function keys */
			Help,
			Print,
			Sysreq,
			Break,
			Menu,
			Power,
			Euro,
			Undo,
			/* Mouse buttons */
			LeftMouse,
			MiddleMouse,
			RightMouse,
			WheelUp,
			WheelDown,
			MouseButton6,
			MouseButton7,
			MouseButton8,
			MouseButton9,
			MouseButton10,
			/* Joystick buttons */
			BZ_Button_1,
			BZ_Button_2,
			BZ_Button_3,
			BZ_Button_4,
			BZ_Button_5,
			BZ_Button_6,
			BZ_Button_7,
			BZ_Button_8,
			BZ_Button_9,
			BZ_Button_10,
			BZ_Button_11,
			BZ_Button_12,
			BZ_Button_13,
			BZ_Button_14,
			BZ_Button_15,
			BZ_Button_16,
			BZ_Button_17,
			BZ_Button_18,
			BZ_Button_19,
			BZ_Button_20,
			BZ_Button_21,
			BZ_Button_22,
			BZ_Button_23,
			BZ_Button_24,
			BZ_Button_25,
			BZ_Button_26,
			BZ_Button_27,
			BZ_Button_28,
			BZ_Button_29,
			BZ_Button_30,
			BZ_Button_31,
			BZ_Button_32,
			BZ_Hatswitch_1_up,
			BZ_Hatswitch_1_right,
			BZ_Hatswitch_1_down,
			BZ_Hatswitch_1_left,
			BZ_Hatswitch_2_up,
			BZ_Hatswitch_2_right,
			BZ_Hatswitch_2_down,
			BZ_Hatswitch_2_left,
			LastButton  // special marker that must be last
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
			Unset,
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
    BzfEvent() :
	type(Unset),
	window((BzfWindow*)NULL)
    {
	mouseMove.x=mouseMove.y=resize.width=resize.height=keyUp.ascii=keyUp.button=keyUp.shift=0;
    }
};

#endif /* BZF_EVENT_H */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
