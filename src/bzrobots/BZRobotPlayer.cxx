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
  tickDuration(2.0),
  speed(1.0),
  nextSpeed(1.0),
  turnRate(1.0),
  nextTurnRate(1.0),
  shoot(false),
  distanceRemaining(0.0),
  nextDistance(0.0),
  turnRemaining(0.0),
  nextTurn(0.0),
  hasStopped(false)
{
#if defined(HAVE_PTHREADS)
  pthread_mutex_init(&player_lock, NULL);
#endif
  for (int i = 0; i < BZRobotPlayer::updateCount; ++i)
    pendingUpdates[i] = false;
}


float BZRobotPlayer::getReloadTime()
{
  LOCK_PLAYER
  float reloadTime = RobotPlayer::getReloadTime();
  UNLOCK_PLAYER
  return reloadTime;
}


void BZRobotPlayer::addShot(ShotPath *shot, const FiringInfo &info)
{
  LOCK_PLAYER
  RobotPlayer::addShot(shot, info);
  UNLOCK_PLAYER
}

void BZRobotPlayer::updateShot (FiringInfo &info, int shotID, double time )
{
  LOCK_PLAYER
  RobotPlayer::updateShot(info, shotID, time );
  UNLOCK_PLAYER
}


void BZRobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);
}


void BZRobotPlayer::doUpdateMotion(float dt)
{
  if (isAlive()) {
    const float *vel = getVelocity();
    distanceRemaining -= sqrt(vel[0]*vel[0] + vel[1]*vel[1] + vel[2]*vel[2]) * dt;
    if (distanceRemaining > 0.0) {
      if (distanceForward)
        setDesiredSpeed((float)speed);
      else
        setDesiredSpeed((float)-speed);
    } else {
      setDesiredSpeed(0);
    }

    if (turnRemaining > 0.0) {
      if (turnLeft) {
        turnRemaining -= getAngularVelocity() * dt;

        if (turnRemaining <= 0.0)
        setDesiredAngVel(0);
        else if (turnRate * dt > turnRemaining)
        setDesiredAngVel((float)turnRemaining/dt);
        else
        setDesiredAngVel((float)turnRate);
      } else {
        turnRemaining += getAngularVelocity() * dt;
        if (turnRemaining <= 0.0)
        setDesiredAngVel(0);
        else if (turnRate * dt > turnRemaining)
        setDesiredAngVel((float)-turnRemaining/dt);
        else
        setDesiredAngVel((float)-turnRate);
      }
    } else {
      setDesiredAngVel(0);
    }
  }
  LocalPlayer::doUpdateMotion(dt);
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


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
