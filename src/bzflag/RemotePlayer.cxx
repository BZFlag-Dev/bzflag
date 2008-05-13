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

/* interface header */
#include "RemotePlayer.h"

/* common implementation headers */
#include "World.h"
#include "SyncClock.h"
#include "playing.h"
#include "ShotList.h"


RemotePlayer::RemotePlayer(const PlayerId& _id, TeamColor _team,
			   const char* _name,
			   const PlayerType _type) :
  Player(_id, _team, _name, _type)
{
  if (World::getWorld()) 
  {
    for ( size_t i = 0; i < World::getWorld()->getMaxShots(); i++ )
      shotSlots.push_back(ShotSlot());
  }
 }

RemotePlayer::~RemotePlayer()
{
}

void			RemotePlayer::addShot(int id)
{
  Player::addShot(ShotList::instance().getShot(id));
}

void			RemotePlayer::died( void)
{
  Player::died();

  const float* pos = getPosition();
  float explodePos[3];
  explodePos[0] = pos[0];
  explodePos[1] = pos[1];
  explodePos[2] = pos[2] + getMuzzleHeight();
  addTankExplosion(explodePos);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
