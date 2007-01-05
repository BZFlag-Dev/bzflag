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
#include "KeyboardMapMenu.h"

// System headers
#include <ctype.h>

/* common implementation headers */
#include "KeyManager.h"
#include "FontManager.h"

/* local implementation headers */
#include "ActionBinding.h"
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "playing.h"
#include "HUDui.h"

KeyboardMapMenuDefaultKey::KeyboardMapMenuDefaultKey(KeyboardMapMenu* _menu) :
  menu(_menu)
{
  // do nothing
}

bool KeyboardMapMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  // escape key has usual effect
  if (key.ascii == 27)
    return MenuDefaultKey::keyPress(key);

  // keys have normal effect if not editing
  if (!menu->isEditing())
    return MenuDefaultKey::keyPress(key);

  // ignore keys we don't know
  if (key.ascii != 0 && isspace(key.ascii)) {
    if (key.ascii != ' ' && key.ascii != '\t' && key.ascii != '\r')
      return true;
  }

  // all other keys modify mapping
  menu->setKey(key);
  return true;
}

bool KeyboardMapMenuDefaultKey::keyRelease(const BzfKeyEvent&)
{
  // ignore key releases
  return true;
}

KeyboardMapMenu::KeyboardMapMenu() : defaultKey(this), editing(-1), quickKeysMenu(NULL)
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();

  controls.push_back(createLabel("Key Mapping"));
  controls.push_back(createLabel("Use up/down arrows to navigate, enter key to enter edit mode"));
  controls.push_back(reset = createLabel(NULL, "Reset Defaults"));
  controls.push_back(createLabel("fire", "Fire shot:"));
  controls.push_back(createLabel(NULL, "Drop flag:"));
  controls.push_back(createLabel(NULL, "Identify/Lock On:"));
  controls.push_back(createLabel(NULL, "Radar Short:"));
  controls.push_back(createLabel(NULL, "Radar Medium:"));
  controls.push_back(createLabel(NULL, "Radar Long:"));
  controls.push_back(createLabel(NULL, "Send to All:"));
  controls.push_back(createLabel(NULL, "Send to Teammates:"));
  controls.push_back(createLabel(NULL, "Send to Nemesis:"));
  controls.push_back(createLabel(NULL, "Send to Recipient:"));
  controls.push_back(createLabel(NULL, "Send to Admin:"));
  controls.push_back(createLabel(NULL, "Jump:"));
  controls.push_back(createLabel(NULL, "Binoculars:"));
  controls.push_back(createLabel(NULL, "Toggle Score:"));
  controls.push_back(createLabel(NULL, "Toggle Radar:"));
  controls.push_back(createLabel(NULL, "Toggle Console:"));
  controls.push_back(createLabel(NULL, "Tank Labels:"));
  controls.push_back(createLabel(NULL, "Flag Help:"));
  controls.push_back(createLabel(NULL, "Time Forward:"));
  controls.push_back(createLabel(NULL, "Time Backward:"));
  controls.push_back(createLabel(NULL, "Pause/Resume:"));
  controls.push_back(createLabel(NULL, "Self Destruct/Cancel:"));
  controls.push_back(createLabel(NULL, "Fast Quit:"));
  controls.push_back(createLabel(NULL, "Scroll Backward:"));
  controls.push_back(createLabel(NULL, "Scroll Forward:"));
  controls.push_back(createLabel(NULL, "Scroll Bottom:"));
  controls.push_back(createLabel(NULL, "Slow Motion:"));
  controls.push_back(createLabel(NULL, "Toggle Flags On Radar:"));
  controls.push_back(createLabel(NULL, "Toggle Flags On Field:"));
  controls.push_back(createLabel(NULL, "Silence/UnSilence Key:"));
  controls.push_back(createLabel(NULL, "Server Command Key:"));
  controls.push_back(createLabel(NULL, "Hunt Key:"));
  controls.push_back(createLabel(NULL, "Add/Modify Hunt Key:"));
  controls.push_back(createLabel(NULL, "AutoPilot Key: "));
  controls.push_back(createLabel(NULL, "Main Message Tab: "));
  controls.push_back(createLabel(NULL, "Chat Message Tab: "));
  controls.push_back(createLabel(NULL, "Server Message Tab: "));
  controls.push_back(createLabel(NULL, "Misc Message Tab: "));
  controls.push_back(createLabel(NULL, "Forward Key: "));
  controls.push_back(createLabel(NULL, "Reverse Key: "));
  controls.push_back(createLabel(NULL, "Left Key: "));
  controls.push_back(createLabel(NULL, "Right Key: "));
  controls.push_back(createLabel(NULL, "Restart:"));
  controls.push_back(createLabel(NULL, "Iconify:"));
  controls.push_back(createLabel(NULL, "Fullscreen:"));
  controls.push_back(quickKeys = createLabel(NULL, "Define Quick Keys"));

  initNavigation(controls, 2, (int)controls.size()-1);

  int i = 3;
  initkeymap("fire", i);
  initkeymap("drop", ++i);
  initkeymap("identify", ++i);
  initkeymap("set displayRadarRange 0.25", ++i);
  initkeymap("set displayRadarRange 0.5", ++i);
  initkeymap("set displayRadarRange 1.0", ++i);
  initkeymap("send all", ++i);
  initkeymap("send team", ++i);
  initkeymap("send nemesis", ++i);
  initkeymap("send recipient", ++i);
  initkeymap("send admin",++i);
  initkeymap("jump", ++i);
  initkeymap("viewZoom toggle", ++i);
  initkeymap("toggle displayScore", ++i);
  initkeymap("toggleRadar", ++i);
  initkeymap("toggleConsole", ++i);
  initkeymap("toggle displayLabels", ++i);
  initkeymap("toggle displayFlagHelp", ++i);
  initkeymap("time forward", ++i);
  initkeymap("time backward", ++i);
  initkeymap("pause", ++i);
  initkeymap("destruct", ++i);
  initkeymap("quit", ++i);
  initkeymap("scrollpanel up_page", ++i);
  initkeymap("scrollpanel down_page", ++i);
  initkeymap("scrollpanel bottom", ++i);
  initkeymap("toggle slowKeyboard", ++i);
  initkeymap("toggleFlags radar", ++i);
  initkeymap("toggleFlags main", ++i);
  initkeymap("silence", ++i);
  initkeymap("servercommand", ++i);
  initkeymap("hunt", ++i);
  initkeymap("addhunt", ++i);
  initkeymap("autopilot", ++i);
  initkeymap("messagepanel all", ++i);
  initkeymap("messagepanel chat", ++i);
  initkeymap("messagepanel server", ++i);
  initkeymap("messagepanel misc", ++i);
  initkeymap("drive forward", ++i);
  initkeymap("drive reverse", ++i);
  initkeymap("turn left", ++i);
  initkeymap("turn right", ++i);
  initkeymap("restart", ++i);
  initkeymap("iconify", ++i);
  initkeymap("fullscreen", ++i);
}

void KeyboardMapMenu::initkeymap(const std::string& name, int index)
{
  mappable[name].key1 = "";
  mappable[name].key2 = "";
  mappable[name].index = index;
}

bool KeyboardMapMenu::isEditing() const
{
  return editing != -1;
}

void KeyboardMapMenu::setKey(const BzfKeyEvent& event)
{
  if (editing == -1)
    return;
  KeyKeyMap::iterator it;
  for (it = mappable.begin(); it != mappable.end(); it++)
    if (it->second.index == editing)
      break;
  if ((KEYMGR.keyEventToString(event) == it->second.key1 && it->second.key2.empty()) || (KEYMGR.keyEventToString(event) == it->second.key2))
    return;
  ActionBinding::instance().associate(KEYMGR.keyEventToString(event),
				      it->first);
  editing = -1;
  update();
}

void KeyboardMapMenu::execute()
{
  const HUDuiControl* const _focus = HUDui::getFocus();
  if (_focus == reset) {
    ActionBinding::instance().resetBindings();
    update();
  } else if (_focus == quickKeys) {
    if (!quickKeysMenu) quickKeysMenu = new QuickKeysMenu;
    HUDDialogStack::get()->push(quickKeysMenu);
  } else {
    // start editing
    std::vector<HUDuiControl*>& listHUD = getControls();
    KeyKeyMap::iterator it;
    for (it = mappable.begin(); it != mappable.end(); it++) {
      if (listHUD[it->second.index] == _focus) {
	editing = it->second.index;
	if (!it->second.key1.empty() && !it->second.key2.empty()) {
	  ActionBinding::instance().deassociate(it->first);
	}
      }
    }
  }
  update();
}

void KeyboardMapMenu::dismiss()
{
  editing = -1;
  notifyBzfKeyMapChanged();
}

void KeyboardMapMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  int i;
  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float bigFontSize = (float)_height / 42.0f;
  const float fontSize = (float)_height / 100.0f;
  FontManager &fm = FontManager::instance();
  const int fontFace = MainMenu::getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(fontFace, titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(fontFace, titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition help
  HUDuiLabel* help = (HUDuiLabel*)listHUD[1];
  help->setFontSize(bigFontSize);
  const float helpWidth = fm.getStrLength(fontFace, bigFontSize, help->getString());
  x = 0.5f * ((float)_width - helpWidth);
  y -= 1.1f * fm.getStrHeight(fontFace, bigFontSize, " ");
  help->setPosition(x, y);

  // reposition options in two columns
  x = 0.30f * (float)_width;
  const float topY = y - (0.6f * titleHeight);
  y = topY;
  listHUD[2]->setFontSize(fontSize);
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const int count = (int)listHUD.size() - 2;
  const int mid = (count / 2);

  for (i = 2; i <= mid+1; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.80f * (float)_width;
  y = topY;
  for (i = mid+2; i < count+2; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  update();
}

void KeyboardMapMenu::update()
{
  KeyKeyMap::iterator it;
  // clear
  for (it = mappable.begin(); it != mappable.end(); it++) {
    it->second.key1 = "";
    it->second.key2 = "";
  }
  // load current settings
  KEYMGR.iterate(&onScanCB, this);
  std::vector<HUDuiControl*>& listHUD = getControls();
  for (it = mappable.begin(); it != mappable.end(); it++) {
    std::string value = "";
    if (it->second.key1.empty()) {
      if (isEditing() && (it->second.index == editing))
	value = "???";
      else
	value = "<not mapped>";
    } else {
      value += it->second.key1;
      if (!it->second.key2.empty()) {
	value += " or " + it->second.key2;
      } else if (isEditing() && (it->second.index == editing)) {
	value += " or ???";
      }
    }
    ((HUDuiLabel*)listHUD[it->second.index])->setString(value);
  }
}

void KeyboardMapMenu::onScan(const std::string& name, bool press,
			     const std::string& cmd)
{
  if (!press && cmd == "fire")
    return;
  KeyKeyMap::iterator it = mappable.find(cmd);
  if (it == mappable.end())
    return;
  if (it->second.key1.empty())
    it->second.key1 = name;
  else if (it->second.key2.empty() && it->second.key1 != name)
    it->second.key2 = name;
}

void KeyboardMapMenu::onScanCB(const std::string& name, bool press,
			       const std::string& cmd, void* userData)
{
  static_cast<KeyboardMapMenu*>(userData)->onScan(name, press, cmd);
}

HUDuiLabel* KeyboardMapMenu::createLabel(const char* str, const char* _label)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  if (str) label->setString(str);
  if (_label) label->setLabel(_label);
  return label;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
