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

#if defined(WIN32)
#pragma warning(4:4503)
#endif

#include "KeyManager.h"
#include <assert.h>
#include <ctype.h>

// initialize the singleton
KeyManager* Singleton<KeyManager>::_instance = (KeyManager*)0;

const char*		KeyManager::buttonNames[] = {
  "???",
  "Pause",
  "Home",
  "End",
  "Left Arrow",
  "Right Arrow",
  "Up Arrow",
  "Down Arrow",
  "Page Up",
  "Page Down",
  "Insert",
  "Delete",
  "F1",
  "F2",
  "F3",
  "F4",
  "F5",
  "F6",
  "F7",
  "F8",
  "F9",
  "F10",
  "F11",
  "F12",
  "Left Mouse",
  "Middle Mouse",
  "Right Mouse"
};
const char*		KeyManager::asciiNames[][2] = {
  { "Tab",		"\t" },
  { "Backspace",	"\b" },
  { "Enter",		"\r" },
  { "Space",		" "  }
};

KeyManager::KeyManager()
{
  unsigned int i;

  // prep string to key map
  BzfKeyEvent key;
  key.ascii  = 0;
  key.shift  = 0;
  for (i = BzfKeyEvent::Pause; i <= BzfKeyEvent::RightMouse; ++i) {
    key.button = static_cast<BzfKeyEvent::Button>(i);
    stringToEvent.insert(std::make_pair(std::string(buttonNames[i]), key));
  }
  key.button = BzfKeyEvent::NoButton;
  for (i = 0; i < countof(asciiNames); ++i) {
    key.ascii = asciiNames[i][1][0];
    stringToEvent.insert(std::make_pair(std::string(asciiNames[i][0]), key));
  }
  char buffer[2];
  buffer[1] = 0;
  for (i = 0x21; i < 0x7f; ++i) {
    buffer[0] = key.ascii = static_cast<char>(i);
    stringToEvent.insert(std::make_pair(std::string(buffer), key));
  }
}

KeyManager::~KeyManager()
{
}

void			KeyManager::bind(const BzfKeyEvent& key,
					 bool press, const std::string& cmd)
{
  if (press) {
    pressEventToCommand.erase(key);
    pressEventToCommand.insert(std::make_pair(key, cmd));
  } else {
    releaseEventToCommand.erase(key);
    releaseEventToCommand.insert(std::make_pair(key, cmd));
  }
  notify(key, press, cmd);
}

void			KeyManager::unbind(const BzfKeyEvent& key,
					   bool press)
{
  if (press)
    pressEventToCommand.erase(key);
  else
    releaseEventToCommand.erase(key);
  notify(key, press, "");
}

std::string		KeyManager::get(const BzfKeyEvent& key,
					bool press) const
{
  const EventToCommandMap* map = press ? &pressEventToCommand :
    &releaseEventToCommand;
  EventToCommandMap::const_iterator index = map->find(key);
  if (index == map->end())
    return "";
  else
    return index->second;
}


std::vector<std::string> KeyManager::getKeysFromCommand(std::string command, bool press) const
{
  std::vector<std::string> keys;
  EventToCommandMap::const_iterator index;
  if (press) {
    for (index = pressEventToCommand.begin(); index != pressEventToCommand.end(); ++index) {
      if (index->second == command) {
	keys.push_back(this->keyEventToString(index->first));
      }
    }
  } else {
    for (index = releaseEventToCommand.begin(); index != releaseEventToCommand.end(); ++index) {
      if (index->second == command) {
	keys.push_back(this->keyEventToString(index->first));
      }
    }
  }
  return keys;
}


std::string		KeyManager::keyEventToString(
					const BzfKeyEvent& key) const
{
  std::string name;
  if (key.shift & BzfKeyEvent::ShiftKey)
    name += "Shift+";
  if (key.shift & BzfKeyEvent::ControlKey)
    name += "Ctrl+";
  if (key.shift & BzfKeyEvent::AltKey)
    name += "Alt+";
  switch (key.ascii) {
    case 0:
      return name + buttonNames[key.button];
    case '\b':
      return name + "Backspace";
    case '\t':
      return name + "Tab";
    case '\r':
      return name + "Enter";
    case ' ':
      return name + "Space";
    default:
      if (!isspace(key.ascii))
	return name + std::string(&key.ascii, 1);
      return name + "???";
  }
}

bool			KeyManager::stringToKeyEvent(
				const std::string& name, BzfKeyEvent& key) const
{
  // find last + in name
  const char* shiftDelimiter = strrchr(name.c_str(), '+');

  // split name into shift part and key name part
  std::string shiftPart, keyPart;
  if (shiftDelimiter == NULL) {
    keyPart = name;
  } else {
    shiftPart  = "+";
    shiftPart += name.substr(0, shiftDelimiter - name.c_str() + 1);
    keyPart    = shiftDelimiter + 1;
  }

  // find key name
  StringToEventMap::const_iterator index = stringToEvent.find(keyPart);
  if (index == stringToEvent.end())
    return false;

  // get key event sans shift state
  key = index->second;

  // apply shift state
  if (strstr(shiftPart.c_str(), "+Shift+") != NULL ||
      strstr(shiftPart.c_str(), "+shift+") != NULL)
    key.shift |= BzfKeyEvent::ShiftKey;
  if (strstr(shiftPart.c_str(), "+Ctrl+") != NULL ||
      strstr(shiftPart.c_str(), "+ctrl+") != NULL)
    key.shift |= BzfKeyEvent::ControlKey;
  if (strstr(shiftPart.c_str(), "+Alt+") != NULL ||
      strstr(shiftPart.c_str(), "+alt+") != NULL)
    key.shift |= BzfKeyEvent::AltKey;

  // success
  return true;
}

void			KeyManager::iterate(
				IterateCallback callback, void* userData)
{
  assert(callback != NULL);

  EventToCommandMap::const_iterator index;
  for (index = pressEventToCommand.begin(); index != pressEventToCommand.end(); ++index)
    (*callback)(keyEventToString(index->first), true, index->second, userData);
  for (index = releaseEventToCommand.begin(); index != releaseEventToCommand.end(); ++index)
    (*callback)(keyEventToString(index->first), false, index->second, userData);
}

void			KeyManager::addCallback(
				ChangeCallback callback, void* userData)
{
  callbacks.add(callback, userData);
}

void			KeyManager::removeCallback(
				ChangeCallback callback, void* userData)
{
  callbacks.remove(callback, userData);
}

void			KeyManager::notify(
				const BzfKeyEvent& key,
				bool press, const std::string& cmd)
{
  CallbackInfo info;
  info.name  = keyEventToString(key);
  info.press = press;
  info.cmd   = cmd;
  callbacks.iterate(&onCallback, &info);
}

bool			KeyManager::onCallback(
				ChangeCallback callback,
				void* userData,
				void* vinfo)
{
  CallbackInfo* info = reinterpret_cast<CallbackInfo*>(vinfo);
  callback(info->name, info->press, info->cmd, userData);
  return true;
}

bool			KeyManager::KeyEventLess::operator()(
				const BzfKeyEvent& a,
				const BzfKeyEvent& b) const
{
  if (a.ascii == 0 && b.ascii == 0) {
    if (a.button < b.button)
      return true;
    if (a.button > b.button)
      return false;

    // check shift
    if (a.shift < b.shift)
      return true;
  } else if (a.ascii == 0 && b.ascii != 0) {
    return true;
  } else if (a.ascii != 0 && b.ascii == 0) {
    return false;
  } else {
    if (toupper(a.ascii) < toupper(b.ascii))
      return true;
    if (toupper(a.ascii) > toupper(b.ascii))
      return false;

    // check shift state without shift key
    if ((a.shift & ~BzfKeyEvent::ShiftKey) < (b.shift & ~BzfKeyEvent::ShiftKey))
      return true;
  }

  return false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

