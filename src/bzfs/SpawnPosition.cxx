/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// object that creates and contains a spawn position

#include "common.h"

#include "SpawnPosition.h"
#include "FlagInfo.h"
#include "TeamBases.h"
#include "WorldInfo.h"
#include "PlayerInfo.h"
#include "PlayerState.h"

// FIXME: from bzfs.cxx
extern int getCurMaxPlayers();
extern bool areFoes(TeamColor team1, TeamColor team2);
extern BasesList bases;
extern WorldInfo *world;
extern PlayerInfo player[];
extern PlayerState lastState[];

SpawnPosition::SpawnPosition(int playerId, bool onGroundOnly, bool notNearEdges) :
		curMaxPlayers(getCurMaxPlayers())
{
  team = player[playerId].getTeam();
  azimuth = (float)bzfrand() * 2.0f * M_PI;

  if (player[playerId].shouldRestartAtBase() &&
      (team >= RedTeam) && (team <= PurpleTeam) && 
      (bases.find(team) != bases.end())) {
    TeamBases &teamBases = bases[team];
    const TeamBase &base = teamBases.getRandomBase((int)(bzfrand() * 100));
    base.getRandomPosition(pos[0], pos[1], pos[2]);
    player[playerId].setRestartOnBase(false);
  } else {
    const float tankHeight = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
    const float tankRadius = BZDB.eval(StateDatabase::BZDB_TANKRADIUS);
    safeSWRadius = (float)((BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS) + BZDB.eval(StateDatabase::BZDB_TANKRADIUS)) * 1.5);
    safeDistance = tankRadius * 20; // FIXME: is this a good value?
    const float size = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
    const float maxWorldHeight = world->getMaxWorldHeight();
    ObstacleLocation *building;

    // keep track of how much time we spend searching for a location
    TimeKeeper start = TimeKeeper::getCurrent();

    int inAirAttempts = 50;
    int tries = 0;
    float minProximity = size / 3.0f;
    float bestDist = -1.0f;
    float testPos[3];
    bool foundspot = false;
    while (!foundspot) {
      if (!world->getZonePoint(std::string(Team::getName(team)), testPos)) {
        if (notNearEdges) {
          // don't spawn close to map edges in CTF mode
          testPos[0] = ((float)bzfrand() - 0.5f) * size * 0.6f;
          testPos[1] = ((float)bzfrand() - 0.5f) * size * 0.6f;
	} else {
          testPos[0] = ((float)bzfrand() - 0.5f) * (size - 2.0f * tankRadius);
          testPos[1] = ((float)bzfrand() - 0.5f) * (size - 2.0f * tankRadius);
	}
        testPos[2] = onGroundOnly ? 0.0f : ((float)bzfrand() * maxWorldHeight);
      }
      tries++;

      int type = world->inBuilding(&building, testPos[0], testPos[1], testPos[2],
                                   tankRadius, tankHeight);

      if (onGroundOnly) {
        if (type == NOT_IN_BUILDING)
          foundspot = true;
      } else {
        if ((type == NOT_IN_BUILDING) && (testPos[2] > 0.0f)) {
          testPos[2] = 0.0f;
          //Find any intersection regardless of z
          type = world->inBuilding(&building, testPos[0], testPos[1], testPos[2],
                                   tankRadius, maxWorldHeight);
        }

        // in a building? try climbing on roof until on top
        int lastType = type;
	int retriesRemaining = 100; // don't climb forever
        while (type != NOT_IN_BUILDING) {
          testPos[2] = building->pos[2] + building->size[2] + 0.0001f;
          tries++;
          lastType = type;
          type = world->inBuilding(&building, testPos[0], testPos[1], testPos[2],
                                   tankRadius, tankHeight);
	  if (--retriesRemaining <= 0) {
	    DEBUG1("Warning: getSpawnLocation had to climb too many buildings\n");
	    break;
	  }
        }
        // ok, when not on top of pyramid or teleporter
        if (lastType != IN_PYRAMID  &&  lastType != IN_TELEPORTER) {
          foundspot = true;
        }
        // only try up in the sky so many times
        if (--inAirAttempts <= 0) {
          onGroundOnly = true;
        }
      }

      // check every now and then if we have already used up 10ms of time
      if (tries >= 50) {
        tries = 0;
        if (TimeKeeper::getCurrent() - start > 0.01f) {
          if (bestDist < 0.0f) { // haven't found a single spot
            //Just drop the sucka in, and pray
            pos[0] = testPos[0];
            pos[1] = testPos[1];
            pos[2] = maxWorldHeight;
	    DEBUG1("Warning: getSpawnLocation ran out of time, just dropping the sucker in\n");
          }
          break;
        }
      }

      // check if spot is safe enough
      if (foundspot && !isImminentlyDangerous()) {
	float enemyAngle;
	float dist = enemyProximityCheck(enemyAngle);
	if (dist > bestDist) { // best so far
	  bestDist = dist;
	  pos[0] = testPos[0];
	  pos[1] = testPos[1];
	  pos[2] = testPos[2];
	  azimuth = fmod((enemyAngle + M_PI), 2.0f * M_PI);
	}
	if (bestDist < minProximity) { // not good enough, keep looking
	  foundspot = false;
	  minProximity *= 0.99f; // relax requirements a little
	}
      }
    }
    delete building;
  }
}

SpawnPosition::~SpawnPosition()
{
}

const bool SpawnPosition::isFacing(const float *enemyPos, const float enemyAzimuth, 
				   const float deviation) const
{
  // determine angle from source to dest
  // (X) using only x & y, resultant will be z rotation
  float dx = testPos[0] - enemyPos[0];
  float dy = testPos[1] - enemyPos[1];
  float angActual;
  if (dx == 0) {
    // avoid divide by zero error
    angActual = (float)tan(dy / (1 / 1e12f));
  } else {
    angActual = (float)tan(dy / dx);
  }

  // see if our heading angle is within the bounds set by deviation
  // (X) only compare to z-rotation since that's all we're using
  if (((angActual + deviation / 2) > enemyAzimuth) &&
      ((angActual - deviation / 2) < enemyAzimuth)) {
    return true;
  } else {
    return false;
  }
}

const bool SpawnPosition::isImminentlyDangerous() const
{
  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].isAlive() && areFoes(player[i].getTeam(), team)) {
      const FlagInfo *finfo =&flag[player[i].getFlag()];
      const FlagType *ftype = finfo->flag.type;
      float *enemyPos = lastState[i].pos;
      float enemyAngle = lastState[i].azimuth;
      // check for dangerous flags, etc
      // FIXME: any more?
      if (ftype == Flags::Laser) {  // don't spawn in the line of sight of an L
	if (isFacing(enemyPos, enemyAngle, M_PI / 9)) { // he's looking within 20 degrees of spawn point
	  return true;	// eek, don't spawn here
	}
      } else if (ftype == Flags::ShockWave) {  // don't spawn next to a SW
	if (distanceFrom(enemyPos) < safeSWRadius) { // too close to SW
	  return true;	// eek, don't spawn here
	}
      }
      // don't spawn in the line of sight of a normal-shot tank within a certain distance
      if (distanceFrom(enemyPos) < safeDistance) { // within danger zone?
	if (isFacing(enemyPos, enemyAngle, M_PI / 9)) { //and he's looking at me
	  return true;
	}
      }
    }
  }

  // TODO: should check world weapons also

  return false;
}

const float SpawnPosition::enemyProximityCheck(float &enemyAngle) const
{
  float worstDist = 1e12f; // huge number
  bool  noEnemy   = true;

  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].isAlive() && areFoes(player[i].getTeam(), team)) {
      float *enemyPos = lastState[i].pos;
      if (fabs(enemyPos[2] - testPos[2]) < 1.0f) {
        float x = enemyPos[0] - testPos[0];
        float y = enemyPos[1] - testPos[1];
        float distSq = x * x + y * y;
        if (distSq < worstDist) {
          worstDist = distSq;
	  enemyAngle = lastState[i].azimuth;
	  noEnemy    = false;
	}
      }
    }
  }
  if (noEnemy)
    enemyAngle = (float)bzfrand() * 2.0f * M_PI;
  return sqrtf(worstDist);
}

const float SpawnPosition::distanceFrom(const float* farPos) const
{
  float dx = farPos[0] - testPos[0];
  float dy = farPos[1] - testPos[1];
  float dz = farPos[2] - testPos[2];
  return (float)sqrt(dx*dx + dy*dy + dz*dz);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
