/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include "ServerListCache.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"
#include "playing.h"

OptionsMenu::OptionsMenu() : guiOptionsMenu(NULL), effectsMenu(NULL),
			     cacheMenu(NULL), saveWorldMenu(NULL),
			     inputMenu(NULL), audioMenu(NULL),
			     displayMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiList* option;
  std::vector<std::string>* options;

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Options");
  listHUD.push_back(label);

  inputSetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Input Settings");
  listHUD.push_back(label);

  audioSetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Audio Settings");
  listHUD.push_back(label);

  displaySetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Display Settings");
  listHUD.push_back(label);

  guiOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("GUI Settings");
  listHUD.push_back(label);

  effectsOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Effects Settings");
  listHUD.push_back(label);

  cacheOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Cache Settings");
  listHUD.push_back(label);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Save Settings:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("On Exit"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Save identity:");
  option->setCallback(callback, (void*)"i");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Username only"));
  options->push_back(std::string("Username and password"));
  option->update();
  listHUD.push_back(option);

  saveWorld = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Save World");
  listHUD.push_back(label);

  initNavigation(listHUD, 1, listHUD.size()-1);
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
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == guiOptions) {
    if (!guiOptionsMenu) guiOptionsMenu = new GUIOptionsMenu;
    HUDDialogStack::get()->push(guiOptionsMenu);
  } else if (_focus == effectsOptions) {
    if (!effectsMenu) effectsMenu = new EffectsMenu;
    HUDDialogStack::get()->push(effectsMenu);
  } else if (_focus == cacheOptions) {
    if (!cacheMenu) cacheMenu = new CacheMenu;
    HUDDialogStack::get()->push(cacheMenu);
  } else if (_focus == saveWorld) {
    if (!saveWorldMenu) saveWorldMenu = new SaveWorldMenu;
    HUDDialogStack::get()->push(saveWorldMenu);
  } else if (_focus == inputSetting) {
    if (!inputMenu) inputMenu = new InputMenu;
    HUDDialogStack::get()->push(inputMenu);
  } else if (_focus == audioSetting) {
    if (!audioMenu) audioMenu = new AudioMenu;
    HUDDialogStack::get()->push(audioMenu);
  } else if (_focus == displaySetting) {
    if (!displayMenu) displayMenu = new DisplayMenu;
    HUDDialogStack::get()->push(displayMenu);
  }
}

void OptionsMenu::resize(int _width, int _height)
{
  int i;
  HUDDialog::resize(_width, _height);

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 45.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(MainMenu::getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(MainMenu::getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options in two columns
  x = 0.5f * (float)_width;
  y -= 0.6f * titleHeight;
  const int count = listHUD.size();
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  for (i = 1; i < count; i++) {
    HUDuiControl *ctl = listHUD[i];
    ctl->setFontSize(fontSize);
    ctl->setPosition(x, y);
    if ((i == 6) || (i == 8)) {
      y -= 1.75f * h;
    } else {
      y -= 1.0f * h;
    }
  }

  // load current settings
  i = 7;

  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("saveSettings"));
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.evalInt("saveIdentity"));
}

void OptionsMenu::callback(HUDuiControl* w, void* data)
{
  HUDuiList* listHUD = (HUDuiList*)w;

  switch (((const char*)data)[0]) {
    case 's': { // save settings
	BZDB.setInt("saveSettings", listHUD->getIndex());
	break;
    }
    case 'i': { // save identity
	BZDB.setInt("saveIdentity", listHUD->getIndex());
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
