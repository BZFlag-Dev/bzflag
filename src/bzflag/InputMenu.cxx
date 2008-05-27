/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "FontSizer.h"
#include "MainMenu.h"
#include "HUDDialogStack.h"
#include "LocalPlayer.h"
#include "playing.h"
#include "LocalFontFace.h"

InputMenu::InputMenu() : keyboardMapMenu(NULL)
{
  std::string currentJoystickDevice = BZDB.get("joystickname");
  // cache font face ID
  const LocalFontFace* fontFace = MainMenu::getFontFace();

  // add controls
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(fontFace);
  label->setString("Input Settings");
  addControl(label, false);

  keyMapping = new HUDuiLabel;
  keyMapping->setFontFace(fontFace);
  keyMapping->setLabel("Change Key Mapping");
  addControl(keyMapping);

  HUDuiList* option = new HUDuiList;

  option = new HUDuiList;
  std::vector<std::string>* options = &option->getList();
  // set joystick Device
  option->setFontFace(fontFace);
  option->setLabel("Joystick device:");
  option->setCallback(callback, (void*)"J");
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
  addControl(option);

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
  addControl(activeInput);

  option = new HUDuiList;
  // force feedback
  option->setFontFace(fontFace);
  option->setLabel("Force feedback:");
  option->setCallback(callback, (void*)"F");
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
  addControl(option);

  option = new HUDuiList;
  // axis settings
  jsx = option;
  option->setFontFace(fontFace);
  option->setLabel("Joystick X Axis:");
  option->setCallback(callback, (void*)"X");
  addControl(option);
  option = new HUDuiList;
  jsy = option;
  option->setFontFace(fontFace);
  option->setLabel("Joystick Y Axis:");
  option->setCallback(callback, (void*)"Y");
  addControl(option);
  fillJSOptions();

  option = new HUDuiList;
  // confine mouse on/off
  option->setFontFace(fontFace);
  option->setLabel("Confine mouse:");
  option->setCallback(callback, (void*)"G");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Yes"));
  option->setIndex(getMainWindow()->isGrabEnabled() ? 1 : 0);
  option->update();
  addControl(option);

  option = new HUDuiList;
  // jump while typing on/off
  option->setFontFace(fontFace);
  option->setLabel("Jump while typing:");
  option->setCallback(callback, (void*)"H");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Yes"));
  option->setIndex(BZDB.isTrue("jumpTyping") ? 1 : 0);
  option->update();
  addControl(option);

  option = new HUDuiList;
  // tie the FOV into turning rate
  option->setFontFace(fontFace);
  option->setLabel("Slow turning with binoculars:");
  option->setCallback(callback, (void*)"B");
  options = &option->getList();
  options->push_back(std::string("No"));
  options->push_back(std::string("Yes"));
  option->setIndex(BZDB.isTrue("slowBinoculars") ? 1 : 0);
  option->update();
  addControl(option);

  initNavigation();
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
  if (joystickAxes.size() == 0)
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
  HUDuiControl* _focus = getNav().get();
  if (_focus == keyMapping) {
    if (!keyboardMapMenu) keyboardMapMenu = new KeyboardMapMenu;
    HUDDialogStack::get()->push(keyboardMapMenu);
  }
}

void			InputMenu::callback(HUDuiControl* w, void* data) {
  HUDuiList* listHUD = (HUDuiList*)w;
  std::vector<std::string> *options = &listHUD->getList();
  std::string selectedOption = (*options)[listHUD->getIndex()];
  switch (((const char*)data)[0]) {

    /* Joystick name */
    case 'J':
      BZDB.set("joystickname", selectedOption);
      getMainWindow()->initJoystick(selectedOption);
      // re-fill all of the joystick-specific options lists
      // fillJSOptions();
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
	bool grabbing = (selectedOption == "Yes");
	BZDB.set("mousegrab", grabbing ? "true" : "false");
	getMainWindow()->enableGrabMouse(grabbing);
      }
      break;

    /* Jump while typing */
    case 'H':
      {
	bool jump = (selectedOption == "Yes");
	BZDB.setBool("jumpTyping", jump ? true : false);
      }
      break;

    /* Reduce turning rate with FOV */
    case 'B':
      {
	bool fov = (selectedOption == "Yes");
	BZDB.setBool("slowBinoculars", fov ? true : false);
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
  FontSizer fs = FontSizer(_width, _height);

  int i;

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
  const float titleWidth = fm.getStringWidth(fontFace->getFMFace(), titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace->getFMFace(), titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width + 0.5f * titleWidth);
  y -= 1.0f * titleHeight;
  const float h = fm.getStringHeight(fontFace->getFMFace(), fontSize);
  const int count = (int)listHUD.size();
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
