/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include <iostream>


int SDLWindow::mx = 0;
int SDLWindow::my = 0;


SDLWindow::SDLWindow(const SDLDisplay* _display, SDLVisual*)
  : BzfWindow(_display), width(-1), height(-1), hasGamma(true), iconified(false)
{
}


void SDLWindow::setTitle(const char * title) {
  SDL_WM_SetCaption(title, title);
}


void SDLWindow::setFullscreen(bool on) {
  ((SDLDisplay *)getDisplay())->setFullscreen(on);
}


bool SDLWindow::getFullscreen() const {
  return ((SDLDisplay *)getDisplay())->getFullscreen();
}


void SDLWindow::deiconify() {
  std::cout << "deiconifying" << std::endl;
  iconified = false;

  create();

  if (width != -1 && height != -1) {
    std::cout << "setting the size to " << width << "x" << height << " with fullscreen set to " << getFullscreen() << std::endl;
    setSize(width, height);
    callResizeCallbacks();
  }

  callExposeCallbacks();
}


void SDLWindow::iconify() {
  int x, y;
  std::cout << "iconify toggle" << std::endl;

  if (iconified) {
    deiconify();
    return;
  }

  /* get out of fullscreen so we can unmap */
  if (getFullscreen()) {
    setFullscreen(false);
    callResizeCallbacks();
  }

  getSize(width, height);
  std::cout << "got size " << width << "x" << height << " with fullscreen set to " << getFullscreen() << std::endl;

  setSize(32, 32);
  getSize(x, y);
  std::cout << "after 32 set, got size " << x << "x" << y << " with fullscreen set to " << getFullscreen() << std::endl;

  SDL_WM_IconifyWindow();
  iconified = true;
}


void SDLWindow::warpMouse(int x, int y) {
  SDL_WarpMouse(x, y);
}


void SDLWindow::getMouse(int& x, int& y) const {
  x = mx;
  y = my;
}


void SDLWindow::getPosition(int& x, int& y) {

// NOTE: the ifdef pattern is taken from SDL_syswm.h
// Mac OS X supports several ways of hardware accelerated rendering, including X11 and Quartz (default)

// FIXME: It is probably better to define what renderer should be used at configure time than to guess at compile time
#if defined(SDL_VIDEO_DRIVER_QUARTZ)
  x = 0;
  y = 0;
#elif defined(SDL_VIDEO_DRIVER_X11)
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (!SDL_GetWMInfo(&info)) {
    x = 0;
    y = 0;
    return;
  }
  info.info.x11.lock_func();
  Display* dpy = info.info.x11.display;
  Window   wnd = info.info.x11.window;
  Window dummy;
  XWindowAttributes attrs;
  XSync(dpy, false);
  XGetWindowAttributes(dpy, wnd, &attrs);
  XTranslateCoordinates(dpy, wnd, attrs.root, 0, 0, &x, &y, &dummy);
  info.info.x11.unlock_func();
#elif defined(SDL_VIDEO_DRIVER_NANOX)
  x = 0;
  y = 0;
#elif defined(SDL_VIDEO_DRIVER_WINDIB) || defined(SDL_VIDEO_DRIVER_DDRAW) || defined(SDL_VIDEO_DRIVER_GAPI)
  SDL_SysWMinfo pInfo;
  SDL_VERSION(&pInfo.version);
  SDL_GetWMInfo(&pInfo);
  RECT r;
  GetWindowRect(pInfo.window, &r);
  x = r.left;
  y = r.bottom;
#elif defined(SDL_VIDEO_DRIVER_RISCOS)
  x = 0;
  y = 0;
#elif defined(SDL_VIDEO_DRIVER_PHOTON)
  x = 0;
  y = 0;
#else // UNKNOWN
  x = 0;
  y = 0;
#endif // video driver type
}


void SDLWindow::setSize(int _width, int _height) {
  ((SDLDisplay *)getDisplay())->setWindowSize(_width, _height);
}


void SDLWindow::getSize(int& _width, int& _height) const {
  ((SDLDisplay *)getDisplay())->getWindowSize(_width, _height);
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
