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

#include "ErrorHandler.h"
#include "BzfWindow.h"

BzfWindow::BzfWindow(const BzfDisplay* _display) : display(_display)
{
#ifdef HAVE_SDL
  if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
    std::vector<std::string> args;
    args.push_back(SDL_GetError());
    printError("Could not initialize SDL Joystick subsystem: %s.\n", &args);
  };
#endif
}

BzfWindow::~BzfWindow()
{
#ifdef HAVE_SDL
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
#endif
}

void			BzfWindow::callExposeCallbacks() const
{
  const int count = exposeCallbacks.size();
  for (int i = 0; i < count; i++) {
    const BzfWindowCB& cb = exposeCallbacks[i];
    (*cb.cb)(cb.data);
  }
}

void			BzfWindow::addExposeCallback(
				void (*_cb)(void*), void* data)
{
  BzfWindowCB cb;
  cb.cb = _cb;
  cb.data = data;
  exposeCallbacks.push_back(cb);
}

void			BzfWindow::removeExposeCallback(
				void (*_cb)(void*), void* data)
{
  std::vector<BzfWindowCB>::iterator it = exposeCallbacks.begin();
  for(; it != exposeCallbacks.end(); it++) {
    if((it->cb == _cb) && (it->data == data)) {
      exposeCallbacks.erase(it);
      break;
    }
  }
}

void			BzfWindow::callResizeCallbacks() const
{
  const int count = resizeCallbacks.size();
  for (int i = 0; i < count; i++) {
    const BzfWindowCB& cb = resizeCallbacks[i];
    (*cb.cb)(cb.data);
  }
}

void			BzfWindow::addResizeCallback(
				void (*_cb)(void*), void* data)
{
  BzfWindowCB cb;
  cb.cb = _cb;
  cb.data = data;
  resizeCallbacks.push_back(cb);
}

void			BzfWindow::removeResizeCallback(
				void (*_cb)(void*), void* data)
{
  std::vector<BzfWindowCB>::iterator it = resizeCallbacks.begin();
  for(; it != resizeCallbacks.end(); it++) {
    if((it->cb == _cb) && (it->data == data)) {
      resizeCallbacks.erase(it);
      break;
    }
  }
}

void			BzfWindow::initJoystick(const char* joystickName)
{
#ifdef HAVE_SDL
  if (!strcmp(joystickName, "off")) {
    joystickID = NULL;
    return;
  }
  int numJoystick = SDL_NumJoysticks();
  if (!numJoystick) {
    printError("no joystick is supported...");
    joystickID = NULL;
    return;
  }
  int i;
  for (i = 0; i < numJoystick; i++)
    if (strcmp(SDL_JoystickName(i), joystickName) == 0)
      break;
  if (i >= numJoystick)
    i = 0;
  joystickID = SDL_JoystickOpen(i);
  if (SDL_JoystickNumAxes(joystickID) < 2) {
    SDL_JoystickClose(joystickID);
    printError("joystick has less then 2 axis:\n");
    joystickID = NULL;
    return;
  }
  joystickButtons = SDL_JoystickNumButtons(joystickID);
#else
  if (strcmp(joystickName, "off") && strcmp(joystickName, "")) {
    std::vector<std::string> args;
    args.push_back(joystickName);
    printError("joystick '{1}' not supported...", &args);
  }
#endif
}

bool			BzfWindow::joystick() const
{
#ifdef HAVE_SDL
  return joystickID != NULL;
#else
  return false;
#endif
}

void			BzfWindow::getJoy(int& x, int& y) const
{
#ifdef HAVE_SDL
  if (!joystickID)
    return;

  SDL_JoystickUpdate();
  x = SDL_JoystickGetAxis(joystickID, 0);
  y = SDL_JoystickGetAxis(joystickID, 1);

  x = x * 1000 / 32768;
  y = y * 1000 / 32768;

  /* balistic */
  x = (x * abs(x))/1000;
  y = (y * abs(y))/1000;
#else
  x = 0;
  y = 0;
#endif
}

unsigned long		BzfWindow::getJoyButtons() const
{
  unsigned long buttons = 0;

#ifdef HAVE_SDL
  if (!joystickID)
    return 0;

  SDL_JoystickUpdate();
  for (int i = 0; i < joystickButtons; i++)
    buttons |= SDL_JoystickGetButton(joystickID, i) << i;
#endif
  return buttons;
}

void                    BzfWindow::getJoyDevices(std::vector<std::string>
						 &list) const {
#ifdef HAVE_SDL
  
  int numJoystick = SDL_NumJoysticks();
  int i;
  for (i = 0; i < numJoystick; i++)
    list.push_back(SDL_JoystickName(i));
#else
  list.clear();
#endif
}

void			BzfWindow::yieldCurrent(void)
{
	// do nothing
}

void			BzfWindow::releaseCurrent(void)
{
	// do nothing
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

