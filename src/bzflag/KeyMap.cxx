/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "KeyMap.h"
#include <ctype.h>

const BzfKeyEvent	KeyMap::defaults1[] = {
				{ 0, BzfKeyEvent::LeftMouse, 0 },
				{ 0, BzfKeyEvent::MiddleMouse, 0 },
				{ 0, BzfKeyEvent::RightMouse, 0 },
				{ '1', 0, 0 },
				{ '2', 0, 0 },
				{ '3', 0, 0 },
				{ 'n', 0, 0 },
				{ 'm', 0, 0 },
				{ '\t', 0, 0 },
				{ 'b', 0, 0 },
				{ 's', 0, 0 },
				{ 'f', 0, 0 },
				{ '=', 0, 0 },
				{ '-', 0, 0 },
				{ 0, BzfKeyEvent::Pause, 0 },
				{ 0, BzfKeyEvent::F12, 0 },
				{ 0, BzfKeyEvent::PageUp, 0 },
				{ 0, BzfKeyEvent::PageDown, 0 }

			};
const BzfKeyEvent	KeyMap::defaults2[] = {
				{ 0, 0, 0 },
				{ ' ', 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ '+', 0, 0 },
				{ '_', 0, 0 },
				{ 'p', 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 }
			};
const char*		KeyMap::keyName[] = {
				"fireKey",
				"dropFlagKey",
				"identifyKey",
				"shortRangeKey",
				"mediumRangeKey",
				"longRangeKey",
				"sendAllKey",
				"sendTeamKey",
				"jumpKey",
				"binocularsKey",
				"scoreKey",
				"flagHelpKey",
				"timeForwardKey",
				"timeBackwardKey",
				"pauseKey",
				"quitKey",
				"scrollBackwardKey",
				"scrollForwardKey",
			};

const char*		KeyMap::eventNames[] = {
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

KeyMap::KeyMap()
{
  resetAll();
}

KeyMap::~KeyMap()
{
  // do nothing
}

void			KeyMap::resetAll()
{
  reset(FireShot);
  reset(DropFlag);
  reset(Identify);
  reset(ShortRange);
  reset(MediumRange);
  reset(LongRange);
  reset(SendAll);
  reset(SendTeam);
  reset(Jump);
  reset(Binoculars);
  reset(Score);
  reset(FlagHelp);
  reset(TimeForward);
  reset(TimeBackward);
  reset(Pause);
  reset(Quit);
  reset(ScrollBackward);
  reset(ScrollForward);
}

void			KeyMap::reset(Key key)
{
  clear(key);
  set(key, defaults1[key]);
  set(key, defaults2[key]);
}

void			KeyMap::clear(Key key)
{
  map1[key].ascii = 0;
  map1[key].button = 0;
  map1[key].shift = 0;
  map2[key].ascii = 0;
  map2[key].button = 0;
  map2[key].shift = 0;
}

void			KeyMap::set(Key key, const BzfKeyEvent& event)
{
  if ((map1[key].ascii != 0 || map1[key].button != 0) &&
      (map2[key].ascii != 0 || map2[key].button != 0))
    clear(key);

  if (map1[key].ascii == 0 && map1[key].button == 0) {
    map1[key].ascii = toupper(event.ascii);
    map1[key].button = event.button;
  }
  else {
    map2[key].ascii = toupper(event.ascii);
    map2[key].button = event.button;
  }
}

void			KeyMap::unset(Key key, const BzfKeyEvent& event)
{
  if (event.ascii == 0 && event.button == 0) return;
  if (map1[key].ascii == toupper(event.ascii) &&
      map1[key].button == event.button) {
    map1[key] = map2[key];
    map2[key].ascii = 0;
    map2[key].button = 0;
  }
  else if (map2[key].ascii == toupper(event.ascii) &&
	   map2[key].button == event.button) {
    map2[key].ascii = 0;
    map2[key].button = 0;
  }
}

const BzfKeyEvent&	KeyMap::get(Key key) const
{
  return map1[key];
}

const BzfKeyEvent&	KeyMap::getAlternate(Key key) const
{
  return map2[key];
}

KeyMap::Key		KeyMap::isMapped(char ascii) const
{
  BzfKeyEvent event;
  event.ascii = ascii;
  event.button = 0;
  event.shift = 0;
  return isMapped(event);
}

KeyMap::Key		KeyMap::isMapped(BzfKeyEvent::Button button) const
{
  BzfKeyEvent event;
  event.ascii = 0;
  event.button = button;
  event.shift = 0;
  return isMapped(event);
}

KeyMap::Key		KeyMap::isMapped(const BzfKeyEvent& event) const
{
  if (event.ascii == 0 && event.button == 0)
    return LastKey;

  for (int i = 0; i < LastKey; i++) {
    if (map1[i].ascii == toupper(event.ascii) &&
	map1[i].button == event.button)
      return (Key)i;
    if (map2[i].ascii == toupper(event.ascii) &&
	map2[i].button == event.button)
      return (Key)i;
  }
  return LastKey;
}

boolean			KeyMap::isMappedTo(Key key,
				const BzfKeyEvent& event) const
{
  if (event.ascii == 0 && event.button == 0)
    return False;

  if (map1[key].ascii == toupper(event.ascii) &&
      map1[key].button == event.button)
    return True;
  if (map2[key].ascii == toupper(event.ascii) &&
      map2[key].button == event.button)
    return True;

  return False;
}

BzfString		KeyMap::getKeyName(Key key)
{
  return BzfString(keyName[key]);
}

KeyMap::Key		KeyMap::lookupKeyName(const BzfString& name)
{
  for (int i = 0; i < (int)(sizeof(keyName) / sizeof(keyName[0])); i++)
    if (name == keyName[i])
      return (Key)i;
  return LastKey;
}

BzfString		KeyMap::getKeyEventString(const BzfKeyEvent& event)
{
  if (event.ascii != 0) {
    if (event.ascii == '\b') return "Backspace";
    if (!isspace(event.ascii)) return BzfString(&event.ascii, 1);
    if (event.ascii == ' ') return "Space";
    if (event.ascii == '\t') return "Tab";
    if (event.ascii == '\r') return "Enter";
    return "???";
  }

  return eventNames[event.button];
}

boolean			KeyMap::translateStringToEvent(
				const BzfString& value, BzfKeyEvent& event)
{
  event.shift = 0;

  // ignore bogus value
  if (value == "???") return False;

  // check for plain ascii (one character) values
  if (value.getLength() == 1) {
    char c = value[0];
    if (isalnum(c) || ispunct(c)) {
      event.ascii = c;
      event.button = 0;
      return True;
    }
    return False;
  }

  // check whitespace values
  if (value == "Space") {
    event.ascii = ' ';
    event.button = 0;
    return True;
  }
  if (value == "Tab") {
    event.ascii = '\t';
    event.button = 0;
    return True;
  }
  if (value == "Backspace") {
    event.ascii = '\b';
    event.button = 0;
    return True;
  }
  if (value == "Enter") {
    event.ascii = '\r';
    event.button = 0;
    return True;
  }

  // check non-ascii button strings
  for (int i = 0; i < (int)(sizeof(eventNames) / sizeof(eventNames[0])); i++)
    if (value == eventNames[i]) {
      event.ascii = 0;
      event.button = i;
      return True;
    }

  return False;
}
