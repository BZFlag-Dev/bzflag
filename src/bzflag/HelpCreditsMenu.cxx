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
#include "HelpCreditsMenu.h"

/* system headers */
#include <vector>

/* local implementation headers */
#include "HUDuiControl.h"

HelpCreditsMenu::HelpCreditsMenu() : HelpMenu("Credits")
{
  // add controls
  std::vector<HUDuiControl*>& listHUD = getControls();
  listHUD.push_back(createLabel("Tim Riker", "Maintainer:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Chris Schoeneman", "Original Author:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("David Hoeferlin, Tom Hubina", "Code Contributors:"));
  listHUD.push_back(createLabel("Dan Kartch, Jed Lengyel", ""));
  listHUD.push_back(createLabel("Jeff Myers, Tim Olson", ""));
  listHUD.push_back(createLabel("Brian Smits, Greg Spencer", ""));
  listHUD.push_back(createLabel("Daryll Strauss, Frank Thilo", ""));
  listHUD.push_back(createLabel("Dave Brosius, David Trowbridge", ""));
  listHUD.push_back(createLabel("Sean Morrison, Tupone Alfredo", ""));
  listHUD.push_back(createLabel("Lars Luthman, Nils McCarthy", ""));
  listHUD.push_back(createLabel("Daniel Remenak", ""));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Tamar Cohen", "Tank Models:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Kevin Novins, Rick Pasetto", "Special Thanks:"));
  listHUD.push_back(createLabel("Adam Rosen, Erin Shaw", ""));
  listHUD.push_back(createLabel("Ben Trumbore, Don Greenberg", ""));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("http://BZFlag.org/", "BZFlag Home Page:"));
  listHUD.push_back(createLabel("", ""));
  listHUD.push_back(createLabel("Tim Riker", "Copyright (c) 1993 - 2006"));
}

float HelpCreditsMenu::getLeftSide(int _width, int _height)
{
  return 0.5f * _width - _height / 20.0f;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
