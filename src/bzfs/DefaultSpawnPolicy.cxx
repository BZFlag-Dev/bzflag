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
#include "DefaultSpawnPolicy.h"

/* common headers */
#include "GameKeeper.h"
#include "TimeKeeper.h"
#include "PlayerInfo.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

/* server headers */
#include "bzfs.h"
#include "DropGeometry.h"


DefaultSpawnPolicy::DefaultSpawnPolicy()
{
  testPos[0] = testPos[1] = testPos[2] = 0.0f;
}

DefaultSpawnPolicy::~DefaultSpawnPolicy()
{
}

void DefaultSpawnPolicy::getPosition(float pos[3], int playerId, bool onGroundOnly, bool notNearEdges)
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
  team = pi.getTeam();

  if (!BZDB.isTrue("freeCtfSpawns") &&
      playerData->player.shouldRestartAtBase() &&
      (team >= RedTeam) && (team <= PurpleTeam) &&
      (bases.find(team) != bases.end())) {

    /* if the player needs to spawn on a base, select a random
     * position on one of their team's available bases.
     */

    TeamBases &teamBases = bases[team];
    const TeamBase &base = teamBases.getRandomBase((int)(bzfrand() * 100));
    base.getRandomPosition(pos[0], pos[1], pos[2]);
    playerData->player.setRestartOnBase(false);

  } else {
    /* *** "random" spawn position selection occurs below here. ***
     *
     * The idea is to basically find a position that is the farthest
     * away as possible from your enemies within a given timeframe.
     * Since the random selection is occurring over a square that is
     * slightly smaller than 0.6 times the world size, this usually
     * results in players spawning near one of the four corners of a
     * map.
     *
     * TODO: this should be a circle instead of square to prevent such
     * predictibility and tendency to select those potentially
     * dangerous corners.
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
	  testPos[0] = ((float)bzfrand() - 0.5f) * size * 0.6f;
	  testPos[1] = ((float)bzfrand() - 0.5f) * size * 0.6f;
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
	    logDebugMessage(1,"Warning: DefaultSpawnPolicy ran out of time, just dropping the sucker in\n");
	  }
	  break;
	}
      }

      // check if spot is safe enough
      bool dangerous = isImminentlyDangerous(testPos);
      if (foundspot && !dangerous) {
	float enemyAngle;
	float dist = enemyProximityCheck(enemyAngle);
	if (dist > bestDist) { // best so far
	  bestDist = dist;
	  pos[0] = testPos[0];
	  pos[1] = testPos[1];
	  pos[2] = testPos[2];
	}
	if (bestDist < minProximity) { // not good enough, keep looking
	  foundspot = false;
	  minProximity *= 0.99f; // relax requirements a little
	}
      } else if (dangerous) {
	foundspot = false;
      }
    }
  }
}

void DefaultSpawnPolicy::getAzimuth(float &azimuth)
{
  bool dangerous = isImminentlyDangerous(testPos);
  if (dangerous) {
    float enemyAngle;
    (void)enemyProximityCheck(enemyAngle);
    azimuth = fmod((float)(enemyAngle + M_PI), (float)(2.0 * M_PI));
  } else {
    azimuth = (float)(bzfrand() * 2.0 * M_PI);
  }
}


float DefaultSpawnPolicy::enemyProximityCheck(float &enemyAngle) const
{
  GameKeeper::Player *playerData;
  float worstDist = 1e12f; // huge number
  bool noEnemy    = true;
  int curmax = getCurMaxPlayers();
  for (int i = 0; i < curmax; i++) {
    playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    if (playerData->player.isAlive()
	&& areFoes(playerData->player.getTeam(), team)) {
      float *enemyPos = playerData->currentPos;
      if (fabs(enemyPos[2] - testPos[2]) < 1.0f) {
	float x = enemyPos[0] - testPos[0];
	float y = enemyPos[1] - testPos[1];
	float distSq = x * x + y * y;
	if (distSq < worstDist) {
	  worstDist  = distSq;
	  enemyAngle = playerData->currentRot;
	  noEnemy    = false;
	}
      }
    }
  }
  if (noEnemy)
    enemyAngle = (float)(bzfrand() * 2.0 * M_PI);
  return sqrtf(worstDist);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
