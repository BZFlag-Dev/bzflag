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
#include "HelpInstructionsMenu.h"

/* system headers */
#include <vector>
#include <string>

/* local implementation headers */
#include "FontManager.h"
#include "HUDuiControl.h"
#include "HUDuiLabel.h"

HelpInstructionsMenu::HelpInstructionsMenu(const char* title, std::vector<std::string> text) 
  : HelpMenu(title)
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel(""));
  
  std::vector<std::string>::iterator it;
  for (it = text.begin(); it != text.end(); ++it)
  {
    listHUD.push_back(createLabel((*it).c_str()));
  }
}

void HelpInstructionsMenu::resize(int _width, int _height)
{
  HelpMenu::resize(_width, _height);

  // find good font size
  FontManager &fm = FontManager::instance();
  float fontSize = _height / 5.0f; // guaranteed to be too big
  const float workingWidth = _width - 2 * getLeftSide(_width, _height);

  // find the longest localized string
  float longestLength = 0;
  std::string longestString = "";
  std::vector<HUDuiControl*>& listHUD = getControls();
  int fontFace = listHUD[2]->getFontFace();
  const int count = (const int)listHUD.size();
  for (int i = 2; i < count; ++i) {
    float thisLength = fm.getStrLength(fontFace, fontSize, ((HUDuiLabel*)listHUD[i])->getString());
    if (thisLength > longestLength)
    {
      longestLength = thisLength;
      longestString = ((HUDuiLabel*)listHUD[i])->getString();
    }
  }

  // make the longest fit perfectly in the working width, and use that font size
  for (fontSize = ((float)_height / 5.0f); longestLength > workingWidth; --fontSize)
  {
    longestLength = fm.getStrLength(fontFace, fontSize, longestString);
  }

  // reposition instruction text
  const float h = fm.getStrHeight(fontFace, fontSize, " ");
  const float x = getLeftSide(_width, _height);
  float y = listHUD[2]->getY();
  for (int i = 2; i < count; ++i) {
    listHUD[i]->setFontSize(fontSize);
    listHUD[i]->setPosition(x, y);
    y -= 1.0f * h;
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
