/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "HUDDialog.h"
#include "playing.h"
#include "MainWindow.h"
#include "BzfWindow.h"
#include "BzfDisplay.h"

//
// HUDDialog
//

HUDDialog::HUDDialog() : focus(NULL)
{
  // do nothing
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

HUDuiControl*		HUDDialog::getFocus() const
{
  return focus;
}

void			HUDDialog::setFocus(HUDuiControl* _focus)
{
  focus = _focus;
}

//
// HUDDialogStack
//

HUDDialogStack		HUDDialogStack::globalStack;

HUDDialogStack::HUDDialogStack()
{
  // do nothing
}

HUDDialogStack::~HUDDialogStack()
{
  if (getMainWindow())
    getMainWindow()->getWindow()->removeResizeCallback(resize, this);
}

HUDDialogStack*		HUDDialogStack::get()
{
  return &globalStack;
}

bool			HUDDialogStack::isActive() const
{
  return stack.size() != 0;
}

HUDDialog*		HUDDialogStack::top() const
{
  const int index = stack.size();
  if (index == 0) return NULL;
  return stack[index - 1];
}

void			HUDDialogStack::push(HUDDialog* dialog)
{
  if (!dialog) return;
  if (isActive()) {
    const int index = stack.size() - 1;
    stack[index]->setFocus(HUDui::getFocus());
    stack[index]->dismiss();
  }
  else {
    getMainWindow()->getWindow()->addResizeCallback(resize, this);
  }
  stack.push_back(dialog);
  HUDui::setDefaultKey(dialog->getDefaultKey());
  HUDui::setFocus(dialog->getFocus());
  dialog->resize(getMainWindow()->getWidth(), getMainWindow()->getHeight());
  dialog->show();
}

void			HUDDialogStack::pop()
{
  if (isActive()) {
    const int index = stack.size() - 1;
    stack[index]->setFocus(HUDui::getFocus());
    stack[index]->dismiss();
    std::vector<HUDDialog*>::iterator it = stack.begin();
    for(int i = 0; i < index; i++) it++;
    stack.erase(it);
    if (index > 0) {
      HUDDialog* dialog = stack[index - 1];
      HUDui::setDefaultKey(dialog->getDefaultKey());
      HUDui::setFocus(dialog->getFocus());
      dialog->resize(getMainWindow()->getWidth(),
			getMainWindow()->getHeight());
      dialog->show();
    }
    else {
      HUDui::setDefaultKey(NULL);
      HUDui::setFocus(NULL);
      getMainWindow()->getWindow()->removeResizeCallback(resize, this);
    }
  }
}

void			HUDDialogStack::render()
{
  if (isActive())
    stack[stack.size() - 1]->render();
}

void			HUDDialogStack::resize(void* _self)
{
  HUDDialogStack* self = (HUDDialogStack*)_self;
  if (self->isActive())
    self->top()->resize(getMainWindow()->getWidth(),
			getMainWindow()->getHeight());
}
// ex: shiftwidth=2 tabstop=8
