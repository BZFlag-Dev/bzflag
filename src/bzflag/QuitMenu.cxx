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
#include "QuitMenu.h"

/* system implementation headers */
#include <vector>

/* common implementation headers */
#include "FontManager.h"

/* local implementation headers */
#include "MainMenu.h"


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
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label;

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Enter to quit, Esc to resume");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFontFace(MainMenu::getFontFace());
  label->setString("Really quit?");
  list.push_back(label);

  initNavigation(list, 1, 1);
}

QuitMenu::~QuitMenu()
{
}

void QuitMenu::resize(int width, int height)
{
  HUDDialog::resize(width, height);

  // use a big font
  float fontSize = (float)height / 15.0f;
  float smallFontSize = (float)height / 54.0f;
  float x, y;
  FontManager &fm = FontManager::instance();
  const int fontFace = MainMenu::getFontFace();

  // heights
  const float fontHeight = fm.getStrHeight(fontFace, fontSize, " ");
  const float smallFontHeight = fm.getStrHeight(fontFace, smallFontSize, " ");

  // get stuff
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label = (HUDuiLabel*)list[0];

  // help message
  label->setFontSize(smallFontSize);
  const float stringWidth = fm.getStrLength(fontFace, smallFontSize, label->getString());
  x = 0.5f * ((float)width - stringWidth);
  y = (float)height - fontHeight - 1.5f * smallFontHeight;
  label->setPosition(x, y);

  // quit message
  label = (HUDuiLabel*)list[1];
  label->setFontSize(fontSize);
  const float labelWidth = fm.getStrLength(fontFace, fontSize, label->getString());
  x = 0.5f * ((float)width - labelWidth);
  y = (float)height - 3.5f * fontHeight;
  label->setPosition(x, y);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
