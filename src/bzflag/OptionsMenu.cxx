/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include "OptionsMenu.h"

/* system implementation headers */
#include <vector>
#include <string>

/* common implementation headers */
#include "FontManager.h"
#include "StartupInfo.h"
#include "StateDatabase.h"
#include "TextUtils.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "ControlPanel.h"
#include "ServerListCache.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"

/* FIXME - from playing.h */
StartupInfo* getStartupInfo();
extern ControlPanel* controlPanel;


OptionsMenu::OptionsMenu() : guiOptionsMenu(NULL), effectsMenu(NULL),
			     cacheMenu(NULL), saveWorldMenu(NULL),
			     inputMenu(NULL), audioMenu(NULL),
			     displayMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiList* option;
  std::vector<std::string>* options;

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Options");
  list.push_back(label);

  inputSetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Input Settings");
  list.push_back(label);

  audioSetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Audio Settings");
  list.push_back(label);

  displaySetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Display Settings");
  list.push_back(label);

  guiOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("GUI Settings");
  list.push_back(label);

  effectsOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Effects Settings");
  list.push_back(label);

  cacheOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Cache Settings");
  list.push_back(label);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Save identity:");
  option->setCallback(callback, (void*)"i");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Username only"));
  options->push_back(std::string("Username and password"));
  option->update();
  list.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("UDP network connection:");
  option->setCallback(callback, (void*)"U");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  list.push_back(option);

  saveWorld = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Save World");
  list.push_back(label);

  initNavigation(list, 1,list.size()-1);
}

OptionsMenu::~OptionsMenu()
{
  delete guiOptionsMenu;
  delete effectsMenu;
  delete cacheMenu;
  delete saveWorldMenu;
  delete inputMenu;
  delete audioMenu;
  delete displayMenu;
}

void OptionsMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == guiOptions) {
    if (!guiOptionsMenu) guiOptionsMenu = new GUIOptionsMenu;
    HUDDialogStack::get()->push(guiOptionsMenu);
  } else if (focus == effectsOptions) {
    if (!effectsMenu) effectsMenu = new EffectsMenu;
    HUDDialogStack::get()->push(effectsMenu);
  } else if (focus == cacheOptions) {
    if (!cacheMenu) cacheMenu = new CacheMenu;
    HUDDialogStack::get()->push(cacheMenu);
  } else if (focus == saveWorld) {
    if (!saveWorldMenu) saveWorldMenu = new SaveWorldMenu;
    HUDDialogStack::get()->push(saveWorldMenu);
  } else if (focus == inputSetting) {
    if (!inputMenu) inputMenu = new InputMenu;
    HUDDialogStack::get()->push(inputMenu);
  } else if (focus == audioSetting) {
    if (!audioMenu) audioMenu = new AudioMenu;
    HUDDialogStack::get()->push(audioMenu);
  } else if (focus == displaySetting) {
    if (!displayMenu) displayMenu = new DisplayMenu;
    HUDDialogStack::get()->push(displayMenu);
  }
}

void OptionsMenu::resize(int width, int height)
{
  int i;
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  const float fontSize = (float)height / 45.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleHeight;
  title->setPosition(x, y);

  // reposition options in two columns
  x = 0.5f * (float)width;
  y -= 0.6f * titleHeight;
  const int count = list.size();
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  for (i = 1; i < count; i++) {
    HUDuiControl *ctl = list[i];
    ctl->setFontSize(fontSize);
    ctl->setPosition(x, y);
    if ((i == 6) || (i == 8)) {
      y -= 1.75f * h;
    } else {
      y -= 1.0f * h;
    }
  }

  // load current settings
  {
    int i = 7;

    ((HUDuiList*)list[i++])->setIndex((int)BZDB.eval("saveIdentity"));

    // mind the ++i !
    const StartupInfo* info = getStartupInfo();
    ((HUDuiList*)list[i++])->setIndex(info->useUDPconnection ? 1 : 0);
  }
}

void OptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* list = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 'U': {
      StartupInfo* info = getStartupInfo();
      info->useUDPconnection = (list->getIndex() != 0);
      break;
    }
    case 'i': { // save identity
	BZDB.setInt("saveIdentity", list->getIndex());
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
