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
#include "EffectsMenu.h"

/* system implementation headers */
#include <string>
#include <vector>

/* common implementation headers */
#include "BundleMgr.h"
#include "BZDBCache.h"
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "MainWindow.h"
#include "World.h"
#include "SceneRenderer.h"
#include "TrackMarks.h"
#include "HUDDialogStack.h"
#include "HUDuiControl.h"
#include "HUDuiList.h"
#include "HUDuiLabel.h"


EffectsMenu::EffectsMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // the menu label
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Effects Options");
  list.push_back(label);

  // the menu options
  HUDuiList* option;
  std::vector<std::string>* options;

  // Rain Scale
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Rain:");
  option->setCallback(callback, (void*)"r");
  options = &option->getList();
  options->push_back(std::string("Off"));
  option->createSlider(10);
  option->update();
  list.push_back(option);

  // The Mirror
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Mirror:");
  option->setCallback(callback, (void*)"m");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);
  
  // Display Treads
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Display Treads:");
  option->setCallback(callback, (void*)"T");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);
  
  // Track Mark Fading Scale
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("TrackMarks:");
  option->setCallback(callback, (void*)"t");
  options = &option->getList();
  options->push_back(std::string("Off"));
  option->createSlider(10);
  option->update();
  list.push_back(option);

  initNavigation(list, 1, list.size() - 1);
}


EffectsMenu::~EffectsMenu()
{
}


void EffectsMenu::execute()
{
}


void EffectsMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  const float fontSize = (float)height / 45.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth =
    fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight =
    fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width + 0.5f * titleWidth);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  const int count = list.size();
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  // load current settings
  int i = 1;
  ((HUDuiList*)list[i++])->setIndex(int((BZDB.eval("userRainScale") * 10.0f) + 0.5f));
  ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("userMirror") ? 1 : 0);
  ((HUDuiList*)list[i++])->setIndex(BZDB.isTrue("showTreads") ? 1 : 0);
  ((HUDuiList*)list[i++])->setIndex(int((TrackMarks::getUserFade() * 10.0f) + 0.5f));
}


void EffectsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'r': {
      int scale = list->getIndex();
      BZDB.setFloat("userRainScale", float(scale) / 10.0f);
      break;
    }
    case 'm': {
      BZDB.set("userMirror", list->getIndex() ? "1" : "0");
      break;
    }
    case 'T': {
      BZDB.set("showTreads", list->getIndex() ? "1" : "0");
      break;
    }
    case 't': {
      int fade = list->getIndex();
      TrackMarks::setUserFade(float(fade) / 10.0f);
      break;
    }
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
