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
#include "HUDuiTypeIn.h"

AudioMenu::AudioMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  std::string currentDriver = BZDB.get("audioDriver");
  std::string currentDevice = BZDB.get("audioDevice");

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Audio Settings");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options;

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
  list.push_back(option);

/* Right now only SDL_Media has a setDriver function.
   Disable driver selection for others as it gets saved in config
   and can screw things up if you switch from non-SDL to SDL build.
   If more platforms get setDriver functions, they can be added. */
#ifdef HAVE_SDL
  driver = new HUDuiTypeIn;
  driver->setFontFace(MainMenu::getFontFace());
  driver->setLabel("Driver:");
  driver->setMaxLength(10);
  driver->setString(currentDriver);
  list.push_back(driver);
#else
  driver = NULL;
#endif // HAVE_SDL

#ifdef HAVE_SDL
  device = new HUDuiTypeIn;
  device->setFontFace(MainMenu::getFontFace());
  device->setLabel("Device:");
  device->setMaxLength(10);
  device->setString(currentDevice);
  list.push_back(device);
#else
  device = NULL;
#endif // HAVE_SDL

  initNavigation(list, 1,list.size()-1);
}

AudioMenu::~AudioMenu()
{
}

void			AudioMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == driver) {
    BZDB.set("audioDriver", driver->getString().c_str());
  } else if (focus == device) {
    BZDB.set("audioDevice", device->getString().c_str());
  }
}

void			AudioMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);
  int i;

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)height / 15.0f;
  const float fontSize = (float)height / 45.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(fontFace, titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(fontFace, titleFontSize, " ");
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const int count = list.size();
  for (i = 1; i < count; i++) {
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  i = 1;
  // sound
  ((HUDuiList*)list[i++])->setIndex(getSoundVolume());
}

void			AudioMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;
  std::vector<std::string> *options = &list->getList();
  std::string selectedOption = (*options)[list->getIndex()];
  switch (((const char*)data)[0]) {
    case 's':
      BZDB.set("volume", string_util::format("%d", list->getIndex()));
      setSoundVolume(list->getIndex());
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
