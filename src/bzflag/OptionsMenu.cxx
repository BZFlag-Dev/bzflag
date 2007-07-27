/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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

/* common implementation headers */
#include "FontManager.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "clientConfig.h"
#include "ConfigFileManager.h"
#include "bzflag.h"

OptionsMenu::OptionsMenu() : guiOptionsMenu(NULL), effectsMenu(NULL),
			     cacheMenu(NULL), saveWorldMenu(NULL),
			     inputMenu(NULL), audioMenu(NULL),
			     displayMenu(NULL)
{
  // cache font face ID
  int fontFace = MainMenu::getFontFace();

  // add controls
  HUDuiList* option;
  std::vector<std::string>* options;

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Options");
  addControl(label, false);

  inputSetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Input Settings");
  addControl(label);

  audioSetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Audio Settings");
  addControl(label);

  displaySetting = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Display Settings");
  addControl(label);

  guiOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("GUI Settings");
  addControl(label);

  effectsOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Effects Settings");
  addControl(label);

  cacheOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Cache Settings");
  addControl(label);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Auto Save Settings:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("On Exit"));
  option->update();
  addControl(option);

  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Save Identity:");
  option->setCallback(callback, (void*)"i");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Username only"));
  options->push_back(std::string("Username and password"));
  option->update();
  addControl(option);

  saveWorld = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Save World");
  addControl(label);

  saveSettings = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Save Settings Now");
  addControl(label);

  initNavigation();
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
  HUDuiControl* _focus = getNav().get();
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
  } else if (_focus == saveSettings) {
    // save resources
    dumpResources();
    if (alternateConfig == "")
      CFGMGR.write(getCurrentConfigFileName());
    else
      CFGMGR.write(alternateConfig);
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
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace, "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(fontFace, "menuFontSize");

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace, titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace, titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options in two columns
  x = 0.5f * (float)_width;
  y -= 1.0f * titleHeight;
  const int count = (const int)listHUD.size();
  const float h = fm.getStringHeight(fontFace, fontSize);
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
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
