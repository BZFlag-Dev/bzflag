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

/* interface header */
#include "common.h"
#include "BaseLocalPlayer.h"

/* common implementation headers */
#include "BZDBCache.h"


BaseLocalPlayer::BaseLocalPlayer(const PlayerId& id,
				 const char* name, const char* email) :
  Player(id, RogueTeam, name, email, TankPlayer),
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
  float dt = TimeKeeper::getTick() - lastTime;
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

#if 0
// BEGIN MASSIVE_NASTY_COMMENT_BLOCK 

// This massive nasty comment block is for client-side spawning!
//
// local update utility functions
//

static float minSafeRange(float angleCosOffBoresight)
{
  // anything farther than this much from dead-center is okay to
  // place at MinRange
  static const float	SafeAngle = 0.5f;	// cos(angle)

  const float shotSpeed = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);
  // don't ever place within this range
  const float	MinRange = 2.0f * shotSpeed;	// meters

  // anything beyond this range is okay at any angle
  const float	MaxRange = 4.0f * shotSpeed;	// meters

  // if more than SafeAngle off boresight then MinRange is okay
  if (angleCosOffBoresight < SafeAngle) return MinRange;

  // ramp up to MaxRange as target comes to dead center
  const float f = (angleCosOffBoresight - SafeAngle) / (1.0f - SafeAngle);
  return (float)(MinRange + f * (MaxRange - MinRange));
}


void BaseLocalPlayer::startingLocation
(float bestStartPoint[3],
 float &startAzimuth,
 World *world,
 Player *player[],
 int curMaxPlayers)
{
  // check for valid starting (no unfair advantage to player or enemies)
  // should find a good location in a few tries... locateCount is a safety
  // check that will probably be invoked when restarting on the team base
  // if the enemy is loitering around waiting for players to reappear.
  // also have to make sure new position isn't in a building;  that must
  // be enforced no matter how many times we need to try new locations.
  // If I can't find a safe spot, try to use the best of the unsafe ones.
  // The best one is that which violates the minimum safe distance by the
  // smallest amount.

  // maximum tries to find a safe place
  static const int	MaxTries = 1000;

  // minimum time before an existing shot can hit us
  static const float	MinShotImpact = 2.0f;		// seconds

  float bestDist = -1e6;
  bool located;
  int locateCount = 0;
  float startPoint[3];
  startPoint[2] = 0.0f;
  float tankRadius = BZDBCache::tankRadius;
  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  do {
    do {
      if (restartOnBase) {
	const float* base = world->getBase(int(getTeam()));
	const float x = (base[4] - 2.0f * tankRadius)
	  * ((float)bzfrand() - 0.5f);
	const float y = (base[5] - 2.0f * tankRadius)
	  * ((float)bzfrand() - 0.5f);
	startPoint[0] = base[0] + x * cosf(base[3]) - y * sinf(base[3]);
	startPoint[1] = base[1] + x * sinf(base[3]) + y * cosf(base[3]);
	startPoint[2] = base[2];
	if(startPoint[2] != 0)
	  startPoint[2]++;
      }
      else {
	if (world->allowTeamFlags()) {
	  startPoint[0] = 0.4f * worldSize * ((float)bzfrand() - 0.5f);
	  startPoint[1] = 0.4f * worldSize * ((float)bzfrand() - 0.5f);
	}
	else {
	  startPoint[0] = (worldSize - 2.0f * tankRadius)
	    * ((float)bzfrand() - 0.5f);
	  startPoint[1] = (worldSize - 2.0f * tankRadius)
	    * ((float)bzfrand() - 0.5f);
	}
      }
      startAzimuth = 2.0f * M_PI * (float)bzfrand();
    } while (world->inBuilding(startPoint, 2.0f * tankRadius));

    // use first point as best point, so we'll have a fallback
    if (locateCount == 0) {
      bestStartPoint[0] = startPoint[0];
      bestStartPoint[1] = startPoint[1];
      bestStartPoint[2] = startPoint[2];
    }

    // get info on my tank
    const TeamColor myColor = getTeam();
    const float myCos = cosf(-startAzimuth);
    const float mySin = sinf(-startAzimuth);

    // check each enemy tank
    located = true;
    float worstDist = 1e6;
    for (int i = 0; i < curMaxPlayers; i++) {
      // ignore missing player
      if (!player[i]) continue;

      // test against all existing shots of all players except mine
      // (mine don't count because I can't come alive before all my
      // shots have expired anyway)
      const int maxShots = World::getWorld()->getMaxShots();
      float tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
      float tankWidth = BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
      for (int j = 0; j < maxShots; j++) {
	// get shot and ignore non-existent ones
	ShotPath* shot = player[i]->getShot(j);
	if (!shot) continue;

	// get shot's current position and velocity and see if it'll
	// hit my tank earlier than MinShotImpact.  use something
	// larger than the actual tank size to give some leeway.
	const Ray ray(shot->getPosition(), shot->getVelocity());
	const float t = timeRayHitsBlock(ray, startPoint, startAzimuth,
					 4.0f * tankLength, 4.0f * tankWidth,
					 2.0f * BZDBCache::tankHeight);
	if (t >= 0.0f && t < MinShotImpact) {
	  located = false;
	  break;
	}
      }
      if (!located) break;

      // test against living enemy tanks
      if (!player[i]->isAlive() ||
	  (myColor != RogueTeam  && player[i]->getTeam() == myColor)) continue;

      // compute enemy position in my local coordinate system
      const float* enemyPos = player[i]->getPosition();
      const float enemyX = myCos * (enemyPos[0] - startPoint[0]) -
	mySin * (enemyPos[1] - startPoint[1]);
      const float enemyY = mySin * (enemyPos[0] - startPoint[0]) +
	myCos * (enemyPos[1] - startPoint[1]);

      // get distance and angle of enemy from boresight
      const float enemyDist = hypotf(enemyX, enemyY);
      const float enemyCos = enemyX / enemyDist;

      // don't allow tank placement if enemy tank is +/- 30 degrees of
      // my boresight and in firing range (our unfair advantage)
      float safeDist = enemyDist - minSafeRange(enemyCos);
      if (safeDist < worstDist)
	worstDist = safeDist;

      // compute my position in enemy coordinate system
      // cos = enemyUnitVect[0], sin = enemyUnitVect[1]
      const float* enemyUnitVect = player[i]->getForward();
      const float myX = enemyUnitVect[0] * (startPoint[0] - enemyPos[0]) -
	enemyUnitVect[1] * (startPoint[1] - enemyPos[1]);
      const float myY = enemyUnitVect[1] * (startPoint[0] - enemyPos[0]) +
	enemyUnitVect[0] * (startPoint[1] - enemyPos[1]);

      // get distance and angle of enemy from boresight
      const float myDist = hypotf(myX, myY);
      const float myCos = myX / myDist;

      // don't allow tank placement if my tank is +/- 30 degrees of
      // the enemy's boresight and in firing range (enemy's unfair advantage)
      safeDist = myDist - minSafeRange(myCos);
      if (safeDist < worstDist)
	worstDist = safeDist;
    }
    if (located && worstDist > bestDist) {
      bestDist = worstDist;
      bestStartPoint[0] = startPoint[0];
      bestStartPoint[1] = startPoint[1];
      bestStartPoint[2] = startPoint[2];
    }
    if (bestDist < 0.0f)
      located = false;
  } while (!located && ++locateCount <= MaxTries);
}

//END MASSIVE_NASTY_COMMENT_BLOCK
#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
