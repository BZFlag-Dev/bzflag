/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "Tank.h"

/* local implementation headers */
#include "MessageUtilities.h"
#include "Team.h"
#include "Flag.h"


std::ostream& operator<<(std::ostream& os, const Tank& tank)
{
  return os << tank.callsign << " " << tank.team << " "
    << tank.paused << " " << tank.alive << " " << tank.frozen << " " << tank.super << " "
    << tank.team << " " << tank.position[0] << " " << tank.position[1] << " " << tank.position[2] << " "
    << tank.angle << " " << tank.velocity[0] << " " << tank.velocity[1] << " " << tank.velocity[2] << " "
    << tank.angularVelocity;
}


Tank::Tank() {}

Tank::Tank(RemotePlayer *tank) :
  callsign(tank->getCallSign()),
  team(Team::getShortName(tank->getTeam())),
  flag(tank->getFlag()->flagName),
  paused(tank->isPaused()),
  alive(tank->isAlive()),
  frozen(tank->canMove()),
  super(tank->isFlagActive()),
  angle(tank->getAngle()*180.0/M_PI),
  angularVelocity(tank->getAngularVelocity())
{
  const float *pos = tank->getPosition();
  position[0] = pos[0];
  position[1] = pos[1];
  position[2] = pos[2];

  const float *vel = tank->getVelocity();
  velocity[0] = vel[0];
  velocity[1] = vel[1];
  velocity[2] = vel[2];
}

messageParseStatus Tank::parse(char **arguments, int count)
{
  if (count != 15)
    return InvalidArgumentCount;

  if (!MessageUtilities::parse(arguments[0], callsign))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[1], team))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[2], paused))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[3], alive))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[4], frozen))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[5], super))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[6], flag))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[7], position[0]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[8], position[1]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[9], position[2]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[10], angle))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[11], velocity[0]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[12], velocity[1]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[13], velocity[2]))
      return InvalidArguments;
  if (!MessageUtilities::parse(arguments[14], angularVelocity))
      return InvalidArguments;

  return ParseOk;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
