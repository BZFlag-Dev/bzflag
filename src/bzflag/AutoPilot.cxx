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

// get our interface
#include "AutoPilot.h"

/* common headers */
#include "BZDBCache.h"
#include "BoxBuilding.h"
#include "MeshFace.h"

/* local headers */
#include "Roster.h"
#include "TargetingUtils.h"
#include "World.h"
#include "WorldPlayer.h"
#include "playing.h"
#include "Plan.h"

typedef std::map<FlagType*, std::pair<int,int> > FlagSuccessMap;

static FlagSuccessMap	flagSuccess;
static int		totalSum = 0;
static int		totalCnt = 0;
static bool		wantJump = false;


void teachAutoPilot(FlagType *type, int adjust)
{
  if (type == Flags::Null)
    return;

  FlagSuccessMap::iterator it = flagSuccess.find(type);
  if (it != flagSuccess.end()) {
    std::pair<int,int> &pr = it->second;
    pr.first += adjust;
    pr.second++;
  } else {
    flagSuccess[type] = std::pair<int,int>(adjust,1);
  }
  totalSum += adjust;
  totalCnt++;
}

static bool isFlagUseful(FlagType *type)
{
  if (type == Flags::Null)
    return false;

  FlagSuccessMap::iterator it = flagSuccess.find(type);
  float flagValue;
  if (it != flagSuccess.end()) {
    std::pair<int,int> &pr = it->second;
    if (pr.second == 0)
      flagValue = 0.0f;
    else
      flagValue = (float)pr.first / (float)pr.second;
  } else {
    return true;
  }

  float avg;
  if (totalCnt == 0)
    avg = 0.0f;
  else
    avg = (float)totalSum / (float)totalCnt;
  return ((float)flagValue) >= avg;
}


static ShotPath *findWorstBullet(float &minDistance)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const fvec3& pos = myTank->getPosition();
  ShotPath *minPath = NULL;

  minDistance = Infinity;
  for (int t = 0; t < curMaxPlayers; t++) {
    if (t == myTank->getId() || !remotePlayers[t])
      continue;

    const int maxShots = remotePlayers[t]->getMaxShots();
    for (int s = 0; s < maxShots; s++) {
      ShotPath* shot = remotePlayers[t]->getShot(s);
      if (!shot || shot->isExpired())
	continue;

      if ((shot->getShotType() == InvisibleShot || shot->getShotType() == CloakedShot) &&
	  (myTank->getFlag() != Flags::Seer))
	continue; //Theoretically Roger could triangulate the sound
      if (remotePlayers[t]->isPhantomZoned() && !myTank->isPhantomZoned())
	continue;
      if ((shot->getShotType() == LaserShot) &&
	  (myTank->getFlag() == Flags::Cloaking))
	continue; //cloaked tanks can't die from lasers

      const fvec3& shotPos = shot->getPosition();
      if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) &&
	  (shot->getShotType() != GMShot))
	continue;

      const float dist = TargetingUtils::getTargetDistance(pos, shotPos);
      if (dist < minDistance) {
	const fvec3& shotVel = shot->getVelocity();
	float shotAngle = atan2f(shotVel[1], shotVel[0]);
	float shotUnitVec[2] = {cosf(shotAngle), sinf(shotAngle)};

	float trueVec[2] = { (pos[0] - shotPos[0]) / dist, (pos[1] - shotPos[1]) / dist };
	float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

	if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
	  continue;

	minDistance = dist;
	minPath = shot;
      }
    }
  }

  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }

  float oldDistance = minDistance;
  WorldPlayer *wp = world->getWorldWeapons();
  for (int w = 0; w < wp->getMaxShots(); w++) {
    ShotPath* shot = wp->getShot(w);
    if (!shot || shot->isExpired())
      continue;

    if ((shot->getShotType() == InvisibleShot || shot->getShotType() == CloakedShot) && myTank->getFlag() != Flags::Seer)
      continue; //Theoretically Roger could triangulate the sound
    if (shot->getShotType() == LaserShot && myTank->getFlag() == Flags::Cloaking)
      continue; //cloaked tanks can't die from lasers

    const fvec3& shotPos = shot->getPosition();
    if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) && (shot->getShotType() != GMShot))
      continue;

    const float dist = TargetingUtils::getTargetDistance( pos, shotPos );
    if (dist < minDistance) {
      const fvec3& shotVel = shot->getVelocity();
      float shotAngle = atan2f(shotVel[1], shotVel[0]);
      float shotUnitVec[2] = {cosf(shotAngle), sinf(shotAngle)};

      float trueVec[2] = { (pos[0] - shotPos[0]) / dist, (pos[1] - shotPos[1]) / dist };
      float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

      if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
	continue;

      minDistance = dist;
      minPath = shot;
    }
  }
  if (oldDistance < minDistance)
    minDistance = oldDistance; //pick the closer bullet
  return minPath;
}


static bool avoidDeathFall(float &/*rotation*/, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  fvec3 pos1 = myTank->getPosition();
  fvec3 pos2 = pos1;
  pos1[2] += 10.0f * BZDBCache::tankHeight;
  float azimuth = myTank->getAngle();
  if (speed < 0.0f)
    azimuth = fmodf(float(azimuth + M_PI), float(2.0 * M_PI));
  else
    azimuth = fmodf(float(azimuth), float(2.0 * M_PI));

  pos2[0] += 8.0f * BZDBCache::tankHeight * cosf(azimuth);
  pos2[1] += 8.0f * BZDBCache::tankHeight * sinf(azimuth);
  pos2[2] += 0.01f;

  World *world = World::getWorld();
  if (!world) {
    return false;
  }

  fvec3 collisionPt;
  if (TargetingUtils::getFirstCollisionPoint( pos1, pos2, collisionPt )) {
    if (collisionPt[2] < 0.0f)
      collisionPt[2] = 0.0f;
    if (collisionPt[2] < world->getWaterLevel()) {
      speed = 0.0f;
      return true;
    }
  } else if (collisionPt[2] < (pos2[2] - 1.0f)) {
    speed *= 0.5f;
  }

  return false;
}


static bool avoidBullet(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    return false;
  }

  const fvec3& pos = myTank->getPosition();

  if ((myTank->getFlag() == Flags::Narrow) || (myTank->getFlag() == Flags::Burrow))
    return false; // take our chances

  float minDistance;
  ShotPath *shot = findWorstBullet(minDistance);

  if ((shot == NULL) || (minDistance > 100.0f))
    return false;

  const fvec3& shotPos = shot->getPosition();
  const fvec3& shotVel = shot->getVelocity();
  float shotAngle = atan2f(shotVel[1],shotVel[0]);
  float shotUnitVec[2] = {cosf(shotAngle), sinf(shotAngle)};

  float trueVec[2] = {(pos[0]-shotPos[0])/minDistance,(pos[1]-shotPos[1])/minDistance};
  float dotProd = trueVec[0]*shotUnitVec[0]+trueVec[1]*shotUnitVec[1];

  World *world = World::getWorld();
  if (!world) {
    return false;
  }

  if ((myTank->canJump())
      && (minDistance < (std::max(dotProd, 0.5f) * BZDBCache::tankLength * 2.25f))) {
    wantJump = true;
    return (myTank->getFlag() != Flags::Wings);
  } else if (dotProd > 0.96f) {
    speed = 1.0;
    float myAzimuth = myTank->getAngle();
    float rotation1 = TargetingUtils::normalizeAngle((float)((shotAngle + M_PI/2.0) - myAzimuth));

    float rotation2 = TargetingUtils::normalizeAngle((float)((shotAngle - M_PI/2.0) - myAzimuth));

    float zCross = shotUnitVec[0]*trueVec[1] - shotUnitVec[1]*trueVec[0];

    if (zCross > 0.0f) { //if i am to the left of the shot from shooter pov
      rotation = rotation1;
      if (fabs(rotation1) < fabs(rotation2))
	speed = 1.0f;
      else if (dotProd > 0.98f)
	speed = -0.5f;
      else
	speed = 0.5f;
    } else {
      rotation = rotation2;
      if (fabs(rotation2) < fabs(rotation1))
	speed = 1.0f;
      else if (dotProd > 0.98f)
	speed = -0.5f;
      else
	speed = 0.5f;
    }
    return true;
  }
  return false;
}


static bool stuckOnWall(float &rotation, float &speed)
{
  static TimeKeeper lastStuckTime;
  static float stuckRot = 0.0f, stuckSpeed = 0.0f;

  float stuckPeriod = float(TimeKeeper::getTick() - lastStuckTime);
  if (stuckPeriod < 0.5f) {
    rotation = stuckRot;
    speed = stuckSpeed;
    return true;
  } else if (stuckPeriod < 1.0f) {
    rotation = stuckRot;
    speed = 1.0;
    return true;
  }

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const fvec3& pos = myTank->getPosition();
  float myAzimuth = myTank->getAngle();

  const bool phased = (myTank->getFlag() == Flags::OscillationOverthruster)
    || myTank->isPhantomZoned();

  if (!phased && (TargetingUtils::getOpenDistance(pos, myAzimuth) < 5.0f)) {
    lastStuckTime = TimeKeeper::getTick();
    if (bzfrand() > 0.8f) {
      // Every once in a while, do something nuts
      speed = (float)(bzfrand() * 1.5f - 0.5f);
      rotation = (float)(bzfrand() * 2.0f - 1.0f);
    } else {
      float leftDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth + (M_PI/4.0)));
      float rightDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth - (M_PI/4.0)));
      if (leftDistance > rightDistance)
	rotation = 1.0f;
      else
	rotation = -1.0f;
      speed = -0.5f;
    }
    stuckRot = rotation;
    stuckSpeed = speed;
    return true;
  }
  return false;
}


static RemotePlayer *findBestTarget()
{
  RemotePlayer *target = NULL;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const fvec3& pos = myTank->getPosition();
  float myAzimuth = myTank->getAngle();
  float distance = Infinity;

  for (int t = 0; t < curMaxPlayers; t++) {
    if ((t != myTank->getId())
	&&  (remotePlayers[t])
	&&  (remotePlayers[t]->isAlive())
	&&  (!remotePlayers[t]->isPaused())
	&&  (!remotePlayers[t]->isNotResponding())
	&&  (myTank->validTeamTarget(remotePlayers[t]))) {

      if (remotePlayers[t]->isPhantomZoned() && !myTank->isPhantomZoned()
	  && (myTank->getFlag() != Flags::ShockWave)
	  && (myTank->getFlag() != Flags::SuperBullet))
	continue;

      if ((remotePlayers[t]->getFlag() == Flags::Cloaking) &&
	  (myTank->getFlag() == Flags::Laser))
	continue;

      World *world = World::getWorld();
      if (!world) {
	return NULL;
      }

      //perform a draft that has us chase the proposed opponent if they have our flag
      if (world->allowTeamFlags() &&
	  ((myTank->getTeam() == RedTeam && remotePlayers[t]->getFlag() == Flags::RedTeam) ||
	   (myTank->getTeam() == GreenTeam && remotePlayers[t]->getFlag() == Flags::GreenTeam) ||
	   (myTank->getTeam() == BlueTeam && remotePlayers[t]->getFlag() == Flags::BlueTeam) ||
	   (myTank->getTeam() == PurpleTeam && remotePlayers[t]->getFlag() == Flags::PurpleTeam))) {
	target = remotePlayers[t];
	break;
      }

      float d = TargetingUtils::getTargetDistance(pos, remotePlayers[t]->getPosition());
      bool isObscured = TargetingUtils::isLocationObscured( pos, remotePlayers[t]->getPosition());
      if (isObscured) //demote the priority of obscured enemies
	d *= 1.25f;

      if (d < distance) {
	if ((remotePlayers[t]->getFlag() != Flags::Stealth)
	    || (myTank->getFlag() == Flags::Seer)
	    || ((!isObscured) &&
		 (TargetingUtils::getTargetAngleDifference(pos, myAzimuth, remotePlayers[t]->getPosition()) <= 30.0f))) {
	  target = remotePlayers[t];
	  distance = d;
	}
      }
    }
  }

  return target;
}


static bool chasePlayer(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const fvec3& pos = myTank->getPosition();

  RemotePlayer *rPlayer = findBestTarget();
  if (rPlayer == NULL)
    return false;

  myTank->setTarget(rPlayer);

  const fvec3& targetPos = rPlayer->getPosition();
  float distance = TargetingUtils::getTargetDistance(pos, targetPos);
  if (distance > 250.0f)
    return false;

  const fvec3& tp = rPlayer->getPosition();
  fvec3 enemyPos;
  //toss in some lag adjustment/future prediction - 300 millis
  enemyPos = tp;
  const fvec3& tv = rPlayer->getVelocity();
  enemyPos[0] += 0.3f * tv[0];
  enemyPos[1] += 0.3f * tv[1];
  enemyPos[2] += 0.3f * tv[2];
  if (enemyPos[2] < 0.0f) //Roger doesn't worry about burrow
    enemyPos[2] = 0.0;

  float myAzimuth = myTank->getAngle();
  float enemyAzimuth = TargetingUtils::getTargetAzimuth( pos, tp );
  rotation = TargetingUtils::getTargetRotation( myAzimuth, enemyAzimuth );

  //If we are driving relatively towards our target and a building pops up jump over it
  if (fabs(rotation) < BZDB.eval(BZDBNAMES.LOCKONANGLE)) {
    const Obstacle *building = NULL;
    float d = distance - 5.0f; //Make sure building is REALLY in front of player (-5)

    fvec3 dir(cosf(myAzimuth), sinf(myAzimuth), 0.0f);
    Ray tankRay(pos, dir);

    building = ShotStrategy::getFirstBuilding(tankRay, -0.5f, d);
    if (building && !myTank->isPhantomZoned() &&
	(myTank->getFlag() != Flags::OscillationOverthruster)) {
      //If roger can drive around it, just do that

      float leftDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth + (M_PI/6.0)));
      if (leftDistance > (2.0f * d)) {
	speed = 0.5f;
	rotation = -0.5f;
	return true;
      }
      float rightDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth - (M_PI/6.0)));
      if (rightDistance > (2.0f * d)) {
	speed = 0.5f;
	rotation = 0.5f;
	return true;
      }

      //Never did good in math, he should really see if he can reach the building
      //based on jumpvel and gravity, but settles for assuming 20-50 is a good range
      if ((d > 20.0f) && (d < 50.0f) &&
	  (building->getTypeID() == boxType)) {
	float jumpVel = BZDB.eval(BZDBNAMES.JUMPVELOCITY);
	float maxJump = (jumpVel * jumpVel) / (2 * -BZDBCache::gravity);

	if (((building->getPosition()[2] - pos[2] + building->getHeight())) < maxJump) {
	  speed = d / 50.0f;
	  wantJump = true;
	  return true;
	}
      }
    }
  }

  // weave towards the player
  const Player *target = myTank->getTarget();
  if ((distance > (BZDB.eval(BZDBNAMES.SHOTSPEED) /2.0f))
      ||  (myTank->getFiringStatus() != LocalPlayer::Ready)) {
    float enemyUnitVec[2] = { cosf(enemyAzimuth), sinf(enemyAzimuth) };
    float myUnitVec[2] = { cosf(myAzimuth), sinf(myAzimuth) };
    float dotProd = (myUnitVec[0] * enemyUnitVec[0] + myUnitVec[1] * enemyUnitVec[1]);
    if (dotProd < 0.866f) {
      //if target is more than 30 degrees away, turn as fast as you can
      rotation *= (float)M_PI / (2.0f * fabs(rotation));
      speed = dotProd; //go forward inverse rel to how much you need to turn
    } else {
      int period = int(TimeKeeper::getTick().getSeconds());
      float absBias = (float)(M_PI/20.0 * (distance / 100.0));
      float bias = ((period % 4) < 2) ? absBias : -absBias;
      rotation += bias;
      rotation = TargetingUtils::normalizeAngle(rotation);
      speed = 1.0;
    }
  } else if (target->getFlag() != Flags::Burrow) {
    speed = -0.5f;
    rotation *= (float)(M_PI / (2.0 * fabs(rotation)));
  }

  return true;
}


static bool lookForFlag(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    return false;
  }

  fvec3 pos;
  int closestFlag = -1;

  pos = myTank->getPosition();
  if (pos[2] < 0.0f) {
    pos[2] = 0.0f;
  }

  World *world = World::getWorld();
  if (!world) {
    return false;
  }

  if ((myTank->getFlag() != Flags::Null)
      && (isFlagUseful(myTank->getFlag()))) {
    return false;
  }

  // if I don't have a base, don't try to pick up a team flag
  bool takeTeamFlag = true;
  if (world->getBase(myTank->getTeam()) == NULL)
    takeTeamFlag = false;

  float minDist = Infinity;
  int teamFlag = -1;
  for (int i = 0; i < numFlags; i++) {
    if ((world->getFlag(i).type == Flags::Null)
	|| (world->getFlag(i).status != FlagOnGround))
      continue;

    if (world->getFlag(i).type->flagTeam != NoTeam) {
      if (takeTeamFlag)
	teamFlag = i;
      else
	continue;
    }
    const fvec3& fpos = world->getFlag(i).position;
    if (fpos[2] == pos[2]) {
      float dist = TargetingUtils::getTargetDistance(pos, fpos);
      bool isTargetObscured = TargetingUtils::isLocationObscured(pos, fpos);
      if (isTargetObscured)
	dist *= 1.25f;

      if ((dist < 200.0f) && (dist < minDist)) {
	minDist = dist;
	closestFlag = i;
      }
    }
  }

  if (teamFlag != -1 && (minDist < 10.0f || closestFlag == -1))
    closestFlag = teamFlag; //FIXME: should a team flag be more significant than a closer flag?
  if (closestFlag != -1) {
    if (minDist < 10.0f) {
      if (myTank->getFlag() != Flags::Null) {
	serverLink->sendDropFlag(myTank->getPosition());
	handleFlagDropped(myTank);
      }
    }

    const fvec3& fpos = world->getFlag(closestFlag).position;
    float myAzimuth = myTank->getAngle();
    float flagAzimuth = TargetingUtils::getTargetAzimuth(pos, fpos);
    rotation = TargetingUtils::getTargetRotation(myAzimuth, flagAzimuth);
    speed = (float)(M_PI/2.0 - fabs(rotation));
    return true;
  }

  return false;
}


static bool navigate(float &rotation, float &speed)
{
  static TimeKeeper lastNavChange;
  static float navRot = 0.0f, navSpeed = 0.0f;
  static const float baseEpsilon = 1.0f;

  if ((TimeKeeper::getTick() - lastNavChange) < 1.0f) {
    rotation = navRot;
    speed = navSpeed;
    return true;
  }

  fvec3 pos;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    return false;
  }

  pos = myTank->getPosition();
  if (pos[2] < 0.0f)
    pos[2] = 0.01f;
  float myAzimuth = myTank->getAngle();

  float leftDistance = TargetingUtils::getOpenDistance(pos, (float)(myAzimuth + (M_PI/4.0)));
  float centerDistance = TargetingUtils::getOpenDistance(pos, myAzimuth);
  float rightDistance = TargetingUtils::getOpenDistance(pos, (float)(myAzimuth - (M_PI/4.0)));
  if (leftDistance > rightDistance) {
    if (leftDistance > centerDistance)
      rotation = 0.75f;
    else
      rotation = 0.0f;
  } else {
    if (rightDistance > centerDistance)
      rotation = -0.75f;
    else
      rotation = 0.0f;
  }
  if (myTank->getFlag()->flagTeam != NoTeam) {
    World *world = World::getWorld();
    if (!world) {
      return false;
    }

    // FIXME: team flag handling does not account for Z distance
    const Obstacle* base = world->getBase(myTank->getTeam());
    if (base == NULL) {
      // I don't have a team base (rogue maybe) - don't want the flag
      serverLink->sendDropFlag(myTank->getPosition());
      handleFlagDropped(myTank);
    }
    else {
      const fvec3& basePos = base->getPosition();
      const float baseDist = (basePos.xy() - pos.xy()).length();
      if ((baseDist < baseEpsilon) &&
          (myTank->getFlag()->flagTeam == myTank->getTeam())) {
	// I am near the center of my team base, drop the flag
	serverLink->sendDropFlag(myTank->getPosition());
	handleFlagDropped(myTank);
      } else {
	// Move towards my base
	float baseAzimuth = TargetingUtils::getTargetAzimuth(pos, basePos);
	rotation = TargetingUtils::getTargetRotation(myAzimuth, baseAzimuth);
	speed = (float)(M_PI/2.0 - fabs(rotation));
      }
    }
  } else {
    speed = 1.0f;
  }
  if (myTank->getLocation() == LocalPlayer::InAir
      && myTank->getFlag() == Flags::Wings)
    wantJump = true;

  navRot = rotation;
  navSpeed = speed;
  lastNavChange = TimeKeeper::getTick();
  return true;
}


static bool fireAtTank()
{
  static TimeKeeper lastShot;
  fvec3 pos;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (!myTank) {
    return false;
  }

  pos = myTank->getPosition();
  if (pos[2] < 0.0f)
    pos[2] = 0.01f;
  float myAzimuth = myTank->getAngle();
  World *world = World::getWorld();
  if (!world) {
    return false;
  }

  fvec3 dir(cosf(myAzimuth), sinf(myAzimuth), 0.0f);
  pos[2] += myTank->getMuzzleHeight();
  Ray tankRay(pos, dir);
  pos[2] -= myTank->getMuzzleHeight();

  if (myTank->getFlag() == Flags::ShockWave) {
    TimeKeeper now = TimeKeeper::getTick();

    if (now - lastShot >= (1.0f / world->getMaxShots())) {
      bool hasSWTarget = false;

      for (int t = 0; t < curMaxPlayers; t++) {
	if (t == myTank->getId() ||
	    !remotePlayers[t] ||
	    !remotePlayers[t]->isAlive() ||
	    remotePlayers[t]->isPaused() ||
	    remotePlayers[t]->isNotResponding()) {
	  continue;
	}

	const fvec3& tp = remotePlayers[t]->getPosition();
	fvec3 enemyPos;
	//toss in some lag adjustment/future prediction - 300 millis
	enemyPos = tp;
	const fvec3& tv = remotePlayers[t]->getVelocity();
	enemyPos[0] += 0.3f * tv[0];
	enemyPos[1] += 0.3f * tv[1];
	enemyPos[2] += 0.3f * tv[2];
	if (enemyPos[2] < 0.0f)
	  enemyPos[2] = 0.0f;
	float dist = TargetingUtils::getTargetDistance( pos, enemyPos );
	if (dist <= BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS)) {
	  if (!myTank->validTeamTarget(remotePlayers[t])) {
	    hasSWTarget = false;
	    t = curMaxPlayers;
	  } else {
	    hasSWTarget = true;
	  }
	}
      }

      if (hasSWTarget) {
        myTank->fireShot();
        lastShot = TimeKeeper::getTick();
        return true;
      }
    }

    return false;
  }

  TimeKeeper now = TimeKeeper::getTick();
  if (now - lastShot >= (1.0f / world->getMaxShots())) {

    float errorLimit = world->getMaxShots() * BZDB.eval(BZDBNAMES.LOCKONANGLE) / 8.0f;
    float closeErrorLimit = errorLimit * 2.0f;

    for (int t = 0; t < curMaxPlayers; t++) {

      /* make sure it's a valid playing target */
      if (t == myTank->getId() ||
	  !remotePlayers[t] ||
	  !remotePlayers[t]->isAlive() ||
	  remotePlayers[t]->isPaused() ||
	  remotePlayers[t]->isNotResponding() ||
	  !myTank->validTeamTarget(remotePlayers[t]))
	continue;

      /* make sure they're worth going after */
      if (remotePlayers[t]->isPhantomZoned() && !myTank->isPhantomZoned()
	  && (myTank->getFlag() != Flags::SuperBullet)
	  && (myTank->getFlag() != Flags::ShockWave))
	continue;

      const fvec3& tp = remotePlayers[t]->getPosition();
      fvec3 enemyPos;

      //toss in some lag adjustment/future prediction - 300 millis
      enemyPos = tp;
      const fvec3& tv = remotePlayers[t]->getVelocity();
      enemyPos[0] += 0.3f * tv[0];
      enemyPos[1] += 0.3f * tv[1];
      enemyPos[2] += 0.3f * tv[2];
      if (enemyPos[2] < 0.0f)
	enemyPos[2] = 0.0f;

      float dist = TargetingUtils::getTargetDistance( pos, enemyPos );

      /* if I have GM or they're just really close but above/below me */
      if ((myTank->getFlag() == Flags::GuidedMissile) || (fabs(pos[2] - enemyPos[2]) < 2.0f * BZDBCache::tankHeight)) {

	/* are they within range? */
	float targetDiff = TargetingUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos );
	if ((targetDiff < errorLimit) ||
	    ((dist < (2.0f * BZDB.eval(BZDBNAMES.SHOTSPEED))) && (targetDiff < closeErrorLimit))) {

	  /* shoot at them if I can see them */
	  if ((myTank->getFlag() == Flags::SuperBullet) ||
	      !TargetingUtils::isLocationObscured( pos, enemyPos )) {
	    myTank->fireShot();
	    lastShot = now;
	    t = curMaxPlayers;
	    return true;
	  }
	}
      }
    }
  } /* damn... */

  return false;
}


static void dropHardFlags()
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  FlagType *type = myTank->getFlag();
  if ((type == Flags::Useless)
      || (type == Flags::MachineGun)
      || (type == Flags::Identify)
      || ((type == Flags::PhantomZone) && !myTank->isFlagActive())) {
    serverLink->sendDropFlag(myTank->getPosition());
    handleFlagDropped(myTank);
  }
}


void	doAutoPilot(float &rotation, float &speed)
{
  wantJump = false;

  dropHardFlags(); //Perhaps we should remove this and let learning do it's work

  if (!avoidBullet(rotation, speed) &&
      !stuckOnWall(rotation, speed) &&
      !chasePlayer(rotation, speed) &&
      !lookForFlag(rotation, speed)) {
    navigate(rotation, speed);
  }

  avoidDeathFall(rotation, speed);

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  myTank->setJumpPressed(wantJump);
  myTank->setJump();

  fireAtTank();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
