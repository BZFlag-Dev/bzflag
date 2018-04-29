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

// Own include
#include "SDL2Display.h"

// System includes
#include <vector>

static int mx = 0;
static int my = 0;

SDLDisplay::SDLDisplay() : min_width(), min_height(),  x(), y()
{
  if (SDL_VideoInit(NULL) < 0) {
    printf("Could not initialize SDL Video subsystem: %s.\n", SDL_GetError());
    exit (-1);
  };

  int defaultResolutionIndex = 0;
  ResInfo** _resolutions;
  int _numResolutions = SDL_GetNumDisplayModes(0);
  SDL_DisplayMode mode;
  std::vector<int> h;
  std::vector<int> w;
  int j = 0;

  if (!_numResolutions)
    // No modes available or All resolutions available
    printf("Could not Get available video modes: %s.\n", SDL_GetError());

  int result = SDL_GetCurrentDisplayMode(0, &mode);
  if (result) {
    printf("Could not get current display mode: %s.\n", SDL_GetError());
  } else {
    h.push_back(mode.h);
    w.push_back(mode.w);
    j++;
  }

  int i;
  for (i = 0; i < _numResolutions; i++) {
    result = SDL_GetDisplayMode(0, i, &mode);
    if (result)
      printf("Could not Get video mode: %s.\n", SDL_GetError());
    int k;
    for (k = 0; k < j; k++) {
      if ((h[k] == mode.h) && (w[k] == mode.w))
	break;
    }
    if (k >= j) {
      h.push_back(mode.h);
      w.push_back(mode.w);
      j++;
    }
  }

  _numResolutions = j;
  _resolutions = new ResInfo*[_numResolutions];

  for (i = 0; i < _numResolutions; i++) {
    char name[80];
    sprintf(name, "%dx%d    ", w[i], h[i]);
    _resolutions[i] = new ResInfo(name, w[i], h[i], 0);
  }

  // register modes
  initResolutions(_resolutions, _numResolutions, defaultResolutionIndex);
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
  if (SDL_PollEvent(&event) == 0)
    return false;

  return setupEvent(_event, event);
}


bool SDLDisplay::peekEvent(BzfEvent& _event) const
{
  SDL_PumpEvents();
  SDL_Event event;
  if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)
      <= 0) {
    return false;
  }

  return setupEvent(_event, event);
}


bool SDLDisplay::getKey(const SDL_Event& sdlEvent, BzfKeyEvent& key, const char asciiText) const
{
  SDL_Keycode sym = sdlEvent.key.keysym.sym;
  Uint16      mod = sdlEvent.key.keysym.mod;

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
  case SDLK_KP_0:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp0;
      key.ascii = '0';
    } else {
      key.button = BzfKeyEvent::Insert;
    }
    break;
  case SDLK_KP_1:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp1;
      key.ascii = '1';
    } else {
      key.button = BzfKeyEvent::End;
    }
    break;
  case SDLK_KP_2:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp2;
      key.ascii = '2';
    } else {
      key.button = BzfKeyEvent::Down;
    }
    break;
  case SDLK_KP_3:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp3;
      key.ascii = '3';
    } else {
      key.button = BzfKeyEvent::PageDown;
    }
    break;
  case SDLK_KP_4:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp4;
      key.ascii = '4';
    } else {
      key.button = BzfKeyEvent::Left;
    }
    break;
  case SDLK_KP_5:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp5;
      key.ascii = '5';
    } else {
      // When numlock is turned off, the keypad 5 key should do nothing
      return false;
    }
    break;
  case SDLK_KP_6:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp6;
      key.ascii = '6';
    } else {
      key.button = BzfKeyEvent::Right;
    }
    break;
  case SDLK_KP_7:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp7;
      key.ascii = '7';
    } else {
      key.button = BzfKeyEvent::Home;
    }
    break;
  case SDLK_KP_8:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp8;
      key.ascii = '8';
    } else {
      key.button = BzfKeyEvent::Up;
    }
    break;
  case SDLK_KP_9:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp9;
      key.ascii = '9';
    } else {
      key.button = BzfKeyEvent::PageUp;
    }
    break;
  case SDLK_KP_PERIOD:
    if (mod & KMOD_NUM) {
      key.button = BzfKeyEvent::Kp_Period;
      key.ascii = '.';
    } else {
      key.button = BzfKeyEvent::Delete;
    }
    break;
  case SDLK_KP_DIVIDE:
    key.button = BzfKeyEvent::Kp_Divide;
    key.ascii = '/';
    break;
  case SDLK_KP_MULTIPLY:
    key.button = BzfKeyEvent::Kp_Multiply;
    key.ascii = '*';
    break;
  case SDLK_KP_MINUS:
    key.button = BzfKeyEvent::Kp_Minus;
    key.ascii = '-';
    break;
  case SDLK_KP_PLUS:
    key.button = BzfKeyEvent::Kp_Plus;
    key.ascii = '+';
    break;
  case SDLK_KP_ENTER:
    key.button = BzfKeyEvent::Kp_Enter;
    key.ascii = 13;
    break;
  case SDLK_KP_EQUALS:
    key.button = BzfKeyEvent::Kp_Equals;
    key.ascii = '=';
    break;
  case SDLK_HELP:
    key.button = BzfKeyEvent::Help;
    break;
  case SDLK_PRINTSCREEN:
    key.button = BzfKeyEvent::Print;
    break;
  case SDLK_SYSREQ:
    key.button = BzfKeyEvent::Sysreq;
    break;
  case SDLK_MENU:
    key.button = BzfKeyEvent::Menu;
    break;
  case SDLK_POWER:
    key.button = BzfKeyEvent::Power;
    break;
  case SDLK_UNDO:
    key.button = BzfKeyEvent::Undo;
    break;
  default:
    key.button = BzfKeyEvent::NoButton;
    if (asciiText)
      key.ascii = asciiText;
    else if ((sym >= 0) && (sym <= 0x7F))
      key.ascii = (char) sym;
    else
      return false;
    break;
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


void SDLDisplay::getWindowSize(int& width, int& height) {
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
}


void SDLDisplay::getModState(bool &shift, bool &ctrl, bool &alt)
{
  SDL_Keymod mode = SDL_GetModState();
  shift       = ((mode & KMOD_SHIFT) != 0);
  ctrl	= ((mode & KMOD_CTRL) != 0);
  alt	 = ((mode & KMOD_ALT) != 0);
}


void SDLDisplay::getMouse(int &_x, int &_y) const {
  _x = mx;
  _y = my;
}


bool SDLDisplay::setupEvent(BzfEvent& _event, const SDL_Event& event) const
{
  SDL_Keymod mode = SDL_GetModState();
  bool shift  = ((mode & KMOD_SHIFT) != 0);
  bool ctrl   = ((mode & KMOD_CTRL) != 0);
  bool alt    = ((mode & KMOD_ALT) != 0);

  switch (event.type) {

  case SDL_MOUSEMOTION:
    _event.type	= BzfEvent::MouseMove;
    mx		 = event.motion.x;
    my		 = event.motion.y;
    _event.mouseMove.x = mx;
    _event.mouseMove.y = my;
    break;

  case SDL_MOUSEWHEEL:
    _event.type	  = BzfEvent::KeyDown;
    _event.keyDown.ascii = 0;
    _event.keyDown.shift = 0;
    if (shift)
      _event.keyDown.shift |= BzfKeyEvent::ShiftKey;
    if (ctrl)
      _event.keyDown.shift |= BzfKeyEvent::ControlKey;
    if (alt)
      _event.keyDown.shift |= BzfKeyEvent::AltKey;

    if (event.wheel.y < 0)
      _event.keyDown.button = BzfKeyEvent::WheelDown;
    else
      _event.keyDown.button = BzfKeyEvent::WheelUp;
    break;

  case SDL_MOUSEBUTTONDOWN:
    _event.type	  = BzfEvent::KeyDown;
    _event.keyDown.ascii = 0;
    _event.keyDown.shift = 0;
    if (shift)
      _event.keyDown.shift |= BzfKeyEvent::ShiftKey;
    if (ctrl)
      _event.keyDown.shift |= BzfKeyEvent::ControlKey;
    if (alt)
      _event.keyDown.shift |= BzfKeyEvent::AltKey;

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
    if (shift)
      _event.keyUp.shift |= BzfKeyEvent::ShiftKey;
    if (ctrl)
      _event.keyUp.shift |= BzfKeyEvent::ControlKey;
    if (alt)
      _event.keyUp.shift |= BzfKeyEvent::AltKey;

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
  {
    _event.type = BzfEvent::KeyDown;

    char asciiText = '\0';

    SDL_Event textInputEvent;
    SDL_PumpEvents();

    if(SDL_PeepEvents(&textInputEvent, 1, SDL_PEEKEVENT, SDL_TEXTINPUT, SDL_TEXTINPUT) > 0)
      if (textInputEvent.text.text[0] >= '!' && textInputEvent.text.text[0] <= '~')
	// on macOS with SDL 2, a text input event is generated for numpad keys
	// regardless of whether numlock is on or off; the SDL team has determined
	// that this is the expected behavior on macOS, but we want to screen out
	// those text input events
	// original bug report: https://bugzilla.libsdl.org/show_bug.cgi?id=2886
#ifdef __APPLE__
	if ((event.key.keysym.mod & KMOD_NUM) || (
	  event.key.keysym.sym != SDLK_KP_0 &&
	  event.key.keysym.sym != SDLK_KP_1 &&
	  event.key.keysym.sym != SDLK_KP_2 &&
	  event.key.keysym.sym != SDLK_KP_3 &&
	  event.key.keysym.sym != SDLK_KP_4 &&
	  event.key.keysym.sym != SDLK_KP_5 &&
	  event.key.keysym.sym != SDLK_KP_6 &&
	  event.key.keysym.sym != SDLK_KP_7 &&
	  event.key.keysym.sym != SDLK_KP_8 &&
	  event.key.keysym.sym != SDLK_KP_9 &&
	  event.key.keysym.sym != SDLK_KP_PERIOD))
#endif // __APPLE
	// we have a valid text input event following, so use that text for
	// proper text case, symbol handling, etc.
	asciiText = textInputEvent.text.text[0];

    if (!getKey(event, _event.keyDown, asciiText))
      return false;

    break;
  }

  case SDL_KEYUP:
    _event.type = BzfEvent::KeyUp;
    if (!getKey(event, _event.keyUp))
      return false;
    break;

  case SDL_QUIT:
    _event.type = BzfEvent::Quit;
    break;

  case SDL_WINDOWEVENT:
    switch (event.window.event) {
    case SDL_WINDOWEVENT_RESIZED:
      _event.type = BzfEvent::Resize;
      _event.resize.width  = event.window.data1;
      _event.resize.height = event.window.data2;
      break;
    case SDL_WINDOWEVENT_EXPOSED:
      _event.type = BzfEvent::Redraw;
      break;
    case SDL_WINDOWEVENT_HIDDEN:
    case SDL_WINDOWEVENT_MINIMIZED:
      _event.type = BzfEvent::Unmap;
      break;
    case SDL_WINDOWEVENT_SHOWN:
    case SDL_WINDOWEVENT_RESTORED:
      _event.type = BzfEvent::Map;
      break;
#ifdef SDL_WINDOW_MOUSE_CAPTURE
    case SDL_WINDOWEVENT_FOCUS_GAINED:
    {
      // make sure the mouse is captured in case the cursor is (or moves) outside the window
      SDL_Window* windowId = SDL_GL_GetCurrentWindow();
      if (windowId) {
	Uint32 currentWindowFlags = SDL_GetWindowFlags(windowId);
	if (! (currentWindowFlags & SDL_WINDOW_MOUSE_CAPTURE))
	  SDL_CaptureMouse(SDL_TRUE);
      }
      break;
    }
#endif
    default:
      break;
    }
    break;

  default:
    return false;
  }
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
