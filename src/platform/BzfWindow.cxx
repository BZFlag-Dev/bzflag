/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ErrorHandler.h"
#include "BzfWindow.h"

BzfWindow::BzfWindow(const BzfDisplay* _display) : display(_display)
{
  // do nothing
}

BzfWindow::~BzfWindow()
{
  // do nothing
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
  std::vector<std::string> args;
  args.push_back(joystickName);
  printError("joystick '{1}' not supported...", &args);
}

// ex: shiftwidth=2 tabstop=8
