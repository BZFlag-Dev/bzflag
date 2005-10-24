/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include "ShotStatsDefaultKey.h"

/* common implementation headers */
#include "KeyManager.h"
#include "CommandManager.h"

/* local implementation headers */
#include "HUDDialogStack.h"
#include "HUDDialog.h"


ShotStatsDefaultKey ShotStatsDefaultKey::instance;

ShotStatsDefaultKey::ShotStatsDefaultKey() { }
ShotStatsDefaultKey::~ShotStatsDefaultKey() { }

ShotStatsDefaultKey* ShotStatsDefaultKey::getInstance()
{
  return &instance;
}

bool ShotStatsDefaultKey::keyPress(const BzfKeyEvent& key)
{
  // special keys to get out
  switch (key.ascii) {
    case 27:	// escape
      HUDDialogStack::get()->pop();
      return true;
    case 13:	// return
      HUDDialogStack::get()->top()->execute();
      return true;
  }
  if (key.button == BzfKeyEvent::Home) {
    HUDDialogStack::get()->pop();
    return true;
  }

  // allow all commands to run
  std::string keyCommand = KEYMGR.get(key, true);
  if (keyCommand != "") {
    CMDMGR.run(keyCommand);
    return true;
  }

  // all other keys return
  HUDDialogStack::get()->pop();
  return true;
}

bool ShotStatsDefaultKey::keyRelease(const BzfKeyEvent& /* key */)
{
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
