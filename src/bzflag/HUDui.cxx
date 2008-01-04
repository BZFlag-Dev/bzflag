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

// interface header
#include "HUDui.h"

//
// HUDui
//

HUDuiControl*		HUDui::focus = NULL;
HUDuiDefaultKey*	HUDui::defaultKey = NULL;

HUDuiControl*		HUDui::getFocus()
{
  return focus;
}

void			HUDui::setFocus(HUDuiControl* _focus)
{
  focus = _focus;
}

HUDuiDefaultKey*	HUDui::getDefaultKey()
{
  return defaultKey;
}

void			HUDui::setDefaultKey(HUDuiDefaultKey* _defaultKey)
{
  defaultKey = _defaultKey;
}

bool			HUDui::keyPress(const BzfKeyEvent& key)
{
  if (defaultKey && defaultKey->keyPress(key)) return true;
  if (focus && focus->doKeyPress(key)) return true;
  return false;
}

bool			HUDui::keyRelease(const BzfKeyEvent& key)
{
  if (defaultKey && defaultKey->keyRelease(key)) return true;
  if (focus && focus->doKeyRelease(key)) return true;
  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
