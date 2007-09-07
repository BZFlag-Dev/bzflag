/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "BzfWindow.h"

// system headers
#include <vector>

// common implementation headers
#include "ErrorHandler.h"

BzfWindow::BzfWindow(const BzfDisplay* _display) : display(_display)
{
}

BzfWindow::~BzfWindow()
{
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

