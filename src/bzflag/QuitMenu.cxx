/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "QuitMenu.h"

/* common implementation headers */
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"
#include "HUDuiLabel.h"


bool QuitMenuDefaultKey::keyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 'Y' || key.ascii == 'y') {
    CommandsStandard::quit();
    return true;
  }
  return MenuDefaultKey::keyPress(key);
}

bool QuitMenuDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  if (key.ascii == 'Y' || key.ascii == 'y')
    return true;
  return MenuDefaultKey::keyRelease(key);
}


QuitMenu::QuitMenu()
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* label;

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Enter to quit, Esc to resume");
  listHUD.push_back(label);

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Really quit?");
  listHUD.push_back(label);

  initNavigation(listHUD, 1, 1);
}

QuitMenu::~QuitMenu()
{
}

void QuitMenu::resize(int _width, int _height)
{
  HUDDialog::resize(_width, _height);

  // use a big font
  float fontSize = (float)_height / 15.0f;
  float smallFontSize = (float)_height / 54.0f;
  float x, y;
  FontManager &fm = FontManager::instance();
  const int fontFace = MainMenu::getFontFace();

  // heights
  const float fontHeight = fm.getStrHeight(fontFace, fontSize, " ");
  const float smallFontHeight = fm.getStrHeight(fontFace, smallFontSize, " ");

  // get stuff
  std::vector<HUDuiControl*>& listHUD = getControls();
  HUDuiLabel* label = (HUDuiLabel*)listHUD[0];

  // help message
  label->setFontSize(smallFontSize);
  const float stringWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)_width - stringWidth);
  y = (float)_height - fontHeight - 1.5f * smallFontHeight;
  label->setPosition(x, y);

  // quit message
  label = (HUDuiLabel*)listHUD[1];
  label->setFontSize(fontSize);
  const float labelWidth = fm.getStrLength(fontFace, fontSize, label->getString());
  x = 0.5f * ((float)_width - labelWidth);
  y = (float)_height - 3.5f * fontHeight;
  label->setPosition(x, y);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
