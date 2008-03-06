/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
  bool				peekEvent(BzfEvent&) const;

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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

