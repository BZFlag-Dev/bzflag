/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* bzflag special common - 1st one */
#include "common.h"

#include <iostream>
#include <string>

#include "ActionBinding.h"
#include "CommandManager.h"
#include "KeyManager.h"

// initialize the singleton
template <>
ActionBinding* Singleton<ActionBinding>::_instance = (ActionBinding*)0;


ActionBinding::ActionBinding() {
  wayToBindActions.insert(std::make_pair(std::string("quit"), press));
  wayToBindActions.insert(std::make_pair(std::string("fire"), both));
  wayToBindActions.insert(std::make_pair(std::string("drop"), press));
  wayToBindActions.insert(std::make_pair(std::string("identify"), press));
  wayToBindActions.insert(std::make_pair(std::string("jump"), both));
  wayToBindActions.insert(std::make_pair(std::string("send all"), press));
  wayToBindActions.insert(std::make_pair(std::string("send team"), press));
  wayToBindActions.insert(std::make_pair(std::string("send nemesis"), press));
  wayToBindActions.insert(std::make_pair(std::string("send recipient"), press));
  wayToBindActions.insert(std::make_pair(std::string("send admin"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayScore"), press));
  wayToBindActions.insert(std::make_pair(std::string("viewZoom toggle"),
					 press));
  wayToBindActions.insert(std::make_pair(std::string("viewZoom in"), press));
  wayToBindActions.insert(std::make_pair(std::string("viewZoom out"), press));
  wayToBindActions.insert(std::make_pair(std::string("pause"), press));
  wayToBindActions.insert(std::make_pair(std::string("fullscreen"), press));
  wayToBindActions.insert(std::make_pair(std::string("mousegrab"), press));
  wayToBindActions.insert(std::make_pair(std::string("iconify"), press));
#ifdef SNAPPING
  wayToBindActions.insert(std::make_pair(std::string("screenshot"), press));
#endif
  wayToBindActions.insert(std::make_pair(std::string("time backward"), press));
  wayToBindActions.insert(std::make_pair(std::string("time forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleRadar"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleConsole"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleFlags radar"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleFlags main"), press));
  wayToBindActions.insert(std::make_pair(std::string("silence"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayLabels"), press));
  wayToBindActions.insert(std::make_pair(std::string("destruct"), press));

  // Movement keys
  wayToBindActions.insert(std::make_pair(std::string("turn left"), both));
  wayToBindActions.insert(std::make_pair(std::string("turn right"), both));
  wayToBindActions.insert(std::make_pair(std::string("drive forward"), both));
  wayToBindActions.insert(std::make_pair(std::string("drive reverse"), both));
  // End movement keys

  wayToBindActions.insert(std::make_pair(std::string("roam cycle subject backward"),
					 press));
  wayToBindActions.insert(std::make_pair(std::string("roam cycle subject forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("roam cycle type forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("roam zoom in"), both));
  wayToBindActions.insert(std::make_pair(std::string("roam zoom out"), both));
  wayToBindActions.insert(std::make_pair(std::string("roam zoom normal"), both));
  wayToBindActions.insert(std::make_pair(std::string("servercommand"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayFlagHelp"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel up"), press));
  wayToBindActions.insert(std::make_pair(std::string("scrollpanel down"), press));
  wayToBindActions.insert(std::make_pair(std::string("radarZoom in"), press));
  wayToBindActions.insert(std::make_pair(std::string("radarZoom out"), press));
  wayToBindActions.insert(std::make_pair(std::string("set displayRadarRange 0.25"), press));
  wayToBindActions.insert(std::make_pair(std::string("set displayRadarRange 0.5"), press));
  wayToBindActions.insert(std::make_pair(std::string("set displayRadarRange 1.0"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle slowKeyboard"), press));
  wayToBindActions.insert(std::make_pair(std::string("hunt"), press));
  wayToBindActions.insert(std::make_pair(std::string("addhunt"), press));
  wayToBindActions.insert(std::make_pair(std::string("restart"), release));
  wayToBindActions.insert(std::make_pair(std::string("autopilot"), press));

  wayToBindActions.insert(std::make_pair(std::string("messagepanel all"),
					 press));
  wayToBindActions.insert(std::make_pair(std::string("messagepanel chat"),
					 press));
  wayToBindActions.insert(std::make_pair(std::string("messagepanel server"),
					 press));
  wayToBindActions.insert(std::make_pair(std::string("messagepanel misc"),
					 press));

  defaultBinding.insert(BindingTable::value_type("F12", "quit"));
  defaultBinding.insert(BindingTable::value_type("Left Mouse", "fire"));
  defaultBinding.insert(BindingTable::value_type("Enter", "fire"));
  defaultBinding.insert(BindingTable::value_type("Middle Mouse", "drop"));
  defaultBinding.insert(BindingTable::value_type("Space", "drop"));
  defaultBinding.insert(BindingTable::value_type("Right Mouse", "identify"));
  defaultBinding.insert(BindingTable::value_type("I", "identify"));
  defaultBinding.insert(BindingTable::value_type("Tab", "jump"));
  defaultBinding.insert(BindingTable::value_type("N", "send all"));
  defaultBinding.insert(BindingTable::value_type("M", "send team"));
  defaultBinding.insert(BindingTable::value_type(",", "send nemesis"));
  defaultBinding.insert(BindingTable::value_type(".", "send recipient"));
  defaultBinding.insert(BindingTable::value_type("Z", "send admin"));
  defaultBinding.insert(BindingTable::value_type("S", "toggle displayScore"));
  defaultBinding.insert(BindingTable::value_type("B", "viewZoom toggle"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Wheel Up", "viewZoom in"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Wheel Down", "viewZoom out"));
  defaultBinding.insert(BindingTable::value_type("Pause", "pause"));
  defaultBinding.insert(BindingTable::value_type("P", "pause"));
  defaultBinding.insert(BindingTable::value_type("F1", "fullscreen"));
  defaultBinding.insert(BindingTable::value_type("Q", "toggleRadar"));
  defaultBinding.insert(BindingTable::value_type("W", "toggleConsole"));
  defaultBinding.insert(BindingTable::value_type("F4", "iconify"));
#ifdef SNAPPING
  defaultBinding.insert(BindingTable::value_type("F5", "screenshot"));
#endif
  defaultBinding.insert(BindingTable::value_type("-", "time backward"));
  defaultBinding.insert(BindingTable::value_type("=", "time forward"));
  defaultBinding.insert(BindingTable::value_type("H", "toggleFlags radar"));
  defaultBinding.insert(BindingTable::value_type("J", "toggleFlags main"));
  defaultBinding.insert(BindingTable::value_type("K", "silence"));
  defaultBinding.insert(BindingTable::value_type("L", "toggle displayLabels"));
  defaultBinding.insert(BindingTable::value_type("Delete", "destruct"));

  // Default movement keys
  defaultBinding.insert(BindingTable::value_type("Left Arrow", "turn left"));
  defaultBinding.insert(BindingTable::value_type("Right Arrow", "turn right"));
  defaultBinding.insert(BindingTable::value_type("Up Arrow", "drive forward"));
  defaultBinding.insert(BindingTable::value_type("Down Arrow", "drive reverse"));
  // End default movement keys

  defaultBinding.insert(BindingTable::value_type("Shift+Wheel Up", "radarZoom in"));
  defaultBinding.insert(BindingTable::value_type("Shift+Wheel Down", "radarZoom out"));

  defaultBinding.insert(BindingTable::value_type("F6", "roam cycle subject backward"));
  defaultBinding.insert(BindingTable::value_type("F7", "roam cycle subject forward"));
  defaultBinding.insert(BindingTable::value_type("F8", "roam cycle type forward"));
  defaultBinding.insert(BindingTable::value_type("F9", "roam zoom in"));
  defaultBinding.insert(BindingTable::value_type("F10", "roam zoom out"));
  defaultBinding.insert(BindingTable::value_type("F11", "roam zoom normal"));
  defaultBinding.insert(BindingTable::value_type("O", "servercommand"));
  defaultBinding.insert(BindingTable::value_type("F", "toggle displayFlagHelp"));
  defaultBinding.insert(BindingTable::value_type("Page Up", "scrollpanel up"));
  defaultBinding.insert(BindingTable::value_type("Wheel Up", "scrollpanel up"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Page Up", "scrollpanel up_page"));
  defaultBinding.insert(BindingTable::value_type("Shift+Page Up", "scrollpanel top"));
  defaultBinding.insert(BindingTable::value_type("Page Down", "scrollpanel down"));
  defaultBinding.insert(BindingTable::value_type("Wheel Down", "scrollpanel down"));
  defaultBinding.insert(BindingTable::value_type("Ctrl+Page Down", "scrollpanel down_page"));
  defaultBinding.insert(BindingTable::value_type("Shift+Page Down", "scrollpanel bottom"));
  defaultBinding.insert(BindingTable::value_type("1", "set displayRadarRange 0.25"));
  defaultBinding.insert(BindingTable::value_type("2", "set displayRadarRange 0.5"));
  defaultBinding.insert(BindingTable::value_type("3", "set displayRadarRange 1.0"));
  defaultBinding.insert(BindingTable::value_type("4", "radarZoom in"));
  defaultBinding.insert(BindingTable::value_type("5", "radarZoom out"));
  defaultBinding.insert(BindingTable::value_type("A", "toggle slowKeyboard"));
  defaultBinding.insert(BindingTable::value_type("U", "hunt"));
  defaultBinding.insert(BindingTable::value_type("7", "addhunt"));
  defaultBinding.insert(BindingTable::value_type("Right Mouse", "restart"));
  defaultBinding.insert(BindingTable::value_type("I", "restart"));
  defaultBinding.insert(BindingTable::value_type("9", "autopilot"));

  defaultBinding.insert(BindingTable::value_type("Shift+F1", "messagepanel all"));
  defaultBinding.insert(BindingTable::value_type("Shift+F2", "messagepanel chat"));
  defaultBinding.insert(BindingTable::value_type("Shift+F3", "messagepanel server"));
  defaultBinding.insert(BindingTable::value_type("Shift+F4", "messagepanel misc"));
}

void ActionBinding::resetBindings() {
  BindingTable::const_iterator index;

  for (index = bindingTable.begin();
       index != bindingTable.end();
       ++index)
    unbind(index->second, index->first);

  bindingTable = defaultBinding;

  for (index = bindingTable.begin();
       index != bindingTable.end();
       ++index)
    bind(index->second, index->first);
}

void ActionBinding::getFromBindings() {
  bindingTable.clear();
  KEYMGR.iterate(&onScanCB, this);
}

void ActionBinding::onScanCB(const std::string& name, bool,
			     const std::string& cmd, void*)
{
  ActionBinding::instance().associate(name, cmd, false);
}

void ActionBinding::associate(std::string key,
			      std::string action,
			      bool	keyBind) {
  BindingTable::iterator index, next;
  if (!wayToBindActions.count(action))
    return;
  PressStatusBind newStatusBind = wayToBindActions[action];
  for (index = bindingTable.lower_bound( key ); index != bindingTable.upper_bound( key ); index = next) {
    next = index;
    ++next;
    if (newStatusBind == both) {
      if (keyBind)
	unbind(index->second, key);
      bindingTable.erase(index);
    } else if (newStatusBind == press) {
      if (wayToBindActions[index->second] != release) {
	if (keyBind)
	  unbind(index->second, key);
	bindingTable.erase(index);
      }
    } else {
      if (wayToBindActions[index->second] != press) {
	if (keyBind)
	  unbind(index->second, key);
	bindingTable.erase(index);
      }
    }
  }
  bindingTable.insert(BindingTable::value_type(key, action));
  if (keyBind)
    bind(action, key);
}

void ActionBinding::deassociate(std::string action) {
  BindingTable::iterator index, next;
  for (index = bindingTable.begin();
       index != bindingTable.end();
       index = next) {
    next = index;
    ++next;
    if (index->second == action) {
      unbind(action, index->first);
      bindingTable.erase(index);
    }
  }
}

void ActionBinding::bind(std::string action, std::string key) {
  PressStatusBind statusBind = wayToBindActions[action];
  std::string command;
  if (statusBind == press || statusBind == both) {
    command = "bind \"" + key + "\" down \"" + action + "\"";
    CMDMGR.run(command);
  };
  if (statusBind == release || statusBind == both) {
    command = "bind \"" + key + "\" up \"" + action + "\"";
    CMDMGR.run(command);
  };
}

void ActionBinding::unbind(std::string action, std::string key) {
  PressStatusBind statusBind = wayToBindActions[action];
  std::string command;
  if (statusBind == press || statusBind == both) {
    command = "unbind \"" + key + "\" down";
    CMDMGR.run(command);
  };
  if (statusBind == release || statusBind == both) {
    command = "unbind \"" + key + "\" up";
    CMDMGR.run(command);
  };
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
