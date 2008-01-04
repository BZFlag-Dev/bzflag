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
#include "DangerousSpawnPolicy.h"

/* common headers */
#include "GameKeeper.h"
#include "TimeKeeper.h"
#include "PlayerInfo.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

/* server headers */
#include "bzfs.h"
#include "DropGeometry.h"


DangerousSpawnPolicy::DangerousSpawnPolicy()
{
}

DangerousSpawnPolicy::~DangerousSpawnPolicy()
{
}

void DangerousSpawnPolicy::getPosition(float pos[3], int playerId, bool onGroundOnly, bool notNearEdges)
{
  /* the player is coming to life, depending on who they are an what
   * style map/configuration is being played determines how they will
   * spawn.
   */

  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerId);
  if (!playerData)
    return;

  const PlayerInfo& pi = playerData->player;
  TeamColor t = pi.getTeam();

  if (!BZDB.isTrue("freeCtfSpawns") &&
      playerData->player.shouldRestartAtBase() &&
      (t >= RedTeam) && (t <= PurpleTeam) &&
      (bases.find(t) != bases.end())) {

    /* if the player needs to spawn on a base, select a random
     * position on one of their team's available bases.
     */

    TeamBases &teamBases = bases[t];
    const TeamBase &base = teamBases.getRandomBase((int)(bzfrand() * 100));
    base.getRandomPosition(pos[0], pos[1], pos[2]);
    playerData->player.setRestartOnBase(false);

  } else {
    /* *** "dangerous" spawn position selection occurs below here. ***
     *
     * Basic idea is to try to pick a dangerous spawn location,
     * hopefully one that is imminently dangerous.  if not, hopefuly
     * one that is just close to an enemy.  otherwise, just toss them
     * in.
     */

    const float tankRadius = BZDBCache::tankRadius;
    const float size = BZDBCache::worldSize;
    const float maxHeight = world->getMaxWorldHeight();

    // keep track of how much time we spend searching for a location
    TimeKeeper start = TimeKeeper::getCurrent();

    int tries = 0;
    float minProximity = size / BZDB.eval("_spawnSafeSRMod");
    float bestDist = -1.0f;
    bool foundspot = false;
    while (!foundspot) {
      if (!world->getPlayerSpawnPoint(&pi, testPos)) {
	if (notNearEdges) {
	  // don't spawn close to map edges in CTF mode
	  testPos[0] = ((float)bzfrand() - 0.5f) * size * 0.5f;
	  testPos[1] = ((float)bzfrand() - 0.5f) * size * 0.5f;
	} else {
	  testPos[0] = ((float)bzfrand() - 0.5f) * (size - 2.0f * tankRadius);
	  testPos[1] = ((float)bzfrand() - 0.5f) * (size - 2.0f * tankRadius);
	}
	testPos[2] = onGroundOnly ? 0.0f : ((float)bzfrand() * maxHeight);
      }
      tries++;

      const float waterLevel = world->getWaterLevel();
      float minZ = 0.0f;
      if (waterLevel > minZ) {
	minZ = waterLevel;
      }
      float maxZ = maxHeight;
      if (onGroundOnly) {
	maxZ = 0.0f;
      }

      if (DropGeometry::dropPlayer(testPos, minZ, maxZ)) {
	foundspot = true;
      }

      // check every now and then if we have already used up 10ms of time
      if (tries >= 50) {
	tries = 0;
	if (TimeKeeper::getCurrent() - start > BZDB.eval("_spawnMaxCompTime")) {
	  if (bestDist < 0.0f) { // haven't found a single spot
	    //Just drop the sucka in, and pray
	    pos[0] = testPos[0];
	    pos[1] = testPos[1];
	    pos[2] = maxHeight;
	    logDebugMessage(1,"Warning: DangerousSpawnPolicy ran out of time, just dropping the sucker in\n");
	  }
	  break;
	}
      }

      // check if spot is dangerous enough
      bool dangerous = isImminentlyDangerous(testPos);
      if (foundspot && !dangerous) {
	float enemyAngle;
	float dist = enemyProximityCheck(enemyAngle);
	if (dist < bestDist) { // best so far
	  bestDist = dist;
	  pos[0] = testPos[0];
	  pos[1] = testPos[1];
	  pos[2] = testPos[2];
	}
	if (bestDist < minProximity) { // close enough, stop looking
	  foundspot = true;
	}
	minProximity *= 1.01f; // relax requirements a little
      } else if (dangerous) {
	foundspot = true;
      }
    }
  }
}

void DangerousSpawnPolicy::getAzimuth(float &azimuth)
{
  azimuth = (float)(bzfrand() * 2.0 * M_PI);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
