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
#include "FontManager.h"

/* local implementation headers */
#include "MainWindow.h"
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"
#include "HUDuiList.h"

/* FIXME - from playing.h */
MainWindow*    getMainWindow();

InputMenu::InputMenu() : keyboardMapMenu(NULL)
{
  std::string currentJoystickDevice = BZDB.get("joystickname");
  // cache font face ID
  int fontFace = MainMenu::getFontFace();
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Input Settings");
  list.push_back(label);

  keyMapping = new HUDuiLabel;
  keyMapping->setFontFace(fontFace);
  keyMapping->setLabel("Change Key Mapping");
  list.push_back(keyMapping);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options = &option->getList();
  // set joystick Device
  option->setFontFace(fontFace);
  option->setLabel("Joystick device:");
  option->setCallback(callback, (void*)"J");
  options = &option->getList();
  options->push_back(std::string("off"));
  std::vector<std::string> joystickDevices;
  getMainWindow()->getJoyDevices(joystickDevices);
  int i;
  for (i = 0; i < (int)joystickDevices.size(); i++) {
    options->push_back(joystickDevices[i]);
  }
  joystickDevices.erase(joystickDevices.begin(), joystickDevices.end());
  for (i = 0; i < (int)options->size(); i++) {
    if ((*options)[i].compare(currentJoystickDevice) == 0) {
      option->setIndex(i);
      break;
    }
  }
  option->update();
  list.push_back(option);

  activeInput = new HUDuiList;
  activeInput->setFontFace(fontFace);
  activeInput->setLabel("Active input device:");
  activeInput->setCallback(callback, (void*)"A");
  options = &activeInput->getList();
  options->push_back("Auto");
  options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Keyboard));
  options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Mouse));
  options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Joystick));
  activeInput->update();
  list.push_back(activeInput);

  option = new HUDuiList;
  // set joystick Device
  option->setFontFace(fontFace);
  option->setLabel("Confine mouse:");
  option->setCallback(callback, (void*)"G");
  options = &option->getList();
  options->push_back(std::string("yes"));
  options->push_back(std::string("no"));
  if (getMainWindow()->isGrabEnabled())
    option->setIndex(0);
  else
    option->setIndex(1);
  option->update();
  list.push_back(option);

  initNavigation(list, 1,list.size()-1);
}

InputMenu::~InputMenu()
{
  delete keyboardMapMenu;
}

void			InputMenu::execute()
{
  HUDuiControl* focus = HUDui::getFocus();
  if (focus == keyMapping) {
    if (!keyboardMapMenu) keyboardMapMenu = new KeyboardMapMenu;
    HUDDialogStack::get()->push(keyboardMapMenu);
  }
}

void			InputMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* list = (HUDuiList*)w;
  std::vector<std::string> *options = &list->getList();
  std::string selectedOption = (*options)[list->getIndex()];
  switch (((const char*)data)[0]) {
    case 'J':
      BZDB.set("joystickname", selectedOption);
      getMainWindow()->initJoystick(selectedOption);
      break;
    case 'A':
      {
	LocalPlayer*   myTank = LocalPlayer::getMyTank();
	// Are we forced to use one input device, or do we allow it to change automatically?
	if (selectedOption == "Auto") {
	  BZDB.set("allowInputChange", "1");
	} else {
	  BZDB.set("allowInputChange", "0");
	  BZDB.set("activeInputDevice", selectedOption);
	  // Set the current input device to whatever we're forced to
	  if (myTank) {
	    myTank->setInputMethod(BZDB.get("activeInputDevice"));
	  }
	}
      }
      break;
    case 'G':
      bool grabbing = (selectedOption == "yes");
      if (grabbing)
	BZDB.set("mousegrab", "true");
      else
	BZDB.set("mousegrab", "false");
      getMainWindow()->enableGrabMouse(grabbing);
      break;
  }
}

void			InputMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);
  int i;

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

  // reposition options
  x = 0.5f * ((float)width + 0.5f * titleWidth);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  const int count = list.size();
  for (i = 1; i < count; i++) {
    list[i]->setFontSize(fontSize);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  // load current settings
  std::vector<std::string> *options = &activeInput->getList();
  for (i = 0; i < (int)options->size(); i++) {
    std::string currentOption = (*options)[i];
    if (BZDB.get("activeInputDevice") == currentOption)
      activeInput->setIndex(i);
  }
  if (BZDB.isTrue("allowInputChange"))
    activeInput->setIndex(0);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
