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

/* interface header */
#include "QuickKeysMenu.h"

/* common implementation headers */
#include "OpenGLTexFont.h"
#include "TextUtils.h"
#include "StateDatabase.h"

/* local implementation headers */
#include "MenuDefaultKey.h"
#include "MainMenu.h"


QuickKeysMenu::QuickKeysMenu()
{
  // add controls
  std::vector<HUDuiControl*>& controls = getControls();

  controls.push_back(createLabel("Define Quick Keys"));
  controls.push_back(createLabel("Notice: depending on platform not all keys might work"));

  controls.push_back(createLabel("Send to All"));
  controls.push_back(createLabel("Send to Team"));

  firstKeyControl = controls.size();

  int i;
  for (i=1; i < 11; i++) {
    std::string keyLabel = string_util::format("Alt-F%d", i);
    controls.push_back(createInput(keyLabel));
  }

  for (i=1; i < 11; i++) {
    std::string keyLabel = string_util::format("Ctrl-F%d", i);
    controls.push_back(createInput(keyLabel));
  }

  initNavigation(controls, firstKeyControl, controls.size()-1);
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
  std::vector<HUDuiControl*>& controls = getControls();

  int i;
  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
    std::string keyName = string_util::format("quickMessage%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }

  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i + 9]);
    std::string keyName = string_util::format("quickTeamMessage%d", i);
    std::string keyValue = BZDB.get(keyName);
    entry->setString(keyValue);
  }
}

void QuickKeysMenu::dismiss()
{
  std::vector<HUDuiControl*>& controls = getControls();

  int i;
  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i - 1]);
    std::string keyValue = entry->getString();
    std::string keyName = string_util::format("quickMessage%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }

  for (i=1; i < 11; i++) {
    HUDuiTypeIn *entry = static_cast<HUDuiTypeIn*>(controls[firstKeyControl + i + 9]);
    std::string keyValue = entry->getString();
    std::string keyName = string_util::format("quickTeamMessage%d", i);
    if (keyValue.empty() && BZDB.isSet(keyName))
      BZDB.unset(keyName);
    else if (!keyValue.empty())
      BZDB.set(keyName, keyValue);
  }
}

void QuickKeysMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  int i;
  // use a big font for title, smaller font for the rest
  const float titleFontWidth = (float)height / 10.0f;
  const float titleFontHeight = (float)height / 10.0f;
  const float bigFontWidth = (float)height / 28.0f;
  const float bigFontHeight = (float)height / 28.0f;

  const float fontWidth = (float)height / 32.0f;
  const float fontHeight = (float)height / 32.0f;

  // reposition title
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* title = (HUDuiLabel*)list[0];
  title->setFontSize(titleFontWidth, titleFontHeight);
  const OpenGLTexFont& titleFont = title->getFont();
  const float titleWidth = titleFont.getWidth(title->getString());
  float x = 0.5f * ((float)width - titleWidth);
  float y = (float)height - titleFont.getHeight();
  title->setPosition(x, y);

  // reposition help
  HUDuiLabel*help = (HUDuiLabel*)list[1];
  help->setFontSize( bigFontWidth, bigFontHeight);
  const OpenGLTexFont& helpFont = help->getFont();
  const float helpWidth = helpFont.getWidth(help->getString());
  x = 0.5f * ((float)width - helpWidth);
  y -= 1.1f * helpFont.getHeight();
  help->setPosition(x, y);

  // reposition column titles
  HUDuiLabel *all = (HUDuiLabel*)list[2];
  all->setFontSize(bigFontWidth, bigFontHeight);
  x = 0.1f * width;
  y -= 1.5f * helpFont.getHeight();
  all->setPosition(x, y);
  HUDuiLabel *team = (HUDuiLabel*)list[3];
  team->setFontSize(bigFontWidth, bigFontHeight);
  x = 0.6f * width;
  team->setPosition(x, y);


  // reposition options in two columns
  x = 0.10f * (float)width;
  const float topY = y - (0.6f * titleFont.getHeight());
  y = topY;
  list[4]->setFontSize(fontWidth, fontHeight);
  const float h = list[4]->getFont().getHeight();
  const int count = list.size() - firstKeyControl;
  const int mid = (count / 2) + firstKeyControl;

  for (i = firstKeyControl; i < mid; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }

  x = 0.60f * (float)width;
  y = topY;
  for (;i < count + firstKeyControl; i++) {
    list[i]->setFontSize(fontWidth, fontHeight);
    list[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

HUDuiLabel* QuickKeysMenu::createLabel(const std::string &str)
{
  HUDuiLabel* label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
  label->setString(str);
  return label;
}


HUDuiTypeIn* QuickKeysMenu::createInput(const std::string &label)
{
  HUDuiTypeIn* entry = new HUDuiTypeIn;
  entry->setFont(MainMenu::getFont());
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
