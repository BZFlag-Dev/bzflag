/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

#ifndef __SDLDISPLAY_H__
#define	__SDLDISPLAY_H__

/* interface headers */
#include "bzfSDL.h"
#include "BzfDisplay.h"
#include "BzfEvent.h"


class SDLDisplay : public BzfDisplay {
 public:
  SDLDisplay();
  ~SDLDisplay();
  bool createWindow();
  virtual bool isValid() const {return true;};
  bool isEventPending() const;
  bool getEvent(BzfEvent&) const;
  bool peekEvent(BzfEvent&) const;
  bool getKey(const SDL_Event& sdlEvent, BzfKeyEvent& key) const;
  void setFullscreen(bool);
  void setWindowSize(int width, int height);
  void getWindowSize(int& width, int& height);
  void doSetVideoMode();
  void enableGrabMouse(bool);
  bool hasGetKeyMode() {return true;};
  void getModState(bool &shift, bool &control, bool &alt);
 private:
  bool setupEvent(BzfEvent&, const SDL_Event&) const;
  bool fullScreen;
  bool doSetResolution(int) {return true;};
  int  base_width;
  int  base_height;
  int  min_width;
  int  min_height;
  int  x;
  int  y;
  bool canGrabMouse;
  // to avoid flashing we memorize the old values used to build the window
  bool oldFullScreen;
  int  oldWidth;
  int  oldHeight;

  int  defaultWidth;
  int  defaultHeight;
};

#endif // __SDLDISPLAY_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
