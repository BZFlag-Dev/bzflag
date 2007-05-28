/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
  addControl(createLabel("Tim Riker", "Maintainer:"), false);
  addControl(createLabel("", ""), false);
  addControl(createLabel("Chris Schoeneman", "Original Author:"), false);
  addControl(createLabel("", ""), false);
  addControl(createLabel("David Hoeferlin, Tom Hubina", "Code Contributors:"), false);
  addControl(createLabel("Dan Kartch, Jed Lengyel", ""), false);
  addControl(createLabel("Jeff Myers, Tim Olson", ""), false);
  addControl(createLabel("Brian Smits, Greg Spencer", ""), false);
  addControl(createLabel("Daryll Strauss, Frank Thilo", ""), false);
  addControl(createLabel("Dave Brosius, David Trowbridge", ""), false);
  addControl(createLabel("Sean Morrison, Tupone Alfredo", ""), false);
  addControl(createLabel("Lars Luthman, Nils McCarthy", ""), false);
  addControl(createLabel("Daniel Remenak", ""), false);
  addControl(createLabel("", ""), false);
  addControl(createLabel("Tamar Cohen", "Tank Models:"), false);
  addControl(createLabel("", ""), false);
  addControl(createLabel("Kevin Novins, Rick Pasetto", "Special Thanks:"), false);
  addControl(createLabel("Adam Rosen, Erin Shaw", ""), false);
  addControl(createLabel("Ben Trumbore, Don Greenberg", ""), false);
  addControl(createLabel("", ""), false);
  addControl(createLabel("http://BZFlag.org/", "BZFlag Home Page:"), false);
  addControl(createLabel("", ""), false);
  addControl(createLabel("Tim Riker", "Copyright (c) 1993 - 2007"), false);
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
