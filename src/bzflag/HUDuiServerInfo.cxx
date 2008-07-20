/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface headers
#include "HUDuiServerInfo.h"

#include "playing.h"

#include "FontManager.h"
#include "LocalFontFace.h"

//
// HUDuiServerInfo
//

HUDuiServerInfo::HUDuiServerInfo() : HUDuiControl()
{
  // Do nothing
}

HUDuiServerInfo::~HUDuiServerInfo()
{
  // Do nothing
}

void HUDuiServerInfo::setServerItem(ServerItem* item)
{
  serverPointer = item;
}

void HUDuiServerInfo::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  // Do nothing at the moment
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
