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
#include "OpenGLTexFont.h"

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
  label->setFont(MainMenu::getFont());
  label->setString("Enter to quit, Esc to resume");
  list.push_back(label);

  label = new HUDuiLabel;
  label->setFont(MainMenu::getFont());
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
  float fontWidth = (float)height / 10.0f;
  float fontHeight = (float)height / 10.0f;
  float smallFontWidth = (float)height / 36.0f;
  float smallFontHeight = (float)height / 36.0f;
  float x, y;

  // get stuff
  std::vector<HUDuiControl*>& list = getControls();
  HUDuiLabel* label = (HUDuiLabel*)list[0];
  label->setFontSize(smallFontWidth, smallFontHeight);
  const OpenGLTexFont& font = label->getFont();
  smallFontWidth = label->getFont().getWidth();
  smallFontHeight = label->getFont().getHeight();

  // help message
  label = (HUDuiLabel*)list[0];
  const float stringWidth = font.getWidth(label->getString());
  x = 0.5f * ((float)width - stringWidth);
  y = (float)height - fontHeight - 1.5f * font.getHeight();
  label->setPosition(x, y);

  // quit message
  label = (HUDuiLabel*)list[1];
  label->setFontSize(fontWidth, fontHeight);
  fontWidth = label->getFont().getWidth();
  fontHeight = label->getFont().getHeight();
  const float labelWidth = label->getFont().getWidth(label->getString());
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
