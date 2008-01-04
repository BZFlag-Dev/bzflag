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
#include "SpawnPolicy.h"

/* common implementation headers */
#include "GameKeeper.h"
#include "FlagInfo.h"
#include "BZDBCache.h"


extern int getCurMaxPlayers();


SpawnPolicy::SpawnPolicy()
{
}

SpawnPolicy::~SpawnPolicy()
{
}


bool
SpawnPolicy::isFacing(const float *selfPos, const float *enemyPos, const float enemyAzimuth, const float deviation) const
{
  // vector points from test to enemy
  float dx = enemyPos[0] - selfPos[0];
  float dy = enemyPos[1] - selfPos[1];
  float dz = enemyPos[2] - selfPos[2];
  float angActual = atan2f (dy, dx);
  float diff = fmodf(enemyAzimuth - angActual, (float)M_PI * 2.0f);

  // Ignore tanks that are above or below us
  if (fabs(dz) > 2.0f * BZDBCache::tankHeight )
    return false;

  // now diff is between {-PI*2 and +PI*2}, and we're looking for values around
  // -PI or +PI, because that's when the enemy is facing the source.
  diff = fabsf (diff); // between {0 and +PI*2}
  diff = fabsf ((float)(diff - M_PI));

  if (diff < (deviation / 2.0f)) {
    return true;
  } else {
    return false;
  }
}


float
SpawnPolicy::distanceFrom(const float *pos, const float* farPos) const
{
  float dx = farPos[0] - pos[0];
  float dy = farPos[1] - pos[1];
  float dz = farPos[2] - pos[2];
  return (float)sqrt(dx*dx + dy*dy + dz*dz);
}


bool
SpawnPolicy::isImminentlyDangerous(const float *selfPos) const
{
  const float tankRadius = BZDBCache::tankRadius;
  const float safeSWRadius = (float)((BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS) + BZDBCache::tankRadius) * BZDB.eval("_spawnSafeSWMod"));
  const float safeSRRadius = tankRadius * BZDB.eval("_spawnSafeSRMod");
  const float safeDistance = tankRadius * BZDB.eval("_spawnSafeRadMod");

  GameKeeper::Player *playerData;
  float twentyDegrees = (float)(M_PI / 9.0); /* +- 10 degrees, i.e. 20 degree arc */
  int curmax = getCurMaxPlayers();
  for (int i = 0; i < curmax; i++) {
    playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    if (playerData->player.isAlive()) {
      float *enemyPos   = playerData->currentPos;
      float  enemyAngle = playerData->currentRot;
      if (playerData->player.getFlag() >= 0) {
	// check for dangerous flags
	const FlagInfo *finfo = FlagInfo::get(playerData->player.getFlag());
	const FlagType *ftype = finfo->flag.type;
	// FIXME: any more?
	if (ftype == Flags::Laser) {  // don't spawn in the line of sight of an L
	  if (isFacing(selfPos, enemyPos, enemyAngle, twentyDegrees)) { // he's looking within 20 degrees of spawn point
	    return true;	// eek, don't spawn here
	  }
	} else if (ftype == Flags::ShockWave) {  // don't spawn next to a SW
	  if (distanceFrom(selfPos, enemyPos) < safeSWRadius) { // too close to SW
	    return true;	// eek, don't spawn here
	  }
	} else if (ftype == Flags::Steamroller || ftype == Flags::Burrow) { // don't spawn if you'll squish or be squished
	  if (distanceFrom(selfPos, enemyPos) < safeSRRadius) { // too close to SR or BU
	    return true;	// eek, don't spawn here
	  }
	}
      }
      // don't spawn in the line of sight of a normal-shot tank within a certain distance
      if (distanceFrom(selfPos, enemyPos) < safeDistance) { // within danger zone?
	if (isFacing(selfPos, enemyPos, enemyAngle, twentyDegrees)) { //and he's looking at me
	  return true;
	}
      }
    }
  }

  // TODO: should check world weapons also

  return false;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
