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
#include "DisplayMenu.h"

/* system implementation headers */
#include <string>
#include <vector>

/* common implementation headers */
#include "BzfDisplay.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "MainWindow.h"

/* FIXME - from playing.h */
BzfDisplay* getDisplay();
MainWindow* getMainWindow();

DisplayMenu::DisplayMenu() : formatMenu(NULL)
{
  // add controls
  std::vector<std::string>* options;
  std::vector<HUDuiControl*>& list  = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Display Setting");
  list.push_back(label);

  BzfWindow* window = getMainWindow()->getWindow();
  HUDuiList* option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Brightness:");
  option->setCallback(callback, (void*)"g");
  if (window->hasGammaControl()) {
    option->createSlider(15);
  } else {
    options = &option->getList();
    options->push_back(std::string("Unavailable"));
  }
  option->update();
  list.push_back(option);

  BzfDisplay* display = getDisplay();
  int numFormats = display->getNumResolutions();
  if (numFormats < 2) {
    videoFormat = NULL;
  } else {
    videoFormat = label = new HUDuiLabel;
    label->setFont(MainMenu::getFont());
    label->setLabel("Change Video Format");
    list.push_back(label);
  }

  initNavigation(list, 1,list.size()-1);
}

DisplayMenu::~DisplayMenu()
{
  delete formatMenu;
}

void			DisplayMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == videoFormat) {
    if (!formatMenu)
      formatMenu = new FormatMenu;
    HUDDialogStack::get()->push(formatMenu);
  }
}

void			DisplayMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);
  int i;

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float fontWidth = (float)height / 30.0f;
  const float fontHeight = (float)height / 30.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width + 0.5f * titleWidth);
  y -= 0.6f * titleFont.getHeight();
  const int count = list.size();
  for (i = 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * list[i]->getFont().getHeight();
  }

  i = 1;
  // brightness
  BzfWindow* window = getMainWindow()->getWindow();
  if (window->hasGammaControl())
    ((HUDuiList*)list[i])->setIndex(gammaToIndex(window->getGamma()));
  i++;

}

int DisplayMenu::gammaToIndex(float gamma)
{
  return (int)(0.5f + 5.0f * (1.0f + logf(gamma) / logf(2.0)));
}

float DisplayMenu::indexToGamma(int index)
{
  // map index 5 to gamma 1.0 and index 0 to gamma 0.5
  return powf(2.0f, (float)index / 5.0f - 1.0f);
}

void			DisplayMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;
  switch (((const char*)data)[0]) {
  case 'g':
    BzfWindow* window = getMainWindow()->getWindow();
    if (window->hasGammaControl())
      window->setGamma(indexToGamma(list->getIndex()));
    break;
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
