/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
  wayToBindActions.insert(std::make_pair(std::string("jump"), press));
  wayToBindActions.insert(std::make_pair(std::string("send all"), press));
  wayToBindActions.insert(std::make_pair(std::string("send team"), press));
  wayToBindActions.insert(std::make_pair(std::string("send nemesis"), press));
  wayToBindActions.insert(std::make_pair(std::string("send recipient"), press));
  wayToBindActions.insert(std::make_pair(std::string("send admin"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayScore"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggle displayBinoculars"), press));
  wayToBindActions.insert(std::make_pair(std::string("pause"), press));
  wayToBindActions.insert(std::make_pair(std::string("fullscreen"), press));
  wayToBindActions.insert(std::make_pair(std::string("iconify"), press));
#ifdef SNAPPING
  wayToBindActions.insert(std::make_pair(std::string("screenshot"), press));
#endif
  wayToBindActions.insert(std::make_pair(std::string("time backward"), press));
  wayToBindActions.insert(std::make_pair(std::string("time forward"), press));
  wayToBindActions.insert(std::make_pair(std::string("toggleConsoleAndRadar"), press));
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

  defaultBinding.insert(std::make_pair(std::string("F12"), std::string("quit")));
  defaultBinding.insert(std::make_pair(std::string("Left Mouse"), std::string("fire")));
  defaultBinding.insert(std::make_pair(std::string("Enter"), std::string("fire")));
  defaultBinding.insert(std::make_pair(std::string("Middle Mouse"), std::string("drop")));
  defaultBinding.insert(std::make_pair(std::string("Space"), std::string("drop")));
  defaultBinding.insert(std::make_pair(std::string("Right Mouse"), std::string("identify")));
  defaultBinding.insert(std::make_pair(std::string("I"), std::string("identify")));
  defaultBinding.insert(std::make_pair(std::string("Tab"), std::string("jump")));
  defaultBinding.insert(std::make_pair(std::string("N"), std::string("send all")));
  defaultBinding.insert(std::make_pair(std::string("M"), std::string("send team")));
  defaultBinding.insert(std::make_pair(std::string(","), std::string("send nemesis")));
  defaultBinding.insert(std::make_pair(std::string("."), std::string("send recipient")));
  defaultBinding.insert(std::make_pair(std::string("Z"), std::string("send admin")));
  defaultBinding.insert(std::make_pair(std::string("S"), std::string("toggle displayScore")));
  defaultBinding.insert(std::make_pair(std::string("B"), std::string("toggle displayBinoculars")));
  defaultBinding.insert(std::make_pair(std::string("Pause"), std::string("pause")));
  defaultBinding.insert(std::make_pair(std::string("P"), std::string("pause")));
  defaultBinding.insert(std::make_pair(std::string("F1"), std::string("fullscreen")));
  defaultBinding.insert(std::make_pair(std::string("F3"), std::string("toggleConsoleAndRadar")));
  defaultBinding.insert(std::make_pair(std::string("F4"), std::string("iconify")));
#ifdef SNAPPING
  defaultBinding.insert(std::make_pair(std::string("F5"), std::string("screenshot")));
#endif
  defaultBinding.insert(std::make_pair(std::string("-"), std::string("time backward")));
  defaultBinding.insert(std::make_pair(std::string("="), std::string("time forward")));
  defaultBinding.insert(std::make_pair(std::string("H"), std::string("toggleFlags radar")));
  defaultBinding.insert(std::make_pair(std::string("J"), std::string("toggleFlags main")));
  defaultBinding.insert(std::make_pair(std::string("K"), std::string("silence")));
  defaultBinding.insert(std::make_pair(std::string("L"), std::string("toggle displayLabels")));
  defaultBinding.insert(std::make_pair(std::string("Delete"), std::string("destruct")));

  // Default movement keys
  defaultBinding.insert(std::make_pair(std::string("Left Arrow"),
				       std::string("turn left")));
  defaultBinding.insert(std::make_pair(std::string("Right Arrow"),
				       std::string("turn right")));
  defaultBinding.insert(std::make_pair(std::string("Up Arrow"),
				       std::string("drive forward")));
  defaultBinding.insert(std::make_pair(std::string("Down Arrow"),
				       std::string("drive reverse")));
  // End default movement keys

  defaultBinding.insert(std::make_pair(std::string("Shift+Wheel Up"),
				       std::string("radarZoom in")));
  defaultBinding.insert(std::make_pair(std::string("Shift+Wheel Down"),
				       std::string("radarZoom out")));


  defaultBinding.insert(std::make_pair(std::string("F6"), std::string("roam cycle subject backward")));
  defaultBinding.insert(std::make_pair(std::string("F7"), std::string("roam cycle subject forward")));
  defaultBinding.insert(std::make_pair(std::string("F8"), std::string("roam cycle type forward")));
  defaultBinding.insert(std::make_pair(std::string("F9"), std::string("roam zoom in")));
  defaultBinding.insert(std::make_pair(std::string("F10"), std::string("roam zoom out")));
  defaultBinding.insert(std::make_pair(std::string("F11"), std::string("roam zoom normal")));
  defaultBinding.insert(std::make_pair(std::string("O"), std::string("servercommand")));
  defaultBinding.insert(std::make_pair(std::string("F"), std::string("toggle displayFlagHelp")));
  defaultBinding.insert(std::make_pair(std::string("Page Up"),
				       std::string("scrollpanel up")));
  defaultBinding.insert(std::make_pair(std::string("Wheel Up"),
				       std::string("scrollpanel up")));
  defaultBinding.insert(std::make_pair(std::string("Page Down"),
				       std::string("scrollpanel down")));
  defaultBinding.insert(std::make_pair(std::string("Wheel Down"),
				       std::string("scrollpanel down")));
  defaultBinding.insert(std::make_pair(std::string("1"), std::string("set displayRadarRange 0.25")));
  defaultBinding.insert(std::make_pair(std::string("2"), std::string("set displayRadarRange 0.5")));
  defaultBinding.insert(std::make_pair(std::string("3"), std::string("set displayRadarRange 1.0")));
  defaultBinding.insert(std::make_pair(std::string("A"), std::string("toggle slowKeyboard")));
  defaultBinding.insert(std::make_pair(std::string("U"), std::string("hunt")));
  defaultBinding.insert(std::make_pair(std::string("Right Mouse"), std::string("restart")));
  defaultBinding.insert(std::make_pair(std::string("I"), std::string("restart")));
  defaultBinding.insert(std::make_pair(std::string("9"), std::string("autopilot")));

  defaultBinding.insert(std::make_pair(std::string("Shift+F1"),
				       std::string("messagepanel all")));
  defaultBinding.insert(std::make_pair(std::string("Shift+F2"),
				       std::string("messagepanel chat")));
  defaultBinding.insert(std::make_pair(std::string("Shift+F3"),
				       std::string("messagepanel server")));
  defaultBinding.insert(std::make_pair(std::string("Shift+F4"),
				       std::string("messagepanel misc")));
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
			      bool        keyBind) {
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
  bindingTable.insert(std::make_pair(key, action));
  if (keyBind)
    bind(action, key);
};

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
};

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
