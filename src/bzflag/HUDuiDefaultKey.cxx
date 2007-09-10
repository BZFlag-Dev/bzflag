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
#include "HUDuiDefaultKey.h"

//
// HUDuiDefaultKey
//

HUDuiDefaultKey::HUDuiDefaultKey()
{
  // do nothing
}

HUDuiDefaultKey::~HUDuiDefaultKey()
{
  // do nothing
}

bool			HUDuiDefaultKey::keyPress(const BzfKeyEvent&)
{
  return false;
}

bool			HUDuiDefaultKey::keyRelease(const BzfKeyEvent&)
{
  return false;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

