/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "BzfWindow.h"
#include "ErrorHandler.h"

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
  const int count = exposeCallbacks.getLength();
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
  exposeCallbacks.append(cb);
}

void			BzfWindow::removeExposeCallback(
				void (*_cb)(void*), void* data)
{
  const int count = exposeCallbacks.getLength();
  for (int i = 0; i < count; i++) {
    const BzfWindowCB& cb = exposeCallbacks[i];
    if (cb.cb == _cb && cb.data == data) {
      exposeCallbacks.remove(i);
      break;
    }
  }
}

void			BzfWindow::callResizeCallbacks() const
{
  const int count = resizeCallbacks.getLength();
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
  resizeCallbacks.append(cb);
}

void			BzfWindow::removeResizeCallback(
				void (*_cb)(void*), void* data)
{
  const int count = resizeCallbacks.getLength();
  for (int i = 0; i < count; i++) {
    const BzfWindowCB& cb = resizeCallbacks[i];
    if (cb.cb == _cb && cb.data == data) {
      resizeCallbacks.remove(i);
      break;
    }
  }
}

void			BzfWindow::initJoystick(const char* joystickName)
{
  printError("joystick '%s' not supported...", joystickName);
}

// ex: shiftwidth=2 tabstop=8
