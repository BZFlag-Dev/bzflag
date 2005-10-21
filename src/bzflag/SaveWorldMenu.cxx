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
#include "SaveWorldMenu.h"

/* common implementation headers */
#include "FontManager.h"

/* local implementation headers */
#include "MenuDefaultKey.h"
#include "World.h"
#include "MainMenu.h"


SaveWorldMenu::SaveWorldMenu()
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();

  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Save World");
  listHUD.push_back(label);

  filename = new HUDuiTypeIn;
  filename->setFontFace(MainMenu::getFontFace());
  filename->setLabel("File Name:");
  filename->setMaxLength(255);
  listHUD.push_back(filename);

  status = new HUDuiLabel;
  status->setFontFace(MainMenu::getFontFace());
  status->setString("");
  status->setPosition(0.5f * (float)width, status->getY());
  listHUD.push_back(status);

  // only navigate to the file name
  initNavigation(listHUD, 1,1);
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
    status->setString("No world loaded to save");
  } else {
    std::string fullname;
    if (World::getWorld()->writeWorld(filename->getString(), fullname)) {
      std::string newLabel = "World Saved: ";
      newLabel += fullname;
      status->setString(newLabel);
    } else {
      std::string newLabel = "Error Saving: ";
      newLabel += fullname;
      status->setString(newLabel);
    }
  }
  FontManager &fm = FontManager::instance();
  const float statusWidth = fm.getStrLength(status->getFontFace(), status->getFontSize(), status->getString());
  status->setPosition(0.5f * ((float)width - statusWidth), status->getY());
}

void SaveWorldMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font for the body, bigger for the title
  const float titleFontSize = (float)_height / 18.0f;
  float fontSize = (float)_height / 36.0f;
  FontManager &fm = FontManager::instance();

  // reposition title
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(title->getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(title->getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width - 0.75f * titleWidth);
  y -= 0.6f * 3 * titleHeight;
  const float h = fm.getStrHeight(listHUD[1]->getFontFace(), fontSize, " ");
  const int count = listHUD.size();
  int i;
  for (i = 1; i < count-1; i++) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 100.0f;
  y -= 100.0f;
  listHUD[i]->setFontSize(fontSize);
  listHUD[i]->setPosition(x, y);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
