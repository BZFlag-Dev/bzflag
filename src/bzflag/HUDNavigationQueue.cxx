/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

HUDNavigationQueue::HUDNavigationQueue() :
    focus(0), cb(NULL)
{
}

void HUDNavigationQueue::next()
{
  if (!size()) return;

  size_t oldfocus = focus;
  if (++focus > (size() - 1))
    focus = 0;

  if (cb)
    focus = cb(oldfocus, focus, hnNext, userData);

  HUDui::setFocus(at(focus));
}

void HUDNavigationQueue::prev()
{
  if (!size()) return;

  size_t oldfocus = focus;
  if (--focus > (size() - 1)) // unsigned, so wraps around
    focus = (size() - 1);

  if (cb)
    focus = cb(oldfocus, focus, hnPrev, userData);

  HUDui::setFocus(at(focus));
}

bool HUDNavigationQueue::set(size_t index)
{
  if (index >= size()) return false;

  if (cb)
    focus = cb(focus, index, hnExplicitIndex, userData);
  else
    focus = index;

  HUDui::setFocus(at(focus));
  return true;
}

bool HUDNavigationQueue::set(HUDuiControl* control)
{
  if (!control || !size()) return false;

  for (size_t i = 0; i < size(); ++i)
    if (at(i) == control) {
      if (cb)
	focus = cb(focus, i, hnExplicitPointer, userData);
      else
	focus = i;

      HUDui::setFocus(at(focus));
      return true;
    }

  return false;
}

HUDuiControl* HUDNavigationQueue::get() const
{
  if (!size()) return NULL;

  return at(focus);
}

size_t HUDNavigationQueue::getIndex() const
{
  return focus;
}

void HUDNavigationQueue::setCallback(HUDNavigationCallback _cb, void* _data)
{
  cb = _cb;
  userData = _data;
}

HUDNavigationCallback HUDNavigationQueue::getCallback() const
{
  return cb;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
