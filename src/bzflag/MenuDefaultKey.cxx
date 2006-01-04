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

/* interface header */
#include "MenuDefaultKey.h"

/* common implementation headers */
#include "KeyManager.h"
#include "CommandsStandard.h"

/* local implementation headers */
#include "sound.h"
#include "HUDDialogStack.h"
#include "HUDDialog.h"


MenuDefaultKey MenuDefaultKey::instance;

MenuDefaultKey::MenuDefaultKey() { }
MenuDefaultKey::~MenuDefaultKey() { }

MenuDefaultKey* MenuDefaultKey::getInstance()
{
  return &instance;
}

bool MenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  switch (key.ascii) {
    case 27:	// escape
      playLocalSound(SFX_DROP_FLAG);
      HUDDialogStack::get()->pop();
      return true;

    case 13:	// return
      playLocalSound(SFX_GRAB_FLAG);
      HUDDialogStack::get()->top()->execute();
      return true;
  }

  if (KEYMGR.get(key, true) == "quit") {
    CommandsStandard::quit();
    return true;
  }

  return false;
}

bool MenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  switch (key.ascii) {
    case 27:	// escape
    case 13:	// return
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
