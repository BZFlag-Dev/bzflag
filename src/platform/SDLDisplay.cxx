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
#include "SDLDisplay.h"

/* system implementation headers */
#include <stdio.h>
#include <iostream>
#include <math.h>

/* common implementation headers */
#include "StateDatabase.h"
#include "bzfio.h"

/* local implementation headers */
#include "SDLVisual.h"
#include "SDLWindow.h"


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
  if (!modeList) {
    printf("Could not Get available video modes: %s.\n", SDL_GetError());
  }

  int defaultResolutionIndex = 0;
  ResInfo** _resolutions;
  int _numResolutions = 1;
  // No modes available or All resolutions available
  if ((modeList != (SDL_Rect **) 0) && (modeList != (SDL_Rect **) -1)) {
    for (int i = 1; modeList[i]; i++) {
      if ((modeList[i - 1]->w != modeList[i]->w)
          || (modeList[i - 1]->h != modeList[i]->h)) {
        _numResolutions++;
      }
    }
  };
  _resolutions = new ResInfo*[_numResolutions];

  if ((modeList != (SDL_Rect **) 0) && (modeList != (SDL_Rect **) -1)) {
    char name[80];
    int  h;
    int  w;
    int  j = 0;

    defaultResolutionIndex = 0;
#ifdef WIN32
    HDC hDC = GetDC(GetDesktopWindow());
    defaultWidth = GetDeviceCaps(hDC, HORZRES);
    defaultHeight = GetDeviceCaps(hDC, VERTRES);
    ReleaseDC(GetDesktopWindow(), hDC);
#endif
    for (int i = 0; modeList[i]; i++) {
      h = modeList[i]->h;
      w = modeList[i]->w;
      if (i != 0) {
	if ((modeList[i - 1]->w == w) && (modeList[i - 1]->h == h)) {
	  continue;
        }
      }
      sprintf(name, "%dx%d    ", w, h);
      _resolutions[j] = new ResInfo(name, w, h, 0);
#ifdef WIN32
      // use a safe default resolution because there are so many screwy drivers out there
      if ((w == defaultWidth) && (h == defaultHeight)) {
	defaultResolutionIndex = j;
      }
#endif
      j++;
    }
  }
  else {
    // if no modes then make default
    _resolutions[0] = new ResInfo ("default", 640, 480, 0);
    defaultWidth = 640;
    defaultHeight = 480;
  }

  // limit us to the main display
  static char singleDisplayEnv[] = "SDL_SINGLEDISPLAY=1";
  putenv(singleDisplayEnv);

  // register modes
  initResolutions(_resolutions, _numResolutions, defaultResolutionIndex);

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
}


bool SDLDisplay::isEventPending() const
{
  return (SDL_PollEvent(NULL) == 1);
}


bool SDLDisplay::getEvent(BzfEvent& _event) const
{
  SDL_Event event;
  if (SDL_PollEvent(&event) == 0) {
    return false;
  }
  return setupEvent(_event, event);
}


bool SDLDisplay::peekEvent(BzfEvent& _event) const
{
  SDL_Event event;
  if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_ALLEVENTS) <= 0) {
    return false;
  }
  return setupEvent(_event, event);
}


bool SDLDisplay::setupEvent(BzfEvent& bzEvent, const SDL_Event& sdlEvent) const
{
  SDLMod mode = SDL_GetModState();
  bool shift  = ((mode & KMOD_SHIFT) != 0);
  bool ctrl   = ((mode & KMOD_CTRL) != 0);
  bool alt    = ((mode & KMOD_ALT) != 0);

  switch (sdlEvent.type) {

    case SDL_MOUSEMOTION: {
      bzEvent.type = BzfEvent::MouseMove;
      SDLWindow::mx = sdlEvent.motion.x;
#ifndef __APPLE__
      SDLWindow::my = sdlEvent.motion.y;
#else
      static const SDL_version *sdlver = SDL_Linked_Version();
      /* deal with a SDL bug when in windowed mode related to
       * Cocoa coordinate system of (0,0) in bottom-left corner.
       */
      if ((fullScreen) ||
	  (sdlver->major > 1) ||
	  (sdlver->minor > 2) ||
	  (sdlver->patch > 6)) {
	SDLWindow::my = sdlEvent.motion.y;
      } else {
	SDLWindow::my = base_height - 1 - sdlEvent.motion.y;
      }
#endif
      bzEvent.mouseMove.x = SDLWindow::mx;
      bzEvent.mouseMove.y = SDLWindow::my;
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      bzEvent.type = BzfEvent::KeyDown;
      bzEvent.keyDown.chr = 0;
      bzEvent.keyDown.shift = 0;

      if (alt)   { bzEvent.keyDown.shift |= BzfKeyEvent::AltKey;     }
      if (ctrl)  { bzEvent.keyDown.shift |= BzfKeyEvent::ControlKey; }
      if (shift) { bzEvent.keyDown.shift |= BzfKeyEvent::ShiftKey;   }

      int& button = bzEvent.keyDown.button;
      switch (sdlEvent.button.button) {
	case SDL_BUTTON_LEFT:      { button = BzfKeyEvent::LeftMouse;     break; }
	case SDL_BUTTON_MIDDLE:    { button = BzfKeyEvent::MiddleMouse;   break; }
	case SDL_BUTTON_RIGHT:     { button = BzfKeyEvent::RightMouse;    break; }
	case SDL_BUTTON_WHEELUP:   { button = BzfKeyEvent::WheelUp;       break; }
	case SDL_BUTTON_WHEELDOWN: { button = BzfKeyEvent::WheelDown;     break; }
	case 6:                    { button = BzfKeyEvent::MouseButton6;  break; }
	case 7:                    { button = BzfKeyEvent::MouseButton7;  break; }
	case 8:                    { button = BzfKeyEvent::MouseButton8;  break; }
	case 9:                    { button = BzfKeyEvent::MouseButton9;  break; }
	case 10:                   { button = BzfKeyEvent::MouseButton10; break; }
	default: {
	  return false;
        }
      }
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      bzEvent.type = BzfEvent::KeyUp;
      bzEvent.keyUp.chr = 0;
      bzEvent.keyUp.shift = 0;

      if (alt)   { bzEvent.keyUp.shift |= BzfKeyEvent::AltKey;     }
      if (ctrl)  { bzEvent.keyUp.shift |= BzfKeyEvent::ControlKey; }
      if (shift) { bzEvent.keyUp.shift |= BzfKeyEvent::ShiftKey;   }

      int& button = bzEvent.keyUp.button;
      switch (sdlEvent.button.button) {
	case SDL_BUTTON_LEFT:      { button = BzfKeyEvent::LeftMouse;     break; }
	case SDL_BUTTON_MIDDLE:    { button = BzfKeyEvent::MiddleMouse;   break; }
	case SDL_BUTTON_RIGHT:     { button = BzfKeyEvent::RightMouse;    break; }
	case SDL_BUTTON_WHEELUP:   { button = BzfKeyEvent::WheelUp;       break; }
	case SDL_BUTTON_WHEELDOWN: { button = BzfKeyEvent::WheelDown;     break; }
	case 6:                    { button = BzfKeyEvent::MouseButton6;  break; }
	case 7:                    { button = BzfKeyEvent::MouseButton7;  break; }
	case 8:                    { button = BzfKeyEvent::MouseButton8;  break; }
	case 9:                    { button = BzfKeyEvent::MouseButton9;  break; }
	case 10:                   { button = BzfKeyEvent::MouseButton10; break; }
	default: {
	  return false;
        }
      }
      break;
    }
    case SDL_KEYDOWN: {
      bzEvent.type = BzfEvent::KeyDown;
      if (!getKey(sdlEvent, bzEvent.keyDown))
	return false;
      break;
    }
    case SDL_KEYUP: {
      bzEvent.type = BzfEvent::KeyUp;
      if (!getKey(sdlEvent, bzEvent.keyUp))
	return false;
      break;
    }
    case SDL_QUIT: {
      bzEvent.type = BzfEvent::Quit;
      break;
    }
    case SDL_VIDEORESIZE: {
      bzEvent.type = BzfEvent::Resize;
      bzEvent.resize.width  = sdlEvent.resize.w;
      bzEvent.resize.height = sdlEvent.resize.h;
      break;
    }
    case SDL_VIDEOEXPOSE: {
      bzEvent.type = BzfEvent::Redraw;
      break;
    }
    case SDL_ACTIVEEVENT: {
      if (sdlEvent.active.state & SDL_APPACTIVE) {
	if (sdlEvent.active.gain == 0) {
	  bzEvent.type = BzfEvent::Unmap;
	} else {
	  bzEvent.type = BzfEvent::Map;
	}
      } else {
	bzEvent.type = BzfEvent::Map;
      }
      break;
    }
    default: {
      return false;
    }
  }
  return true;
}


void SDLDisplay::getModState(bool &shift, bool &ctrl, bool &alt)
{
  SDLMod mode = SDL_GetModState();
  alt   = ((mode & KMOD_ALT)   != 0);
  ctrl  = ((mode & KMOD_CTRL)  != 0);
  shift = ((mode & KMOD_SHIFT) != 0);
}


bool SDLDisplay::getKey(const SDL_Event& sdlEvent, BzfKeyEvent& key) const
{
  Uint16 unicode = sdlEvent.key.keysym.unicode;
  SDLKey sym     = sdlEvent.key.keysym.sym;
  SDLMod mod     = sdlEvent.key.keysym.mod;

  key.chr = 0;
  switch (sym) {
    case SDLK_PAUSE:       { key.button = BzfKeyEvent::Pause;       break; }
    case SDLK_HOME:        { key.button = BzfKeyEvent::Home;        break; }
    case SDLK_END:         { key.button = BzfKeyEvent::End;         break; }
    case SDLK_LEFT:        { key.button = BzfKeyEvent::Left;        break; }
    case SDLK_RIGHT:       { key.button = BzfKeyEvent::Right;       break; }
    case SDLK_UP:          { key.button = BzfKeyEvent::Up;          break; }
    case SDLK_DOWN:        { key.button = BzfKeyEvent::Down;        break; }
    case SDLK_PAGEUP:      { key.button = BzfKeyEvent::PageUp;      break; }
    case SDLK_PAGEDOWN:    { key.button = BzfKeyEvent::PageDown;    break; }
    case SDLK_INSERT:      { key.button = BzfKeyEvent::Insert;      break; }
    case SDLK_BACKSPACE:   { key.button = BzfKeyEvent::Backspace;   break; }
    case SDLK_DELETE:      { key.button = BzfKeyEvent::Delete;      break; }
    case SDLK_F1:          { key.button = BzfKeyEvent::F1;          break; }
    case SDLK_F2:          { key.button = BzfKeyEvent::F2;          break; }
    case SDLK_F3:          { key.button = BzfKeyEvent::F3;          break; }
    case SDLK_F4:          { key.button = BzfKeyEvent::F4;          break; }
    case SDLK_F5:          { key.button = BzfKeyEvent::F5;          break; }
    case SDLK_F6:          { key.button = BzfKeyEvent::F6;          break; }
    case SDLK_F7:          { key.button = BzfKeyEvent::F7;          break; }
    case SDLK_F8:          { key.button = BzfKeyEvent::F8;          break; }
    case SDLK_F9:          { key.button = BzfKeyEvent::F9;          break; }
    case SDLK_F10:         { key.button = BzfKeyEvent::F10;         break; }
    case SDLK_F11:         { key.button = BzfKeyEvent::F11;         break; }
    case SDLK_F12:         { key.button = BzfKeyEvent::F12;         break; }
    case SDLK_KP0:         { key.button = BzfKeyEvent::Kp0;         break; }
    case SDLK_KP1:         { key.button = BzfKeyEvent::Kp1;         break; }
    case SDLK_KP2:         { key.button = BzfKeyEvent::Kp2;         break; }
    case SDLK_KP3:         { key.button = BzfKeyEvent::Kp3;         break; }
    case SDLK_KP4:         { key.button = BzfKeyEvent::Kp4;         break; }
    case SDLK_KP5:         { key.button = BzfKeyEvent::Kp5;         break; }
    case SDLK_KP6:         { key.button = BzfKeyEvent::Kp6;         break; }
    case SDLK_KP7:         { key.button = BzfKeyEvent::Kp7;         break; }
    case SDLK_KP8:         { key.button = BzfKeyEvent::Kp8;         break; }
    case SDLK_KP9:         { key.button = BzfKeyEvent::Kp9;         break; }
    case SDLK_KP_PERIOD:   { key.button = BzfKeyEvent::Kp_Period;   break; }
    case SDLK_KP_DIVIDE:   { key.button = BzfKeyEvent::Kp_Divide;   break; }
    case SDLK_KP_MULTIPLY: { key.button = BzfKeyEvent::Kp_Multiply; break; }
    case SDLK_KP_MINUS:    { key.button = BzfKeyEvent::Kp_Minus;    break; }
    case SDLK_KP_PLUS:     { key.button = BzfKeyEvent::Kp_Plus;     break; }
    case SDLK_KP_ENTER:    { key.button = BzfKeyEvent::Kp_Enter;    break; }
    case SDLK_KP_EQUALS:   { key.button = BzfKeyEvent::Kp_Equals;   break; }
    case SDLK_HELP:        { key.button = BzfKeyEvent::Help;        break; }
    case SDLK_PRINT:       { key.button = BzfKeyEvent::Print;       break; }
    case SDLK_SYSREQ:      { key.button = BzfKeyEvent::Sysreq;      break; }
    case SDLK_BREAK:       { key.button = BzfKeyEvent::Break;       break; }
    case SDLK_MENU:        { key.button = BzfKeyEvent::Menu;        break; }
    case SDLK_POWER:       { key.button = BzfKeyEvent::Power;       break; }
    case SDLK_EURO:        { key.button = BzfKeyEvent::Euro;        break; }
    case SDLK_UNDO:        { key.button = BzfKeyEvent::Undo;        break; }
    default:               { key.button = BzfKeyEvent::NoButton;    break; }
  }

  // When NUM LOCK treat the KP number as numbers and Kp_Enter as Enter
  if (mod & KMOD_NUM) {
    if (((key.button >= BzfKeyEvent::Kp0) && (key.button <= BzfKeyEvent::Kp9))
	|| (key.button == BzfKeyEvent::Kp_Enter))
      key.button = BzfKeyEvent::NoButton;
  }

  if (key.button == BzfKeyEvent::NoButton) {
    if (unicode) {
      key.chr = unicode;
    }
    else {
      if ((sym >= SDLK_FIRST) && (sym <= SDLK_DELETE)) {
	key.chr = sym;
      }
      else if ((sym >= SDLK_KP0) && (sym <= SDLK_KP9)) {
	key.chr = (sym - SDLK_KP0) + SDLK_0; // translate to normal number
      }
      else if (sym == SDLK_KP_ENTER) {
	key.chr = SDLK_RETURN; // enter
      }
      else {
	return false;
      }
    }
  }

  key.shift = 0;
  if (mod & KMOD_ALT)   { key.shift |= BzfKeyEvent::AltKey;     }
  if (mod & KMOD_CTRL)  { key.shift |= BzfKeyEvent::ControlKey; }
  if (mod & KMOD_SHIFT) { key.shift |= BzfKeyEvent::ShiftKey;   }

  return true;
}


bool SDLDisplay::createWindow()
{
  int width;
  int height;
  Uint32 flags = SDL_OPENGL;

  /* anti-aliasing */
  if (BZDB.isSet("multisamples")) {
    int ms = BZDB.evalInt("multisamples");
    if (ms == 2 || ms == 4) {
      SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1);
      SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, ms);
    }
  }

  // getting width, height & flags for SetVideoMode
  getWindowSize(width, height);
  if (fullScreen) {
    flags |= SDL_FULLSCREEN;
  } else {
    flags |= SDL_RESIZABLE;
  }

  // if they are the same, don't bother building a new window
  if ((width == oldWidth) &&
      (height == oldHeight) &&
      (fullScreen == oldFullScreen)) {
    return true;
  }

  // save the values for the next
  oldWidth      = width;
  oldHeight     = height;
  oldFullScreen = fullScreen;

  // Set the video mode and hope for no errors
  if (!SDL_SetVideoMode(width, height, 0, flags)) {
    printf("Could not set Video Mode: %s.\n", SDL_GetError());
    return false;
  } else {
    return true;
  }
}


void SDLDisplay::setFullscreen(bool on)
{
  fullScreen = on;
}


bool SDLDisplay::getFullscreen() const
{
  return fullScreen;
}


void SDLDisplay::setWindowSize(int _width, int _height)
{
  logDebugMessage(1, "setting size to %ix%i\n", _width, _height);
  base_width  = _width;
  base_height = _height;
}


void SDLDisplay::getWindowSize(int& width, int& height)
{
  if (fullScreen) {

    if (modeIndex < 0)
      modeIndex = 0;

    const BzfDisplay::ResInfo *resolution = getResolution(modeIndex);

    if (resolution != NULL) {
      width  = resolution->width;
      height = resolution->height;
    } else {
      modeIndex = -1;
      width  = 640;
      height = 480;
    }

  } else {
    width  = base_width;
    height = base_height;
  }

  logDebugMessage(1, "returning window size %ix%i with fullscreen set to %c\n",
                  width, height, fullScreen ? '1' : '0');

  /* sanity checks */
  if (width <= 0) {
    modeIndex = -1;
    width = defaultWidth;
    printf("ERROR: Non-positive window width encountered (%d)\n", width);
  }
  if (height <= 0) {
    modeIndex = -1;
    height = defaultHeight;
    printf("ERROR: Non-positive window height encountered (%d)\n", height);
  }
}


void SDLDisplay::enableGrabMouse(bool on)
{
  canGrabMouse = on;
  if (canGrabMouse) {
    SDL_WM_GrabInput(SDL_GRAB_ON);
  } else {
    SDL_WM_GrabInput(SDL_GRAB_OFF);
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
