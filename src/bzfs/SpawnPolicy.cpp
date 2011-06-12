/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "game/BZDBCache.h"


extern int getCurMaxPlayers();


SpawnPolicy::SpawnPolicy() {
}

SpawnPolicy::~SpawnPolicy() {
}


bool SpawnPolicy::isFacing(const fvec3& selfPos, const fvec3& enemyPos,
                           const float enemyAzimuth, const float deviation) const {
  // vector points from test to enemy
  const fvec3 d = (enemyPos - selfPos);
  const float angActual = atan2f(d.y, d.x);
  float diff = fmodf(enemyAzimuth - angActual, (float)M_PI * 2.0f);

  // Ignore tanks that are above or below us
  if (fabs(d.z) > 2.0f * BZDBCache::tankHeight) {
    return false;
  }

  // now diff is between {-PI*2 and +PI*2}, and we're looking for values around
  // -PI or +PI, because that's when the enemy is facing the source.
  diff = fabsf(diff);  // between {0 and +PI*2}
  diff = fabsf((float)(diff - M_PI));

  if (diff < (deviation / 2.0f)) {
    return true;
  }
  else {
    return false;
  }
}


bool SpawnPolicy::isImminentlyDangerous(const fvec3& selfPos) const {
  const float tankRadius = BZDBCache::tankRadius;
  const float safeSWRadius = (float)((BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS) + BZDBCache::tankRadius) * BZDB.eval("_spawnSafeSWMod"));
  const float safeSRRadius = tankRadius * BZDB.eval("_spawnSafeSRMod");
  const float safeDistance = tankRadius * BZDB.eval("_spawnSafeRadMod");

  GameKeeper::Player* playerData;
  float twentyDegrees = (float)(M_PI / 9.0); /* +- 10 degrees, i.e. 20 degree arc */
  int curmax = getCurMaxPlayers();
  for (int i = 0; i < curmax; i++) {
    playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData) {
      continue;
    }
    if (playerData->player.isAlive()) {
      const fvec3& enemyPos   = playerData->currentPos;
      float        enemyAngle = playerData->currentRot;
      if (playerData->player.getFlag() >= 0) {
        // check for dangerous flags
        const FlagInfo* finfo = FlagInfo::get(playerData->player.getFlag());
        const FlagType* ftype = finfo->flag.type;
        // FIXME: any more?
        if (ftype == Flags::Laser) {  // don't spawn in the line of sight of an L
          if (isFacing(selfPos, enemyPos, enemyAngle, twentyDegrees)) { // he's looking within 20 degrees of spawn point
            return true;  // eek, don't spawn here
          }
        }
        else if (ftype == Flags::ShockWave) {    // don't spawn next to a SW
          if ((selfPos - enemyPos).length() < safeSWRadius) { // too close to SW
            return true;  // eek, don't spawn here
          }
        }
        else if (ftype == Flags::Steamroller || ftype == Flags::Burrow) {   // don't spawn if you'll squish or be squished
          if ((selfPos - enemyPos).length() < safeSRRadius) { // too close to SR or BU
            return true;  // eek, don't spawn here
          }
        }
      }
      // don't spawn in the line of sight of a normal-shot tank within a certain distance
      if ((selfPos - enemyPos).length() < safeDistance) { // within danger zone?
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
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
