/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <iostream>

#include <config.h>
#include "ActionBinding.h"
#include "CommandManager.h"
#include "KeyManager.h"

ActionBinding::ActionBinding() {
  wayToBindActions.insert(std::make_pair("quit", press));
  wayToBindActions.insert(std::make_pair("fire", both));
  wayToBindActions.insert(std::make_pair("drop", press));
  wayToBindActions.insert(std::make_pair("identify", press));
  wayToBindActions.insert(std::make_pair("jump", press));
  wayToBindActions.insert(std::make_pair("send all", press));
  wayToBindActions.insert(std::make_pair("send team", press));
  wayToBindActions.insert(std::make_pair("send nemesis", press));
  wayToBindActions.insert(std::make_pair("send recipient", press));
  wayToBindActions.insert(std::make_pair("toggle displayScore", press));
  wayToBindActions.insert(std::make_pair("toggle displayBinoculars", press));
  wayToBindActions.insert(std::make_pair("pause", press));
#ifdef SNAPPING
  wayToBindActions.insert(std::make_pair("screenshot", press));
#endif
  wayToBindActions.insert(std::make_pair("time backward", press));
  wayToBindActions.insert(std::make_pair("time forward", press));
  wayToBindActions.insert(std::make_pair("toggle displayRadarFlags", press));
  wayToBindActions.insert(std::make_pair("toggle displayMainFlags", press));
  wayToBindActions.insert(std::make_pair("silence", press));
  wayToBindActions.insert(std::make_pair("toggle displayLabels", press));
  wayToBindActions.insert(std::make_pair("destruct", press));
  wayToBindActions.insert(std::make_pair("roam rotate left", both));
  wayToBindActions.insert(std::make_pair("roam rotate right", both));
  wayToBindActions.insert(std::make_pair("roam rotate up", both));
  wayToBindActions.insert(std::make_pair("roam rotate down", both));
  wayToBindActions.insert(std::make_pair("roam translate left", both));
  wayToBindActions.insert(std::make_pair("roam translate right", both));
  wayToBindActions.insert(std::make_pair("roam translate forward", both));
  wayToBindActions.insert(std::make_pair("roam translate backward", both));
  wayToBindActions.insert(std::make_pair("roam translate up", both));
  wayToBindActions.insert(std::make_pair("roam translate down", both));
  wayToBindActions.insert(std::make_pair("roam cycle subject backward",
					 press));
  wayToBindActions.insert(std::make_pair("roam cycle subject forward", press));
  wayToBindActions.insert(std::make_pair("roam cycle type forward", press));
  wayToBindActions.insert(std::make_pair("roam zoom in", both));
  wayToBindActions.insert(std::make_pair("roam zoom out", both));
  wayToBindActions.insert(std::make_pair("roam zoom normal", both));
  wayToBindActions.insert(std::make_pair("servercommand", press));
  wayToBindActions.insert(std::make_pair("toggle displayFlagHelp", press));
  wayToBindActions.insert(std::make_pair("scrollpanel up", press));
  wayToBindActions.insert(std::make_pair("scrollpanel down", press));
  wayToBindActions.insert(std::make_pair("set displayRadarRange 0.25", press));
  wayToBindActions.insert(std::make_pair("set displayRadarRange 0.5", press));
  wayToBindActions.insert(std::make_pair("set displayRadarRange 1.0", press));
  wayToBindActions.insert(std::make_pair("toggle slowKeyboard", press));
  wayToBindActions.insert(std::make_pair("hunt", press));
  wayToBindActions.insert(std::make_pair("restart", release));
  wayToBindActions.insert(std::make_pair("autopilot", press));

  defaultBinding.insert(std::make_pair("F12", "quit"));
  defaultBinding.insert(std::make_pair("Left Mouse", "fire"));
  defaultBinding.insert(std::make_pair("Enter", "fire"));
  defaultBinding.insert(std::make_pair("Middle Mouse", "drop"));
  defaultBinding.insert(std::make_pair("Space", "drop"));
  defaultBinding.insert(std::make_pair("Right Mouse", "identify"));
  defaultBinding.insert(std::make_pair("I", "identify"));
  defaultBinding.insert(std::make_pair("Tab", "jump"));
  defaultBinding.insert(std::make_pair("N", "send all"));
  defaultBinding.insert(std::make_pair("M", "send team"));
  defaultBinding.insert(std::make_pair(",", "send nemesis"));
  defaultBinding.insert(std::make_pair(".", "send recipient"));
  defaultBinding.insert(std::make_pair("S", "toggle displayScore"));
  defaultBinding.insert(std::make_pair("B", "toggle displayBinoculars"));
  defaultBinding.insert(std::make_pair("Pause", "pause"));
  defaultBinding.insert(std::make_pair("P", "pause"));
#ifdef SNAPPING
  defaultBinding.insert(std::make_pair("F5", "screenshot"));
#endif
  defaultBinding.insert(std::make_pair("-", "time backward"));
  defaultBinding.insert(std::make_pair("=", "time forward"));
  defaultBinding.insert(std::make_pair("H", "toggle displayRadarFlags"));
  defaultBinding.insert(std::make_pair("J", "toggle displayMainFlags"));
  defaultBinding.insert(std::make_pair("K", "silence"));
  defaultBinding.insert(std::make_pair("L", "toggle displayLabels"));
  defaultBinding.insert(std::make_pair("Delete", "destruct"));
  defaultBinding.insert(std::make_pair("Ctrl+Left Arrow", "roam rotate left"));
  defaultBinding.insert(std::make_pair("Ctrl+Right Arrow",
				       "roam rotate right"));
  defaultBinding.insert(std::make_pair("Ctrl+Up Arrow", "roam rotate up"));
  defaultBinding.insert(std::make_pair("Ctrl+Down Arrow", "roam rotate down"));
  defaultBinding.insert(std::make_pair("Shift+Left Arrow",
				       "roam translate left"));
  defaultBinding.insert(std::make_pair("Shift+Right Arrow",
				       "roam translate right"));
  defaultBinding.insert(std::make_pair("Shift+Up Arrow",
				       "roam translate forward"));
  defaultBinding.insert(std::make_pair("Shift+Down Arrow",
				       "roam translate backward"));
  defaultBinding.insert(std::make_pair("Alt+Up Arrow", "roam translate up"));
  defaultBinding.insert(std::make_pair("Alt+Down Arrow",
				       "roam translate down"));
  defaultBinding.insert(std::make_pair("F6", "roam cycle subject backward"));
  defaultBinding.insert(std::make_pair("F7", "roam cycle subject forward"));
  defaultBinding.insert(std::make_pair("F8", "roam cycle type forward"));
  defaultBinding.insert(std::make_pair("F9", "roam zoom in"));
  defaultBinding.insert(std::make_pair("F10", "roam zoom out"));
  defaultBinding.insert(std::make_pair("F11", "roam zoom normal"));
  defaultBinding.insert(std::make_pair("O", "servercommand"));
  defaultBinding.insert(std::make_pair("F", "toggle displayFlagHelp"));
  defaultBinding.insert(std::make_pair("Page Up", "scrollpanel up"));
  defaultBinding.insert(std::make_pair("Page Down", "scrollpanel down"));
  defaultBinding.insert(std::make_pair("1", "set displayRadarRange 0.25"));
  defaultBinding.insert(std::make_pair("2", "set displayRadarRange 0.5"));
  defaultBinding.insert(std::make_pair("3", "set displayRadarRange 1.0"));
  defaultBinding.insert(std::make_pair("A", "toggle slowKeyboard"));
  defaultBinding.insert(std::make_pair("U", "hunt"));
  defaultBinding.insert(std::make_pair("Right Mouse", "restart"));
  defaultBinding.insert(std::make_pair("I", "restart"));
  defaultBinding.insert(std::make_pair("9", "autopilot"));
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
  BindingTable::iterator index;
  if (!wayToBindActions.count(action))
    return;
  PressStatusBind newStatusBind = wayToBindActions[action];
  for (index = bindingTable.begin(); index != bindingTable.end(); ++index) {
    if (index->first != key)
      continue;
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
  BindingTable::iterator index;
  for (index = bindingTable.begin();
       index != bindingTable.end();
       index++) {
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
