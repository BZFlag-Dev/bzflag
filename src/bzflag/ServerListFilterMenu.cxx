/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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
#include "ServerListFilterMenu.h"

/* common implementation headers */
#include "TextUtils.h"
#include "StateDatabase.h"
#include "FontManager.h"

/* local implementation headers */
#include "MenuDefaultKey.h"
#include "HUDDialogStack.h"
#include "MainMenu.h"
#include "ServerListFilter.h"
#include "HUDui.h"
#include "ServerListFilterHelpMenu.h"

ServerListFilterMenu::ServerListFilterMenu()
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();

  controls.push_back(createLabel("Server List Filters"));

  firstKeyControl = controls.size();

  for (int i = 1; i <= 9; i++) {
    std::string keyLabel = TextUtils::format("Quick Filter %d:", i);
    controls.push_back(createInput(keyLabel));
  }

  lastKeyControl = controls.size() - 1;

  resetPresets = createLabel("Restore default quick filters");
  controls.push_back(resetPresets);

  help = createLabel("Quick filter help");
  controls.push_back(help);

  initNavigation(controls, firstKeyControl, controls.size()-1);
}

ServerListFilterMenu::~ServerListFilterMenu()
{
  ServerListFilterHelpMenu::done();
}

HUDuiDefaultKey* ServerListFilterMenu::getDefaultKey()
{
  return MenuDefaultKey::getInstance();
}

void ServerListFilterMenu::show()
{
  std::vector<HUDuiControl*>& controls = getControls();

  for (int  i = 1; i <= 9; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
    std::string keyName = TextUtils::format("listFilter%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }
}

void ServerListFilterMenu::execute()
{
  HUDuiControl* _focus = HUDui::getFocus();
  if (_focus == resetPresets) {
    std::vector<HUDuiControl*>& controls = getControls();

    for (int i = 1; i <= 9; i++) {
      HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
      std::string keyName = TextUtils::format("listFilter%d", i);
      BZDB.set(keyName, BZDB.getDefault(keyName));
      std::string keyValue = BZDB.get(keyName);
      entry->setString(keyValue);
    }
  }
  else if (_focus == help) {
    HUDDialogStack::get()->push(ServerListFilterHelpMenu::getServerListFilterHelpMenu());
  }
}

void ServerListFilterMenu::dismiss()
{
  std::vector<HUDuiControl*>& controls = getControls();

  int i;
  for (i=1; i <= 9; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
    std::string keyValue = entry->getString();
    std::string keyName = TextUtils::format("listFilter%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }
}

void ServerListFilterMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);
  int i;

  // use a big font for title, smaller font for the rest
  const float titleFontSize = (float)_height / 15.0f;
  const float fontSize = (float)_height / 45.0f;
  FontManager &fm = FontManager::instance();
  int fontFace = MainMenu::getFontFace();

  // reposition title
  std::vector<HUDuiControl*>& controls = getControls();
  HUDuiLabel* title = (HUDuiLabel*)controls[0];
  title->setFontSize(titleFontSize);
  const float titleWidth = fm.getStrLength(fontFace, titleFontSize, title->getString());
  const float titleHeight = fm.getStrHeight(fontFace, titleFontSize, " ");
  float x = 0.5f * ((float)_width - titleWidth);
  float y = (float)_height - titleHeight;
  title->setPosition(x, y);

  // reposition options
  const float labelWidth = fm.getStrLength(controls[firstKeyControl]->getFontFace(), fontSize, controls[firstKeyControl]->getLabel());
  x = (0.1f * (float)_width) + labelWidth;
  y -= 0.6f * titleHeight;
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const int count = controls.size();
  for (i = 1; i < count; i++) {
    controls[i]->setFontSize(fontSize);
    // Add extra padding after the last quick filter, and between restore defaults and help
    if (i == lastKeyControl + 1 || i == count - 1)
      y -= 1.0f * h;
    controls[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

HUDuiLabel* ServerListFilterMenu::createLabel(const std::string &str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString(str);
  return label;
}


HUDuiTypeIn* ServerListFilterMenu::createInput(const std::string &label)
{
  HUDuiTypeIn* entry = new HUDuiTypeIn;
  entry->setFontFace(MainMenu::getFontFace());
  entry->setLabel(label);
  entry->setMaxLength(42);
  entry->setColorFunc(ServerListFilter::colorizeSearch);
  return entry;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
