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

/* BeOSDisplay:
 *	Encapsulates an BeOS windows display
 */

#ifndef BZF_BEOSDISPLAY_H
#define BZF_BEOSDISPLAY_H

#include <Screen.h>

#include "BzfDisplay.h"

class BzfKeyEvent;
class Resolution;
class BeOSWindow;

class BeOSDisplay : public BzfDisplay {
public:
	BeOSDisplay(const char* displayName, const char* videoFormat);
	~BeOSDisplay();

	bool				isValid() const;
	bool				isEventPending() const;
	bool				getEvent(BzfEvent&) const;

	void				postBzfEvent(BzfEvent&);

private:
	BeOSDisplay(const BeOSDisplay&);
	BeOSDisplay&			operator=(const BeOSDisplay&);

	bool				doSetResolution(int);
	bool				doSetDefaultResolution();

private:
	BScreen				*bScreen;
	/* Event stuff */
	port_id				eventPort;
public: //XXX
	BeOSWindow			*beosWin;
};

#endif // BZF_BEOSDISPLAY_H
// ex: shiftwidth=4 tabstop=4
