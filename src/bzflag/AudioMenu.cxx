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

/* local implementation headers */
#include "MainMenu.h"
#include "sound.h"

AudioMenu::AudioMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();
  std::string currentDriver = BZDB.get("audioDriver");
  std::string currentDevice = BZDB.get("audioDevice");

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Audio Setting");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options;

  option = new HUDuiList;
  option->setFont(MainMenu::getFont());
  option->setLabel("Sound Volume:");
  option->setCallback(callback, (void*)"s");
  options = &option->getList();
  if (isSoundOpen()) {
    options->push_back(std::string("Off"));
    option->createSlider(10);
  }
  else {
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
  driver->setFont(MainMenu::getFont());
  driver->setLabel("Driver:");
  driver->setMaxLength(10);
  driver->setString(currentDriver);
  list.push_back(driver);
#else
  driver = NULL;
#endif // HAVE_SDL

#ifdef HAVE_SDL
  device = new HUDuiTypeIn;
  device->setFont(MainMenu::getFont());
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
