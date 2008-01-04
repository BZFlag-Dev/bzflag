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

/* SDLWindow:
 *	Encapsulates an SDL window
 */

#ifndef __SDLWINDOW_H__
#define	__SDLWINDOW_H__

/* interface headers */
#include "BzfWindow.h"
#include "SDLDisplay.h"
#include "SDLVisual.h"

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
  void  setFullscreen(bool);
  void  iconify(void);
  void  warpMouse(int x, int y);
  void  getMouse(int& x, int& y) const;
  void  grabMouse() {;};
  void  ungrabMouse() {;};
  void  enableGrabMouse(bool);
  void  showMouse() {;};
  void  hideMouse() {;};
  void  setGamma(float newGamma);
  float getGamma() const;
  bool  hasGammaControl() const;
  void  makeCurrent() {;};
  void  swapBuffers();
  void  makeContext() {;};
  void  freeContext() {;};
  bool  create(void);
 private:
  int	  x;
  int	  y;
  bool	 hasGamma;
};

#endif // __SDLWINDOW_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
