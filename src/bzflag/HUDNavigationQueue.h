/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDNavigationQueue:
 *	User interface class to control menu navigation and focus.
 */

#ifndef	__HUDNAVIGATIONQUEUE_H__
#define	__HUDNAVIGATIONQUEUE_H__

/* common header */
#include "common.h"

/* system headers */
#include <deque>
#include <list>

class HUDuiControl;

enum HUDNavChangeMethod {
  hnNext,
  hnPrev,
  hnExplicitIndex,
  hnExplicitPointer,
  hnCount // last item
};

typedef size_t (*HUDNavigationCallback)(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void*);
typedef std::list< std::pair<HUDNavigationCallback, void*> > HUDuiNavCallbackList;

class HUDNavigationQueue : public std::deque<HUDuiControl*> {
public:
  HUDNavigationQueue();

  void next();
  void prev();

  bool set(size_t index);
  bool set(HUDuiControl* control);

  bool setWithoutFocus(size_t index);
  bool setWithoutFocus(HUDuiControl* control);

  HUDuiControl* get() const;
  size_t getIndex() const;

  void addCallback(HUDNavigationCallback cb, void* data);
  void removeCallback(HUDNavigationCallback cb, void* data);

  static const size_t SkipSetFocus = ~0U;

private:
  bool internal_set(size_t index, HUDNavChangeMethod changeMethod, bool setFocus = true);

  size_t focus;
  HUDuiNavCallbackList callbackList;
};

#endif // __HUDNAVIGATIONQUEUE_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
