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

/* interface header */
#include "HUDDialog.h"

// local interface headers
#include "HUDuiControl.h"

HUDDialog::HUDDialog() : focus(NULL)
{
  height = width = 0;
}

HUDDialog::~HUDDialog()
{
  // delete all controls left on list
  const int count = list.size();
  for (int i = 0; i < count; i++)
    delete list[i];
}

void			HUDDialog::render()
{
  const int count = list.size();
  for (int i = 0; i < count; i++)
    list[i]->render();
}

void			HUDDialog::resize(int _width, int _height)
{
  width		= _width;
  height	= _height;
}

HUDuiControl*		HUDDialog::getFocus() const
{
  return focus;
}

void			HUDDialog::setFocus(HUDuiControl* _focus)
{
  focus = _focus;
}

void HUDDialog::initNavigation(std::vector<HUDuiControl*> &listHUD,
			       int start, int end)
{
  int i;
  const int count = listHUD.size();

  for (i = 0; i < start; i++) {
    listHUD[i]->setNext(listHUD[i]);
    listHUD[i]->setPrev(listHUD[i]);
  }

  if (start < end) {
    listHUD[start]->setNext(listHUD[start+1]);
    listHUD[start]->setPrev(listHUD[end]);
    for (i = start+1; i < end; i++) {
	listHUD[i]->setNext(listHUD[i+1]);
	listHUD[i]->setPrev(listHUD[i-1]);
    }
    listHUD[end]->setNext(listHUD[start]);
    listHUD[end]->setPrev(listHUD[end-1]);
  }

  for (i = end+1; i < count; i++) {
    listHUD[i]->setNext(listHUD[i]);
    listHUD[i]->setPrev(listHUD[i]);
  }

  setFocus(listHUD[start]);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
