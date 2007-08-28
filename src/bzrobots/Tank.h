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

#ifndef BZROBOTS_TANK_H
#define BZROBOTS_TANK_H

#include "RemotePlayer.h"
#include "RCMessage.h"

#include <ostream>

struct Tank {
  Tank();
  Tank(RemotePlayer *tank);

  messageParseStatus parse(char **arguments, int count);

  std::string callsign, team, flag;
  bool paused, alive, frozen, super;

  float position[3], angle;
  float velocity[3], angularVelocity;
};

std::ostream& operator<<(std::ostream& os, const Tank& tank);

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
