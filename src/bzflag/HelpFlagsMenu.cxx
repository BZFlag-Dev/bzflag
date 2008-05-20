/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include <string.h>

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

  qString.append(":");
  addControl(createLabel("", qString.c_str()), false);

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

    addControl(createLabel((*it)->flagHelp, (*it)->label().c_str()), false);
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
