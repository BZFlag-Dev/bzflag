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
#include <iostream>

static int mx = 0;
static int my = 0;

SDLDisplay::SDLDisplay() : fullScreen(false), base_width(640),
			   base_height(480), canGrabMouse(true),
			   oldFullScreen(false),
			   oldWidth(0), oldHeight(0)
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

    defaultResolutionIndex = 0;
#ifdef WIN32
    HDC hDC = GetDC(GetDesktopWindow());
    int defaultWidth = GetDeviceCaps(hDC, HORZRES);
    int defaultHeight = GetDeviceCaps(hDC, VERTRES);
    ReleaseDC(GetDesktopWindow(), hDC);
#endif
    for (int i = 0; modeList[i]; i++) {
      h = modeList[i]->h;
      w = modeList[i]->w;
      if (i != 0)
	if ((modeList[i - 1]->w == w) && (modeList[i - 1]->h == h))
	  continue;
      sprintf(name, "%dx%d    ", w, h);
      resolutions[j] = new ResInfo(name, w, h, 0);
      j++;
#ifdef WIN32
      // use a safe default resolution because there are so many screwy drivers out there
      if ((w == defaultWidth) && (h == defaultHeight))
	defaultResolutionIndex = j;
#endif
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

  case SDL_VIDEORESIZE:
    _event.type          = BzfEvent::Resize;
    _event.resize.width  = event.resize.w;
    _event.resize.height = event.resize.h;
    break;

  case SDL_ACTIVEEVENT:
    if (event.active.state == SDL_APPACTIVE)
      if (event.active.gain == 0) {
	_event.type = BzfEvent::Unmap;
      };
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
  case SDLK_BACKSPACE:
    key.button = BzfKeyEvent::Backspace;
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
  case SDLK_KP0:
    key.button = BzfKeyEvent::Kp0;
    break;
  case SDLK_KP1:
    key.button = BzfKeyEvent::Kp1;
    break;
  case SDLK_KP2:
    key.button = BzfKeyEvent::Kp2;
    break;
  case SDLK_KP3:
    key.button = BzfKeyEvent::Kp3;
    break;
  case SDLK_KP4:
    key.button = BzfKeyEvent::Kp4;
    break;
  case SDLK_KP5:
    key.button = BzfKeyEvent::Kp5;
    break;
  case SDLK_KP6:
    key.button = BzfKeyEvent::Kp6;
    break;
  case SDLK_KP7:
    key.button = BzfKeyEvent::Kp7;
    break;
  case SDLK_KP8:
    key.button = BzfKeyEvent::Kp8;
    break;
  case SDLK_KP9:
    key.button = BzfKeyEvent::Kp9;
    break;
  case SDLK_KP_PERIOD:
    key.button = BzfKeyEvent::Kp_Period;
    break;
  case SDLK_KP_DIVIDE:
    key.button = BzfKeyEvent::Kp_Divide;
    break;
  case SDLK_KP_MULTIPLY:
    key.button = BzfKeyEvent::Kp_Multiply;
    break;
  case SDLK_KP_MINUS:
    key.button = BzfKeyEvent::Kp_Minus;
    break;
  case SDLK_KP_PLUS:
    key.button = BzfKeyEvent::Kp_Plus;
    break;
  case SDLK_KP_ENTER:
    key.button = BzfKeyEvent::Kp_Enter;
    break;
  case SDLK_KP_EQUALS:
    key.button = BzfKeyEvent::Kp_Equals;
    break;
  case SDLK_HELP:
    key.button = BzfKeyEvent::Help;
    break;
  case SDLK_PRINT:
    key.button = BzfKeyEvent::Print;
    break;
  case SDLK_SYSREQ:
    key.button = BzfKeyEvent::Sysreq;
    break;
  case SDLK_BREAK:
    key.button = BzfKeyEvent::Break;
    break;
  case SDLK_MENU:
    key.button = BzfKeyEvent::Menu;
    break;
  case SDLK_POWER:
    key.button = BzfKeyEvent::Power;
    break;
  case SDLK_EURO:
    key.button = BzfKeyEvent::Euro;
    break;
  case SDLK_UNDO:
    key.button = BzfKeyEvent::Undo;
    break;
  default:
    key.button = BzfKeyEvent::NoButton;
    break;
  }

  // When NUM LOCK treat the KP number as numbers and Enter as Enter
  if (mod & KMOD_NUM) {
    if (((key.button >= BzfKeyEvent::Kp0) && (key.button <= BzfKeyEvent::Kp9))
	|| (key.button == BzfKeyEvent::Kp_Enter))
      key.button = BzfKeyEvent::NoButton;
  }

  if (key.button == BzfKeyEvent::NoButton)
    if (unicode) {
      if ((unicode & 0xFF80))
	return false;
      key.ascii = unicode & 0x7F;
    } else {
      if ((sym >= SDLK_FIRST) && (sym <= SDLK_DELETE))
	key.ascii = sym;
      else if ((sym >= SDLK_KP0) && (sym <= SDLK_KP9))
	key.ascii = sym - 208; // translate to normal number
      else if (sym == SDLK_KP_ENTER)
	key.ascii = 13; // enter
      else
	return false;
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
  int    width;
  int    height;
  Uint32 flags = SDL_OPENGL;
  // getting width, height & flags for SetVideoMode
  getWindowSize(width, height);
  if (fullScreen) {
    flags |= SDL_FULLSCREEN;
  } else {
    flags |= SDL_RESIZABLE;
  }
  // if they are the same, don't bother building a new window
  if ((width == oldWidth) && (height == oldHeight)
      && (fullScreen == oldFullScreen))
    return;
  // save the values for the next
  oldWidth      = width;
  oldHeight     = height;
  oldFullScreen = fullScreen;
  // Set the video mode and hope for no errors
  if (!SDL_SetVideoMode(width, height, 0, flags))
    printf("Could not set Video Mode: %s.\n", SDL_GetError());
};

void SDLDisplay::setFullscreen(bool on) {
  fullScreen = on;
}

void SDLDisplay::setWindowSize(int _width, int _height) {
  base_width  = _width;
  base_height = _height;
}

void SDLDisplay::getWindowSize(int& width, int& height) const {
  if (fullScreen) {
    if (modeIndex >= 0) {
      width  = getResolution(modeIndex)->width;
      height = getResolution(modeIndex)->height;
    } else {
      width  = 640;
      height = 480;
    }
  } else {
    width  = base_width;
    height = base_height;
  }
};

void SDLDisplay::enableGrabMouse(bool on) {
  canGrabMouse = on;
  if (canGrabMouse)
    SDL_WM_GrabInput(SDL_GRAB_ON);
  else
    SDL_WM_GrabInput(SDL_GRAB_OFF);
}

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

void SDLWindow::setFullscreen(bool on) {
  ((SDLDisplay *)getDisplay())->setFullscreen(on);
};

void SDLWindow::iconify(void) {
  SDL_WM_IconifyWindow();
};

void SDLWindow::warpMouse(int x, int y) {
  SDL_WarpMouse(x, y);
};

void SDLWindow::getMouse(int& x, int& y) const {
  x = mx;
  y = my;
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

void SDLWindow::enableGrabMouse(bool on) {
  ((SDLDisplay *)getDisplay())->enableGrabMouse(on);
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
