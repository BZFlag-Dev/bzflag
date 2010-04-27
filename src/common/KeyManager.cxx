/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(_MSC_VER)
#pragma warning(4:4503)
#endif

// interface header
#include "KeyManager.h"

// system headers
#include <assert.h>
#include <ctype.h>
#include <string.h> // strstr, etc
#include <wctype.h>
#include <string>   // std::string
#include <vector>


// common headers
#include "bzUnicode.h"
#include "TextUtils.h"

// local headers
#include "BzfEvent.h"


// initialize the singleton
template <>
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
  "???",
  "Delete",
  "Kp0",
  "Kp1",
  "Kp2",
  "Kp3",
  "Kp4",
  "Kp5",
  "Kp6",
  "Kp7",
  "Kp8",
  "Kp9",
  "Kp_Period",
  "Kp_Divide",
  "Kp_Multiply",
  "Kp_Minus",
  "Kp_Plus",
  "Kp_Enter",
  "Kp_Equals",
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
  "Help",
  "Print",
  "Sysreq",
  "Break",
  "Menu",
  "Power",
  "Euro",
  "Undo",
  "Left Mouse",
  "Middle Mouse",
  "Right Mouse",
  "Wheel Up",
  "Wheel Down",
  "Mouse Button 6",
  "Mouse Button 7",
  "Mouse Button 8",
  "Mouse Button 9",
  "Mouse Button 10",
  "Joystick Button 1",
  "Joystick Button 2",
  "Joystick Button 3",
  "Joystick Button 4",
  "Joystick Button 5",
  "Joystick Button 6",
  "Joystick Button 7",
  "Joystick Button 8",
  "Joystick Button 9",
  "Joystick Button 10",
  "Joystick Button 11",
  "Joystick Button 12",
  "Joystick Button 13",
  "Joystick Button 14",
  "Joystick Button 15",
  "Joystick Button 16",
  "Joystick Button 17",
  "Joystick Button 18",
  "Joystick Button 19",
  "Joystick Button 20",
  "Joystick Button 21",
  "Joystick Button 22",
  "Joystick Button 23",
  "Joystick Button 24",
  "Joystick Button 25",
  "Joystick Button 26",
  "Joystick Button 27",
  "Joystick Button 28",
  "Joystick Button 29",
  "Joystick Button 30",
  "Joystick Button 31",
  "Joystick Button 32",
  "Hatswitch 1 up",
  "Hatswitch 1 right",
  "Hatswitch 1 down",
  "Hatswitch 1 left",
  "Hatswitch 2 up",
  "Hatswitch 2 right",
  "Hatswitch 2 down",
  "Hatswitch 2 left",
  "LastButton"  // should always be last item listed
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
  key.unicode  = 0;
  key.modifiers  = 0;
  for (i = BzfKeyEvent::Pause; i < BzfKeyEvent::LastButton; ++i) {
    key.button = static_cast<BzfKeyEvent::Button>(i);
    stringToEvent.insert(std::make_pair(std::string(buttonNames[i]), key));
  }
  key.button = BzfKeyEvent::NoButton;
  for (i = 0; i < countof(asciiNames); ++i) {
    key.unicode = asciiNames[i][1][0];
    stringToEvent.insert(std::make_pair(std::string(asciiNames[i][0]), key));
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

void			KeyManager::unbindCommand(const char* command)
{
  EventToCommandMap::iterator index;
  EventToCommandMap::iterator deleteme;

  index = pressEventToCommand.begin();
  while (index != pressEventToCommand.end()) {
    if (index->second == command) {
      deleteme = index;
      index++;
      unbind(deleteme->first, true);
    } else {
      index++;
    }
  }

  index = releaseEventToCommand.begin();
  while (index != releaseEventToCommand.end()) {
    if (index->second == command) {
      deleteme = index;
      index++;
      unbind(deleteme->first, false);
    } else {
      index++;
    }
  }
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
  if (key.modifiers & BzfKeyEvent::ShiftKey)
    name += "Shift+";
  if (key.modifiers & BzfKeyEvent::ControlKey)
    name += "Ctrl+";
  if (key.modifiers & BzfKeyEvent::AltKey)
    name += "Alt+";
  switch (key.unicode) {
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
      if (!iswspace(key.unicode))
      {
        bzUTF8Char chr(key.unicode);
	return name + chr.str();
      }
      return name + "???";
  }
}

bool			KeyManager::stringToKeyEvent(
				const std::string& name, BzfKeyEvent& key) const
{
  // no empties
  if (name.length() == 0) return false;

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
  // can't find it?  try creating it.
  if (index == stringToEvent.end()) {
    UTF8StringItr itr(keyPart.c_str());
    unsigned int chr = (*itr);
    if (chr > 0x21 && iswprint(chr)) {
      if (*(++itr)) // more than one unicode code point...no can do
	return false;
      key.unicode = chr;
      key.modifiers = 0;
      stringToEvent.insert(std::make_pair(keyPart, key));
    }
    // try finding it again
    index = stringToEvent.find(keyPart);
    // if you can't find it now, it's no good
    if (index == stringToEvent.end()) return false;
  }

  // get key event sans shift state
  key = index->second;

  // apply shift state
  if (strstr(shiftPart.c_str(), "+Shift+") != NULL ||
      strstr(shiftPart.c_str(), "+shift+") != NULL)
    key.modifiers |= BzfKeyEvent::ShiftKey;
  if (strstr(shiftPart.c_str(), "+Ctrl+") != NULL ||
      strstr(shiftPart.c_str(), "+ctrl+") != NULL)
    key.modifiers |= BzfKeyEvent::ControlKey;
  if (strstr(shiftPart.c_str(), "+Alt+") != NULL ||
      strstr(shiftPart.c_str(), "+alt+") != NULL)
    key.modifiers |= BzfKeyEvent::AltKey;

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
  CallbackInfo* info = static_cast<CallbackInfo*>(vinfo);
  callback(info->name, info->press, info->cmd, userData);
  return true;
}

bool			KeyManager::KeyEventLessThan::operator()(
				const BzfKeyEvent& a,
				const BzfKeyEvent& b) const
{
  if (a.unicode == 0 && b.unicode == 0) {
    if (a.button < b.button)
      return true;
    if (a.button > b.button)
      return false;

    // check shift
    if (a.modifiers < b.modifiers)
      return true;
  } else if (a.unicode == 0 && b.unicode != 0) {
    return true;
  } else if (a.unicode != 0 && b.unicode == 0) {
    return false;
  } else {
    if (towupper(a.unicode) < towupper(b.unicode))
      return true;
    if (towupper(a.unicode) > towupper(b.unicode))
      return false;

    // check shift state without shift key
    if ((a.modifiers & ~BzfKeyEvent::ShiftKey) < (b.modifiers & ~BzfKeyEvent::ShiftKey))
      return true;
  }

  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
