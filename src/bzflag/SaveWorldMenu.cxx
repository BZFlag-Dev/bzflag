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
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Save World");
  addControl(label, false);

  filename = new HUDuiTypeIn;
  filename->setFontFace(MainMenu::getFontFace());
  filename->setLabel("File Name:");
  filename->setMaxLength(255);
  addControl(filename);

  status = new HUDuiLabel;
  status->setFontFace(MainMenu::getFontFace());
  status->setString("");
  status->setPosition(0.5f * (float)width, status->getY());
  addControl(status, false);

  initNavigation();
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
  World *world = World::getWorld();
  if (world == NULL) {
    status->setString("No world loaded to save");
  } else {
    std::string fullname;
    BZDB.set("saveAsMeshes", "0");
    BZDB.set("saveFlatFile", "0");
    BZDB.set("saveAsOBJ",    "0");
    if (world->writeWorld(filename->getString(), fullname)) {
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
  const float statusWidth = fm.getStringLength(status->getFontFace(), status->getFontSize(), status->getString());
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
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringLength(title->getFontFace(), titleFontSize, title->getString());
  const float titleHeight = fm.getStringHeight(title->getFontFace(), titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  x = 0.5f * ((float)_width - 0.75f * titleWidth);
  y -= 0.6f * 3 * titleHeight;
  const float h = fm.getStringHeight(listHUD[1]->getFontFace(), fontSize, " ");
  const int count = (const int)listHUD.size();
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
