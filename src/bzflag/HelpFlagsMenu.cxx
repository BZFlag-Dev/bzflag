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

/* interface headers */
#include "HelpMenu.h"
#include "HelpFlagsMenu.h"

/* system headers */
#include <vector>
#include <string>

/* local implementation headers */
#include "Flag.h"

HelpFlagsMenu::HelpFlagsMenu(FlagQuality quality)
{
  std::string qString;
  if (quality == FlagGood)
    qString = "Good Flags";
  else if (quality == FlagBad)
    qString = "Bad Flags";
  HelpMenu(qString.c_str());

  std::vector<HUDuiControl*>& listHUD = getControls();
  qString.append(":");
  listHUD.push_back(createLabel("", qString.c_str()));

  FlagSet fs;
  if (quality == FlagGood)
    fs = Flag::getGoodFlags();
  else if (quality == FlagBad)
    fs = Flag::getBadFlags();

  for (FlagSet::iterator it = fs.begin(); it != fs.end(); it++) {

    if (((*it)->flagQuality != quality) ||
	((*it)->flagTeam != NoTeam) ||
	(strcmp((*it)->flagName, "") == 0)) {
      continue;
    }

    listHUD.push_back(createLabel((*it)->flagHelp, (*it)->label().c_str()));
  }
}

float HelpFlagsMenu::getLeftSide(int _width, int)
{
  return 0.35f * _width;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
