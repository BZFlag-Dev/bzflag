/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface headers */
#include "HelpMenu.h"
#include "HelpKeymapMenu.h"

/* system headers */
#include <vector>

/* common implementation headers */
#include "KeyManager.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"

HelpKeymapMenu::HelpKeymapMenu() : HelpMenu("Controls")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Mouse Position", "Controls Tank Position:"));
  listHUD.push_back(createLabel(NULL, "Fires Shot:"));
  listHUD.push_back(createLabel(NULL, "Drops Flag (if not bad):"));
  listHUD.push_back(createLabel(NULL, "Identifies Player (locks on GM):"));
  listHUD.push_back(createLabel(NULL, "Short Radar Range:"));
  listHUD.push_back(createLabel(NULL, "Medium Radar Range:"));
  listHUD.push_back(createLabel(NULL, "Long Radar Range:"));
  listHUD.push_back(createLabel(NULL, "Send Message to Everybody:"));
  listHUD.push_back(createLabel(NULL, "Send Message to Teammates:"));
  listHUD.push_back(createLabel(NULL, "Send Message to Nemesis:"));
  listHUD.push_back(createLabel(NULL, "Send Message to Recipient:"));
  listHUD.push_back(createLabel(NULL, "Jump (if allowed):"));
  listHUD.push_back(createLabel(NULL, "Toggle Binoculars:"));
  listHUD.push_back(createLabel(NULL, "Toggle Score Sheet:"));
  listHUD.push_back(createLabel(NULL, "Toggle Tank Labels:"));
  listHUD.push_back(createLabel(NULL, "Toggle Heads-up Flag Help:"));
  listHUD.push_back(createLabel(NULL, "Set Time of Day Backward:"));
  listHUD.push_back(createLabel(NULL, "Set Time of Day Forward:"));
  listHUD.push_back(createLabel(NULL, "Pause/Resume:"));
  listHUD.push_back(createLabel(NULL, "Self destruct/Cancel:"));
  listHUD.push_back(createLabel(NULL, "Quit:"));
  listHUD.push_back(createLabel(NULL, "Scroll Message Log Backward:"));
  listHUD.push_back(createLabel(NULL, "Scroll Message Log Forward:"));
  listHUD.push_back(createLabel(NULL, "Slow Motion:"));
  listHUD.push_back(createLabel(NULL, "Toggle Radar Flags:"));
  listHUD.push_back(createLabel(NULL, "Toggle Main Flags:"));
  listHUD.push_back(createLabel(NULL, "Silence/UnSilence:"));
  listHUD.push_back(createLabel(NULL, "Server Admin:"));
  listHUD.push_back(createLabel(NULL, "Hunt:"));
  listHUD.push_back(createLabel(NULL, "Auto Pilot:"));
  listHUD.push_back(createLabel(NULL, "Main Message Tab:"));
  listHUD.push_back(createLabel(NULL, "Chat Message Tab:"));
  listHUD.push_back(createLabel(NULL, "Server Message Tab:"));
  listHUD.push_back(createLabel(NULL, "Misc Message Tab:"));
  listHUD.push_back(createLabel("Esc", "Show/Dismiss menu:"));

  initKeymap("fire", 3);
  initKeymap("drop", 4);
  initKeymap("identify", 5);
  initKeymap("set displayRadarRange 0.25", 6);
  initKeymap("set displayRadarRange 0.5", 7);
  initKeymap("set displayRadarRange 1.0", 8);
  initKeymap("send all", 9);
  initKeymap("send team", 10);
  initKeymap("send nemesis", 11);
  initKeymap("send recipient", 12);
  initKeymap("jump", 13);
  initKeymap("viewZoom toggle", 14);
  initKeymap("toggle displayScore", 15);
  initKeymap("toggle displayLabels", 16);
  initKeymap("toggle displayFlagHelp", 17);
  initKeymap("time backward", 18);
  initKeymap("time forward", 19);
  initKeymap("pause", 20);
  initKeymap("destruct", 21);
  initKeymap("quit", 22);
  initKeymap("scrollpanel up_page", 23);
  initKeymap("scrollpanel down_page", 24);
  initKeymap("toggle slowKeyboard", 25);
  initKeymap("toggleFlags radar", 26);
  initKeymap("toggleFlags main", 27);
  initKeymap("silence", 28);
  initKeymap("servercommand", 29);
  initKeymap("hunt", 30);
  initKeymap("autopilot", 31);
  initKeymap("messagepanel all", 32);
  initKeymap("messagepanel chat", 33);
  initKeymap("messagepanel server", 34);
  initKeymap("messagepanel misc", 35);
}

void HelpKeymapMenu::onScan(const std::string& name, bool press,
		       const std::string& cmd)
{
  if (!press)
    return;
  KeyKeyMap::iterator it = mappable.find(cmd);
  if (it == mappable.end())
    return;
  if (it->second.key1.empty())
    it->second.key1 = name;
  else if (it->second.key2.empty())
    it->second.key2 = name;
}

void HelpKeymapMenu::onScanCB(const std::string& name, bool press,
			 const std::string& cmd, void* userData)
{
  static_cast<HelpKeymapMenu*>(userData)->onScan(name, press, cmd);
}

void HelpKeymapMenu::initKeymap(const std::string& name, int index)
{
  mappable[name].key1 = "";
  mappable[name].key2 = "";
  mappable[name].index = index;
}

float HelpKeymapMenu::getLeftSide(int _width, int _height)
{
  return 0.5f * _width - _height / 20.0f;
}

void HelpKeymapMenu::resize(int _width, int _height)
{
  // get current key mapping and set strings appropriately
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
      value = "<not mapped>";
    } else {
      value += it->second.key1;
      if (!it->second.key2.empty())
	value += " or " + it->second.key2;
    }
    ((HUDuiLabel*)listHUD[it->second.index])->setString(value);
  }

  // now do regular resizing
  HelpMenu::resize(_width, _height);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
