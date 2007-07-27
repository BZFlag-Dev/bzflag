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
#include "QuickKeysMenu.h"

/* common implementation headers */
#include "TextUtils.h"
#include "StateDatabase.h"
#include "FontManager.h"

/* local implementation headers */
#include "FontSizer.h"
#include "MenuDefaultKey.h"
#include "MainMenu.h"


QuickKeysMenu::QuickKeysMenu()
{
  // add controls
  addControl(createLabel("Define Quick Keys"), false);
  addControl(createLabel("Notice: depending on platform not all keys might work"), false);

  addControl(createLabel("Send to All"), false);
  addControl(createLabel("Send to Team"), false);

  int i;
  for (i=1; i < 11; ++i) {
    std::string keyLabel = TextUtils::format("Alt-F%d", i);
    addControl(createInput(keyLabel));
  }

  for (i=1; i < 11; ++i) {
    std::string keyLabel = TextUtils::format("Ctrl-F%d", i);
    addControl(createInput(keyLabel));
  }

  initNavigation();
}

QuickKeysMenu::~QuickKeysMenu()
{
}

HUDuiDefaultKey* QuickKeysMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void QuickKeysMenu::show()
{
  HUDNavigationQueue& controls = getNav();

  int i;
  for (i=1; i < 11; ++i) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[i - 1]);
    std::string keyName = TextUtils::format("quickMessage%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }

  for (i=1; i < 11; ++i) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[i + 9]);
    std::string keyName = TextUtils::format("quickTeamMessage%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }
}

void QuickKeysMenu::dismiss()
{
  HUDNavigationQueue& controls = getNav();

  int i;
  for (i=1; i < 11; ++i) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[i - 1]);
    std::string keyValue = entry->getString();
    std::string keyName = TextUtils::format("quickMessage%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }

  for (i=1; i < 11; ++i) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[i + 9]);
    std::string keyValue = entry->getString();
    std::string keyName = TextUtils::format("quickTeamMessage%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }
}

void QuickKeysMenu::resize(int _width, int _height)
{
  int i;
  HUDDialog::resize(_width, _height);
  FontSizer fs = FontSizer(_width, _height);

  FontManager &fm = FontManager::instance();
  const int fontFace = MainMenu::getFontFace();

  // use a big font for title, smaller font for the rest
  fs.setMin(0, (int)(1.0 / BZDB.eval("headerFontSize") / 2.0));
  const float titleFontSize = fs.getFontSize(fontFace, "headerFontSize");

  fs.setMin(0, 20);
  const float bigFontSize = fs.getFontSize(fontFace, "menuFontSize");

  fs.setMin(0, 40);
  const float fontSize = fs.getFontSize(fontFace, "infoFontSize");

  // reposition title
  std::vector<HUDuiElement*>& listHUD = getElements();
  HUDuiLabel* title = (HUDuiLabel*)listHUD[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStringWidth(fontFace, titleFontSize, title->getString().c_str());
  const float titleHeight = fm.getStringHeight(fontFace, titleFontSize);
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition help
  HUDuiLabel*help = (HUDuiLabel*)listHUD[1];
  help->setFontSize(bigFontSize);
  const float helpWidth = fm.getStringWidth(fontFace, bigFontSize, help->getString().c_str());
  const float bigHeight = fm.getStringHeight(fontFace, bigFontSize);
  x = 0.5f * ((float)_width - helpWidth);
  y -= 1.1f * bigHeight;
  help->setPosition(x, y);

  // reposition column titles
  HUDuiLabel *all = (HUDuiLabel*)listHUD[2];
  all->setFontSize(bigFontSize);
  x = 0.1f * _width;
  y -= 1.5f * bigHeight;
  all->setPosition(x, y);
  HUDuiLabel *team = (HUDuiLabel*)listHUD[3];
  team->setFontSize(bigFontSize);
  x = 0.6f * _width;
  team->setPosition(x, y);


  // reposition options in two columns
  HUDNavigationQueue& navItems = getNav();
  x = 0.10f * (float)_width;
  const float topY = y - (0.6f * titleHeight);
  y = topY;
  const float h = fm.getStringHeight(fontFace, fontSize);
  const int count = (int)navItems.size();
  const int mid = (count / 2);

  for (i = 0; i < mid; ++i) {
    navItems[i]->setFontSize(fontSize);
    navItems[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.60f * (float)_width;
  y = topY;
  for (;i < count; ++i) {
    navItems[i]->setFontSize(fontSize);
    navItems[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

HUDuiLabel* QuickKeysMenu::createLabel(const std::string &str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString(str);
  return label;
}


HUDuiTypeIn* QuickKeysMenu::createInput(const std::string &label)
{
  HUDuiTypeIn* entry = new HUDuiTypeIn;
  entry->setFontFace(MainMenu::getFontFace());
  entry->setLabel(label);
  entry->setMaxLength(40); // some strings >20 won't already fit into column
  return entry;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
