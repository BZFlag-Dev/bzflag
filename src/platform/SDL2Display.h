/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDLDisplay:
 *	Encapsulates an SDL display
 */

#ifndef BZF_SDLDISPLAY_H
#define	BZF_SDLDISPLAY_H

// Inherits from
#include "BzfDisplay.h"

// common includes
#include "bzfSDL.h"
#include "BzfEvent.h"

// system interface headers
#include <map>

class SDLDisplay : public BzfDisplay {
 public:
  SDLDisplay();
  ~SDLDisplay();
  virtual bool isValid() const {return true;};
  bool isEventPending() const;
  bool getEvent(BzfEvent&) const;
  bool peekEvent(BzfEvent&) const;
  bool getKey(const SDL_Event& sdlEvent, BzfKeyEvent& key, const char asciiText = '\0') const;
  void getWindowSize(int& width, int& height);
  bool hasGetKeyMode() {return true;};
  void getModState(bool &shift, bool &control, bool &alt);
  void getMouse(int& x, int& y) const;
 private:
  bool setupEvent(BzfEvent&, const SDL_Event&) const;
  bool doSetResolution(int) {return true;};
  int  min_width;
  int  min_height;
  int  x;
  int  y;
};

#endif // BZF_SDLDISPLAY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
