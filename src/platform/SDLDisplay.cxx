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

#include "common.h"

#ifdef HAVE_SDL
#include <stdio.h>
#include "SDLDisplay.h"
#include "OpenGLGState.h"

static int mx = 0;
static int my = 0;

SDLDisplay::SDLDisplay() : fullScreen(false), base_width(640),
			   base_height(480)
{
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
    printf("Could not initialize SDL Video subsystem: %s.\n", SDL_GetError());
    exit (-1);
  };
  SDL_Rect **modeList
    = SDL_ListModes(NULL, SDL_HWSURFACE | SDL_OPENGL | SDL_FULLSCREEN
		    | SDL_HWPALETTE);
  if (!modeList)
    printf("Could not Get available video modes: %s.\n", SDL_GetError());

  int defaultResolutionIndex = 0;
  ResInfo** resolutions;
  int numResolutions = 1;
  // No modes available or All resolutions available
  if ((modeList != (SDL_Rect **) 0) && (modeList != (SDL_Rect **) -1)) {
      for (int i = 1; modeList[i]; i++)
	if ((modeList[i - 1]->w != modeList[i]->w)
	    || (modeList[i - 1]->h != modeList[i]->h))
	  numResolutions++;
  };
  resolutions = new ResInfo*[numResolutions];

  if ((modeList != (SDL_Rect **) 0) && (modeList != (SDL_Rect **) -1)) {
    char name[80];
    int  h;
    int  w;
    int  j = 0; 

    for (int i = 0; modeList[i]; i++) {
      h = modeList[i]->h;
      w = modeList[i]->w;
      if (i != 0)
	if ((modeList[i - 1]->w == w) && (modeList[i - 1]->h == h))
	  continue;
      sprintf(name, "%dx%d    ", w, h);
      resolutions[j] = new ResInfo(name, w, h, 0);
      if (w == 640 && h == 480)
	defaultResolutionIndex = j;
      j++;
    }
  } else {
    // if no modes then make default
    resolutions[0] = new ResInfo ("default", 640, 480, 0);
  }
  
  // register modes
  initResolutions(resolutions, numResolutions, defaultResolutionIndex);

  SDL_EnableUNICODE(1);

  if (SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
			  SDL_DEFAULT_REPEAT_INTERVAL) == -1) {
    printf("Could not EnableKeyRepeat: %s.\n", SDL_GetError());
    exit (-1);
  };
}

SDLDisplay::~SDLDisplay()
{
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
};


bool SDLDisplay::isEventPending() const
{
  return SDL_PollEvent(NULL) == 1;
};

bool SDLDisplay::getEvent(BzfEvent& _event) const
{
  SDL_Event event;
  if (SDL_PollEvent(&event) == 0)
    return false;

  switch (event.type) {

  case SDL_MOUSEMOTION:
    _event.type        = BzfEvent::MouseMove;
    mx                 = event.motion.x;
    my                 = event.motion.y;
    _event.mouseMove.x = event.motion.x;
    _event.mouseMove.y = event.motion.y;
    break;

  case SDL_MOUSEBUTTONDOWN:
    _event.type          = BzfEvent::KeyDown;
    _event.keyDown.ascii = 0;
    _event.keyDown.shift = 0;
    switch (event.button.button) {
    case SDL_BUTTON_LEFT:
      _event.keyDown.button = BzfKeyEvent::LeftMouse;
      break;
    case SDL_BUTTON_MIDDLE:
      _event.keyDown.button = BzfKeyEvent::MiddleMouse;
      break;
    case SDL_BUTTON_RIGHT:
      _event.keyDown.button = BzfKeyEvent::RightMouse;
      break;
    case SDL_BUTTON_WHEELUP:
      _event.keyDown.button = BzfKeyEvent::WheelUp;
      break;
    case SDL_BUTTON_WHEELDOWN:
      _event.keyDown.button = BzfKeyEvent::WheelDown;
      break;
    case 6:
      _event.keyDown.button = BzfKeyEvent::MouseButton6;
      break;
    case 7:
      _event.keyDown.button = BzfKeyEvent::MouseButton7;
      break;
    case 8:
      _event.keyDown.button = BzfKeyEvent::MouseButton8;
      break;
    case 9:
      _event.keyDown.button = BzfKeyEvent::MouseButton9;
      break;
    case 10:
      _event.keyDown.button = BzfKeyEvent::MouseButton10;
      break;
    default:
      return false;
    }
    break;

  case SDL_MOUSEBUTTONUP:
    _event.type = BzfEvent::KeyUp;
    _event.keyUp.ascii = 0;
    _event.keyUp.shift = 0;
    switch (event.button.button) {
    case SDL_BUTTON_LEFT:
      _event.keyDown.button = BzfKeyEvent::LeftMouse;
      break;
    case SDL_BUTTON_MIDDLE:
      _event.keyDown.button = BzfKeyEvent::MiddleMouse;
      break;
    case SDL_BUTTON_RIGHT:
      _event.keyDown.button = BzfKeyEvent::RightMouse;
      break;
    case SDL_BUTTON_WHEELUP:
      _event.keyDown.button = BzfKeyEvent::WheelUp;
      break;
    case SDL_BUTTON_WHEELDOWN:
      _event.keyDown.button = BzfKeyEvent::WheelDown;
      break;
    case 6:
      _event.keyDown.button = BzfKeyEvent::MouseButton6;
      break;
    case 7:
      _event.keyDown.button = BzfKeyEvent::MouseButton7;
      break;
    case 8:
      _event.keyDown.button = BzfKeyEvent::MouseButton8;
      break;
    case 9:
      _event.keyDown.button = BzfKeyEvent::MouseButton9;
      break;
    case 10:
      _event.keyDown.button = BzfKeyEvent::MouseButton10;
      break;
    default:
      return false;
    }
    break;

  case SDL_KEYDOWN:
    _event.type = BzfEvent::KeyDown;
    if (!getKey(event, _event.keyDown))
      return false;
    break;

  case SDL_KEYUP:
    _event.type = BzfEvent::KeyUp;
    if (!getKey(event, _event.keyUp))
      return false;
    break;

  case SDL_QUIT:
    _event.type = BzfEvent::Quit;
    break;

  default:
    return false;
  }
  return true;
};

bool SDLDisplay::getKey(const SDL_Event& sdlEvent, BzfKeyEvent& key) const
{
  Uint16 unicode = sdlEvent.key.keysym.unicode;
  SDLKey sym     = sdlEvent.key.keysym.sym;
  SDLMod mod     = sdlEvent.key.keysym.mod;

  if (unicode != 0) {
    char ch;
    if ((unicode & 0xFF80) == 0) {
      ch = unicode & 0x7F;
    }
    else {
      return false;
    }
    key.ascii = ch;
    key.button = BzfKeyEvent::NoButton;
    if (sym == SDLK_DELETE) {
      key.ascii  = 0;
      key.button = BzfKeyEvent::Delete;
    }
  } else {
    key.ascii = 0;
    switch (sym) {
    case SDLK_PAUSE:
      key.button = BzfKeyEvent::Pause;
      break;
    case SDLK_HOME:
      key.button = BzfKeyEvent::Home;
      break;
    case SDLK_END:
      key.button = BzfKeyEvent::End;
      break;
    case SDLK_LEFT:
      key.button = BzfKeyEvent::Left;
      break;
    case SDLK_RIGHT:
      key.button = BzfKeyEvent::Right;
      break;
    case SDLK_UP:
      key.button = BzfKeyEvent::Up;
      break;
    case SDLK_DOWN:
      key.button = BzfKeyEvent::Down;
      break;
    case SDLK_PAGEUP:
      key.button = BzfKeyEvent::PageUp;
      break;
    case SDLK_PAGEDOWN:
      key.button = BzfKeyEvent::PageDown;
      break;
    case SDLK_INSERT:
      key.button = BzfKeyEvent::Insert;
      break;
    case SDLK_DELETE:
      key.button = BzfKeyEvent::Delete;
      break;
    case SDLK_F1:
      key.button = BzfKeyEvent::F1;
      break;
    case SDLK_F2:
      key.button = BzfKeyEvent::F2;
      break;
    case SDLK_F3:
      key.button = BzfKeyEvent::F3;
      break;
    case SDLK_F4:
      key.button = BzfKeyEvent::F4;
      break;
    case SDLK_F5:
      key.button = BzfKeyEvent::F5;
      break;
    case SDLK_F6:
      key.button = BzfKeyEvent::F6;
      break;
    case SDLK_F7:
      key.button = BzfKeyEvent::F7;
      break;
    case SDLK_F8:
      key.button = BzfKeyEvent::F8;
      break;
    case SDLK_F9:
      key.button = BzfKeyEvent::F9;
      break;
    case SDLK_F10:
      key.button = BzfKeyEvent::F10;
      break;
    case SDLK_F11:
      key.button = BzfKeyEvent::F11;
      break;
    case SDLK_F12:
      key.button = BzfKeyEvent::F12;
      break;
    default:
      return false;
    }
  }

  key.shift = 0;
  if (mod & KMOD_SHIFT)
    key.shift |= BzfKeyEvent::ShiftKey;
  if (mod & KMOD_CTRL)
    key.shift |= BzfKeyEvent::ControlKey;
  if (mod & KMOD_ALT)
    key.shift |= BzfKeyEvent::AltKey;
  return true;
}

void SDLDisplay::createWindow() {
  SDL_Surface *surface;
  if (fullScreen)
    surface = SDL_SetVideoMode(getWidth(), getHeight(), 0,
			       SDL_OPENGL  | SDL_FULLSCREEN);
  else
    surface = SDL_SetVideoMode(base_width, base_height, 0, SDL_OPENGL);
  if (!surface)
    printf("Could not set Video Mode: %s.\n", SDL_GetError());
};

void SDLDisplay::setFullscreen() {
  fullScreen = true;
}

void SDLDisplay::setWindowSize(int _width, int _height) {
  base_width  = _width;
  base_height = _height;
}

void SDLDisplay::getWindowSize(int& width, int& height) const {
  if (fullScreen) {
    width  = getWidth();
    height = getHeight();
  } else {
    width  = base_width;
    height = base_height;
  }
};

void SDLVisual::setDoubleBuffer(bool on) {
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, on ? 1 : 0);
};

void SDLVisual::setRGBA(int minRed, int minGreen,
			int minBlue, int minAlpha) {
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   minRed);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, minGreen);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  minBlue);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, minAlpha);
};

void SDLVisual::setDepth(int minDepth) {
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, minDepth);
};

void SDLVisual::setStencil(int minDepth) {
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, minDepth);
};

void SDLVisual::setStereo(bool on) {
  SDL_GL_SetAttribute(SDL_GL_STEREO, on ? 1 : 0);
};
  
SDLWindow::SDLWindow(const SDLDisplay* _display, SDLVisual*)
  : BzfWindow(_display), x(-1), y(-1), hasGamma(true)
{
};

void SDLWindow::setTitle(const char * title) {
  SDL_WM_SetCaption(title, title);
};

void SDLWindow::setFullscreen() {
  ((SDLDisplay *)getDisplay())->setFullscreen();
};

void SDLWindow::warpMouse(int x, int y) {
  SDL_WarpMouse(x, y);
};

void SDLWindow::getMouse(int& x, int& y) const {
  x = mx;
  y = my;
};

void SDLWindow::grabMouse() {
  SDL_WM_GrabInput(SDL_GRAB_ON);
};

void SDLWindow::ungrabMouse() {
  SDL_WM_GrabInput(SDL_GRAB_OFF);
};

void SDLWindow::setSize(int width, int height) {
  ((SDLDisplay *)getDisplay())->setWindowSize(width, height);
};

void SDLWindow::getSize(int& width, int& height) const {
  ((SDLDisplay *)getDisplay())->getWindowSize(width, height);
};

void SDLWindow::setGamma(float newGamma) {
  gamma = newGamma;
  int result = SDL_SetGamma(gamma, gamma, gamma);
  if (result == -1) {
    printf("Could not set Gamma: %s.\n", SDL_GetError());
    hasGamma = false;
  }
};

float SDLWindow::getGamma() const {
  return gamma;
};

bool SDLWindow::hasGammaControl() const {
  return hasGamma;
};

void SDLWindow::swapBuffers() {
  SDL_GL_SwapBuffers();
};

void SDLWindow::create(void) {
  ((SDLDisplay *)getDisplay())->createWindow();
  // reload context data
  OpenGLGState::initContext();  
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
