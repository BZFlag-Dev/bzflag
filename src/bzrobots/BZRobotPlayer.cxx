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

// interface header
#include "BZRobotPlayer.h"

// common implementation headers
#include "BZDBCache.h"

// local implementation headers
#include "World.h"
#include "Intersect.h"
#include "TargetingUtils.h"


// event priority sorting
bool compareEventPriority(BZRobotEvent a, BZRobotEvent b)
{
  if(a.getPriority() < b.getPriority())
    return true;
  return false;
}


BZRobotPlayer::BZRobotPlayer(const PlayerId& _id,
			     const char* _name,
			     ServerLink* _server) :
  RobotPlayer(_id, _name, _server),
  lastExec(0.0f),
  robot(NULL),
  tsName(_name),
  tsGunHeat(0.0),
  tsShoot(false),
  tsSpeed(BZDBCache::tankSpeed),
  tsNextSpeed(BZDBCache::tankSpeed),
  tsTurnRate(BZDBCache::tankAngVel),
  tsNextTurnRate(BZDBCache::tankAngVel),
  tsDistanceRemaining(0.0f),
  tsNextDistance(0.0f),
  tsTurnRemaining(0.0f),
  tsNextTurn(0.0f),
  tsHasStopped(false),
  tsStoppedDistance(0.0f),
  tsStoppedTurn(0.0f),
  tsStoppedForward(false),
  tsStoppedLeft(false),
  tsShotReloadTime(BZDB.eval(BZDBNAMES.RELOADTIME))
{
#if defined(HAVE_PTHREADS)
  pthread_mutex_init(&player_lock, NULL);
#elif defined(_WIN32) 
  InitializeCriticalSection(&player_lock);
#endif
  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    tsPendingUpdates[i] = false;
  tsBattleFieldSize = BZDBCache::worldSize;
}

BZRobotPlayer::~BZRobotPlayer()
{
#if defined(_WIN32) 
  DeleteCriticalSection (&player_lock);
#endif
}

void BZRobotPlayer::setRobot(BZRobot * /*_robot*/)
{
  LOCK_PLAYER

  UNLOCK_PLAYER
}

void BZRobotPlayer::pushEvent(BZRobotEvent *e)
{
  LOCK_PLAYER

  UNLOCK_PLAYER
}

void BZRobotPlayer::execEvents()
{
  LOCK_PLAYER

  UNLOCK_PLAYER

}

void BZRobotPlayer::explodeTank()
{
  LocalPlayer::explodeTank();
}

void BZRobotPlayer::restart(const double* _pos, double _azimuth)
{
  fvec3 pos((float)_pos[0], (float)_pos[1], (float)_pos[2]);
  LocalPlayer::restart(pos, (float)_azimuth);
}

// Called by bzrobots client thread
void BZRobotPlayer::update(float inputDT)
{
  LOCK_PLAYER
  BaseLocalPlayer::update(inputDT); // There is no LocalPlayer::update
  UNLOCK_PLAYER
}

// Called by bzrobots client thread
// Note that LOCK_PLAYER is already set by BZRobotPlayer::update
void BZRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
  // Copy data accessed by both threads
  const fvec3& vvec = getVelocity();
  tsTankSize = getDimensions();
  tsGunHeat = getReloadTime();
  tsPosition = getPosition();
  tsCurrentHeading = getAngle();
  tsCurrentSpeed = sqrt(vvec.x*vvec.x + vvec.y*vvec.y);
  tsCurrentTurnRate = getAngularVelocity();
  
  if (tsShoot) {
    tsShoot = false;
    fireShot();
  }
}

// Called by bzrobots client thread
// Note that LOCK_PLAYER is already set by BZRobotPlayer::update
void BZRobotPlayer::doUpdateMotion(float dt)
{
  const fvec3& vvec = getVelocity();
  float dist = dt *sqrt(vvec.x*vvec.x + vvec.y*vvec.y); // no z vector
  tsDistanceRemaining -= dist;
  if (tsDistanceRemaining > 0.0001) {
    setDesiredSpeed((float)(tsDistanceForward ? tsSpeed : -tsSpeed));
  } else {
    setDesiredSpeed(0);
    tsDistanceRemaining = 0.0f;
  }
  if (tsTurnRemaining > 0.0001) {
    double turnAdjust = getAngularVelocity() * dt;
    if (tsTurnLeft) {
      tsTurnRemaining -= turnAdjust;
      if (tsTurnRemaining <= 0.0001)
        setDesiredAngVel(0);
      else if (tsTurnRate * dt > tsTurnRemaining)
        setDesiredAngVel((float)tsTurnRemaining/dt);
      else
        setDesiredAngVel((float)tsTurnRate);
    } else {
      tsTurnRemaining += turnAdjust;
      if (tsTurnRemaining <= 0.0001)
        setDesiredAngVel(0);
      else if (tsTurnRate * dt > tsTurnRemaining)
        setDesiredAngVel((float)-tsTurnRemaining/dt);
      else
        setDesiredAngVel((float)-tsTurnRate);
    }
  }
  if (tsTurnRemaining <= 0.0001) {
    setDesiredAngVel(0);
    tsTurnRemaining = 0.0f;
  }
  LocalPlayer::doUpdateMotion(dt);
}

void BZRobotPlayer::botAhead(double distance)
{
  botSetAhead(distance);
  botExecute();
  while(botGetDistanceRemaining() > 0.0f)
	  TimeKeeper::sleep(0.01);
}

void BZRobotPlayer::botBack(double distance)
{
	botAhead(-distance);
}

void BZRobotPlayer::botExecute()
{
  LOCK_PLAYER
  if (tsPendingUpdates[BZRobotPlayer::speedUpdate])
    tsSpeed = tsNextSpeed;
  if (tsPendingUpdates[BZRobotPlayer::turnRateUpdate])
    tsTurnRate = tsNextTurnRate;

  if (tsPendingUpdates[BZRobotPlayer::distanceUpdate]) {
    if (tsNextDistance < 0.0f)
      tsDistanceForward = false;
    else
      tsDistanceForward = true;
    tsDistanceRemaining = (tsDistanceForward ? 1 : -1) * tsNextDistance;
  }
  if (tsPendingUpdates[BZRobotPlayer::turnUpdate]) {
    if (tsNextTurn < 0.0f)
      tsTurnLeft = false;
    else
      tsTurnLeft = true;
    tsTurnRemaining = (tsTurnLeft ? 1 : -1) * tsNextTurn;
  }

  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    tsPendingUpdates[i] = false;
  UNLOCK_PLAYER
  double thisExec = TimeKeeper::getCurrent().getSeconds();
  double diffExec = (thisExec - lastExec);
  if(diffExec < 0.02) {
    TimeKeeper::sleep(0.02 - diffExec);
    lastExec = TimeKeeper::getCurrent().getSeconds();
  } else {
    lastExec = thisExec;
  }
}

void BZRobotPlayer::botFire()
{
  botSetFire();
  botExecute();
}

double BZRobotPlayer::botGetDistanceRemaining()
{
  LOCK_PLAYER
  double distanceRemaining = tsDistanceRemaining;
  UNLOCK_PLAYER
  return distanceRemaining;
}

const char * BZRobotPlayer::botGetName()
{
  return tsName.c_str();
}

double BZRobotPlayer::botGetGunCoolingRate()
{
  return tsShotReloadTime;
}


double BZRobotPlayer::botGetBattleFieldSize()
{
  LOCK_PLAYER
  double battleFieldSize = tsBattleFieldSize;
  UNLOCK_PLAYER
  return battleFieldSize;
}

double BZRobotPlayer::botGetGunHeat()
{
  LOCK_PLAYER
  double gunHeat = tsGunHeat;
  UNLOCK_PLAYER
  return gunHeat;
}

double BZRobotPlayer::botGetVelocity()
{
  LOCK_PLAYER
  double velocity = tsCurrentSpeed;
  UNLOCK_PLAYER
  return velocity;
}

double BZRobotPlayer::botGetHeading()
{
  LOCK_PLAYER
  double heading = tsCurrentHeading * 180.0f/M_PI;
  UNLOCK_PLAYER
  return heading;
}

double BZRobotPlayer::botGetWidth()
{
  LOCK_PLAYER
  double width = tsTankSize.x;
  UNLOCK_PLAYER
  return width;
}

double BZRobotPlayer::botGetLength()
{
  LOCK_PLAYER
  double length = tsTankSize.y;
  UNLOCK_PLAYER
  return length;
}

double BZRobotPlayer::botGetHeight()
{
  LOCK_PLAYER
  double height = tsTankSize.z;
  UNLOCK_PLAYER
  return height;
}

double BZRobotPlayer::botGetTime()
{
  return TimeKeeper::getCurrent().getSeconds();
}

double BZRobotPlayer::botGetX()
{
  LOCK_PLAYER
  double xPos = tsPosition.x;
  UNLOCK_PLAYER
  return xPos;
}

double BZRobotPlayer::botGetY()
{
  LOCK_PLAYER
  double yPos = tsPosition.y;
  UNLOCK_PLAYER
  return yPos;
}

double BZRobotPlayer::botGetZ()
{
  LOCK_PLAYER
  double zPos = tsPosition.z;
  UNLOCK_PLAYER
  return zPos;
}

double BZRobotPlayer::botGetTurnRemaining()
{
  LOCK_PLAYER
  double turnRemaining = tsTurnRemaining * 180.0f/M_PI;
  UNLOCK_PLAYER
  return turnRemaining;
}

void BZRobotPlayer::botResume()
{
  botSetResume();
  botExecute();
}

void BZRobotPlayer::botScan()
{
}

void BZRobotPlayer::botSetAhead(double distance)
{
  LOCK_PLAYER
  tsNextDistance = distance;
  tsPendingUpdates[BZRobotPlayer::distanceUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetFire()
{
  LOCK_PLAYER
  tsShoot = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetTurnRate(double rate)
{
  LOCK_PLAYER
  tsNextTurnRate = rate * M_PI/180.0f;
  tsPendingUpdates[BZRobotPlayer::turnRateUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetMaxVelocity(double speed)
{
  LOCK_PLAYER
  tsNextSpeed = speed;
  tsPendingUpdates[BZRobotPlayer::speedUpdate] = true;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetResume()
{
  LOCK_PLAYER
  if (tsHasStopped) {
    tsHasStopped = false;
    tsDistanceRemaining = tsStoppedDistance;
    tsTurnRemaining = tsStoppedTurn;
    tsDistanceForward = tsStoppedForward;
    tsTurnLeft = tsStoppedLeft;
  }
  UNLOCK_PLAYER
}

void BZRobotPlayer::botStop(bool overwrite)
{
  botSetStop(overwrite);
  botExecute();
}

void BZRobotPlayer::botSetStop(bool overwrite)
{
  LOCK_PLAYER
  if (!tsHasStopped || overwrite) {
    tsHasStopped = true;
    tsStoppedDistance = tsDistanceRemaining;
    tsStoppedTurn = tsTurnRemaining;
    tsStoppedForward = tsDistanceForward;
    tsStoppedLeft = tsTurnLeft;
  }
  UNLOCK_PLAYER
}

void BZRobotPlayer::botSetTurnLeft(double turn)
{
  LOCK_PLAYER
  tsPendingUpdates[BZRobotPlayer::turnUpdate] = true;
  tsNextTurn = turn * M_PI/180.0f;
  UNLOCK_PLAYER
}

void BZRobotPlayer::botTurnLeft(double turn)
{
  botSetTurnLeft(turn);
  botExecute();
  while(botGetTurnRemaining() > 0.0f)
	  TimeKeeper::sleep(0.01);
}

void BZRobotPlayer::botTurnRight(double turn)
{
  botSetTurnLeft(-turn);
  botExecute();
  while(botGetTurnRemaining() < 0.0f)
	  TimeKeeper::sleep(0.01);
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
