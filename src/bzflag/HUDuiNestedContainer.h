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
 * HUDuiNestedContainer:
 *	User interface class for the heads-up display's label (text display) control
 */

#ifndef	__HUDUINESTEDCONTAINER_H__
#define	__HUDUINESTEDCONTAINER_H__

// ancestor class
#include "HUDuiControl.h"

#include <string>
#include <vector>

#include "HUDNavigationQueue.h"

#include "BzfEvent.h"

class HUDuiNestedContainer : public HUDuiControl {
  public:
			HUDuiNestedContainer();
			~HUDuiNestedContainer();

    HUDuiControl* getFocus() const { return nestedNavList.get(); }

  protected:
    void addControl(HUDuiControl* control);

    const HUDNavigationQueue& getNav() const { return nestedNavList; }
    HUDNavigationQueue& getNav() { return nestedNavList; }

    void setNavQueue(HUDNavigationQueue*);
	
    static size_t gotFocus(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* data);

  private:
    HUDNavigationQueue nestedNavList;

};

#endif // __HUDUINESTEDCONTAINER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
