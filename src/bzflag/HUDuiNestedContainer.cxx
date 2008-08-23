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

// interface headers
#include "HUDuiNestedContainer.h"

// common implementation headers
#include "HUDNavigationQueue.h"

//
// HUDuiNestedContainer
//

HUDuiNestedContainer::HUDuiNestedContainer() : HUDuiControl()
{
  // Do nothing
}

HUDuiNestedContainer::~HUDuiNestedContainer()
{
  // clean up
  getNavList()->removeCallback(gotFocus, this);
}

void HUDuiNestedContainer::addControl(HUDuiControl *control)
{
  if (control == NULL)
    return;
  nestedNavList.push_back(control);
  if ((nestedNavList.size() == 1)&&(hasFocus()))
    nestedNavList.setWithoutFocus((size_t) 0);
  control->setNavQueue(&nestedNavList);
  control->isNested(true);
  control->setParent(this);
}

void HUDuiNestedContainer::setNavQueue(HUDNavigationQueue* _navList)
{
  // if we're changing nav lists, pull the old callback
  if (getNavList())
    getNavList()->removeCallback(gotFocus, this);

  // then set the list
  HUDuiControl::setNavQueue(_navList);

  // then install a new callback
  _navList->addCallback(gotFocus, this);
}

// Pass the focus down to the container's children, unless the container itself has focus
size_t HUDuiNestedContainer::gotFocus(size_t /*oldFocus*/, size_t proposedFocus, HUDNavChangeMethod /*changeMethod*/, void* data)
{
  // If the nested container is getting focus, decide who to pass focus to
  if (((HUDuiNestedContainer*)data)->isAtNavQueueIndex((int) proposedFocus))
  {
    // If the container is empty...
    if (((HUDuiNestedContainer*)data)->getNav().size() <= 0)
      return HUDNavigationQueue::SkipSetFocus;

    // Find out which control inside the container has focus
    HUDuiControl* childInFocus = ((HUDuiNestedContainer*)data)->getNav().get();

    // If the container itself has focus, we're done here
    if (childInFocus == ((HUDuiControl*) data))
      return proposedFocus;

    // Otherwise recurse through and find the actual control that should be in focus
    childInFocus = recursiveFocus(childInFocus);

    // Set that control in it's parent navList, to grab focus
    if (childInFocus != NULL)
      ((HUDuiNestedContainer*)childInFocus)->getNavList()->set(childInFocus);

    return HUDNavigationQueue::SkipSetFocus;
  }
  else
  {
    return proposedFocus;
  }
}

HUDuiControl* HUDuiNestedContainer::recursiveFocus(HUDuiControl* control)
{
  // Base case, the control isn't a container, give it focus
  if (!control->isContainer()) {
    return control;
  }
  else {
    // Container is empty, set focus to the actual container
    if (((HUDuiNestedContainer*)control)->getNav().size() == 0)
    {
      ((HUDuiNestedContainer*)control)->getNavList()->set(control);
      return NULL;
    }
    HUDuiControl* childControl = ((HUDuiNestedContainer*)control)->getNav().get();
    if (childControl == control)
    {
      ((HUDuiNestedContainer*)control)->getNavList()->set(childControl);
      return NULL;
    }
    return recursiveFocus(childControl);
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
