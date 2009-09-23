/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "LocalFontFace.h"


OptionsMenu::OptionsMenu()
: guiOptionsMenu(NULL)
, textOptionsMenu(NULL)
, radarOptionsMenu(NULL)
, effectsMenu(NULL)
, cacheMenu(NULL)
, hubMenu(NULL)
, saveWorldMenu(NULL)
, inputMenu(NULL)
, audioMenu(NULL)
, displayMenu(NULL)
, saveMenu(NULL)
{
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

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

  textOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Text Settings");
  addControl(label);

  radarOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Radar Settings");
  addControl(label);

  effectsOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Effects Settings");
  addControl(label);

  cacheOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Cache Settings");
  addControl(label);

  hubOptions = label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setLabel("Hub Settings");
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
  delete textOptionsMenu;
  delete radarOptionsMenu;
  delete effectsMenu;
  delete cacheMenu;
  delete hubMenu;
  delete saveWorldMenu;
  delete inputMenu;
  delete audioMenu;
  delete displayMenu;
  delete saveMenu;
}


void OptionsMenu::execute()
{
  HUDuiControl* _focus = getNav().get();
  if (_focus == guiOptions) {
    if (!guiOptionsMenu) guiOptionsMenu = new GUIOptionsMenu;
    HUDDialogStack::get()->push(guiOptionsMenu);
  }
  else if (_focus == textOptions) {
    if (!textOptionsMenu) textOptionsMenu = new TextOptionsMenu;
    HUDDialogStack::get()->push(textOptionsMenu);
  }
  else if (_focus == radarOptions) {
    if (!radarOptionsMenu) radarOptionsMenu = new RadarOptionsMenu;
    HUDDialogStack::get()->push(radarOptionsMenu);
  }
  else if (_focus == effectsOptions) {
    if (!effectsMenu) effectsMenu = new EffectsMenu;
    HUDDialogStack::get()->push(effectsMenu);
  }
  else if (_focus == cacheOptions) {
    if (!cacheMenu) cacheMenu = new CacheMenu;
    HUDDialogStack::get()->push(cacheMenu);
  }
  else if (_focus == hubOptions) {
    if (!hubMenu) hubMenu = new HubMenu;
    HUDDialogStack::get()->push(hubMenu);
  }
  else if (_focus == saveWorld) {
    if (!saveWorldMenu) saveWorldMenu = new SaveWorldMenu;
    HUDDialogStack::get()->push(saveWorldMenu);
  }
  else if (_focus == saveSettings) {
    // save resources
    dumpResources();
    if (!saveMenu) saveMenu = new SaveMenu;
    if (alternateConfig == "") {
      std::string fname = getCurrentConfigFileName();
      CFGMGR.write(fname);
      saveMenu->setFileName(fname);
    } else {
      CFGMGR.write(alternateConfig);
      saveMenu->setFileName(alternateConfig);
    }
    HUDDialogStack::get()->push(saveMenu);
  }
  else if (_focus == inputSetting) {
    if (!inputMenu) inputMenu = new InputMenu;
    HUDDialogStack::get()->push(inputMenu);
  }
  else if (_focus == audioSetting) {
    if (!audioMenu) audioMenu = new AudioMenu;
    HUDDialogStack::get()->push(audioMenu);
  }
  else if (_focus == displaySetting) {
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
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace->getFMFace(), "headerFontSize");

  fs.setMin(0, 20);
  const float fontSize = fs.getFontSize(fontFace->getFMFace(), "menuFontSize");

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options in two columns
  x = 0.5f * (float)_width;
  y -= 1.0f * titleHeight;
  const int count = (const int)listHUD.size();
  const float h = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    if ((i == 9) || (i == 11)) {
      y -= 1.75f * h;
    } else {
      y -= 1.0f * h;
    }
  }

  // load current settings
  i = 10;
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
