/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface headers
#include "HUDNavigationQueue.h"

// system headers
#include <deque>

// local implementation headers
#include "HUDui.h"

//
// HUDNavigationQueue
//

HUDNavigationQueue::HUDNavigationQueue() : focus(0) {
}

void HUDNavigationQueue::next() {
  if (!size()) { return; }

  size_t newfocus = focus;
  if (++newfocus > (size() - 1)) {
    newfocus = 0;
  }

  internal_set(newfocus, hnNext);
}

void HUDNavigationQueue::prev() {
  if (!size()) { return; }

  size_t newfocus = focus;
  if (--newfocus > (size() - 1)) { // unsigned, so wraps around
    newfocus = (size() - 1);
  }

  internal_set(newfocus, hnPrev);
}

bool HUDNavigationQueue::set(size_t index) {
  return internal_set(index, hnExplicitIndex);
}

bool HUDNavigationQueue::set(HUDuiControl* control) {
  if (!control || !size()) { return false; }

  for (size_t i = 0; i < size(); ++i)
    if (at(i) == control) {
      return internal_set(i, hnExplicitPointer);
    }

  return false;
}

bool HUDNavigationQueue::setWithoutFocus(size_t index) {
  return internal_set(index, hnExplicitIndex, false);
}

bool HUDNavigationQueue::setWithoutFocus(HUDuiControl* control) {
  if (!control || !size()) { return false; }

  for (size_t i = 0; i < size(); ++i)
    if (at(i) == control) {
      return internal_set(i, hnExplicitPointer, false);
    }

  return false;
}

HUDuiControl* HUDNavigationQueue::get() const {
  if (!size()) { return NULL; }

  return at(focus);
}

size_t HUDNavigationQueue::getIndex() const {
  return focus;
}

bool HUDNavigationQueue::internal_set(size_t index, HUDNavChangeMethod changeMethod, bool setFocus) {
  if (index >= size()) { return false; }

  for (HUDuiNavCallbackList::iterator itr = callbackList.begin();
       itr != callbackList.end(); ++itr) {
    index = itr->first(focus, index, changeMethod, itr->second);
    if (index == SkipSetFocus) { break; }
  }

  if (index != SkipSetFocus) {
    focus = index;
    if (setFocus) {
      HUDui::setFocus(at(focus));
    }
  }
  return true;
}

void HUDNavigationQueue::addCallback(HUDNavigationCallback _cb, void* _data) {
  callbackList.push_back(std::make_pair<HUDNavigationCallback, void*>(_cb, _data));
}

void HUDNavigationQueue::removeCallback(HUDNavigationCallback _cb, void* data) {
  for (HUDuiNavCallbackList::iterator itr = callbackList.begin();
       itr != callbackList.end(); ++itr) {
    if (itr->first == _cb && itr->second == data) {
      callbackList.remove(*itr);
      return;
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
