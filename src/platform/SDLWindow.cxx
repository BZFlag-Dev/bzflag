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

#include "common.h"

/* interface header */
#include "SDLWindow.h"


extern int mx, my; /* from SDLDisplay.cxx */

SDLWindow::SDLWindow(const SDLDisplay* _display, SDLVisual*)
  : BzfWindow(_display), x(-1), y(-1), hasGamma(true)
{
}

void SDLWindow::setTitle(const char * title) {
  SDL_WM_SetCaption(title, title);
}

void SDLWindow::setFullscreen(bool on) {
  ((SDLDisplay *)getDisplay())->setFullscreen(on);
}

void SDLWindow::iconify(void) {
  SDL_WM_IconifyWindow();
}

void SDLWindow::warpMouse(int _x, int _y) {
  SDL_WarpMouse(_x, _y);
}

void SDLWindow::getMouse(int& _x, int& _y) const {
  _x = mx;
  _y = my;
}

void SDLWindow::setSize(int width, int height) {
  ((SDLDisplay *)getDisplay())->setWindowSize(width, height);
}

void SDLWindow::getSize(int& width, int& height) const {
  ((SDLDisplay *)getDisplay())->getWindowSize(width, height);
}

void SDLWindow::setGamma(float gamma) {
  int result = SDL_SetGamma(gamma, gamma, gamma);
  if (result == -1) {
    printf("Could not set Gamma: %s.\n", SDL_GetError());
    hasGamma = false;
  }
}

// Code taken from SDL (not available through the headers)
static float CalculateGammaFromRamp(Uint16 ramp[256]) {
  /* The following is adapted from a post by Garrett Bass on OpenGL
     Gamedev list, March 4, 2000.
  */
  float sum = 0.0;
  int count = 0;

  float gamma = 1.0;
  for (int i = 1; i < 256; ++i) {
    if ((ramp[i] != 0) && (ramp[i] != 65535)) {
      double B = (double)i / 256.0;
      double A = ramp[i] / 65535.0;
      sum += (float) (log(A) / log(B));
      count++;
    }
  }
  if ( count && sum ) {
    gamma = 1.0f / (sum / count);
  }
  return gamma;
}

float SDLWindow::getGamma() const {
  Uint16 redRamp[256];
  Uint16 greenRamp[256];
  Uint16 blueRamp[256];
  float gamma = 1.0;
  int result = SDL_GetGammaRamp(redRamp, greenRamp, blueRamp);
  if (result == -1) {
    printf("Could not get Gamma: %s.\n", SDL_GetError());
  } else {
    float red   = CalculateGammaFromRamp(redRamp);
    float green = CalculateGammaFromRamp(greenRamp);
    float blue  = CalculateGammaFromRamp(blueRamp);
    gamma = (red + green + blue) / 3.0;
  }
  return gamma;
}

bool SDLWindow::hasGammaControl() const {
  return hasGamma;
}

void SDLWindow::swapBuffers() {
  SDL_GL_SwapBuffers();
}

bool SDLWindow::create(void) {
  if (!((SDLDisplay *)getDisplay())->createWindow()) {
    return false;
  }
  return true;
}

void SDLWindow::enableGrabMouse(bool on) {
  ((SDLDisplay *)getDisplay())->enableGrabMouse(on);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
