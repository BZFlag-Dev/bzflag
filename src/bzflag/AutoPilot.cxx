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

#include "TimeKeeper.h"
#include "Address.h"
#include "Roster.h"
#include "TargetingUtils.h"
#include "World.h"
#include "WorldPlayer.h"
#include "BZDBCache.h"
#include "ServerLink.h"
#include "playing.h"

typedef std::map<FlagType*, std::pair<int,int> > FlagSuccessMap;

static FlagSuccessMap flagSuccess;
static int			  totalSum = 0;
static int			  totalCnt = 0;

void teachAutoPilot(FlagType *type, int adjust)
{
	if (type == Flags::Null)
		return;

	FlagSuccessMap::iterator it = flagSuccess.find(type);
	if (it != flagSuccess.end()) {
		std::pair<int,int> &pr = it->second;
		pr.first += adjust;
		pr.second++;
	}
	else
		flagSuccess[type] = std::pair<int,int>(adjust,1);
	totalSum += adjust;
	totalCnt++;
}

bool isFlagUseful( FlagType *type )
{
	if (type == Flags::Null)
		return false;

	FlagSuccessMap::iterator it = flagSuccess.find( type );
	float flagValue;
	if (it != flagSuccess.end()) {
		std::pair<int,int> &pr = it->second;
		if (pr.second == 0)
		  flagValue = 0.0f;
		else
		  flagValue = (float)pr.first / (float)pr.second;
	}
	else
		return true;

	float avg;
	if (totalCnt == 0)
		avg = 0.0f;
	else
		avg = (float)totalSum/(float)totalCnt;
	return ((float)flagValue) >= avg;
}

float normalizeAngle(float ang)
{
  if (ang < -1.0f * M_PI) ang += 2.0f * M_PI;
  if (ang > 1.0f * M_PI) ang -= 2.0f * M_PI;
  return ang;
}

ShotPath *findWorstBullet(float &minDistance)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();
  ShotPath *minPath = NULL;

  minDistance = Infinity;
  for (int t = 0; t < curMaxPlayers; t++) {
    if (t == myTank->getId() || !player[t])
        continue;

    const int maxShots = player[t]->getMaxShots();
    for (int s = 0; s < maxShots; s++) {
      ShotPath* shot = player[t]->getShot(s);
      if (!shot || shot->isExpired())
        continue;
    
      if (shot->getFlag() == Flags::InvisibleBullet)
        continue; //Theoretically Roger could triangulate the sound

      const float* shotPos = shot->getPosition();
      if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) && (shot->getFlag() != Flags::GuidedMissile))
	continue;

      const float dist = TargetingUtils::getTargetDistance( pos, shotPos );
      if (dist < minDistance) {
	const float *shotVel = shot->getVelocity();
	float shotAngle = atan2f(shotVel[1],shotVel[0]);
	float shotUnitVec[2] = {cos(shotAngle), sin(shotAngle)};

	float trueVec[2] = {(pos[0]-shotPos[0])/dist,(pos[1]-shotPos[1])/dist};
	float dotProd = trueVec[0]*shotUnitVec[0]+trueVec[1]*shotUnitVec[1];

	if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
	  continue;

	minDistance = dist;
	minPath = shot;
      }
    }
  }

  WorldPlayer *wp = World::getWorld()->getWorldWeapons();
  for (int w = 0; w < wp->getMaxShots(); w++) {
    ShotPath* shot = wp->getShot(w);
    if (!shot || shot->isExpired())
      continue;

    if (shot->getFlag() == Flags::InvisibleBullet)
      continue; //Theoretically Roger could triangulate the sound

    const float* shotPos = shot->getPosition();
    if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) && (shot->getFlag() != Flags::GuidedMissile))
      continue;

    const float dist = TargetingUtils::getTargetDistance( pos, shotPos );
    if (dist < minDistance) {
      const float *shotVel = shot->getVelocity();
      float shotAngle = atan2f(shotVel[1],shotVel[0]);
      float shotUnitVec[2] = {cos(shotAngle), sin(shotAngle)};

      float trueVec[2] = {(pos[0]-shotPos[0])/dist,(pos[1]-shotPos[1])/dist};
      float dotProd = trueVec[0]*shotUnitVec[0]+trueVec[1]*shotUnitVec[1];

      if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
	continue;

      minDistance = dist;
      minPath = shot;
    }
  }
  return minPath;
}

bool	avoidBullet(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();

  if ((myTank->getFlag() == Flags::Narrow) || (myTank->getFlag() == Flags::Burrow))
    return false; // take our chances

  float minDistance;
  ShotPath *shot = findWorstBullet(minDistance);

  if ((shot == NULL) || (minDistance > 100.0f))
    return false;

  const float *shotPos = shot->getPosition();
  const float *shotVel = shot->getVelocity();
  float shotAngle = atan2f(shotVel[1],shotVel[0]);
  float shotUnitVec[2] = {cos(shotAngle), sin(shotAngle)};

  float trueVec[2] = {(pos[0]-shotPos[0])/minDistance,(pos[1]-shotPos[1])/minDistance};
  float dotProd = trueVec[0]*shotUnitVec[0]+trueVec[1]*shotUnitVec[1];

#ifdef _MSC_VER
  if (((World::getWorld()->allowJumping() || (myTank->getFlag()) == Flags::Jumping
   || (myTank->getFlag()) == Flags::Wings))
   && (minDistance < (max(dotProd,0.5f) * BZDB.eval(StateDatabase::BZDB_TANKLENGTH) * 2.25f))
   && (myTank->getFlag() != Flags::NoJumping)) {
#else
  if (((World::getWorld()->allowJumping() || (myTank->getFlag()) == Flags::Jumping
   || (myTank->getFlag()) == Flags::Wings))
   && (minDistance < (std::max(dotProd,0.5f) * BZDB.eval(StateDatabase::BZDB_TANKLENGTH) * 2.25f))
   && (myTank->getFlag() != Flags::NoJumping)) {
#endif
    myTank->jump();
    return (myTank->getFlag() != Flags::Wings);
  }
  else if (dotProd > 0.96f) {
    speed = 1.0;
    float myAzimuth = myTank->getAngle();
    float rotation1 = normalizeAngle((shotAngle + M_PI/2.0f) - myAzimuth);

    float rotation2 = normalizeAngle((shotAngle - M_PI/2.0f) - myAzimuth);

    float zCross = shotUnitVec[0]*trueVec[1] - shotUnitVec[1]*trueVec[0];

    if (zCross > 0.0f) { //if i am to the left of the shot from shooter pov
      rotation = rotation1;
      if (fabs(rotation1) < fabs(rotation2))
        speed = 1.0f;
      else if (dotProd > 0.98f)
        speed = -0.5f;
      else
        speed = 0.5f;
    }
    else {
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

bool	stuckOnWall(float &rotation, float &speed)
{
  TimeKeeper lastStuckTime;
  static float stuckRot = 0.0f, stuckSpeed = 0.0f;

  if ((TimeKeeper::getCurrent() - lastStuckTime) < 1.0f) {
    rotation = stuckRot;
    speed = stuckSpeed;
    return true;
  }

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();
  float myAzimuth = myTank->getAngle();

  const bool phased = myTank->getFlag() == Flags::OscillationOverthruster 
	              || ((myTank->getFlag() == Flags::PhantomZone) && myTank->isFlagActive());


  if (!phased && (TargetingUtils::getOpenDistance( pos, myAzimuth ) < 5.0f)) {
    lastStuckTime = TimeKeeper::getCurrent();
    if (bzfrand() > 0.8f) {
      // Every once in a while, do something nuts
      speed = (float)(bzfrand() * 1.5f - 0.5f);
      rotation = (float)(bzfrand() * 2.0f - 1.0f);
    }
    else {
      float leftDistance = TargetingUtils::getOpenDistance( pos, myAzimuth + (M_PI/4.0f));
      float rightDistance = TargetingUtils::getOpenDistance( pos, myAzimuth - (M_PI/4.0f));
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

RemotePlayer *findBestTarget()
{
  RemotePlayer *target = NULL;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();
  float myAzimuth = myTank->getAngle();
  float distance = Infinity;

  for (int t = 0; t < curMaxPlayers; t++) {
    if ((t != myTank->getId())
    &&  (player[t]) 
    &&  (player[t]->isAlive())
    &&  (!player[t]->isPaused())
    &&  (!player[t]->isNotResponding()) 
    &&  (myTank->validTeamTarget(player[t]))) {

      if((player[t]->getFlag() == Flags::PhantomZone && player[t]->isFlagActive()))
        continue;

      float d = TargetingUtils::getTargetDistance( pos, player[t]->getPosition());
      bool isObscured = TargetingUtils::isLocationObscured( pos, player[t]->getPosition());
      if (isObscured) //demote the priority of obscured enemies
        d *= 1.25f;

      if (d < distance) {
        if ((player[t]->getFlag() != Flags::Stealth)
	||  (myTank->getFlag() == Flags::Seer)
        ||  ((!isObscured) &&  
	     (TargetingUtils::getTargetAngleDifference(pos, myAzimuth, player[t]->getPosition()) <= 30.0f)))
	  target = player[t];
	  distance = d;
      }
    }
  }

  return target;
}

bool chasePlayer( float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();

  RemotePlayer *player = findBestTarget();
  if (player == NULL)
    return false;

  myTank->setTarget(player);

  const float *targetPos = player->getPosition();
  float distance = TargetingUtils::getTargetDistance(pos, targetPos);
  if (distance > 250.0f)
    return false;

  const float *tp = player->getPosition();
  float enemyPos[3];
  //toss in some lag adjustment/future prediction - 300 millis
  memcpy(enemyPos,tp,sizeof(enemyPos));
  const float *tv = player->getVelocity();
  enemyPos[0] += 0.3f * tv[0];
  enemyPos[1] += 0.3f * tv[1];
  enemyPos[2] += 0.3f * tv[2];
  if (enemyPos[2] < 0.0f) //Roger doesn't worry about burrow
    enemyPos[2] = 0.0;

  float myAzimuth = myTank->getAngle();
  float enemyAzimuth = TargetingUtils::getTargetAzimuth( pos, tp );
  rotation = TargetingUtils::getTargetRotation( myAzimuth, enemyAzimuth );

  //If we are driving relatively towards our target and a building pops up jump over it
  if (fabs(rotation) < BZDB.eval(StateDatabase::BZDB_LOCKONANGLE)) {
    const Obstacle *building = NULL;
    float d = distance - 5.0f; //Make sure building is REALLY in front of player (-5)

    float dir[3] = {cosf(myAzimuth), sinf(myAzimuth), 0.0f};
    Ray tankRay(pos, dir);
  
    building = ShotStrategy::getFirstBuilding(tankRay, -0.5f, d);
    if (building) {
      //If roger can drive around it, just do that
      
      float leftDistance = TargetingUtils::getOpenDistance( pos, myAzimuth + (M_PI/6.0f));
      if (leftDistance > (2.0f * d)) {
        speed = 0.5f;
	rotation = -0.5f;
	return true;
      }
      float rightDistance = TargetingUtils::getOpenDistance( pos, myAzimuth - (M_PI/6.0f));
      if (rightDistance > (2.0f * d)) {
        speed = 0.5f;
	rotation = 0.5f;
	return true;
      }

      //Never did good in math, he should really see if he can reach the building
      //based on jumpvel and gravity, but settles for assuming 20-50 is a good range
      if ((d > 20.0f) && (d < 50.0f) && (building->getType() == BoxBuilding::typeName)) {
        float jumpVel = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
        float maxJump = (jumpVel * jumpVel) / (2 * -BZDB.eval(StateDatabase::BZDB_GRAVITY));

        if (((building->getPosition()[2] - pos[2] + building->getHeight()) ) < maxJump) {
          speed = d / 50.0f;
          myTank->jump();
	  return true;
	}
      }
    }
  }

  // weave towards the player
  const Player *target = myTank->getTarget();
  if ((distance > (BZDB.eval(StateDatabase::BZDB_SHOTSPEED) /2.0f))
  ||  (myTank->getFiringStatus() != LocalPlayer::Ready)) {
    float enemyUnitVec[2] = { cos(enemyAzimuth), sin(enemyAzimuth) };
    float myUnitVec[2] = { cos(myAzimuth), sin(myAzimuth) };
    float dotProd = (myUnitVec[0]*enemyUnitVec[0] + myUnitVec[1]*enemyUnitVec[1]);
    if (dotProd < 0.866f) {
      //if target is more than 30 degrees away, turn as fast as you can
      rotation *= M_PI / (2.0f * fabs(rotation));
      speed = dotProd; //go forward inverse rel to how much you need to turn
    }
    else {
      int period = int(TimeKeeper::getTick().getSeconds());
      float absBias = M_PI/20.0f * (distance / 100.0f);
      float bias = ((period % 4) < 2) ? absBias : -absBias;
      rotation += bias;
      rotation = normalizeAngle(rotation);
      speed = 1.0;
    }
  }
  else if (target->getFlag() != Flags::Burrow) {
    speed = -0.5f;
    rotation *= M_PI / (2.0f * fabs(rotation));
  }

  return true;
}

bool lookForFlag( float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  float pos[3];
  
  memcpy( pos, myTank->getPosition(), sizeof( pos ));
  if (pos[2] < 0.0f)
	  pos[2] = 0.0f;
  World *world = World::getWorld();
  int closestFlag = -1;

  if ((myTank->getFlag() != Flags::Null)
  &&  (isFlagUseful(myTank->getFlag())))
    return false;

  float minDist = Infinity;
  for (int i = 0; i < numFlags; i++) {
    if ((world->getFlag(i).type == Flags::Null) 
     || (world->getFlag(i).status != FlagOnGround))
      continue;

    const float* fpos = world->getFlag(i).position;
    if (fpos[2] == pos[2]) {
      float dist = TargetingUtils::getTargetDistance( pos, fpos );
      bool isTargetObscured = TargetingUtils::isLocationObscured( pos, fpos );
      if (isTargetObscured)
	dist *= 1.25f;

      if ((dist < 200.0f) && (dist < minDist)) {
        minDist = dist;
        closestFlag = i;
      }
    }
  }

  if (closestFlag != -1) {
	if (minDist < 10.0f) {
	  if (myTank->getFlag() != Flags::Null) {
        serverLink->sendDropFlag(myTank->getPosition());
        handleFlagDropped(myTank);
	  }
	}

    const float *fpos = world->getFlag(closestFlag).position;
    float myAzimuth = myTank->getAngle();
    float flagAzimuth = TargetingUtils::getTargetAzimuth( pos, fpos );
    rotation = TargetingUtils::getTargetRotation( myAzimuth, flagAzimuth );
    speed = M_PI/2.0f - fabs(rotation);
    return true;
  }

  return false;
}

bool navigate( float &rotation, float &speed)
{
  static TimeKeeper lastNavChange;
  static float navRot = 0.0f, navSpeed = 0.0f;

  if ((TimeKeeper::getCurrent() - lastNavChange) < 1.0f) {
    rotation = navRot;
    speed = navSpeed;
    return true;
  }

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  float pos[3];
  
  memcpy( pos, myTank->getPosition(), sizeof( pos ));
  if (pos[2] < 0.0f)
	  pos[2] = 0.01f;
  float myAzimuth = myTank->getAngle();

  float leftDistance = TargetingUtils::getOpenDistance( pos, myAzimuth + (M_PI/4.0f));
  float centerDistance = TargetingUtils::getOpenDistance( pos, myAzimuth);
  float rightDistance = TargetingUtils::getOpenDistance( pos, myAzimuth - (M_PI/4.0f));
  if (leftDistance > rightDistance) {
    if (leftDistance > centerDistance)
      rotation = 0.75f;
    else
      rotation = 0.0f;
  }
  else {
    if (rightDistance > centerDistance)
      rotation = -0.75f;
    else
      rotation = 0.0f;
  }
  speed = 1.0f;
  navRot = rotation;
  navSpeed = speed;
  lastNavChange = TimeKeeper::getCurrent();
  return true;
}

bool fireAtTank()
{
  static TimeKeeper lastShot;
  float pos[3];
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  memcpy( pos, myTank->getPosition(), sizeof(pos));
  if (pos[2] < 0.0f)
	  pos[2] = 0.01f;
  float myAzimuth = myTank->getAngle();

  float dir[3] = {cosf(myAzimuth), sinf(myAzimuth), 0.0f};
  pos[2] += BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
  Ray tankRay(pos, dir);
  pos[2] -= BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);

  if (myTank->getFlag() == Flags::ShockWave) {
    TimeKeeper now = TimeKeeper::getTick();
    if (now - lastShot >= (1.0f / World::getWorld()->getMaxShots())) {
      bool hasSWTarget = false;
      for (int t = 0; t < curMaxPlayers; t++) {
        if (t != myTank->getId() && player[t] &&
	    player[t]->isAlive() && !player[t]->isPaused() &&
	    !player[t]->isNotResponding()) {

	  if ((player[t]->getFlag() == Flags::PhantomZone) 
	  &&  (player[t]->isFlagActive()))
	    continue;

	  const float *tp = player[t]->getPosition();
	  float enemyPos[3];
          //toss in some lag adjustment/future prediction - 300 millis
          memcpy(enemyPos,tp,sizeof(enemyPos));
          const float *tv = player[t]->getVelocity();
          enemyPos[0] += 0.3f * tv[0];
          enemyPos[1] += 0.3f * tv[1];
          enemyPos[2] += 0.3f * tv[2];
	  if (enemyPos[2] < 0.0f)
	    enemyPos[2] = 0.0f;

          float dist = TargetingUtils::getTargetDistance( pos, enemyPos );
          if (dist <= BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS)) {
            if (!myTank->validTeamTarget(player[t])) {
              hasSWTarget = false;
              t = curMaxPlayers;
	    }
	    else
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
  }
  else {
    TimeKeeper now = TimeKeeper::getTick();
    if (now - lastShot >= (1.0f / World::getWorld()->getMaxShots())) {

      float errorLimit = World::getWorld()->getMaxShots() * BZDB.eval(StateDatabase::BZDB_LOCKONANGLE) / 8.0f;
      float closeErrorLimit = errorLimit * 2.0f;

      for (int t = 0; t < curMaxPlayers; t++) {
        if (t != myTank->getId() && player[t] &&
	    player[t]->isAlive() && !player[t]->isPaused() &&
	    !player[t]->isNotResponding() &&
	    myTank->validTeamTarget(player[t])) {

	  if ((player[t]->getFlag() == Flags::PhantomZone) 
	  &&  (player[t]->isFlagActive()))
	    continue;

	  const float *tp = player[t]->getPosition();
	  float enemyPos[3];
          //toss in some lag adjustment/future prediction - 300 millis
          memcpy(enemyPos,tp,sizeof(enemyPos));
          const float *tv = player[t]->getVelocity();
          enemyPos[0] += 0.3f * tv[0];
          enemyPos[1] += 0.3f * tv[1];
          enemyPos[2] += 0.3f * tv[2];
	  if (enemyPos[2] < 0.0f)
	    enemyPos[2] = 0.0f;

	  float dist = TargetingUtils::getTargetDistance( pos, enemyPos );

	  if ((myTank->getFlag() == Flags::GuidedMissile) || (fabs(pos[2] - enemyPos[2]) < 2.0f * BZDBCache::tankHeight)) {

	    float targetDiff = TargetingUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos );

	    if ((targetDiff < errorLimit)
	    ||  ((dist < (2.0f * BZDB.eval(StateDatabase::BZDB_SHOTSPEED))) && (targetDiff < closeErrorLimit))) {
	      bool isTargetObscured;
	      if (myTank->getFlag() != Flags::SuperBullet)
	        isTargetObscured = TargetingUtils::isLocationObscured( pos, enemyPos );
	      else
	        isTargetObscured = false;

	      if (!isTargetObscured) {
	        myTank->fireShot();
	        lastShot = now;
	        t = curMaxPlayers;
		return true;
	      }
	    }
	  }
	}
      }
    }
  }

  return false;
}

void    dropHardFlags()
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  FlagType *type = myTank->getFlag();
  if ((type == Flags::Useless)
  ||  (type == Flags::MachineGun)
  ||  (type == Flags::PhantomZone)
  ||  (type == Flags::Identify)) {
    serverLink->sendDropFlag(myTank->getPosition());
    handleFlagDropped(myTank);
  }
}

void	doAutoPilot(float &rotation, float &speed)
{
  dropHardFlags(); //Perhaps we should remove this and learning do it's work
  if (!avoidBullet(rotation, speed)) {
    if (!stuckOnWall(rotation, speed)) {
      if (!chasePlayer(rotation, speed)) {
        if (!lookForFlag(rotation, speed)) {
	  navigate(rotation, speed);
	}
      }
    }
  }

  fireAtTank();
}
