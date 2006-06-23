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

HelpInstructionsMenu::HelpInstructionsMenu(const char* title, std::vector<std::string> text) 
  : HelpMenu(title)
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  
  std::vector<std::string>::iterator it;
  for (it = text.begin(); it != text.end(); ++it)
  {
    listHUD.push_back(createLabel((*it).c_str()));
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
