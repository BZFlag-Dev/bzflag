/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "HUDDialog.h"

/* system headers */
#include <vector>


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

void			HUDDialog::initNavigation(std::vector<HUDuiControl*> &list, int start, int end)
{
  int i;
  const int count = list.size();

  for (i = 0; i < start; i++) {
    list[i]->setNext(list[i]);
    list[i]->setPrev(list[i]);
  }

  if (start < end) {
    list[start]->setNext(list[start+1]);
    list[start]->setPrev(list[end]);
    for (i = start+1; i < end; i++) {
	list[i]->setNext(list[i+1]);
	list[i]->setPrev(list[i-1]);
    }
    list[end]->setNext(list[start]);
    list[end]->setPrev(list[end-1]);
  }

  for (i = end+1; i < count; i++) {
    list[i]->setNext(list[i]);
    list[i]->setPrev(list[i]);
  }

  setFocus(list[start]);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
