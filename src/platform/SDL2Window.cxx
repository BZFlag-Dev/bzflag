/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Own include
#include "SDL2Window.h"

// Common includes
#include "OpenGLGState.h"

SDLWindow::SDLWindow(const SDLDisplay* _display, SDLVisual*)
  : BzfWindow(_display), hasGamma(true), windowId(NULL), glContext(NULL),
  canGrabMouse(true), fullScreen(false), base_width(640), base_height(480)
{
}

void SDLWindow::setTitle(const char *_title) {
  title = _title;
  if (windowId)
    SDL_SetWindowTitle(windowId, title.c_str());
}

void SDLWindow::setFullscreen(bool on) {
  fullScreen = on;
}

void SDLWindow::iconify(void) {
  SDL_MinimizeWindow(windowId);
}

void SDLWindow::warpMouse(int _x, int _y) {
  SDL_WarpMouseInWindow(windowId, _x, _y);

  // workaround since SDL 2 doesn't (currently) generate a SDL_MOUSEMOTION
  // event if it was warped from outside the window
  SDL_PumpEvents();
  SDL_Event lastEvent;
  if(SDL_PeepEvents(&lastEvent, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0)
    if(lastEvent.type == SDL_MOUSEMOTION)
      return;

  SDL_Event motionEvent;
  memset(&motionEvent, 0, sizeof motionEvent);
  motionEvent.type = SDL_MOUSEMOTION;
  motionEvent.motion.x = _x;
  motionEvent.motion.y = _y;
  SDL_PushEvent(&motionEvent);
}

void SDLWindow::getMouse(int& _x, int& _y) const {
  const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->getMouse(_x, _y);
}

void SDLWindow::setSize(int _width, int _height) {
  base_width  = _width;
  base_height = _height;
  if (!fullScreen && windowId) {
    SDL_SetWindowSize(windowId, base_width, base_height);
  }
}

void SDLWindow::getSize(int& width, int& height) const {
  if (fullScreen) {
    const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->getWindowSize(width, height);
  } else {
    width  = base_width;
    height = base_height;
  }
}

void SDLWindow::setGamma(float gamma) {
  int result = SDL_SetWindowBrightness(windowId, gamma);
  if (result == -1) {
    printf("Could not set Gamma: %s.\n", SDL_GetError());
    hasGamma = false;
  }
}

float SDLWindow::getGamma() const {
  return SDL_GetWindowBrightness(windowId);
}

bool SDLWindow::hasGammaControl() const {
  return hasGamma;
}

void SDLWindow::swapBuffers() {
  SDL_GL_SwapWindow(windowId);
}

bool SDLWindow::create(void) {
  int targetWidth, targetHeight;
  getSize(targetWidth, targetHeight);

  // if we have an existing identical window, go no further
  if(windowId != NULL) {
    int currentWidth, currentHeight;
    SDL_GetWindowSize(windowId, &currentWidth, &currentHeight);

    Uint32 priorWindowFlags = SDL_GetWindowFlags(windowId);
    if(fullScreen == (priorWindowFlags & SDL_WINDOW_FULLSCREEN) &&
    	targetWidth == currentWidth && targetHeight == currentHeight)
      return true;
  }

  // destroy the pre-existing window if it exists
  if(windowId != NULL) {
    if(glContext)
      SDL_GL_DeleteContext(glContext);
    glContext = NULL;

    SDL_DestroyWindow(windowId);
  }

  // (re)create the window
  const Uint32 flags = SDL_WINDOW_OPENGL |
      (fullScreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

  windowId = SDL_CreateWindow(
      title.c_str(),
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      targetWidth,
      targetHeight,
      flags);

  if (!windowId) {
    printf("Could not set Video Mode: %s.\n", SDL_GetError());
    return false;
  }

  makeContext();
  makeCurrent();

  // always disable vsync when building with SDL
  SDL_GL_SetSwapInterval(0);

  // init opengl context
  OpenGLGState::initContext();

  return true;
}

void SDLWindow::enableGrabMouse(bool on) {
  canGrabMouse = on;
  if (canGrabMouse)
    SDL_SetWindowGrab(windowId, SDL_TRUE);
  else
    SDL_SetWindowGrab(windowId, SDL_FALSE);
}

void SDLWindow::makeContext() {
  glContext = SDL_GL_CreateContext(windowId);
  if (!glContext)
    printf("Could not Create GL Context: %s.\n", SDL_GetError());
}

void SDLWindow::makeCurrent() {
  if (!windowId)
    return;
  if (!glContext)
    return;
  int result = SDL_GL_MakeCurrent(windowId, glContext);
  if (result < 0) {
    printf("Could not Make GL Context Current: %s.\n", SDL_GetError());
    abort();
  }
}

void SDLWindow::freeContext() {
  SDL_GL_DeleteContext(glContext);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
