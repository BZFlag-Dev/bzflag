/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

/* interface header */
#include "SaveWorldMenu.h"

/* system implementation headers */
#include <vector>
#include <string>

/* common implementation headers */
#include "OpenGLTexFont.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "OpenGLTexture.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "DirectoryNames.h"

/* local implementation headers */
#include "MenuDefaultKey.h"
#include "World.h"
#include "FormatMenu.h"
#include "KeyboardMapMenu.h"
#include "GUIOptionsMenu.h"
#include "SaveWorldMenu.h"
#include "ServerListCache.h"
#include "MainMenu.h"


SaveWorldMenu::SaveWorldMenu()
{
  // add controls
  std::vector<HUDuiControl*>& list = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString("Save World");
  list.push_back(label);

  filename = new HUDuiTypeIn;
  filename->setFont(MainMenu::getFont());
  filename->setLabel("File Name:");
  filename->setMaxLength(255);
  list.push_back(filename);

  status = new HUDuiLabel;
  status->setFont(MainMenu::getFont());
  status->setString("");
  status->setPosition(0.5f * (float)width, status->getY());
  list.push_back(status);

  // only navigate to the file name
  initNavigation(list, 1,1);
}

SaveWorldMenu::~SaveWorldMenu()
{
}


HUDuiDefaultKey* SaveWorldMenu::getDefaultKey()
{ 
  return MenuDefaultKey::getInstance(); 
}

void SaveWorldMenu::execute()
{
  World *pWorld = World::getWorld();
  if (pWorld == NULL) {
    status->setString( "No world loaded to save" );
  } else {
    std::string fullname = getWorldDirName();
    fullname += filename->getString();
    fullname += ".bzw";
    bool success = World::getWorld()->writeWorld(fullname);
    if (success) {
      std::string newLabel = "File Saved: ";
      newLabel += fullname;
      status->setString( newLabel );
    } else {
      status->setString( "Error saving file" );
    }
  }
  const OpenGLTexFont& font = status->getFont();
  const float statusWidth = font.getWidth(status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}

void SaveWorldMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 12.0f;
  const float titleFontHeight = (float)height / 12.0f;

  // use a big font
  float fontWidth = (float)height / 24.0f;
  float fontHeight = (float)height / 24.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)width - 0.75f * titleWidth);
  y -= 0.6f * 3 * titleFont.getHeight();
  list[1]->setFontSize(fontWidth, fontHeight);
  const float h = list[1]->getFont().getHeight();
  const int count = list.size();
  int i;
  for (i = 1; i < count-1; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 100.0f;
  y -= 100.0f;
  list[i]->setFontSize(fontWidth, fontHeight);
  list[i]->setPosition(x, y);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
