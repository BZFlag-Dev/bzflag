/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "callbacks.h"
#include "LocalPlayer.h"
#include "HUDRenderer.h"

extern LocalPlayer*	myTank;
extern HUDRenderer*	hud;

void setFlagHelp(const std::string& name, void*)
{ 
  static const float FlagHelpDuration = 60.0f;
  if (BZDB->isTrue(name))
    hud->setFlagHelp(myTank->getFlag(), FlagHelpDuration);
  else
    hud->setFlagHelp(Flags::Null, 0.0);
}

// ex: shiftwidth=2 tabstop=8
