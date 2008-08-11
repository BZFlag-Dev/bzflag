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
 *	User interface base class for nested containers. Nested containers
 *	allow child controls to be added to a control.
 */

#ifndef	__HUDUINESTEDCONTAINER_H__
#define	__HUDUINESTEDCONTAINER_H__

// ancestor class
#include "HUDuiControl.h"

#include "HUDNavigationQueue.h"

class HUDuiNestedContainer : public HUDuiControl {
  public:
      HUDuiNestedContainer();
      ~HUDuiNestedContainer();

    HUDuiControl* getFocus() const { return nestedNavList.get(); }
    const HUDNavigationQueue& getNav() const { return nestedNavList; }
    HUDNavigationQueue& getNav() { return nestedNavList; }

    bool isContainer() { return true; }

  protected:
    void addControl(HUDuiControl* control);

    void setNavQueue(HUDNavigationQueue*);
	
    static size_t gotFocus(size_t oldFocus, size_t proposedFocus, HUDNavChangeMethod changeMethod, void* data);

    static HUDuiControl* recursiveFocus(HUDuiControl* control);

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