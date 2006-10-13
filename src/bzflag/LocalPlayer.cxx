/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "LocalPlayer.h"

/* common implementation headers */
#include "CommandManager.h"
#include "BZDBCache.h"
#include "FlagSceneNode.h"
#include "CollisionManager.h"
#include "PhysicsDriver.h"
#include "BzfEvent.h"
#include "WallObstacle.h"
#include "MeshObstacle.h"

/* local implementation headers */
#include "World.h"
#include "sound.h"
#include "ForceFeedback.h"
#include "effectsRenderer.h"
#include "playing.h"

LocalPlayer*		LocalPlayer::mainPlayer = NULL;

LocalPlayer::LocalPlayer(const PlayerId& _id,
			 const char* name, const char* _email) :
  BaseLocalPlayer(_id, name, _email),
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
  anyShotActive(false),
  target(NULL),
  nemesis(NULL),
  recipient(NULL),
  inputChanged(false),
  spawning(false),
  wingsFlapCount(0),
  left(false),
  right(false),
  up(false),
  down(false),
  entryDrop(true),
  wantJump(false),
  jumpPressed(false),
  deathPhyDrv(-1)
{
  // initialize shots array to no shots fired
  World *world = World::getWorld();
  int numShots = 0;
  if (world) {
    numShots = world->getMaxShots();
  }
  shots.resize(numShots);
  for (int i = 0; i < numShots; i++)
    shots[i] = NULL;
  // set input method
  if (BZDB.isTrue("allowInputChange")) {
    inputMethod = Mouse;
  } else {
    setInputMethod(BZDB.get("activeInputDevice"));
  }

  if (headless) {
    gettingSound = false;
  }
  
  stuckStartTime = TimeKeeper::getSunExplodeTime();
}

LocalPlayer::~LocalPlayer()
{
  // free antidote flag
  delete antidoteFlag;
}

void			LocalPlayer::setMyTank(LocalPlayer* player)
{
  mainPlayer = player;
}

void			LocalPlayer::doUpdate(float dt)
{
  const bool hadShotActive = anyShotActive;
  const int numShots = getMaxShots();
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
      setStatus(getStatus() & ~PlayerState::FlagActive);
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
  World *world = World::getWorld();
  if (!isPaused() && dt > 0.0f && world && world->allowShakeTimeout() &&
      getFlag() != Flags::Null && getFlag()->endurance == FlagSticky &&
      flagShakingTime > 0.0f) {
    flagShakingTime -= dt;
    if (flagShakingTime <= 0.0f) {
      flagShakingTime = 0.0f;
      server->sendDropFlag(getPosition());
    }
  }
}


void LocalPlayer::doSlideMotion(float dt, float slideTime,
				float newAngVel, float* newVelocity)
{
  const float oldAzimuth = getAngle();
  const float* oldVelocity = getVelocity();

  const float angle = oldAzimuth + (0.5f * dt * newAngVel);
  const float cos_val = cosf(angle);
  const float sin_val = sinf(angle);
  const float scale = (dt / slideTime);
  const float speedAdj = desiredSpeed * scale;
  const float* ov = oldVelocity;
  const float oldSpeed = sqrtf((ov[0] * ov[0]) + (ov[1] * ov[1]));
  float* nv = newVelocity;
  nv[0] = ov[0] + (cos_val * speedAdj);
  nv[1] = ov[1] + (sin_val * speedAdj);
  const float newSpeed = sqrtf((nv[0] * nv[0]) + (nv[1] * nv[1]));

  float maxSpeed = getMaxSpeed();

  if (newSpeed > maxSpeed) {
    float adjSpeed;
    if (oldSpeed > maxSpeed) {
      adjSpeed = oldSpeed - (dt * (maxSpeed / slideTime));
      if (adjSpeed < 0.0f) {
	adjSpeed = 0.0f;
      }
    } else {
      adjSpeed = maxSpeed;
    }
    const float speedScale = adjSpeed / newSpeed;
    nv[0] *= speedScale;
    nv[1] *= speedScale;
  }
  return;
}


float LocalPlayer::getNewAngVel(float old, float desired)
{
  float newAngVel;

  if ((inputMethod != Keyboard) || (getPhysicsDriver() >= 0)) {
    // mouse and joystick users
    newAngVel = desired;

  } else {

    /* keybaord users
     * the larger the oldAngVel contribution, the more slowly an
     * angular velocity converges to the desired "max" velocity; the
     * contribution of the desired and old velocity should add up to
     * one for a linear convergence rate.
     */
    newAngVel = (old * 0.8f) + (desired * 0.2f);

    // instant stop
    if ((old * desired < 0.0f) ||
	(NEAR_ZERO(desired, ZERO_TOLERANCE))) {
      newAngVel = desired;
    }
  }
  return newAngVel;
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

  if (!headless) {
    clearRemoteSounds();
  }
  World *world = World::getWorld();
  if (!world) {
    return; /* no world, no motion */
  }

  // if was teleporting and exceeded teleport time then not teleporting anymore
  if (isTeleporting() &&
      ((lastTime - getTeleportTime()) >= BZDB.eval(StateDatabase::BZDB_TELEPORTTIME)))
    setStatus(getStatus() & ~short(PlayerState::Teleporting));

  // phased means we can pass through buildings
  const bool phased = ((location == Dead) || (location == Exploding) ||
		       (getFlag() == Flags::OscillationOverthruster) ||
		       isPhantomZoned());

  float groundLimit = 0.0f;
  if (getFlag() == Flags::Burrow)
    groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);

  // get linear and angular speed at start of time step
  if (!NEAR_ZERO(dt,ZERO_TOLERANCE)) {
    if (location == Dead || isPaused()) {
      // can't move if paused or dead -- don't move the player
      // skip location computation as it's not needed
      setAngularVelocity(0.0f);
      return;
    } else if (location == Exploding) {
      // see if explosing time has expired
      if (lastTime - getExplodeTime() >= BZDB.eval(StateDatabase::BZDB_EXPLODETIME)) {
	dt -= float((lastTime - getExplodeTime()) - BZDB.eval(StateDatabase::BZDB_EXPLODETIME));
	if (dt < 0.0f) {
	  dt = 0.0f;
	}
	setStatus(PlayerState::DeadStatus);
	location = Dead;
	if (isAutoPilot()) {
	  CMDMGR.run("restart");
	}
      }

      // can't control explosion motion
      newVelocity[2] += BZDBCache::gravity * dt;
      newAngVel = 0.0f;	// or oldAngVel to spin while exploding
    } else if ((location == OnGround) || (location == OnBuilding) ||
	       (location == InBuilding && oldPosition[2] == groundLimit)) {
      // full control
      float speed = desiredSpeed;

      // angular velocity
      newAngVel = getNewAngVel(oldAngVel, desiredAngVel);

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
	newVelocity[2] += 4 * BZDBCache::gravity * dt;
      else if (oldPosition[2] > groundLimit)
	newVelocity[2] += BZDBCache::gravity * dt;

      // save speed for next update
      lastSpeed = speed;
    } else {
      // can't control motion in air unless have wings
      if (getFlag() == Flags::Wings) {
	float speed = desiredSpeed;

	// angular velocity
	newAngVel = getNewAngVel(oldAngVel, desiredAngVel);

	// compute horizontal velocity so far
	const float slideTime = BZDB.eval(StateDatabase::BZDB_WINGSSLIDETIME);
	if (slideTime > 0.0) {
	  doSlideMotion(dt, slideTime, newAngVel, newVelocity);
	} else {
	  const float angle = oldAzimuth + 0.5f * dt * newAngVel;
	  newVelocity[0] = speed * cosf(angle);
	  newVelocity[1] = speed * sinf(angle);
	}

	newVelocity[2] += BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY) * dt;
	lastSpeed = speed;
      } else {
	newVelocity[2] += BZDBCache::gravity * dt;
	newAngVel = oldAngVel;
      }
    }


    // now apply outside forces
    doForces(dt, newVelocity, newAngVel);

    // below the ground: however I got there, creep up
    if (oldPosition[2] < groundLimit) {
      newVelocity[2] = std::max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
    }
  }

  // jump here, we allow a little change in horizontal motion
  if (wantJump) {
    doJump();
    if (!wantJump) {
      newVelocity[2] = oldVelocity[2];
      if ((lastObstacle != NULL) && !lastObstacle->isFlatTop()
	  && BZDB.isTrue(StateDatabase::BZDB_NOCLIMB)) {
	newVelocity[0] = 0.0f;
	newVelocity[1] = 0.0f;
      }
    }
  }

  // do the physics driver stuff
  const int driverId = getPhysicsDriver();
  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driverId);
  if (phydrv != NULL) {
    const float* v = phydrv->getLinearVel();

    newVelocity[2] += v[2];

    if (phydrv->getIsSlide()) {
      const float slideTime = phydrv->getSlideTime();
      doSlideMotion(dt, slideTime, newAngVel, newVelocity);
    }
    else {
      // adjust the horizontal velocity
      newVelocity[0] += v[0];
      newVelocity[1] += v[1];

      const float av = phydrv->getAngularVel();
      const float* ap = phydrv->getAngularPos();

      if (av != 0.0f) {
	// the angular velocity is in radians/sec
	newAngVel += av;
	const float dx = oldPosition[0] - ap[0];
	const float dy = oldPosition[1] - ap[1];
	newVelocity[0] -= av * dy;
	newVelocity[1] += av * dx;
      }
    }
  }
  lastObstacle = NULL;

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
  bool expel;
  const Obstacle* obstacle;
  float timeStep = dt;
  static bool stuck = false;
  if (location != Dead && location != Exploding) {
    location = OnGround;

    // anti-stuck code is useful only when alive
    // then only any 100 frames while stuck, take an action

    // try to see if we are stuck on a building
    obstacle = getHitBuilding(newPos, newAzimuth, newPos, newAzimuth,
			      phased, expel);

    if (obstacle && expel) {
      // just got stuck?
      if (!stuck) {
	stuckStartTime = TimeKeeper::getCurrent();
      }
      stuck = true;
    } else {
      // weee, we're free
      stuckStartTime = TimeKeeper::getNullTime();
      stuck = false;
    }

    // unstick if stuck for more than a half a second
    if (obstacle && stuck && TimeKeeper::getCurrent() - stuckStartTime > 0.5) {
      stuckStartTime = TimeKeeper::getSunExplodeTime();
      // we are using a maximum value on time for frame to avoid lagging problem
      setDesiredSpeed(0.25f);
      float delta = dt > 0.1f ? 0.1f : dt;
      float normalStuck[3];
      obstacle->getNormal(newPos, normalStuck);
      // use all the given speed to exit
      float movementMax = desiredSpeed * delta;

      newVelocity[0] = movementMax * normalStuck[0];
      newVelocity[1] = movementMax * normalStuck[1];
      if ((world->allowJumping() || (getFlag() == Flags::Jumping) || (getFlag() == Flags::Wings)) &&
	  (getFlag() != Flags::NoJumping))
	newVelocity[2] = movementMax * normalStuck[2];
      else
	newVelocity[2] = 0.0f;

      // exit will be in the normal direction
      newPos[0] += newVelocity[0];
      newPos[1] += newVelocity[1];
      newPos[2] += newVelocity[2];
      // compute time for all other kind of movements
      timeStep -= delta;
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
    if ((newPos[2] < groundLimit) && (newVelocity[2] < 0)) {
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
    obstacle = getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth,
			      phased, expel);

    if (!obstacle || !expel) break;

    float obstacleTop = obstacle->getPosition()[2] + obstacle->getHeight();
    if ((oldLocation != InAir) && obstacle->isFlatTop() &&
	(obstacleTop != tmpPos[2]) &&
	(obstacleTop < (tmpPos[2] + BZDB.eval(StateDatabase::BZDB_MAXBUMPHEIGHT)))) {
      newPos[0] = oldPosition[0];
      newPos[1] = oldPosition[1];
      newPos[2] = obstacleTop;

      // drive over bumps
      const Obstacle* bumpObstacle = getHitBuilding(newPos, tmpAzimuth,
						    newPos, newAzimuth,
						    phased, expel);
      if (!bumpObstacle) {
	move(newPos, getAngle());
	newPos[0] += newVelocity[0] * (dt * 0.5f);
	newPos[1] += newVelocity[1] * (dt * 0.5f);
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
      newAzimuth = tmpAzimuth + (t * newAngVel);
      newPos[0] = tmpPos[0] + (t * newVelocity[0]);
      newPos[1] = tmpPos[1] + (t * newVelocity[1]);
      newPos[2] = tmpPos[2] + (t * newVelocity[2]);
      if ((newPos[2] < groundLimit) && (newVelocity[2] < 0)) {
	newPos[2] = groundLimit;
      }

      // see if we hit anything
      bool searchExpel;
      const Obstacle* searchObstacle =
	getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth,
		       phased, searchExpel);

      if (!searchObstacle || !searchExpel) {
	// if no hit then search latter half of time step
	searchTime = t;
      } else if (searchObstacle) {
	// if we hit a building then record which one and where
	obstacle = searchObstacle;

	expel = searchExpel;
	hitAzimuth = newAzimuth;
	hitPos[0] = newPos[0];
	hitPos[1] = newPos[1];
	hitPos[2] = newPos[2];
      }
    }

    // get position just before impact
    newAzimuth = tmpAzimuth + (searchTime * newAngVel);
    newPos[0] = tmpPos[0] + (searchTime * newVelocity[0]);
    newPos[1] = tmpPos[1] + (searchTime * newVelocity[1]);
    newPos[2] = tmpPos[2] + (searchTime * newVelocity[2]);
    if (oldPosition[2] < groundLimit) {
      newVelocity[2] = std::max(newVelocity[2], -oldPosition[2] / 2.0f + 0.5f);
    }


    // record how much time is left in time step
    timeStep -= searchTime;

    // get normal at intersection.  sometimes fancy test says there's
    // no intersection but we're expecting one so, in that case, fall
    // back to simple normal calculation.
    float normal[3];
    if (!getHitNormal(obstacle, newPos, newAzimuth, hitPos, hitAzimuth, normal)) {
      obstacle->getNormal(newPos, normal);
    }

    // check for being on a building
    if ((newPos[2] > 0.0f) && (normal[2] > 0.001f)) {
      if (location != Dead && location != Exploding && expel) {
	location = OnBuilding;
	lastObstacle = obstacle;
      }
      newVelocity[2] = 0.0f;
    } else {
      // get component of velocity in normal direction (in horizontal plane)
      float mag = (normal[0] * newVelocity[0]) +
		  (normal[1] * newVelocity[1]);

      // handle upward normal component to prevent an upward force
      if (!NEAR_ZERO(normal[2], ZERO_TOLERANCE)) {
	// if going down then stop falling
	if (newVelocity[2] < 0.0f && newVelocity[2] -
	    (mag + normal[2] * newVelocity[2]) * normal[2] > 0.0f) {
	  newVelocity[2] = 0.0f;
	}

	// normalize force magnitude in horizontal plane
	float horNormal = normal[0] * normal[0] + normal[1] * normal[1];
	if (!NEAR_ZERO(horNormal, ZERO_TOLERANCE)) {
	  mag /= horNormal;
	}
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
    if (obstacle && (!expel || stuck)) {
      location = InBuilding;
    }
    else if (newPos[2] > 0.0f) {
      location = InAir;
    }
  }

  // see if we're crossing a wall
  if (location == InBuilding && getFlag() == Flags::OscillationOverthruster) {
    if (obstacle->isCrossing(newPos, newAzimuth,
			     0.5f * BZDBCache::tankLength,
			     0.5f * BZDBCache::tankWidth,
			     BZDBCache::tankHeight, NULL)) {
      setStatus(getStatus() | int(PlayerState::CrossingWall));
    } else {
      setStatus(getStatus() & ~int(PlayerState::CrossingWall));
    }
  } else if (world->crossingTeleporter( newPos, newAzimuth,
		      0.5f * BZDBCache::tankLength,
		      0.5f * BZDBCache::tankWidth,
		      BZDBCache::tankHeight, crossingPlane)) {
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
  const Teleporter* teleporter;
  if (!isAlive()) {
    teleporter = NULL;
  } else {
    teleporter = world->crossesTeleporter(oldPosition, newPos, face);
  }

  if (teleporter) {
    if (getFlag() == Flags::PhantomZone) {
      // change zoned state
      setStatus(getStatus() ^ PlayerState::PhantomZoned);
      setStatus(getStatus() ^ PlayerState::FlagActive);
      if (gettingSound) {
	playLocalSound(SFX_PHANTOM);
      }
    } else {
      // teleport
      const int source = world->getTeleporter(teleporter, face);
      int targetTele = world->getTeleportTarget(source);

      int outFace;
      const Teleporter* outPort = world->getTeleporter(targetTele, outFace);
      teleporter->getPointWRT(*outPort, face, outFace,
			      newPos, newVelocity, newAzimuth,
			      newPos, newVelocity, &newAzimuth);

      // check for a hit on the other side
      const Obstacle* teleObs =
	getHitBuilding(newPos, newAzimuth, newPos, newAzimuth, phased, expel);
      if (teleObs != NULL) {
	// revert
	memcpy (newPos, oldPosition, sizeof(float[3]));
	newVelocity[0] = newVelocity[1] = 0.0f;
	newVelocity[2] = oldVelocity[2];
	newAzimuth = oldAzimuth;
      } else {
	// save teleport info
	setTeleport(lastTime, source, targetTele);
	server->sendTeleport(source, targetTele);
	if (gettingSound) {
	  playLocalSound(SFX_TELEPORT);
	}
      }
    }
  }

  // setup the physics driver
  setPhysicsDriver(-1);
  if ((lastObstacle != NULL) &&
      (lastObstacle->getType() == MeshFace::getClassName())) {
    const MeshFace* meshFace = (const MeshFace*) lastObstacle;
    int driverIdent = meshFace->getPhysicsDriver();
    const PhysicsDriver* phydriver = PHYDRVMGR.getDriver(driverIdent);
    if (phydriver != NULL) {
      setPhysicsDriver(driverIdent);
    }
  }

  // deal with drop sounds and effects
  if (entryDrop) {
    // because the starting position that the server sends can result
    // in an initial InAir condition, we use this bool to avoid having
    // a false jump mess with the spawnEffect()
    // FIXME: this isn't a clean way to do it
    if ((oldLocation == InAir) == (location == InAir)) {
      entryDrop = false;
    }
  }

  const bool justLanded =
    (oldLocation == InAir) &&
    ((location == OnGround) || (location == OnBuilding));

  if (justLanded) {
    setLandingSpeed(oldVelocity[2]);
    if (!headless) {
      EFFECTS.addLandEffect(getColor(),newPos,getAngle());
    }
  }
  if (gettingSound) {
    const PhysicsDriver* phydriver = PHYDRVMGR.getDriver(getPhysicsDriver());
    if ((phydriver != NULL) && (phydriver->getLinearVel()[2] > 0.0f)) {
      playLocalSound(SFX_BOUNCE);
      addRemoteSound(PlayerState::BounceSound);
    } else if (justLanded && !entryDrop) {
      playLocalSound(SFX_LAND);
    } else if ((location == OnGround) &&
	       (oldPosition[2] == 0.0f) && (newPos[2] < 0.f)) {
      playLocalSound(SFX_BURROW);
    }
  }

  // set falling status
  if (location == OnGround || location == OnBuilding ||
      (location == InBuilding && newPos[2] == 0.0f)) {
    setStatus(getStatus() & ~short(PlayerState::Falling));
  }
  else if (location == InAir || location == InBuilding) {
    setStatus(getStatus() | short(PlayerState::Falling));
  }
  
  // set InBuilding status
  if (location == InBuilding) {
    setStatus(getStatus() | PlayerState::InBuilding);
  } else {
    setStatus(getStatus() & ~PlayerState::InBuilding);
  }

  // set UserInput status (determines how animated treads are drawn)
  const PhysicsDriver* phydrv2 = PHYDRVMGR.getDriver(getPhysicsDriver());
  if (((phydrv2 != NULL) && phydrv2->getIsSlide()) ||
      ((getFlag() == Flags::Wings) && (location == InAir) &&
       (BZDB.eval(StateDatabase::BZDB_WINGSSLIDETIME) > 0.0f))) {
    setStatus(getStatus() | short(PlayerState::UserInputs));
  } else {
    setStatus(getStatus() & ~short(PlayerState::UserInputs));
  }

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
    if (isPhantomZoned())
      firingStatus = Zoned;
    else if ((getReloadTime() > 0.0f) && (getFlag() != Flags::TriggerHappy))
      firingStatus = Loading;
    else
      firingStatus = Ready;
    break;
  }

  // burrowed and oscillating tanks get some resistance in their joystick
  // if they have ff on
  if ((location == InBuilding) || (newPos[2] < -0.5f))
    ForceFeedback::solidMatterFriction();

  // calculate the list of inside buildings
  insideBuildings.clear();
  if (location == InBuilding) {
    collectInsideBuildings();
  }

  // move tank
  move(newPos, newAzimuth);
  setVelocity(newVelocity);
  setAngularVelocity(newAngVel);
  setRelativeMotion();
  newAzimuth = getAngle(); // pickup the limited angle range from move()

  // see if I'm over my antidote
  if (antidoteFlag && location == OnGround) {
    float dist =
      ((flagAntidotePos[0] - newPos[0]) * (flagAntidotePos[0] - newPos[0])) +
      ((flagAntidotePos[1] - newPos[1]) * (flagAntidotePos[1] - newPos[1]));
    const float twoRads = getRadius() + BZDBCache::flagRadius;
    if (dist < (twoRads * twoRads)) {
      server->sendDropFlag(getPosition());
    }
  }

  if ((getFlag() == Flags::Bouncy) && ((location == OnGround) || (location == OnBuilding))) {
    if (oldLocation != InAir) {
      if ((TimeKeeper::getCurrent() - bounceTime) > 0) {
	doJump();
      }
    }
    else {
      bounceTime = TimeKeeper::getCurrent();
      bounceTime += 0.2f;
    }
  }

  if (gettingSound) {
    if (oldPosition[0] != newPos[0] || oldPosition[1] != newPos[1] ||
	oldPosition[2] != newPos[2] || oldAzimuth != newAzimuth) {
      moveSoundReceiver(newPos[0], newPos[1], newPos[2], newAzimuth,
			NEAR_ZERO(dt, ZERO_TOLERANCE) ||
			((teleporter != NULL) && (getFlag() != Flags::PhantomZone)));
    }
    if (NEAR_ZERO(dt, ZERO_TOLERANCE)) {
      speedSoundReceiver(newVelocity[0], newVelocity[1], newVelocity[2]);
    } else {
      speedSoundReceiver((newPos[0] - oldPosition[0]) / dt,
			 (newPos[1] - oldPosition[1]) / dt,
			 (newPos[2] - oldPosition[2]) / dt);
    }
  }
}


const Obstacle* LocalPlayer::getHitBuilding(const float* p, float a,
					    bool phased, bool& expel) const
{
  const bool hasOOflag = getFlag() == Flags::OscillationOverthruster;
  const float* dims = getDimensions();
  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }
  const Obstacle* obstacle = world->hitBuilding(p, a, dims[0], dims[1], dims[2]);

  expel = (obstacle != NULL);
  if (expel && phased)
    expel = (obstacle->getType() == WallObstacle::getClassName() ||
		obstacle->getType() == Teleporter::getClassName() ||
		(hasOOflag && desiredSpeed < 0.0f && NEAR_ZERO(p[2], ZERO_TOLERANCE)));
  return obstacle;
}


const Obstacle* LocalPlayer::getHitBuilding(const float* oldP, float oldA,
					    const float* p, float a,
					    bool phased, bool& expel)
{
  const bool hasOOflag = getFlag() == Flags::OscillationOverthruster;
  const float* dims = getDimensions();
  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }
  const Obstacle* obstacle = world->hitBuilding(oldP, oldA, p, a, dims[0], dims[1], dims[2], !hasOOflag);

  expel = (obstacle != NULL);
  if (expel && phased)
    expel = (obstacle->getType() == WallObstacle::getClassName() ||
		obstacle->getType() == Teleporter::getClassName() ||
		(hasOOflag && desiredSpeed < 0.0f && NEAR_ZERO(p[2], ZERO_TOLERANCE)));

  if (obstacle != NULL) {
    if (obstacle->getType() == MeshFace::getClassName()) {
      const MeshFace* face = (const MeshFace*) obstacle;
      const int driver = face->getPhysicsDriver();
      const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
      if ((phydrv != NULL) && phydrv->getIsDeath()) {
	deathPhyDrv = driver;
      }
    }
  }

  return obstacle;
}


bool LocalPlayer::getHitNormal(const Obstacle* o,
			       const float* pos1, float azimuth1,
			       const float* pos2, float azimuth2,
			       float* normal) const
{
  const float* dims = getDimensions();
  return o->getHitNormal(pos1, azimuth1, pos2, azimuth2,
			 dims[0], dims[1], dims[2], normal);
}


static bool notInObstacleList(const Obstacle* obs,
			      const std::vector<const Obstacle*>& list)
{
  for (unsigned int i = 0; i < list.size(); i++) {
    if (obs == list[i]) {
      return false;
    }
  }
  return true;
}


void LocalPlayer::collectInsideBuildings()
{
  const float* pos = getPosition();
  const float angle = getAngle();
  const float* dims = getDimensions();

  // get the list of possible inside buildings
  const ObsList* olist =
    COLLISIONMGR.boxTest (pos, angle, dims[0], dims[1], dims[2]);

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inBox(pos, angle, dims[0], dims[1], dims[2])) {
      if (obs->getType() == MeshFace::getClassName()) {
	const MeshFace* face = (const MeshFace*) obs;
	const MeshObstacle* mesh = (const MeshObstacle*) face->getMesh();
	// check it for the death touch
	if (deathPhyDrv < 0) {
	  const int driver = face->getPhysicsDriver();
	  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
	  if ((phydrv != NULL) && (phydrv->getIsDeath())) {
	    deathPhyDrv = driver;
	  }
	}
	// add the mesh if not already present
	if (!obs->isDriveThrough() &&
	    notInObstacleList(mesh, insideBuildings)) {
	  insideBuildings.push_back(mesh);
	}
      }
      else if (!obs->isDriveThrough()) {
	if (obs->getType() == MeshObstacle::getClassName()) {
	  // add the mesh if not already present
	  if (notInObstacleList(obs, insideBuildings)) {
	    insideBuildings.push_back(obs);
	  }
	} else {
	  insideBuildings.push_back(obs);
	}
      }
    }
  }

  return;
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

void			LocalPlayer::restart(const float* pos, float _azimuth)
{
  // put me in limbo
  setStatus(short(PlayerState::DeadStatus));

  // can't have a flag
  setFlag(Flags::Null);

  // get rid of existing shots
  const int numShots = getMaxShots();
  // get rid of existing shots
  for (int i = 0; i < numShots; i++)
    if (shots[i]) {
      delete shots[i];
      shots[i] = NULL;
    }
  anyShotActive = false;

  // no target
  target = NULL;

  // no death
  deathPhyDrv = -1;

  // initialize position/speed state
  static const float zero[3] = { 0.0f, 0.0f, 0.0f };
  location = (pos[2] > 0.0f) ? OnBuilding : OnGround;
  lastObstacle = NULL;
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
  wantJump = false;
  doUpdateMotion(0.0f);
  entryDrop = true;

  // make me alive now
  setStatus(getStatus() | short(PlayerState::Alive));
}

void			LocalPlayer::setTeam(TeamColor _team)
{
  changeTeam(_team);
}

void			LocalPlayer::setDesiredSpeed(float fracOfMaxSpeed)
{
  FlagType* flag = getFlag();

  // If we aren't allowed to move, then the desired speed is 0.
  if (!canMove()) {
    fracOfMaxSpeed = 0.0;
  }

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
      float oldFrac = desiredSpeed / BZDBCache::tankSpeed;
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
  fracOfMaxSpeed *= (1.0f + (handicap * (BZDB.eval(StateDatabase::BZDB_HANDICAPVELAD) - 1.0f)));

  // set desired speed
  desiredSpeed = BZDBCache::tankSpeed * fracOfMaxSpeed;
  Player::setUserSpeed(desiredSpeed);

  return;
}


void			LocalPlayer::setDesiredAngVel(float fracOfMaxAngVel)
{
  FlagType* flag = getFlag();

  // If we aren't allowed to move, then the desired angular velocity is 0.
  if (!canMove()) {
    fracOfMaxAngVel = 0.0;
  }

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

  // apply handicap advantage to tank speed
  fracOfMaxAngVel *= (1.0f + (handicap * (BZDB.eval(StateDatabase::BZDB_HANDICAPANGAD) - 1.0f)));

  // set desired turn speed
  desiredAngVel = fracOfMaxAngVel * BZDB.eval(StateDatabase::BZDB_TANKANGVEL);
  Player::setUserAngVel(desiredAngVel);

  return;
}


void			LocalPlayer::setPause(bool pause)
{
  if (isAlive()) {
    if (pause && !isPaused()) {
      setStatus(getStatus() | short(PlayerState::Paused));
      // sendPaused is done in clientCommands
    }
    else if (!pause && isPaused()) {
      setStatus(getStatus() & ~short(PlayerState::Paused));
      server->sendPaused(false);
    }
  }
}

void			LocalPlayer::activateAutoPilot(bool autopilot)
{
  if (autopilot && !isAutoPilot()) {
    setAutoPilot();
    server->sendAutoPilot(true);
  }
  else if (!autopilot && isAutoPilot()) {
    setAutoPilot(false);
    server->sendAutoPilot(false);
  }
}

bool			LocalPlayer::fireShot()
{
  if (! (firingStatus == Ready || firingStatus == Zoned))
    return false;

  if (! canShoot())
    return false;

  // find an empty slot
  const int numShots = getMaxShots();
  int i;
  for (i = 0; i < numShots; i++)
    if (!shots[i])
      break;
  if (i == numShots) return false;

  // make sure we're allowed to shoot
  if (!isAlive() || isPaused() ||
      ((location == InBuilding) && !isPhantomZoned())) {
    return false;
  }

  // prepare shot
  FiringInfo firingInfo;
  firingInfo.shot.player = getId();
  firingInfo.shot.id     = uint16_t(i + getSalt());
  prepareShotInfo(firingInfo);
  // make shot and put it in the table
  addShot(new LocalShotPath(firingInfo), firingInfo);

  // Insert timestamp, useful for dead reckoning jitter fixing
  const float timeStamp = float(TimeKeeper::getCurrent() - TimeKeeper::getNullTime());
  firingInfo.timeSent = timeStamp;

  // always send a player-update message. To synchronize movement and
  // shot start. They should generally travel on the same frame, when
  // flushing the output queues.
  server->sendPlayerUpdate(this);
  server->sendBeginShot(firingInfo);

  if (BZDB.isTrue("enableLocalShotEffect") && SceneRenderer::instance().useQuality() >= _MEDIUM_QUALITY)
    EFFECTS.addShotEffect(getColor(), firingInfo.shot.pos, getAngle(), getVelocity());

  if (gettingSound) {
    if (firingInfo.flagType == Flags::ShockWave) {
      playLocalSound(SFX_SHOCK);
      ForceFeedback::shockwaveFired();
    }
    else if (firingInfo.flagType == Flags::Laser) {
      playLocalSound(SFX_LASER);
      ForceFeedback::laserFired();
    }
    else if (firingInfo.flagType == Flags::GuidedMissile) {
      playLocalSound(SFX_MISSILE);
      ForceFeedback::shotFired();
    }
    else if (firingInfo.flagType == Flags::Thief) {
      playLocalSound(SFX_THIEF);
      ForceFeedback::shotFired();
    }
    else {
      playLocalSound(SFX_FIRE);
      ForceFeedback::shotFired();
    }
  }

  if (getFlag() == Flags::TriggerHappy) {
    // make sure all the shots don't go off at once
    forceReload(BZDB.eval(StateDatabase::BZDB_RELOADTIME) / numShots);
  }
  return true;
}



bool LocalPlayer::doEndShot(int ident, bool isHit, float* pos)
{
  const int index = ident & 255;
  const int slt   = (ident >> 8) & 127;

  // special id used in some messages (and really shouldn't be sent here)
  if (ident == -1)
    return false;

  // ignore bogus shots (those with a bad index or for shots that don't exist)
  if (index < 0 || index >= getMaxShots() || !shots[index])
    return false;

  // ignore shots that already ending
  if (shots[index]->isExpired() || shots[index]->isExpiring())
    return false;

  // ignore shots that have the wrong salt.  since we reuse shot indices
  // it's possible for an old MsgShotEnd to arrive after we've started a
  // new shot.  that's where the salt comes in.  it changes for each shot
  // so we can identify an old shot from a new one.
  if (slt != ((shots[index]->getShotId() >> 8) & 127))
    return false;

  // keep shot statistics
  shotStatistics.recordHit(shots[index]->getFlag());

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

void			LocalPlayer::setJump()
{
  wantJump = jumpPressed;
}

void			LocalPlayer::setJumpPressed(bool value)
{
  jumpPressed = value;
}

void			LocalPlayer::doJump()
{
  FlagType* flag = getFlag();
  World *world = World::getWorld();
  if (!world) {
    return;
  }

  // can't jump while burrowed
  if (getPosition()[2] < 0.0f) {
    return;
  }

  if (flag == Flags::Wings) {
    if (wingsFlapCount <= 0) {
      return;
    }
    wingsFlapCount--;
  } else if ((location != OnGround) && (location != OnBuilding)) {
    // can't jump unless on the ground or a building
    if (flag != Flags::Wings)
      return;
    if (wingsFlapCount <= 0)
      return;
    wingsFlapCount--;
  } else if ((flag != Flags::Bouncy) &&
	     ((flag != Flags::Jumping && !world->allowJumping()) ||
	      (flag == Flags::NoJumping))) {
    return;
  }

  // add jump velocity (actually, set the vertical component since you
  // can only jump if resting on something)
  const float* oldVelocity = getVelocity();
  float newVelocity[3];
  newVelocity[0] = oldVelocity[0];
  newVelocity[1] = oldVelocity[1];
  if (flag == Flags::Wings) {
    newVelocity[2] = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
  } else if (flag == Flags::Bouncy) {
    const float factor = 0.25f + ((float)bzfrand() * 0.75f);
    newVelocity[2] = factor * BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
  }  else {
    newVelocity[2] = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
  }
  setVelocity(newVelocity);
  location = InAir;

  // setup the graphics
  fireJumpJets();

  // setup the sound
  if (gettingSound) {
    if (flag == Flags::Wings) {
      playLocalSound(SFX_FLAP);
      addRemoteSound(PlayerState::WingsSound);
    } else {
      playLocalSound(SFX_JUMP);
      addRemoteSound(PlayerState::JumpSound);
    }
  }

  wantJump = false;
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
  float gravity      = BZDBCache::gravity;
  float explodeTim   = BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
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
    newVelocity[2] = - 0.5f * gravity * explodeTim;
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
  float linearAcc = (getFlag() == Flags::Momentum)
    ? BZDB.eval(StateDatabase::BZDB_MOMENTUMLINACC)
    : BZDB.eval(StateDatabase::BZDB_INERTIALINEAR);
  float angularAcc = (getFlag() == Flags::Momentum)
    ? BZDB.eval(StateDatabase::BZDB_MOMENTUMANGACC)
    : BZDB.eval(StateDatabase::BZDB_INERTIAANGULAR);

  // limit linear acceleration
  if (linearAcc > 0.0f) {
    const float acc = (speed - lastSpeed) / dt;
    if (acc > 20.0f * linearAcc) speed = lastSpeed + dt * 20.0f*linearAcc;
    else if (acc < -20.0f * linearAcc) speed = lastSpeed - dt * 20.0f*linearAcc;
  }

  // limit angular acceleration
  if (angularAcc > 0.0f) {
    const float oldAngVel = getAngularVelocity();
    const float angAcc = (angVel - oldAngVel) / dt;
    if (angAcc > angularAcc) angVel = oldAngVel + dt * angularAcc;
    else if (angAcc < -angularAcc) angVel = oldAngVel - dt * angularAcc;
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

	// if no team kills, shots of my team can't kill me
	if ( shot->getTeam() != RogueTeam && !World::getWorld()->allowTeamKills() && shot->getTeam() == getTeam())
		 continue;

    // short circuit test if shot can't possibly hit.
    // only superbullet or shockwave can kill zoned dude
    const FlagType* shotFlag = shot->getFlag();
    if (isPhantomZoned() &&
	(shotFlag != Flags::ShockWave) &&
	(shotFlag != Flags::SuperBullet) &&
	(shotFlag != Flags::PhantomZone))
      continue;

    // laser can't hit a cloaked tank
    if ((getFlag() == Flags::Cloaking) && (shotFlag == Flags::Laser))
      continue;

    // zoned shots only kill zoned tanks
    if ((shotFlag == Flags::PhantomZone) && !isPhantomZoned()) {
      continue;
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

bool		LocalPlayer::checkCollision(const Player* otherTank)
{
  if (!otherTank) return false;

  TimeKeeper current = TimeKeeper::getTick();
  // Don't flood the network with MsgCollide
  if (current - lastCollisionTime < BZDBCache::collisionLimit) {
    return false;
  }

  const float *myPosition = getPosition();
  const float *otherPosition = otherTank->getPosition();

  float dx = otherPosition[0] - myPosition[0];
  float dy = otherPosition[1] - myPosition[1];

  float dist = sqrt(dx * dx + dy * dy);
  float radius = BZDBCache::freezeTagRadius;

  if (dist < radius) {
    server->sendCollide(getId(), otherTank->getId(), myPosition);
    lastCollisionTime = current;
    return true;
  } else {
    return false;
  }
}

void			LocalPlayer::setFlag(FlagType* flag)
{
  Player::setFlag(flag);
  World *world = World::getWorld();
  if (!world) {
    return;
  }

  float worldSize = BZDBCache::worldSize;
  // if it's bad then reset countdowns and set antidote flag
  if (getFlag() != Flags::Null && getFlag()->endurance == FlagSticky) {
    if (world->allowShakeTimeout())
      flagShakingTime = world->getFlagShakeTimeout();
    if (world->allowShakeWins())
      flagShakingWins = world->getFlagShakeWins();
    if (world->allowAntidote()) {
      float tankRadius = BZDBCache::tankRadius;
      float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);
      do {
	if (world->allowTeamFlags()) {
	  flagAntidotePos[0] = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[1] = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[2] = 0.0f;
	} else {
	  flagAntidotePos[0] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[1] = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	  flagAntidotePos[2] = 0.0f;
	}
      } while (world->inBuilding(flagAntidotePos, tankRadius,
					     BZDBCache::tankHeight));
      antidoteFlag = new FlagSceneNode(flagAntidotePos);
      antidoteFlag->setColor(1.0f, 1.0f, 0.0f, 1.0f); // yellow
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
  World *world = World::getWorld();
  if (!world) {
    return;
  }
  if (deltaWins > 0 && world->allowShakeWins() &&
      flagShakingWins > 0) {
    flagShakingWins -= deltaWins;
    if (flagShakingWins <= 0) {
      flagShakingWins = 0;
      server->sendDropFlag(getPosition());
    }
  }
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

