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
#include "FontSizer.h"
#include "ActionBinding.h"
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "playing.h"


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
  addControl(createLabel("Key Mapping"), false);
  addControl(createLabel("Use up/down arrows to navigate, enter key to enter edit mode"), false);
  addControl(reset = createLabel(NULL, "Reset Defaults"));
  addControl(createLabel("fire", "Fire shot:"));
  addControl(createLabel(NULL, "Drop flag:"));
  addControl(createLabel(NULL, "Identify/Lock On:"));
  addControl(createLabel(NULL, "Radar Short:"));
  addControl(createLabel(NULL, "Radar Medium:"));
  addControl(createLabel(NULL, "Radar Long:"));
  addControl(createLabel(NULL, "Send to All:"));
  addControl(createLabel(NULL, "Send to Teammates:"));
  addControl(createLabel(NULL, "Send to Nemesis:"));
  addControl(createLabel(NULL, "Send to Recipient:"));
  addControl(createLabel(NULL, "Send to Admin:"));
  addControl(createLabel(NULL, "Jump:"));
  addControl(createLabel(NULL, "Binoculars:"));
  addControl(createLabel(NULL, "Toggle Score:"));
  addControl(createLabel(NULL, "Toggle Radar:"));
  addControl(createLabel(NULL, "Toggle Console:"));
  addControl(createLabel(NULL, "Tank Labels:"));
  addControl(createLabel(NULL, "Flag Help:"));
  addControl(createLabel(NULL, "Time Forward:"));
  addControl(createLabel(NULL, "Time Backward:"));
  addControl(createLabel(NULL, "Pause/Resume:"));
  addControl(createLabel(NULL, "Self Destruct/Cancel:"));
  addControl(createLabel(NULL, "Fast Quit:"));
  addControl(createLabel(NULL, "Scroll Backward:"));
  addControl(createLabel(NULL, "Scroll Forward:"));
  addControl(createLabel(NULL, "Scroll Bottom:"));
  addControl(createLabel(NULL, "Slow Motion:"));
  addControl(createLabel(NULL, "Toggle Flags On Radar:"));
  addControl(createLabel(NULL, "Toggle Flags On Field:"));
  addControl(createLabel(NULL, "Silence/UnSilence Key:"));
  addControl(createLabel(NULL, "Server Command Key:"));
  addControl(createLabel(NULL, "Hunt Key:"));
  addControl(createLabel(NULL, "Add/Modify Hunt Key:"));
  addControl(createLabel(NULL, "AutoPilot Key: "));
  addControl(createLabel(NULL, "Main Message Tab: "));
  addControl(createLabel(NULL, "Chat Message Tab: "));
  addControl(createLabel(NULL, "Server Message Tab: "));
  addControl(createLabel(NULL, "Misc Message Tab: "));
  addControl(createLabel(NULL, "Forward Key: "));
  addControl(createLabel(NULL, "Reverse Key: "));
  addControl(createLabel(NULL, "Left Key: "));
  addControl(createLabel(NULL, "Right Key: "));
  addControl(createLabel(NULL, "Restart:"));
  addControl(createLabel(NULL, "Iconify:"));
  addControl(createLabel(NULL, "Fullscreen:"));
  addControl(quickKeys = createLabel(NULL, "Define Quick Keys"));

  initNavigation();

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
  const HUDuiControl* const _focus = getNav().get();
  if (_focus == reset) {
    ActionBinding::instance().resetBindings();
    update();
  } else if (_focus == quickKeys) {
    if (!quickKeysMenu) quickKeysMenu = new QuickKeysMenu;
    HUDDialogStack::get()->push(quickKeysMenu);
  } else {
    // start editing
    std::vector<HUDuiElement*>& listHUD = getElements();
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
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
  const int fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace, "headerFontSize");

  fs.setMin(0, 20);
  const float bigFontSize = fs.getFontSize(fontFace, "menuFontSize");

  fs.setMin(0, 40);
  const float fontSize = fs.getFontSize(fontFace, "infoFontSize");

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace, titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace, titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition help
  HUDuiLabel* help = (HUDuiLabel*)listHUD[1];
  help->setFontSize(bigFontSize);
  const float helpWidth = fm.getStringWidth(fontFace, bigFontSize, help->getString().c_str());
  x = 0.5f * ((float)_width - helpWidth);
  y -= 1.1f * fm.getStringHeight(fontFace, bigFontSize);
  help->setPosition(x, y);

  // reposition options in two columns
  x = 0.30f * (float)_width;
  const float topY = y - (0.6f * titleHeight);
  y = topY;
  listHUD[2]->setFontSize(fontSize);
  const float h = fm.getStringHeight(fontFace, fontSize);
  const int count = (int)listHUD.size() - 2;
  const int mid = (count / 2);

  int i;
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
  std::vector<HUDuiElement*>& listHUD = getElements();
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
