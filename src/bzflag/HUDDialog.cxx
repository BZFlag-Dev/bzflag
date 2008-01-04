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

/* interface header */
#include "HUDDialog.h"

// local interface headers
#include "HUDuiElement.h"
#include "HUDuiControl.h"
#include "HUDNavigationQueue.h"

HUDDialog::HUDDialog()
{
  height = width = 0;
}

HUDDialog::~HUDDialog()
{
  // delete all elements left on render list
  size_t count = renderList.size();
  for (size_t i = 0; i < count; i++)
    delete renderList[i];
}

void			HUDDialog::render()
{
  // render all elements on the render list
  size_t count = renderList.size();
  for (size_t i = 0; i < count; i++)
    renderList[i]->render();
}

void			HUDDialog::resize(int _width, int _height)
{
  width		= _width;
  height	= _height;
}

void			HUDDialog::addControl(HUDuiElement *element)
{
  renderList.push_back(element);
}

void			HUDDialog::addControl(HUDuiControl *control, bool navigable)
{
  addControl((HUDuiElement*)control);
  if (navigable)
    navList.push_back(control);
}

void			HUDDialog::initNavigation()
{
  for (HUDNavigationQueue::iterator itr = navList.begin(); itr != navList.end(); ++itr)
    (*itr)->setNavQueue(&navList);
  navList.set((size_t)0);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
