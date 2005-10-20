/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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


BaseLocalPlayer::BaseLocalPlayer(const PlayerId& _id,
				 const char* name, const char* _email) :
  Player(_id, RogueTeam, name, _email, TankPlayer),
  lastTime(TimeKeeper::getTick()),
  salt(0)
{
  lastPosition[0] = 0.0f;
  lastPosition[1] = 0.0f;
  lastPosition[2] = 0.0f;
  bbox[0][0] = bbox[1][0] = 0.0f;
  bbox[0][1] = bbox[1][1] = 0.0f;
  bbox[0][2] = bbox[1][2] = 0.0f;
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

void BaseLocalPlayer::update()
{
  // save last position
  const float* oldPosition = getPosition();
  lastPosition[0] = oldPosition[0];
  lastPosition[1] = oldPosition[1];
  lastPosition[2] = oldPosition[2];

  // update by time step
  float dt = float(TimeKeeper::getTick() - lastTime);
  lastTime = TimeKeeper::getTick();
  if (dt < 0.001f) dt = 0.001f;
  doUpdateMotion(dt);

  // compute motion's bounding box around center of tank
  const float* newVelocity = getVelocity();
  bbox[0][0] = bbox[1][0] = oldPosition[0];
  bbox[0][1] = bbox[1][1] = oldPosition[1];
  bbox[0][2] = bbox[1][2] = oldPosition[2];
  if (newVelocity[0] > 0.0f)
    bbox[1][0] += dt * newVelocity[0];
  else
    bbox[0][0] += dt * newVelocity[0];
  if (newVelocity[1] > 0.0f)
    bbox[1][1] += dt * newVelocity[1];
  else
    bbox[0][1] += dt * newVelocity[1];
  if (newVelocity[2] > 0.0f)
    bbox[1][2] += dt * newVelocity[2];
  else
    bbox[0][2] += dt * newVelocity[2];

  // expand bounding box to include entire tank
  float size = BZDBCache::tankRadius;
  if (getFlag() == Flags::Obesity) size *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  else if (getFlag() == Flags::Tiny) size *= BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  else if (getFlag() == Flags::Thief) size *= BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  bbox[0][0] -= size;
  bbox[1][0] += size;
  bbox[0][1] -= size;
  bbox[1][1] += size;
  bbox[1][2] += BZDBCache::tankHeight;

  // do remaining update stuff
  doUpdate(dt);
}

Ray BaseLocalPlayer::getLastMotion() const
{
  return Ray(lastPosition, getVelocity());
}

const float (*BaseLocalPlayer::getLastMotionBBox() const)[3]
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
