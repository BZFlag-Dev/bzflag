/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "BackendShot.h"

/* bzflag implementation headers */
#include "Roster.h"

/* local implementation headers */
#include "MessageUtilities.h"
#include "RCRequest.h"
#include "RCRequests.h"

ShotPath *searchShot(PlayerId plr, uint16_t shotid)
{
  if (plr >= curMaxPlayers) {
    return NULL;
  }

  for (int i = 0; i < remotePlayers[plr]->getMaxShots(); i++) {
    ShotPath *path = remotePlayers[plr]->getShot(i);
    if (!path || path->getFiringInfo().shot.id != shotid)
      continue;

    return path;
  }

  return NULL;
}

void BackendShot::getPosition(double &x, double &y, double &z, double dt) const
{
  ShotPath *path = searchShot(getPlayerId(), getShotId());

  if(path) {
    /*
     * We found the shot.
     */

    if (dt == 0) {
      const float *pos = path->getPosition();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    } else {
      //Make a copy of the ShotPath/ShotStrategy, run update(dt) and check the new position
      //TODO: Find a way to do this, we can easily copy ShotPath but to get ShotStrategy
      //we'd have to access a protected member function
      //
      //Now just return the current position...
      //

      const float *pos = path->getPosition();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    }
  }
}

void BackendShot::getVelocity(double &x, double &y, double &z, double dt) const
{
  ShotPath *path = searchShot(getPlayerId(), getShotId());

  if(path) {
    /*
     * We found the shot.
     */

    if (dt == 0) {
      const float *pos = path->getVelocity();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    } else {
      //Make a copy of the ShotPath/ShotStrategy, run update(dt) and check the new velocity
      //TODO: Find a way to do this, we can easily copy ShotPath but to get ShotStrategy
      //we'd have to access a protected member function
      //
      //Now just return the current velocity...
      //

      const float *pos = path->getVelocity();
      x = pos[0];
      y = pos[1];
      z = pos[2];
    }
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
