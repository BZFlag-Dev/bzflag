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
#include "LocalPlayer.h"

/* system headers */
#include <string>

/* common implementation headers */
#include "CommandManager.h"
#include "WallObstacle.h"
#include "BZDBCache.h"

/* local implementation headers */
#include "World.h"
#include "sound.h"
#include "RemotePlayer.h"


LocalPlayer*		LocalPlayer::mainPlayer = NULL;

LocalPlayer::LocalPlayer(const PlayerId& id,
			 const char* name, const char* email) :
  BaseLocalPlayer(id, name, email),
  gettingSound(true),
  server(ServerLink::getServer()),
  location(Dead),
  firingStatus(Deceased),
  flagShakingTime(0.0f),
  flagShakingWins(0),
  antidoteFlag(NULL),
  desiredSpeed(0.0f),
  desiredAngVel(0.0f),
  lastSpeed(0.0f),
  insideBuilding(NULL),
  anyShotActive(false),
  target(NULL),
  nemesis(NULL),
  recipient(NULL),
  inputChanged(false),
  stuckingFrameCount(0),
  spawning(false),
  wingsFlapCount(0),
  handicap(0.0f),
  left(false),
  right(false),
  up(false),
  down(false)
{
  // initialize shots array to no shots fired
  const int numShots = World::getWorld()->getMaxShots();
  shots = new LocalShotPath*[numShots];
  for (int i = 0; i < numShots; i++)
    shots[i] = NULL;
  // set input method
  if (BZDB.isTrue("allowInputChange")) {
    inputMethod = Mouse;
  } else {
    setInputMethod(BZDB.get("forceInputDevice"));
  }
}

LocalPlayer::~LocalPlayer()
{
  // free shots
  const int numShots = World::getWorld()->getMaxShots();
  for (int i = 0; i < numShots; i++)
    if (shots[i])
      delete shots[i];
  delete[] shots;

  // free antidote flag
  delete antidoteFlag;
}

LocalPlayer*		LocalPlayer::getMyTank()
{
  return mainPlayer;
}

void			LocalPlayer::setMyTank(LocalPlayer* player)
{
  mainPlayer = player;
}

void			LocalPlayer::doUpdate(float dt)
{
  const bool hadShotActive = anyShotActive;
  const int numShots = World::getWorld()->getMaxShots();
  static TimeKeeper pauseTime = TimeKeeper::getNullTime();
  static bool wasPaused = false;

  // if paused then boost the reload times by dt (so that, effectively,
  // reloading isn't performed)
  int i;
  if (isPaused()) {
    for (i = 0; i < numShots; i++) {
      if (shots[i]) {
	shots[i]->boostReloadTime(dt);
      }
    }

    // if we've been paused for a long time, drop our flag
    if (!wasPaused) {
      pauseTime = TimeKeeper::getCurrent();
      wasPaused = true;
    }
    if (TimeKeeper::getCurrent() -  pauseTime > BZDB.eval(StateDatabase::BZDB_PAUSEDROPTIME)) {
      server->sendDropFlag(getPosition());
      pauseTime = TimeKeeper::getSunExplodeTime();
    }

  } else {
    pauseTime = TimeKeeper::getNullTime();
    wasPaused = false;
  }

  // reap dead (reloaded) shots
  for (i = 0; i < numShots; i++)
    if (shots[i] && shots[i]->isReloaded()) {
      if (!shots[i]->isExpired())
	shots[i]->setExpired();
      delete shots[i];
      shots[i] = NULL;
    }

  // update shots
  anyShotActive = false;
  for (i = 0; i < numShots; i++)
    if (shots[i]) {
      shots[i]->update(dt);
      if (!shots[i]->isExpired()) anyShotActive = true;
    }

  // if no shots now out (but there had been) then reset target
  if (!anyShotActive && hadShotActive)
    target = NULL;

  // drop bad flag if timeout has expired
  if (!isPaused() && dt > 0.0f && World::getWorld()->allowShakeTimeout() &&
      getFlag() != Flags::Null && getFlag()->endurance == FlagSticky &&
      flagShakingTime > 0.0f) {
    flagShakingTime -= dt;
    if (flagShakingTime <= 0.0f) {
      flagShakingTime = 0.0f;
      server->sendDropFlag(getPosition());
    }
  }
}

void			LocalPlayer::doUpdateMotion(float dt)
{
  static const float MinSearchStep = 0.0001f;
  static const int MaxSearchSteps = 7;
  static const int MaxSteps = 4;
  static const float TinyDistance = 0.001f;

  // save old state
  const Location oldLocation = location;
  const float* oldPosition = getPosition();
  const float oldAzimuth = getAngle();
  const float oldAngVel = getAngularVelocity();
  const float* oldVelocity = getVelocity();

  // prepare new state
  float newVelocity[3];
  newVelocity[0] = oldVelocity[0];
  newVelocity[1] = oldVelocity[1];
  newVelocity[2] = oldVelocity[2];
  float newAngVel = 0.0f;

  // if was teleporting and exceeded teleport time then not teleporting anymore
  if (isTeleporting() && getTeleportTime() - lastTime >= BZDB.eval(StateDatabase::BZDB_TELEPORTTIME))
    setStatus(getStatus() & ~short(PlayerState::Teleporting));

  // phased means we can pass through buildings
  const bool phased = (location == Dead || location == Exploding ||
		       (getFlag() == Flags::OscillationOverthruster) ||
		       (getFlag() == Flags::PhantomZone && isFlagActive()));

  float groundLimit = 0.0f;
  if (getFlag() == Flags::Burrow)
    groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);

  // get linear and angular speed at start of time step
  if (!NEAR_ZERO(dt,ZERO_TOLERANCE)) {
    if (location == Dead || isPaused()) {
      // can't move if paused or dead -- set dt to zero instead of
      // clearing velocity and newAngVel for when we resume (if paused)
      dt = 0.0f;
      newAngVel = oldAngVel;
    } else if (location == Exploding) {
      // see if explosing time has expired
      if (lastTime - getExplodeTime() >= BZDB.eval(StateDatabase::BZDB_EXPLODETIME)) {
	dt -= (lastTime - getExplodeTime()) - BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
	if (dt < 0.0f) dt = 0.0f;
	setStatus(PlayerState::DeadStatus);
	location = Dead;
	if (isAutoPilot())
	  CMDMGR.run("restart");
      }

      // can't control explosion motion
      newVelocity[2] += BZDB.eval(StateDatabase::BZDB_GRAVITY) * dt;
      newAngVel = 0.0f;	// or oldAngVel to spin while exploding
    } else if ((location == OnGround) 
      ||       (location == OnBuilding) 
      ||       (location == InBuilding && oldPosition[2] == groundLimit)) {
      // full control
      float speed = desiredSpeed;

      if (inputMethod == Keyboard) {
	/* the larger the oldAngVel contribution, the more slowly an
	 * angular velocity converges to the desired "max" velocity; the
	 * contribution of the desired and old velocity should add up to
	 * one for a linear convergence rate.
	 */
	newAngVel = oldAngVel*0.8f + desiredAngVel*0.2f;

	// instant stop
	if ((oldAngVel * desiredAngVel < 0.0f) || (NEAR_ZERO(desiredAngVel, ZERO_TOLERANCE))) {
	  newAngVel = desiredAngVel;
	}
      } else { // mouse or joystick
	newAngVel = desiredAngVel;
      }

      // limit acceleration
      doMomentum(dt, speed, newAngVel);

      // compute velocity so far
      const float angle = oldAzimuth + 0.5f * dt * newAngVel;
      newVelocity[0] = speed * cosf(angle);
      newVelocity[1] = speed * sinf(angle);
      newVelocity[2] = 0.0f;

      // now friction, if any
      doFriction(dt, oldVelocity, newVelocity);

      // reset our flap count if we have wings
      if (getFlag() == Flags::Wings)
         wingsFlapCount = (int) BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT);

      if ((oldPosition[2] < 0.0f) && (getFlag() == Flags::Burrow))
	newVelocity[2] += 4 * BZDB.eval(StateDatabase::BZDB_GRAVITY) * dt;
      else if (oldPosition[2] > groundLimit)
	newVelocity[2] += BZDB.eval(StateDatabase::BZDB_GRAVITY) * dt;

      // save speed for next update
      lastSpeed = speed;
    } else {
      // can't control motion in air unless have wings
      if (getFlag() == Flags::Wings) {
        float speed = desiredSpeed;
        if (inputMethod == Keyboard) {
	  /* the larger the oldAngVel contribution, the more slowly an
	   * angular velocity converges to the desired "max" velocity; the
	   * contribution of the desired and old velocity should add up to
	   * one for a linear convergence rate.
	   */
	  newAngVel = oldAngVel*0.8f + desiredAngVel*0.2f;

	  // instant stop
	  if ((oldAngVel * desiredAngVel < 0.0f) || (NEAR_ZERO(desiredAngVel, ZERO_TOLERANCE))) {
	    newAngVel = desiredAngVel;
	  }
	} 
	else { // mouse or joystick
	  newAngVel = desiredAngVel;
	}
        // compute velocity so far
	const float angle = oldAzimuth + 0.5f * dt * newAngVel;
        newVelocity[0] = speed * cosf(angle);
        newVelocity[1] = speed * sinf(angle);
        newVelocity[2] += BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY) * dt;
        lastSpeed = speed;
      }
      else {
        newVelocity[2] += BZDB.eval(StateDatabase::BZDB_GRAVITY) * dt;
        newAngVel = oldAngVel;
      }
    }

    // now apply outside forces
    doForces(dt, newVelocity, newAngVel);

    // below the ground: however I got there, creep up
    if (oldPosition[2] < groundLimit) {
#ifdef _MSC_VER
      newVelocity[2] = max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
#else
      newVelocity[2] = std::max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
#endif
    }
  }

  // get new position so far (which is just the current position)
  float newPos[3];
  newPos[0] = oldPosition[0];
  newPos[1] = oldPosition[1];
  newPos[2] = oldPosition[2];
  float newAzimuth = oldAzimuth;

  // move tank through the time step.  if there's a collision then
  // move the tank up to the collision, adjust the velocity to
  // prevent interpenetration, and repeat.  avoid infinite loops
  // by only allowing a maximum number of repeats.
  bool expelled;
  const Obstacle* obstacle;
  float timeStep = dt;
  int   stucked  = false;
  if (location != Dead && location != Exploding) {
    location = OnGround;

    // anti-stuck code is useful only when alive
    // then only any 100 frames while stucked, take an action

    // try to see if we are stuck on a building
    obstacle = getHitBuilding(newPos, newAzimuth, newPos, newAzimuth, phased,
			      expelled);
    if (obstacle && expelled) {
      stuckingFrameCount++;
      stucked = true;
    } else {
      stuckingFrameCount = 0;
    }

    if (stuckingFrameCount > 100) {
      stuckingFrameCount = 0;
      // we are using a maximum value on time for frame to avoid lagging problem
      setDesiredSpeed(0.25f);
      float deltaTime = dt > 0.1f ? 0.1f : dt;
      float normalStuck[3];
      obstacle->getNormal(newPos, normalStuck);
      // use all the given speed to exit
      float movementMax = desiredSpeed * deltaTime;

      newVelocity[0] = movementMax * normalStuck[0];
      newVelocity[1] = movementMax * normalStuck[1];
      if ((World::getWorld()->allowJumping() || (getFlag() == Flags::Jumping) || (getFlag() == Flags::Wings)) &&
	  (getFlag() != Flags::NoJumping))
	newVelocity[2] = movementMax * normalStuck[2];
      else
	newVelocity[2] = 0.0f;

      // exit will be in the normal direction
      newPos[0] += newVelocity[0];
      newPos[1] += newVelocity[1];
      newPos[2] += newVelocity[2];
      // compute time for all other kind of movements
      timeStep -= deltaTime;
    }
  }

  for (int numSteps = 0; numSteps < MaxSteps; numSteps++) {
    // record position at beginning of time step
    float tmpPos[3], tmpAzimuth;
    tmpAzimuth = newAzimuth;
    tmpPos[0] = newPos[0];
    tmpPos[1] = newPos[1];
    tmpPos[2] = newPos[2];

    // get position at end of time step
    newAzimuth = tmpAzimuth + timeStep * newAngVel;
    newPos[0] = tmpPos[0] + timeStep * newVelocity[0];
    newPos[1] = tmpPos[1] + timeStep * newVelocity[1];
    newPos[2] = tmpPos[2] + timeStep * newVelocity[2];
    if (newPos[2]<groundLimit && newVelocity[2]<0) {
      // Hit lower limit, stop falling
      newPos[2] = groundLimit;
      if (location == Exploding) {
	// tank pieces reach the ground, friction
	// stop them, & mainly player view
	newPos[0] = tmpPos[0];
	newPos[1] = tmpPos[1];
      }
    }

    // see if we hit anything.  if not then we're done.
    obstacle = getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth, phased,
			      expelled);
    if (!obstacle || !expelled) break;

    float obstacleTop = obstacle->getPosition()[2] + obstacle->getHeight();
    if ((oldLocation != InAir) 
    &&  (obstacle->getType() != WallObstacle::getClassName())
    &&  (obstacle->getType() != PyramidBuilding::getClassName())
    &&  (obstacleTop != tmpPos[2]) && obstacleTop < (tmpPos[2] + BZDB.eval( StateDatabase::BZDB_MAXBUMPHEIGHT))) {
      newPos[0] = oldPosition[0];
      newPos[1] = oldPosition[1];
      newPos[2] = obstacleTop;
      const Obstacle* bumpObstacle = getHitBuilding(newPos, tmpAzimuth, newPos, newAzimuth, phased, expelled);
      if (bumpObstacle == NULL) {
        move(newPos, getAngle());
        newPos[0] += newVelocity[0]*dt*0.5f;
        newPos[1] += newVelocity[1]*dt*0.5f;
        break;
      }
    }


    // record position when hitting
    float hitPos[3], hitAzimuth;
    hitAzimuth = newAzimuth;
    hitPos[0] = newPos[0];
    hitPos[1] = newPos[1];
    hitPos[2] = newPos[2];

    // find the latest time before the collision
    float searchTime = 0.0f, searchStep = 0.5f * timeStep;
    for (int i = 0; searchStep > MinSearchStep && i < MaxSearchSteps;
	 searchStep *= 0.5f, i++) {
      // get intermediate position
      const float t = searchTime + searchStep;
      newAzimuth = tmpAzimuth + t * newAngVel;
      newPos[0] = tmpPos[0] + t * newVelocity[0];
      newPos[1] = tmpPos[1] + t * newVelocity[1];
      newPos[2] = tmpPos[2] + t * newVelocity[2];
      if (newPos[2]<groundLimit && newVelocity[2]<0) newPos[2] = groundLimit;

      // see if we hit anything
      bool searchExpelled;
      const Obstacle* searchObstacle =
	getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth, phased,
		       searchExpelled);

      if (!searchObstacle || !searchExpelled) {
        // if no hit then search latter half of time step
	searchTime = t;
      } else if (searchObstacle) {
        // if we hit a building then record which one and where
	obstacle = searchObstacle;
	expelled = searchExpelled;
	hitAzimuth = newAzimuth;
	hitPos[0] = newPos[0];
	hitPos[1] = newPos[1];
	hitPos[2] = newPos[2];
      }
    }

    // get position just before impact
    newAzimuth = tmpAzimuth + searchTime * newAngVel;
    newPos[0] = tmpPos[0] + searchTime * newVelocity[0];
    newPos[1] = tmpPos[1] + searchTime * newVelocity[1];
    newPos[2] = tmpPos[2] + searchTime * newVelocity[2];
    if (oldPosition[2] < groundLimit) {
#ifdef _MSC_VER
      newVelocity[2] = max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
#else
      newVelocity[2] = std::max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
#endif
    }


    // record how much time is left in time step
    timeStep -= searchTime;

    // get normal at intersection.  sometimes fancy test says there's
    // no intersection but we're expecting one so, in that case, fall
    // back to simple normal calculation.
    float normal[3];
    if (!getHitNormal(obstacle, newPos, newAzimuth, hitPos, hitAzimuth, normal))
      obstacle->getNormal(newPos, normal);

    // check for being on a building
    if (newPos[2] > 0.0 && normal[2] > 0.001f) {
      if (location != Dead && location != Exploding && expelled)
	location = OnBuilding;
      newVelocity[2] = 0.0f;
    } else {
      // get component of velocity in normal direction (in horizontal plane)
      float mag = normal[0] * newVelocity[0] + normal[1] * newVelocity[1];

      // handle upward normal component to prevent an upward force
      if (!NEAR_ZERO(normal[2], ZERO_TOLERANCE)) {
	// if going down then stop falling
	if (newVelocity[2] < 0.0f && newVelocity[2] -
	    (mag + normal[2] * newVelocity[2]) * normal[2] > 0.0f)
	  newVelocity[2] = 0.0f;

	// normalize force magnitude in horizontal plane
	float horNormal = normal[0] * normal[0] + normal[1] * normal[1];
	if (!NEAR_ZERO(horNormal, ZERO_TOLERANCE))
	  mag /= horNormal;
      }

      // cancel out component in normal direction (if velocity and
      // normal point in opposite directions).  also back off a tiny
      // amount to prevent a spurious collision against the same
      // obstacle.
      if (mag < 0.0f) {
	newVelocity[0] -= mag * normal[0];
	newVelocity[1] -= mag * normal[1];

	newPos[0] -= TinyDistance * mag * normal[0];
	newPos[1] -= TinyDistance * mag * normal[1];
      }
      if (mag > -0.01f) {
	// assume we're not allowed to turn anymore if there's no
	// significant velocity component to cancel out.
	newAngVel = 0.0f;
      }
    }
  }

  // pick new location if we haven't already done so
  if (location == OnGround) {
    if (obstacle && (!expelled || stucked)) {
      location = InBuilding;
    }
    else if (newPos[2] > 0.0f) {
      location = InAir;
    }
  }
  insideBuilding = (const Obstacle*)(location == InBuilding ? obstacle : NULL);

  // see if we're crossing a wall
  if (location == InBuilding && getFlag() == Flags::OscillationOverthruster) {
    if (insideBuilding->isCrossing(newPos, newAzimuth,
				   0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH),
                                   0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH),
                                   BZDBCache::tankHeight, NULL))
      setStatus(getStatus() | int(PlayerState::CrossingWall));
    else
      setStatus(getStatus() & ~int(PlayerState::CrossingWall));
  } else if (World::getWorld()->crossingTeleporter(newPos, newAzimuth,
						 0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH), 0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH), crossingPlane)) {
    setStatus(getStatus() | int(PlayerState::CrossingWall));
  } else {
    setStatus(getStatus() & ~int(PlayerState::CrossingWall));
  }

  // compute actual velocities.  do this before teleportation.
  if (!NEAR_ZERO(dt, ZERO_TOLERANCE)) {
    const float oodt = 1.0f / dt;
    newAngVel = (newAzimuth - oldAzimuth) * oodt;
    newVelocity[0] = (newPos[0] - oldPosition[0]) * oodt;
    newVelocity[1] = (newPos[1] - oldPosition[1]) * oodt;
    newVelocity[2] = (newPos[2] - oldPosition[2]) * oodt;
  }

  // see if we teleported
  int face;
  const Teleporter* teleporter = (const Teleporter*)(!isAlive() ? NULL :
						     World::getWorld()->crossesTeleporter(oldPosition, newPos, face));
  if (teleporter) {
    if (getFlag() == Flags::PhantomZone) {
      // change zoned state
      setStatus(getStatus() ^ PlayerState::FlagActive);
      playLocalSound(SFX_PHANTOM);
    } else {
      // teleport
      const int source = World::getWorld()->getTeleporter(teleporter, face);
      int target = World::getWorld()->getTeleportTarget(source);
      if (target == randomTeleporter) {
        target = rand() % (2 * World::getWorld()->getTeleporters().size());
      }

      int outFace;
      const Teleporter* outPort = World::getWorld()->
	getTeleporter(target, outFace);
      teleporter->getPointWRT(*outPort, face, outFace,
			      newPos, newVelocity, newAzimuth,
			      newPos, newVelocity, &newAzimuth);

      // save teleport info
      setTeleport(lastTime, source, target);
      server->sendTeleport(source, target);
      if (gettingSound)
      playLocalSound(SFX_TELEPORT);
    }
  }

  if (gettingSound) {
    // play landing sound if we weren't on something and now we are
    if (oldLocation == InAir && (location == OnGround || location == OnBuilding))
      playLocalSound(SFX_LAND);
    else if (location == OnGround && oldPosition[2] == 0.0f && newPos[2] < 0.f)
      playLocalSound(SFX_BURROW);
  }

  // set falling status
  if (location == OnGround || location == OnBuilding ||
      (location == InBuilding && newPos[2] == 0.0f))
    setStatus(getStatus() & ~short(PlayerState::Falling));
  else if (location == InAir || location == InBuilding)
    setStatus(getStatus() | short(PlayerState::Falling));

  // compute firing status
  switch (location) {
  case Dead:
  case Exploding:
    firingStatus = Deceased;
    break;
  case InBuilding:
    firingStatus = (getFlag() == Flags::PhantomZone) ? Zoned : Sealed;
    break;
  default:
    if (getFlag() == Flags::PhantomZone && isFlagActive())
      firingStatus = Zoned;
    else if (getReloadTime() > 0.0f)
      firingStatus = Loading;
    else
      firingStatus = Ready;
    break;
  }

  // move tank
  move(newPos, newAzimuth);
  setVelocity(newVelocity);
  setAngularVelocity(newAngVel);
  newAzimuth = getAngle();

  // see if I'm over my antidote
  if (antidoteFlag && location == OnGround) {
    float dist = (flagAntidotePos[0] - newPos[0]) *
      (flagAntidotePos[0] - newPos[0]) +
      (flagAntidotePos[1] - newPos[1]) *
      (flagAntidotePos[1] - newPos[1]);
    if (dist < (getRadius() + BZDBCache::flagRadius) * (getRadius() + BZDBCache::flagRadius))
      server->sendDropFlag(getPosition());
  }
  // don't forget to wave
  if (antidoteFlag)
    antidoteFlag->waveFlag(dt, 0.0f);

  if ((getFlag() == Flags::Bouncy) && ((location == OnGround) || (location == OnBuilding))) {
    if (oldLocation != InAir) {
      if ((TimeKeeper::getCurrent() - bounceTime) > 0)
        this->jump();
    }
    else {
      bounceTime = TimeKeeper::getCurrent();
      bounceTime += ((float) (bzfrand()*4.0f));
    }
  }

  if (gettingSound) {
    if (oldPosition[0] != newPos[0] || oldPosition[1] != newPos[1] ||
	oldPosition[2] != newPos[2] || oldAzimuth != newAzimuth)
      moveSoundReceiver(newPos[0], newPos[1], newPos[2], newAzimuth,
			(NEAR_ZERO(dt, ZERO_TOLERANCE) || teleporter != NULL && getFlag() != Flags::PhantomZone));
    if (NEAR_ZERO(dt, ZERO_TOLERANCE))
      speedSoundReceiver(newVelocity[0], newVelocity[1], newVelocity[2]);
    else
      speedSoundReceiver((newPos[0] - oldPosition[0]) / dt,
			 (newPos[1] - oldPosition[1]) / dt,
			 (newPos[2] - oldPosition[2]) / dt);
  }
}

const Obstacle*		LocalPlayer::getHitBuilding(const float* p, float a,
						    bool phased, bool& expelled) const
{
  float length = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  float width = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  float factor;

  if (getFlag() == Flags::Obesity) {
    factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Tiny) {
    factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Thief) {
    factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Narrow) {
    width = 0.0f;
  }

  const Obstacle* obstacle = World::getWorld()->
    hitBuilding(p, a, length, width);
  expelled = (obstacle != NULL);
  if (expelled && phased)
    expelled = (obstacle->getType() == WallObstacle::getClassName() ||
		obstacle->getType() == Teleporter::getClassName() ||
		(getFlag() == Flags::OscillationOverthruster && desiredSpeed < 0.0f &&
		 p[2] == 0.0f));
  return obstacle;
}

const Obstacle*		LocalPlayer::getHitBuilding(
						    const float* oldP, float oldA,
						    const float* p, float a,
						    bool phased, bool& expelled) const
{
  float length = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  float width = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  float factor;

  if (getFlag() == Flags::Obesity) {
    factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Tiny) {
    factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Thief) {
    factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Narrow) {
    width = 0.0f;
  }

  const Obstacle* obstacle = World::getWorld()->
    hitBuilding(oldP, oldA, p, a, length, width);
  expelled = (obstacle != NULL);
  if (expelled && phased)
    expelled = (obstacle->getType() == WallObstacle::getClassName() ||
		obstacle->getType() == Teleporter::getClassName() ||
		(getFlag() == Flags::OscillationOverthruster &&
		 desiredSpeed < 0.0f && p[2] == 0.0f));
  return obstacle;
}

bool			LocalPlayer::getHitNormal(const Obstacle* o,
						  const float* pos1, float azimuth1,
						  const float* pos2, float azimuth2,
						  float* normal) const
{
  float length = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  float width = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  float factor;
  if (getFlag() == Flags::Obesity) {
    factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Tiny) {
    factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Thief) {
    factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    length *= factor;
    width *= 2.0f * factor;
  }
  else if (getFlag() == Flags::Narrow) {
    width = 0.0f;
  }

  return o->getHitNormal(pos1, azimuth1, pos2, azimuth2,
			 length, width, BZDBCache::tankHeight, normal);
}

float			LocalPlayer::getReloadTime() const
{
  float time = jamTime - TimeKeeper::getCurrent();
  if (time > 0.0f)
    return time;

    // look for an empty slot
  const int numShots = World::getWorld()->getMaxShots();
  int i;
  for (i = 0; i < numShots; i++)
    if (!shots[i])
      return 0.0f;

  // look for the shot fired least recently
  float minTime = shots[0]->getReloadTime() -
    (shots[0]->getCurrentTime() - shots[0]->getStartTime());
  for (i = 1; i < numShots; i++) {
    const float t = shots[i]->getReloadTime() -
      (shots[i]->getCurrentTime() - shots[i]->getStartTime());
    if (t < minTime) minTime = t;
  }

  // apply any handicap advantage to reload
  minTime = minTime * (1.0f + handicap);

  if (minTime < 0.0f) minTime = 0.0f;
  return minTime;
}

float			LocalPlayer::getFlagShakingTime() const
{
  return flagShakingTime;
}

int			LocalPlayer::getFlagShakingWins() const
{
  return flagShakingWins;
}

const GLfloat*		LocalPlayer::getAntidoteLocation() const
{
  return (const GLfloat*)(antidoteFlag ? antidoteFlag->getSphere() : NULL);
}

ShotPath*		LocalPlayer::getShot(int index) const
{
  index &= 0x00FF;
  if ((index < 0) || (index >= World::getWorld()->getMaxShots()))
    return NULL;
  return shots[index];
}

void			LocalPlayer::restart(const float* pos, float _azimuth)
{
  // put me in limbo
  setStatus(short(PlayerState::DeadStatus));

  // can't have a flag
  setFlag(Flags::Null);

  // get rid of existing shots
  const int numShots = World::getWorld()->getMaxShots();
  // get rid of existing shots
  for (int i = 0; i < numShots; i++)
    if (shots[i]) {
      delete shots[i];
      shots[i] = NULL;
    }
  anyShotActive = false;

  // no target
  target = NULL;

  // initialize position/speed state
  static const float zero[3] = { 0.0f, 0.0f, 0.0f };
  location = (pos[2] > 0.0f) ? OnBuilding : OnGround;
  lastSpeed = 0.0f;
  desiredSpeed = 0.0f;
  desiredAngVel = 0.0f;
  move(pos, _azimuth);
  setVelocity(zero);
  setAngularVelocity(0.0f);
  left  = false;
  right = false;
  up    = false;
  down  = false;
  doUpdateMotion(0.0f);
  updateHandicap();

  // make me alive now
  setStatus(getStatus() | short(PlayerState::Alive));
}

void			LocalPlayer::setTeam(TeamColor team)
{
  changeTeam(team);
}

void			LocalPlayer::setDesiredSpeed(float fracOfMaxSpeed)
{
  FlagType* flag = getFlag();
  // can't go faster forward than at top speed, and backward at half speed
  if (fracOfMaxSpeed > 1.0f) fracOfMaxSpeed = 1.0f;
  else if (fracOfMaxSpeed < -0.5f) fracOfMaxSpeed = -0.5f;

  // oscillation overthruster tank in building can't back up
  if (fracOfMaxSpeed < 0.0f && getLocation() == InBuilding &&
      flag == Flags::OscillationOverthruster) {
    fracOfMaxSpeed = 0.0f;
  }

  // boost speed for certain flags
  if (flag == Flags::Velocity) {
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  } else if (flag == Flags::Thief) {
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
  } else if ((flag == Flags::Burrow) && (getPosition()[2] < 0.0f)) {
    fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_BURROWSPEEDAD);
  } else if ((flag == Flags::ForwardOnly) && (fracOfMaxSpeed < 0.0)) {
    fracOfMaxSpeed = 0.0f;
  } else if ((flag == Flags::ReverseOnly) && (fracOfMaxSpeed > 0.0)) {
    fracOfMaxSpeed = 0.0f;
  } else if (flag == Flags::Agility) {
    if ((TimeKeeper::getCurrent() - agilityTime) < BZDB.eval(StateDatabase::BZDB_AGILITYTIMEWINDOW)) {
      fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
    } else {
      float oldFrac = desiredSpeed / BZDB.eval(StateDatabase::BZDB_TANKSPEED);
      if (oldFrac > 1.0f)
	oldFrac = 1.0f;
      else if (oldFrac < -0.5f)
	oldFrac = -0.5f;
      float limit = BZDB.eval(StateDatabase::BZDB_AGILITYVELDELTA);
      if (fracOfMaxSpeed < 0.0f)
	limit /= 2.0f;
      if (fabs(fracOfMaxSpeed - oldFrac) > limit) {
        fracOfMaxSpeed *= BZDB.eval(StateDatabase::BZDB_AGILITYADVEL);
	agilityTime = TimeKeeper::getCurrent();
      }
    }      
  }

  // apply handicap advantage to tank speed
  fracOfMaxSpeed *= (1.0f - handicap);

  // set desired speed
  desiredSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED) * fracOfMaxSpeed;
}

void			LocalPlayer::setDesiredAngVel(float fracOfMaxAngVel)
{
  FlagType* flag = getFlag();

  // limit turn speed to maximum
  if (fracOfMaxAngVel > 1.0f) fracOfMaxAngVel = 1.0f;
  else if (fracOfMaxAngVel < -1.0f) fracOfMaxAngVel = -1.0f;

  // further limit turn speed for certain flags
  if (fracOfMaxAngVel < 0.0f && getFlag() == Flags::LeftTurnOnly)
    fracOfMaxAngVel = 0.0f;
  else if (fracOfMaxAngVel > 0.0f && getFlag() == Flags::RightTurnOnly)
    fracOfMaxAngVel = 0.0f;

  // boost turn speed for other flags
  if (flag == Flags::QuickTurn) {
    fracOfMaxAngVel *= BZDB.eval(StateDatabase::BZDB_ANGULARAD);
  } else if ((flag == Flags::Burrow) && (getPosition()[2] < 0.0f)) {
    fracOfMaxAngVel *= BZDB.eval(StateDatabase::BZDB_BURROWANGULARAD);
  }

  // set desired turn speed
  desiredAngVel = fracOfMaxAngVel * BZDB.eval(StateDatabase::BZDB_TANKANGVEL);
}

void			LocalPlayer::setPause(bool pause)
{
  if (isAlive()) {
    if (pause && !isPaused()) {
      setStatus(getStatus() | short(PlayerState::Paused));
      server->sendPaused(true);
    }
    else if (!pause && isPaused()) {
      setStatus(getStatus() & ~short(PlayerState::Paused));
      server->sendPaused(false);
    }
  }
}

bool			LocalPlayer::fireShot()
{
  if (! (firingStatus == Ready || firingStatus == Zoned))
    return false;

  // find an empty slot
  const int numShots = World::getWorld()->getMaxShots();
  int i;
  for (i = 0; i < numShots; i++)
    if (!shots[i])
      break;
  if (i == numShots) return false;

  // make sure we're allowed to shoot
  if (!isAlive() || isPaused() || location == InBuilding) // ||
      //(getFlag() == Flags::PhantomZone && isFlagActive()))
    return false;

  // prepare shot
  FiringInfo firingInfo(*this, i + getSalt());
  // FIXME team coloring of shot is never used; it was meant to be used
  // for rabbit mode to correctly calculcate team kills when rabbit changes
  firingInfo.shot.team = getTeam();
  if (firingInfo.flagType == Flags::ShockWave) {
    // move shot origin under tank and make it stationary
    const float* pos = getPosition();
    firingInfo.shot.pos[0] = pos[0];
    firingInfo.shot.pos[1] = pos[1];
    firingInfo.shot.pos[2] = pos[2];
    firingInfo.shot.vel[0] = 0.0f;
    firingInfo.shot.vel[1] = 0.0f;
    firingInfo.shot.vel[2] = 0.0f;
  }
  else {
    // Set _shotsKeepVerticalVelocity on the server if you want shots 
    // to have the same vertical velocity as the tank when fired.  
    // keeping shots moving horizontally makes the game more playable.
    if (!BZDB.isTrue(StateDatabase::BZDB_SHOTSKEEPVERTICALV)) firingInfo.shot.vel[2] = 0.0f;
  }

  // make shot and put it in the table
  shots[i] = new LocalShotPath(firingInfo);

  server->sendBeginShot(firingInfo);
  if (gettingSound) {
  if (firingInfo.flagType == Flags::ShockWave)
    playLocalSound(SFX_SHOCK);
  else if (firingInfo.flagType == Flags::Laser)
    playLocalSound(SFX_LASER);
  else if (firingInfo.flagType == Flags::GuidedMissile)
    playLocalSound(SFX_MISSILE);
  else if (firingInfo.flagType == Flags::Thief)
    playLocalSound(SFX_THIEF);
  else
    playLocalSound(SFX_FIRE);
  }

  if (getFlag() == Flags::TriggerHappy) {
    // make sure all the shots don't go off at once
    forceReload(BZDB.eval(StateDatabase::BZDB_RELOADTIME) / numShots);
  }
  return true;
}


void LocalPlayer::forceReload(float time)
{
  jamTime = TimeKeeper::getCurrent();
  jamTime+= time;
}


bool			LocalPlayer::doEndShot(int id, bool isHit, float* pos)
{
  const int index = id & 255;
  const int salt = (id >> 8) & 127;

  // special id used in some messages (and really shouldn't be sent here)
  if (id == -1)
    return false;

  // ignore bogus shots (those with a bad index or for shots that don't exist)
  if (index < 0 || index >= World::getWorld()->getMaxShots() || !shots[index])
    return false;

  // ignore shots that already ending
  if (shots[index]->isExpired() || shots[index]->isExpiring())
    return false;

  // ignore shots that have the wrong salt.  since we reuse shot indices
  // it's possible for an old MsgShotEnd to arrive after we've started a
  // new shot.  that's where the salt comes in.  it changes for each shot
  // so we can identify an old shot from a new one.
  if (salt != ((shots[index]->getShotId() >> 8) & 127))
    return false;

  // don't stop if it's because were hitting something and we don't stop
  // when we hit something.
  if (isHit && !shots[index]->isStoppedByHit())
    return false;

  // end it
  const float* shotPos = shots[index]->getPosition();
  pos[0] = shotPos[0];
  pos[1] = shotPos[1];
  pos[2] = shotPos[2];
  shots[index]->setExpired();
  return true;
}

void			LocalPlayer::jump()
{
  // can't jump while burrowed
  if (getPosition()[2] < 0.0f)
    return;

  if (getFlag() == Flags::Wings) {
     if (wingsFlapCount <= 0) return;
     wingsFlapCount--;
  } else if (location != OnGround && location != OnBuilding) {
     // can't jump unless on the ground or a building
     if (getFlag() != Flags::Wings)
        return;
     if (wingsFlapCount <= 0)
        return;
     wingsFlapCount--;
  } else if ((getFlag() != Flags::Bouncy)
    &&       ((getFlag() != Flags::Jumping && !World::getWorld()->allowJumping()) ||
	      (getFlag() == Flags::NoJumping)))
     return;

  // add jump velocity (actually, set the vertical component since you
  // can only jump if resting on something)
  const float* oldVelocity = getVelocity();
  float newVelocity[3];
  newVelocity[0] = oldVelocity[0];
  newVelocity[1] = oldVelocity[1];
  if (getFlag() == Flags::Wings)
	newVelocity[2] = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
  else if (getFlag() == Flags::Bouncy)
	newVelocity[2] = (float)(bzfrand() * BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY));
  else
	newVelocity[2] = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
  setVelocity(newVelocity);
  location = InAir;
  playLocalSound(SFX_JUMP);
}

void			LocalPlayer::setTarget(const Player* _target)
{
  target = _target;
}

void			LocalPlayer::setNemesis(const Player* _nemesis)
{
  if ((_nemesis == NULL) || _nemesis->getPlayerType() == TankPlayer)
    nemesis = _nemesis;
}

void			LocalPlayer::setRecipient(const Player* _recipient)
{
  if ((_recipient == NULL) || (_recipient->getId() <= LastRealPlayer))
    recipient = _recipient;
}

void			LocalPlayer::explodeTank()
{
  if (location == Dead || location == Exploding) return;
  float gravity      = BZDB.eval(StateDatabase::BZDB_GRAVITY);
  float explodeTime  = BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
  // Limiting max height increment to this value (the old default value)
  const float zMax  = 49.0f;
  setExplode(TimeKeeper::getTick());
  const float* oldVelocity = getVelocity();
  float newVelocity[3];
  float maxSpeed;
  newVelocity[0] = oldVelocity[0];
  newVelocity[1] = oldVelocity[1];
  if (gravity < 0) {
    // comparing 2 speed:
    //   to have a simmetric path (ending at same height as starting)
    //   to reach the acme of parabola, under the max height established
    // take the less
    newVelocity[2] = - 0.5f * gravity * explodeTime;
    maxSpeed       = sqrtf(- 2.0f * zMax * gravity);
    if (newVelocity[2] > maxSpeed)
      newVelocity[2] = maxSpeed;
  } else {
    newVelocity[2] = oldVelocity[2];
  }
  setVelocity(newVelocity);
  location = Exploding;
  target = NULL;		// lose lock when dead
}

void			LocalPlayer::doMomentum(float dt,
						float& speed, float& angVel)
{
  // get maximum linear and angular accelerations
  const float linearAcc = (getFlag() == Flags::Momentum) ? BZDB.eval(StateDatabase::BZDB_MOMENTUMLINACC) :
    World::getWorld()->getLinearAcceleration();
  const float angularAcc = (getFlag() == Flags::Momentum) ? BZDB.eval(StateDatabase::BZDB_MOMENTUMANGACC) :
    World::getWorld()->getAngularAcceleration();

  // limit linear acceleration
  if (linearAcc > 0.0f) {
    const float acc = (speed - lastSpeed) / dt;
    if (acc > 20.0f * linearAcc) speed = lastSpeed + dt * 20.0f*linearAcc;
    else if (acc < -20.0f * linearAcc) speed = lastSpeed - dt * 20.0f*linearAcc;
  }

  // limit angular acceleration
  if (angularAcc > 0.0f) {
    const float oldAngVel = getAngularVelocity();
    const float acc = (angVel - oldAngVel) / dt;
    if (acc > angularAcc) angVel = oldAngVel + dt * angularAcc;
    else if (acc < -angularAcc) angVel = oldAngVel - dt * angularAcc;
  }
}

void			LocalPlayer::doFriction(float dt,
						  const float *oldVelocity, float *newVelocity)
{
  const float friction = (getFlag() == Flags::Momentum) ? BZDB.eval(StateDatabase::BZDB_MOMENTUMFRICTION) :
    BZDB.eval(StateDatabase::BZDB_FRICTION);

  if (friction > 0.0f) {
    // limit vector acceleration

    float delta[2] = {newVelocity[0] - oldVelocity[0], newVelocity[1] - oldVelocity[1]};
    float acc2 = (delta[0] * delta[0] + delta[1] * delta[1]) / (dt*dt);
    float accLimit = 20.0f * friction;

    if (acc2 > accLimit*accLimit) {
      float ratio = accLimit / sqrtf(acc2);
      newVelocity[0] = oldVelocity[0] + delta[0]*ratio;
      newVelocity[1] = oldVelocity[1] + delta[1]*ratio;
    }
  }
}

void			LocalPlayer::doForces(float /*dt*/,
					      float* /*velocity*/,
					      float& /*angVel*/)
{
  // apply external forces
  // do nothing -- no external forces right now
}

// NOTE -- minTime should be initialized to Infinity by the caller
bool			LocalPlayer::checkHit(const Player* source,
					      const ShotPath*& hit, float& minTime) const
{
  bool goodHit = false;

  // if firing tank is paused then it doesn't count
  if (source->isPaused()) return goodHit;

  const int maxShots = source->getMaxShots();
  for (int i = 0; i < maxShots; i++) {
    // get shot
    const ShotPath* shot = source->getShot(i);
    if (!shot || shot->isExpired()) continue;

    // my own shock wave cannot kill me
    if (source == this && ((shot->getFlag() == Flags::ShockWave) || (shot->getFlag() == Flags::Thief))) continue;

    // short circuit test if shot can't possibly hit.
    // only superbullet or shockwave can kill zoned dude
    const FlagType* shotFlag = shot->getFlag();
    if (getFlag() == Flags::PhantomZone && isFlagActive() &&
	shotFlag != Flags::SuperBullet && shotFlag != Flags::ShockWave &&
	shotFlag != Flags::PhantomZone)
      continue;

    // laser can't hit a cloaked tank
    if (getFlag() == Flags::Cloaking && shotFlag == Flags::Laser)
      continue;

    // zoned shots only kill zoned tanks
    if (shotFlag == Flags::PhantomZone){ // zoned shot
      if (getFlag() != Flags::PhantomZone) { // I don't have PZ
        continue; // no way
      } else { // I have PZ
        if (!isFlagActive()) { // not zoned, can't get shot
          continue;
        } else { 
          // do nothing - I am zoned, I can get shot
        }
      }

    }

    // test myself against shot
    float position[3];
    const float t = shot->checkHit(this, position);
    if (t >= minTime) continue;

    // test if shot hit a part of my tank that's through a teleporter.
    // hit is no good if hit point is behind crossing plane.
    if (isCrossingWall() && position[0] * crossingPlane[0] +
	position[1] * crossingPlane[1] +
	position[2] * crossingPlane[2] + crossingPlane[3] < 0.0)
      continue;

    // okay, shot hit
    goodHit = true;
    hit = shot;
    minTime = t;
  }
  return goodHit;
}

void			LocalPlayer::setFlag(FlagType* flag)
{
  Player::setFlag(flag);

  float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  // if it's bad then reset countdowns and set antidote flag
  if (getFlag() != Flags::Null && getFlag()->endurance == FlagSticky) {
    if (World::getWorld()->allowShakeTimeout())
      flagShakingTime = World::getWorld()->getFlagShakeTimeout();
    if (World::getWorld()->allowShakeWins())
      flagShakingWins = World::getWorld()->getFlagShakeWins();
    if (World::getWorld()->allowAntidote()) {
      float tankRadius = BZDBCache::tankRadius;
      float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);
      do {
	if (World::getWorld()->allowTeamFlags()) {
	  flagAntidotePos[0] = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[1] = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[2] = 0.0f;
	} else {
	  flagAntidotePos[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[2] = 0.0f;
	}
      } while (World::getWorld()->inBuilding(flagAntidotePos, tankRadius));
      antidoteFlag = new FlagSceneNode(flagAntidotePos);
      antidoteFlag->setColor(1.0f, 1.0f, 0.0f);
      World::setFlagTexture(antidoteFlag);
    }
  } else {
    delete antidoteFlag;
    antidoteFlag = NULL;
    flagShakingTime = 0.0f;
    flagShakingWins = 0;
  }
}

void			LocalPlayer::changeScore(short deltaWins,
						 short deltaLosses,
						 short deltaTks)
{
  Player::changeScore(deltaWins, deltaLosses, deltaTks);
  if (deltaWins > 0 && World::getWorld()->allowShakeWins() &&
      flagShakingWins > 0) {
    flagShakingWins -= deltaWins;
    if (flagShakingWins <= 0) {
      flagShakingWins = 0;
      server->sendDropFlag(getPosition());
    }
  }
}

float LocalPlayer::updateHandicap()
{
  /* compute and return the handicap */

  if (World::getWorld()->allowHandicap()) {
    int wins = 0;
    int losses = 0;
    const int maxplayers = World::getWorld()->getMaxPlayers();

    // compute overall wins and losses in relation to other players
    for (int i = 0; i < maxplayers; i++) {
      RemotePlayer *player = World::getWorld()->getPlayer(i);
      if ((player == NULL) || (player->getId() == LocalPlayer::getMyTank()->getId())) {
	continue;
      }
      wins += player->getLocalWins();
      losses += player->getLocalLosses();
    }

    // a standard deviation of 20 points will provide a handicap
    handicap = float(wins - losses) / 20.0f;

    /* limit how much of a handicap is afforded, and only provide
     * handicap advantages instead of disadvantages.
     */
    if (handicap > 0.0f) {
      // disadvantage
      handicap = 0.0f;
    } else if (handicap < -0.5f) {
      // advantage
      handicap = -0.5f;
    }

    return handicap;
  }

  // no handicap gameplay
  return 0.0f;
}

void			LocalPlayer::addAntidote(SceneDatabase* scene)
{
  if (antidoteFlag)
    scene->addDynamicNode(antidoteFlag);
}

std::string		LocalPlayer::getInputMethodName(InputMethod whatInput)
{
  switch (whatInput) {
    case Keyboard:
      return std::string("Keyboard");
      break;
    case Mouse:
      return std::string("Mouse");
      break;
    case Joystick:
      return std::string("Joystick");
      break;
    default:
      return std::string("Unnamed Device");
  }
}

void LocalPlayer::setKey(int button, bool pressed) {
  switch (button) {
  case BzfKeyEvent::Left:
    left = pressed;
    break;
  case BzfKeyEvent::Right:
    right = pressed;
    break;
  case BzfKeyEvent::Up:
    up = pressed;
    break;
  case BzfKeyEvent::Down:
    down = pressed;
    break;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

