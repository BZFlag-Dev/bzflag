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


BZRobotPlayer::BZRobotPlayer(const PlayerId& _id,
			     const char* _name,
			     ServerLink* _server) :
  RobotPlayer(_id, _name, _server),
  tsShoot(false),
  tsSpeed(1.0),
  tsNextSpeed(1.0),
  tsTurnRate(1.0),
  tsNextTurnRate(1.0),
  tsDistanceRemaining(0.0),
  tsNextDistance(0.0),
  tsTurnRemaining(0.0),
  tsNextTurn(0.0),
  tsHasStopped(false)
{
#if defined(HAVE_PTHREADS)
  pthread_mutex_init(&player_lock, NULL);
#elif defined(_WIN32) 
  InitializeCriticalSection (&player_lock);
#endif
  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    tsPendingUpdates[i] = false;
}

BZRobotPlayer::~BZRobotPlayer()
{
#if defined(_WIN32) 
  DeleteCriticalSection (&player_lock);
#endif
}

void BZRobotPlayer::explodeTank()
{
  LocalPlayer::explodeTank();
}


void BZRobotPlayer::restart(const double* _pos, double _azimuth)
{
  fvec3 pos((float)_pos[0], (float)_pos[1], (float)_pos[2]);;
  LocalPlayer::restart(pos, (float)_azimuth);
}

// Called by bzrobots client thread
void BZRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
  LOCK_PLAYER
  // Copy data accessed by both threads
  //tsBattleFieldSize;
  const float *vel = getVelocity();
  tsTankSize = getDimensions();
  tsGunHeat = getReloadTime();
  tsPosition = getPosition();
  tsCurrentHeading = getAngle()*180.0f/M_PI;
  tsCurrentSpeed = sqrt(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]);
  tsCurrentTurnRate = getAngularVelocity();

  if (tsShoot) {
    tsShoot = false;
    fireShot();
  }
  UNLOCK_PLAYER
}

// Called by bzrobots client thread
void BZRobotPlayer::doUpdateMotion(float dt)
{
  LOCK_PLAYER
  if (isAlive()) {
    const float *vel = getVelocity();
    tsDistanceRemaining -= sqrt(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]) * dt;
    if (tsDistanceRemaining > 0.0) {
      if (tsDistanceForward)
        setDesiredSpeed((float)tsSpeed);
      else
        setDesiredSpeed((float)-tsSpeed);
    } else {
      setDesiredSpeed(0);
    }

    if (tsTurnRemaining > 0.0) {
      if (tsTurnLeft) {
        tsTurnRemaining -= getAngularVelocity() * dt;

        if (tsTurnRemaining <= 0.0)
        setDesiredAngVel(0);
        else if (tsTurnRate * dt > tsTurnRemaining)
        setDesiredAngVel((float)tsTurnRemaining/dt);
        else
        setDesiredAngVel((float)tsTurnRate);
      } else {
        tsTurnRemaining += getAngularVelocity() * dt;
        if (tsTurnRemaining <= 0.0)
        setDesiredAngVel(0);
        else if (tsTurnRate * dt > tsTurnRemaining)
        setDesiredAngVel((float)-tsTurnRemaining/dt);
        else
        setDesiredAngVel((float)-tsTurnRate);
      }
    } else {
      setDesiredAngVel(0);
    }
  }
  UNLOCK_PLAYER
  LocalPlayer::doUpdateMotion(dt);
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
    tsTurnRemaining = (tsTurnLeft ? 1 : -1) * tsNextTurn * M_PI/180.0f; /* We have to convert to radians! */
  }

  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    tsPendingUpdates[i] = false;
  UNLOCK_PLAYER
}

double BZRobotPlayer::botGetDistanceRemaining()
{
  LOCK_PLAYER
  double distanceRemaining = tsDistanceRemaining;
  UNLOCK_PLAYER
  return distanceRemaining;
}

double BZRobotPlayer::botGetTurnRemaining()
{
  LOCK_PLAYER
  double turnRemaining = tsTurnRemaining * 180.0f/M_PI;
  UNLOCK_PLAYER
  return turnRemaining;
}

void BZRobotPlayer::botSetAhead(double distance)
{
  LOCK_PLAYER
  tsPendingUpdates[BZRobotPlayer::distanceUpdate] = true;
  tsNextDistance = distance;
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
  tsNextTurnRate = rate;
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
  tsNextTurn = turn;
  UNLOCK_PLAYER
}

double BZRobotPlayer::botGetBattleFieldSize()
{
  //
  // FIXME: Return proper value
  return 0.0;
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
  double heading = tsCurrentHeading;
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

long BZRobotPlayer::botGetTime()
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

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
