/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* SDLDisplay:
 *	Encapsulates a windows display
 */

#ifndef BZF_SDLDISPLAY_H
#define	BZF_SDLDISPLAY_H

#include "bzfSDL.h"
#include "BzfDisplay.h"
#include "BzfVisual.h"
#include "BzfWindow.h"
#include "BzfEvent.h"

class SDLDisplay : public BzfDisplay {
 public:
  SDLDisplay();
  ~SDLDisplay();
  bool isValid() const {return true;};
  bool isEventPending() const;
  bool getEvent(BzfEvent&) const;
  bool getKey(const SDL_Event& sdlEvent, BzfKeyEvent& key) const;
  void setFullscreen();
  void setWindowSize(int width, int height);
  void getWindowSize(int& width, int& height) const;
  void doSetVideoMode();
  void makeCurrent();
 private:
  bool fullScreen;
  bool doSetResolution(int) {return true;};
  int  base_width;
  int  base_height;
  int  min_width;
  int  min_height;
  int  x;
  int  y;
};

class SDLVisual : public BzfVisual {
 public:
  SDLVisual(const SDLDisplay*) { ;};
  void setLevel(int) {;};
  void setDoubleBuffer(bool);
  void setIndex(int) {;};
  void setRGBA(int minRed, int minGreen,
	       int minBlue, int minAlpha);
  void setDepth(int minDepth);
  void setStencil(int minDepth);
  void setAccum(int, int, int, int) {;};
  void setStereo(bool);
  void setMultisample(int) {;};
  bool build() {return true;};
};

class SDLWindow : public BzfWindow {
 public:
  SDLWindow(const SDLDisplay* _display, SDLVisual*);
  bool  isValid() const {return true;};
  void  showWindow(bool) {;};
  void  getPosition(int &, int &) {;};
  void  getSize(int& width, int& height) const;
  void  setSize(int width, int height);
  void  setTitle(const char * title);
  void  setPosition(int, int) {;};
  void  setMinSize(int, int) {;};
  void  setFullscreen();
  void  warpMouse(int x, int y);
  void  getMouse(int& x, int& y) const;
  void  grabMouse();
  void  ungrabMouse();
  void  showMouse() {;};
  void  hideMouse() {;};
  void  setGamma(float newGamma);
  float getGamma() const;
  bool  hasGammaControl() const;
  void  makeCurrent();
  void  swapBuffers();
  void  makeContext() {;};
  void  freeContext() {;};
 private:
  int          x;
  int          y;
  bool         hasGamma;
  float        gamma;
};

#endif // BZF_SDLDISPLAY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
