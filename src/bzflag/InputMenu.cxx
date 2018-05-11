/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "InputMenu.h"

/* common implementation headers */
#include "StateDatabase.h"
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "playing.h"
#include "HUDui.h"

InputMenu::InputMenu() : keyboardMapMenu(NULL)
{
  std::string currentJoystickDevice = BZDB.get("joystickname");
  // cache font face ID
  int fontFace = MainMenu::getFontFace();
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Input Settings");
  listHUD.push_back(label);

  keyMapping = new HUDuiLabel;
  keyMapping->setFontFace(fontFace);
  keyMapping->setLabel("Change Key Mapping");
  listHUD.push_back(keyMapping);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options = &option->getList();
  // set joystick Device
  option->setFontFace(fontFace);
  option->setLabel("Joystick device:");
  option->setCallback(callback, "J");
  options = &option->getList();
  options->push_back(std::string("Off"));
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
  listHUD.push_back(option);

  activeInput = new HUDuiList;
  activeInput->setFontFace(fontFace);
  activeInput->setLabel("Active input device:");
  activeInput->setCallback(callback, "A");
  options = &activeInput->getList();
  options->push_back("Auto");
  options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Keyboard));
  options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Mouse));
  options->push_back(LocalPlayer::getInputMethodName(LocalPlayer::Joystick));
  activeInput->update();
  listHUD.push_back(activeInput);

  option = new HUDuiList;
  // force feedback
  option->setFontFace(fontFace);
  option->setLabel("Force feedback:");
  option->setCallback(callback, "F");
  options = &option->getList();
  options->push_back(std::string("None"));
  options->push_back(std::string("Rumble"));
  options->push_back(std::string("Directional"));
  for (i = 0; i < (int)options->size(); i++) {
    std::string currentOption = (*options)[i];
    if (BZDB.get("forceFeedback") == currentOption)
      option->setIndex(i);
  }
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  // axis settings
  jsx = option;
  option->setFontFace(fontFace);
  option->setLabel("Joystick X Axis:");
  option->setCallback(callback, "X");
  listHUD.push_back(option);
  option = new HUDuiList;
  jsy = option;
  option->setFontFace(fontFace);
  option->setLabel("Joystick Y Axis:");
  option->setCallback(callback, "Y");
  listHUD.push_back(option);
  fillJSOptions();
  option = new HUDuiList;
  option->setFontFace(fontFace);
  option->setLabel("Invert Joystick Axes:");
  option->setCallback(callback, "I");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("X"));
  options->push_back(std::string("Y"));
  options->push_back(std::string("X and Y"));
  option->setIndex(BZDB.evalInt("jsInvertAxes"));
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  // confine mouse
  option->setFontFace(fontFace);
  option->setLabel("Confine mouse:");
  option->setCallback(callback, "G");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Window"));
  options->push_back(std::string("MotionBox"));
  if (getMainWindow()->isGrabEnabled()) {
    option->setIndex(1);
  } else if (BZDB.isTrue("mouseClamp")) {
    option->setIndex(2);
  } else {
    option->setIndex(0);
  }
  option->update();
  listHUD.push_back(option);

  option = new HUDuiList;
  // jump while typing on/off
  option->setFontFace(fontFace);
  option->setLabel("Jump while typing:");
  option->setCallback(callback, "H");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Yes"));
  option->setIndex(BZDB.isTrue("jumpTyping") ? 1 : 0);
  option->update();
  listHUD.push_back(option);

  initNavigation(listHUD, 1,listHUD.size()-1);
}

InputMenu::~InputMenu()
{
  delete keyboardMapMenu;
}

void InputMenu::fillJSOptions()
{
  std::vector<std::string>* xoptions = &jsx->getList();
  std::vector<std::string>* yoptions = &jsy->getList();
  std::vector<std::string> joystickAxes;
  getMainWindow()->getJoyDeviceAxes(joystickAxes);
  if (joystickAxes.empty())
    joystickAxes.push_back("N/A");
  int i;
  for (i = 0; i < (int)joystickAxes.size(); i++) {
    xoptions->push_back(joystickAxes[i]);
    yoptions->push_back(joystickAxes[i]);
  }
  bool found = false;
  for (i = 0; i < (int)xoptions->size(); i++) {
    std::string currentOption = (*xoptions)[i];
    if (BZDB.get("jsXAxis") == currentOption) {
      jsx->setIndex(i);
      found = true;
    }
  }
  if (!found)
    jsx->setIndex(0);
  jsx->update();
  found = false;
  for (i = 0; i < (int)yoptions->size(); i++) {
    std::string currentOption = (*yoptions)[i];
    if (BZDB.get("jsYAxis") == currentOption) {
      jsy->setIndex(i);
      found = true;
    }
  }
  if (!found) {
    if (yoptions->size() > 1)
      jsy->setIndex(1);
    else
      jsy->setIndex(0);
  }
  jsy->update();
}

void			InputMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == keyMapping) {
    if (!keyboardMapMenu) keyboardMapMenu = new KeyboardMapMenu;
    HUDDialogStack::get()->push(keyboardMapMenu);
  }
}

void			InputMenu::callback(HUDuiControl* w, const void* data) {
  HUDuiList* listHUD = (HUDuiList*)w;
  std::vector<std::string> *options = &listHUD->getList();
  std::string selectedOption = (*options)[listHUD->getIndex()];
  InputMenu *menu = (InputMenu *) HUDDialogStack::get()->top();
  switch (((const char*)data)[0]) {

    /* Joystick name */
    case 'J':
      BZDB.set("joystickname", selectedOption);
      getMainWindow()->initJoystick(selectedOption);
      // re-fill all of the joystick-specific options lists
      if (menu)
	menu->fillJSOptions();
      break;

    /* Joystick x-axis */
    case 'X':
      BZDB.set("jsXAxis", selectedOption);
      getMainWindow()->setJoyXAxis(selectedOption);
      break;

    /* Joystick y-axis */
    case 'Y':
      BZDB.set("jsYAxis", selectedOption);
      getMainWindow()->setJoyYAxis(selectedOption);
      break;

    /* Joystick axes inversion */
    case 'I': {
      BZDB.setInt("jsInvertAxes", listHUD->getIndex());
      break;
    }

    /* Active input device */
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

    /* Grab mouse */
    case 'G':
      {
	const bool grabbing = (selectedOption == "Window");
	BZDB.set("mousegrab", grabbing ? "1" : "0");
	getMainWindow()->enableGrabMouse(grabbing);

	const bool clamped = (selectedOption == "MotionBox");
	BZDB.set("mouseClamp", clamped ? "1" : "0");
      }
      break;

    /* Jump while typing */
    case 'H':
      {
	bool jump = (selectedOption == "Yes");
	BZDB.setBool("jumpTyping", jump ? true : false);
      }
      break;

    /* Force feedback */
    case 'F':
      BZDB.set("forceFeedback", selectedOption);
      break;

  }
}

void			InputMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  int i;

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

  // reposition options
  x = 0.5f * ((float)_width + 0.5f * titleWidth);
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(MainMenu::getFontFace(), fontSize, " ");
  const int count = listHUD.size();
  for (i = 1; i < count; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
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
