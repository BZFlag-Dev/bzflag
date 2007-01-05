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

/* interface header */
#include "RemotePlayer.h"

/* common implementation headers */
#include "World.h"


RemotePlayer::RemotePlayer(const PlayerId& _id, TeamColor _team,
			   const char* _name, const char* _email,
			   const PlayerType _type) :
  Player(_id, _team, _name, _email, _type)
{
  if (World::getWorld()) {
    numShots = World::getWorld()->getMaxShots();
  } else {
    numShots = 0;
  }
  shots.resize(numShots);
  for (int i = 0; i < numShots; i++) {
    shots[i] = NULL;
  }
}

RemotePlayer::~RemotePlayer()
{
}

void			RemotePlayer::addShot(FiringInfo& info)
{
  prepareShotInfo(info);
  Player::addShot(new RemoteShotPath(info), info);
}

bool			RemotePlayer::doEndShot(
				int ident, bool isHit, float* pos)
{
  const int index = ident & 255;
  const int salt = (ident >> 8) & 127;

  // special id used in some messages (and really shouldn't be sent here)
  if (ident == -1)
    return false;

  // ignore bogus shots (those with a bad index or for shots that don't exist)
  if (index < 0 || index >= numShots || !shots[index])
    return false;

  // ignore shots that already ending
  if (shots[index]->isExpired() || shots[index]->isExpiring())
    return false;

  // ignore shots that have the wrong salt.  since we reuse shot indices
  // it's possible for an old MsgShotEnd to arrive after we've started a
  // new shot.  that's where the salt comes in.  it changes for each shot
  // so we can identify an old shot from a new one.
  if (salt != ((shots[index]->getShotId() >> 8) & 127))
    return false;

  // keep statistics
  shotStatistics.recordHit(shots[index]->getFlag());

  // don't stop if it's because were hitting something and we don't stop
  // when we hit something.
  if (isHit && !shots[index]->isStoppedByHit())
    return false;

  // end it
  const float* shotPos = shots[index]->getPosition();
  pos[0] = shotPos[0];
  pos[1] = shotPos[1];
  pos[2] = shotPos[2];
  shots[index]->setExpired();
  return true;
}

void			RemotePlayer::updateShots(float dt)
{
  for (int i = 0; i < numShots; i++)
    if (shots[i])
      shots[i]->update(dt);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

