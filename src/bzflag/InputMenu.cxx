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
#include "InputMenu.h"

/* system implementation headers */
#include <string>
#include <vector>

/* common implementation headers */
#include "BzfWindow.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "MainWindow.h"
#include "MainMenu.h"

/* FIXME - from playing.h */
MainWindow*    getMainWindow();


InputMenu::InputMenu()
{
  std::string currentJoystickDevice = BZDB.get("joystickname");
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Input Setting");
  list.push_back(label);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options = &option->getList();

  // set joystick Device
  option->setFont(MainMenu::getFont());
  option->setLabel("Joystick device:");
  option->setCallback(callback, (void*)"J");
  options = &option->getList();
  options->push_back(std::string("off"));
  std::vector<std::string> joystickDevices;
  getMainWindow()->getJoyDevices(joystickDevices);
  for (int i = 0; i < (int)joystickDevices.size(); i++) {
    options->push_back(joystickDevices[i]);
  }
  joystickDevices.erase(joystickDevices.begin(), joystickDevices.end());
  for (int i = 0; i < (int)options->size(); i++) {
    if ((*options)[i].compare(currentJoystickDevice) == 0) {
      option->setIndex(i);
      break;
    }
  }
  option->update();
  list.push_back(option);

  initNavigation(list, 1,list.size()-1);
}

InputMenu::~InputMenu()
{
}

void			InputMenu::execute()
{
}

void			InputMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;
  switch (((const char*)data)[0]) {
    case 'J':
      std::vector<std::string> *options = &list->getList();
      std::string joyDev = (*options)[list->getIndex()];
      BZDB.set("joystickname", joyDev);
      getMainWindow()->initJoystick(joyDev);
  }
}

void			InputMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

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
  for (int i = 1; i < count; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * list[i]->getFont().getHeight();
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
