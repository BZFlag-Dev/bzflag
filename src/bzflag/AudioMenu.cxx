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
#include "AudioMenu.h"

/* system implementation headers */
#include <string>
#include <vector>

/* common implementation headers */
#include "TextUtils.h"
#include "FontManager.h"

/* local implementation headers */
#include "StateDatabase.h"
#include "MainMenu.h"
#include "sound.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"

AudioMenu::AudioMenu()
{
  // add controls
  std::string currentDriver = BZDB.get("audioDriver");
  std::string currentDevice = BZDB.get("audioDevice");

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Audio Settings");
  addControl(label, false);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options;

  // Sound Volume
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Sound Volume:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  if (isSoundOpen()) {
    options->push_back(std::string("Off"));
    option->createSlider(10);
  } else {
    options->push_back(std::string("Unavailable"));
  }
  option->update();
  addControl(option);

/* Right now only SDL_Media has a setDriver function.
   Disable driver selection for others as it gets saved in config
   and can screw things up if you switch from non-SDL to SDL build.
   If more platforms get setDriver functions, they can be added. */

  // Driver
#ifdef HAVE_SDL
  driver = new HUDuiTypeIn;
  driver->setFontFace(MainMenu::getFontFace());
  driver->setLabel("Driver:");
  driver->setMaxLength(10);
  driver->setString(currentDriver);
  addControl(driver);
#else
  driver = NULL;
#endif // HAVE_SDL

  // Device
#ifdef HAVE_SDL
  device = new HUDuiTypeIn;
  device->setFontFace(MainMenu::getFontFace());
  device->setLabel("Device:");
  device->setMaxLength(10);
  device->setString(currentDevice);
  addControl(device);
#else
  device = NULL;
#endif // HAVE_SDL

  // Remotes Sounds
  option = new HUDuiList;
  option->setFontFace(MainMenu::getFontFace());
  option->setLabel("Remote Sounds:");
  option->setCallback(callback, (void*)"r");
  options = &option->getList();
  options->push_back(std::string("Off"));
  options->push_back(std::string("On"));
  option->update();
  addControl(option);

  initNavigation();
}

AudioMenu::~AudioMenu()
{
}

void			AudioMenu::execute()
{
  HUDuiControl* _focus = getNav().get();
  if (_focus == driver) {
    BZDB.set("audioDriver", driver->getString().c_str());
  } else if (_focus == device) {
    BZDB.set("audioDevice", device->getString().c_str());
  }
}

void			AudioMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  int i;

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 45.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(fontFace, titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(fontFace, titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const int count = (const int)listHUD.size();
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  i = 1;
  // sound
  ((HUDuiList*)listHUD[i++])->setIndex(getSoundVolume());
#ifdef HAVE_SDL
  i++; // driver
  i++; // device
#endif // HAVE_SDL
  ((HUDuiList*)listHUD[i++])->setIndex(BZDB.isTrue("remoteSounds") ? 1 : 0);
}

void			AudioMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;
  std::vector<std::string> *options = &list->getList();
  std::string selectedOption = (*options)[list->getIndex()];
  switch (((const char*)data)[0]) {
    case 's':
      BZDB.set("volume", TextUtils::format("%d", list->getIndex()));
      setSoundVolume(list->getIndex());
      break;
    case 'r':
      BZDB.setBool("remoteSounds", (list->getIndex() == 0) ? false : true);
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
