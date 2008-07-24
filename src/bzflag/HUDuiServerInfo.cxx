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

HUDuiServerInfo::HUDuiServerInfo() : HUDuiControl(), server(NULL)
{
  // Do nothing
}

HUDuiServerInfo::~HUDuiServerInfo()
{
  // Do nothing
}

void HUDuiServerInfo::setServerItem(ServerItem* item)
{
  server = item;
}

void HUDuiServerInfo::doRender()
{
  if (getFontFace() < 0) {
    return;
  }

  if (server == NULL) {
    return;
  }

  // Just blank filler information at the moment, for testing.
  FontManager &fm = FontManager::instance();
  fm.drawString(getX(), getY(), 0, getFontFace()->getFMFace(), getFontSize(), server->description.c_str());
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
