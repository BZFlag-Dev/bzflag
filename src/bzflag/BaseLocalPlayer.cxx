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
#include "BaseLocalPlayer.h"

/* common implementation headers */
#include "BZDBCache.h"

#include "playing.h"

BaseLocalPlayer::BaseLocalPlayer(const PlayerId& _id,
				 const char* name,
				 const PlayerType _type)
: Player(_id, RogueTeam, name, _type)
, lastTime(TimeKeeper::getTick())
, lastPosition(0.0f, 0.0f, 0.0f)
, salt(0)
{
}


BaseLocalPlayer::~BaseLocalPlayer()
{
  // do nothing
}


int BaseLocalPlayer::getSalt()
{
  salt = (salt + 1) & 127;
  return salt << 8;
}


void BaseLocalPlayer::update( float inputDT )
{
  // save last position
  const fvec3 oldPosition = getPosition();
  lastPosition = oldPosition;

  // update by time step
  float dt = float(TimeKeeper::getTick() - lastTime);
  lastTime = TimeKeeper::getTick();

  if (inputDT > 0)
    dt = inputDT;

  if (dt < MIN_DT_LIMIT)
    dt = MIN_DT_LIMIT;

  float dtLimit = MAX_DT_LIMIT;
  float doneDT = dt;
  if (dt > dtLimit) {
    dt = dtLimit;
    doneDT -= dtLimit;
  }

  while (doneDT > 0) {
    doUpdateMotion(dt);

    // compute motion's bounding box around center of tank
    const fvec3 newVelocity = getVelocity();
    bbox.set(oldPosition, oldPosition);
    bbox.expandToPoint(oldPosition + (dt * newVelocity));

    // expand bounding box to include entire tank
    float size = BZDBCache::tankRadius;
    if (getFlag() == Flags::Obesity)
      size *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    else if (getFlag() == Flags::Tiny)
      size *= BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    else if (getFlag() == Flags::Thief)
      size *= BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);

    bbox.mins.x -= size;
    bbox.maxs.x += size;
    bbox.mins.y -= size;
    bbox.maxs.y += size;
    bbox.maxs.z += BZDBCache::tankHeight;

    // do remaining update stuff
    doUpdate(dt);

    // subtract another chunk
    doneDT -= dtLimit;

    // if we only have a nubby left, don't do a full dt.
    if (doneDT < dtLimit)
      dt = doneDT;
  }
}


Ray BaseLocalPlayer::getLastMotion() const
{
  return Ray(lastPosition, getVelocity());
}


const Extents& BaseLocalPlayer::getLastMotionBBox() const
{
  return bbox;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
