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

/* interface header */
#include "LocalPlayer.h"

/* common implementation headers */
#include "bzfio.h"
#include "BZDBCache.h"
#include "BzfEvent.h"
#include "CollisionManager.h"
#include "CommandManager.h"
#include "FlagSceneNode.h"
#include "MeshObstacle.h"
#include "PhysicsDriver.h"
#include "TextUtils.h"
#include "WallObstacle.h"
#include "LinkManager.h"
#include "LinkPhysics.h"

/* local implementation headers */
#include "World.h"
#include "sound.h"
#include "ForceFeedback.h"
#include "EffectsRenderer.h"
#include "SyncClock.h"
#include "ClientIntangibilityManager.h"
#include "MotionUtils.h"
#include "playing.h"


static BZDB_int debugTele  ("debugTele");
static BZDB_int debugMotion("debugMotion");


LocalPlayer* LocalPlayer::mainPlayer = NULL;


//============================================================================//

LocalPlayer::LocalPlayer(const PlayerId& _id,
			 const char* name,
			 const PlayerType _type)
: BaseLocalPlayer(_id, name, _type)
, gettingSound(true)
, server(ServerLink::getServer())
, location(Dead)
, firingStatus(Deceased)
, flagShakingTime(0.0f)
, flagShakingWins(0)
, antidoteFlag(NULL)
, desiredSpeed(0.0f)
, desiredAngVel(0.0f)
, lastSpeed(0.0f)
, anyShotActive(false)
, target(NULL)
, nemesis(NULL)
, recipient(NULL)
, inputChanged(false)
, spawning(false)
, wingsFlapCount(0)
, left(false)
, right(false)
, up(false)
, down(false)
, entryDrop(true)
, wantJump(false)
, jumpPressed(false)
, deathPhyDrv(-1)
, hitWall(false)
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

  stuckStartTime = TimeKeeper::getNullTime();

  requestedAutopilot = false;
}


LocalPlayer::~LocalPlayer()
{
  // free antidote flag
  delete antidoteFlag;
}


void LocalPlayer::setMyTank(LocalPlayer* player)
{
  mainPlayer = player;
}


//============================================================================//

void LocalPlayer::doUpdate(float dt)
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
  }
  else {
    pauseTime = TimeKeeper::getNullTime();
    wasPaused = false;
  }

  // reap dead (reloaded) shots
  for (i = 0; i < numShots; i++) {
    if (shots[i] && shots[i]->isReloaded()) {
      if (!shots[i]->isExpired())
	shots[i]->setExpired();
      delete shots[i];
      shots[i] = NULL;
    }
  }

  // update shots
  anyShotActive = false;
  for (i = 0; i < numShots; i++) {
    if (shots[i]) {
      shots[i]->update(dt);
      if (!shots[i]->isExpired()) anyShotActive = true;
    }
  }

  // if no shots now out (but there had been) then reset target
  if (!anyShotActive && hadShotActive) {
    target = NULL;
  }

  // drop bad flag if timeout has expired
  World *world = World::getWorld();
  if (!isPaused() && dt > 0.0f && world && world->allowShakeTimeout() &&
      getFlag() != Flags::Null && getFlag()->endurance == FlagSticky &&
      flagShakingTime > 0.0f) {
    flagShakingTime -= dt;
    if (flagShakingTime <= 0.0f) {
      flagShakingTime = 0.0f;
      server->sendDropFlag(getPosition());
      setShotType(StandardShot);
    }
  }
}


void LocalPlayer::doSlideMotion(float dt, float slideTime,
				float newAngVel, fvec3& newVelocity)
{
  const float oldAzimuth = getAngle();
  const fvec3& oldVelocity = getVelocity();

  const float angle = oldAzimuth + (0.5f * dt * newAngVel);
  const float cos_val = cosf(angle);
  const float sin_val = sinf(angle);

  const float scale = (dt / slideTime);
  const float speedAdj = desiredSpeed * scale;

  const fvec2& ov = oldVelocity.xy();
  fvec2&       nv = newVelocity.xy();

  const float oldSpeed = ov.length();
  nv = ov;
  nv += speedAdj * fvec2(cos_val, sin_val);
  const float newSpeed = nv.length();

  const float maxSpeed = getMaxSpeed();

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
    nv *= (adjSpeed / newSpeed);
  }
  return;
}


float LocalPlayer::getNewAngVel(float old, float desired, float dt)
{
  if (getPhysicsDriver() >= 0) {
    return desired;
  }
  return computeAngleVelocity(old, desired, dt);
}


//============================================================================//

void LocalPlayer::doUpdateMotion(float dt)
{
  static const float MinSearchStep = 0.0001f;
  static const int   MaxSearchSteps = 7;
  static const int   MaxSteps = 4;
  static const float TinyDistance = 0.0001f;

  // save old state
  const Location oldLocation = location;
  const fvec3&   oldPosition = getPosition();
  const fvec3&   oldVelocity = getVelocity();
  const float    oldAzimuth  = getAngle();
  const float    oldAngVel   = getAngularVelocity();

  if (debugMotion >= 1) {
    const std::string locationString = getLocationString(location);
    logDebugMessage(0, "doUpdateMotion: %f\n", dt);
    logDebugMessage(0, "  location = %s\n", locationString.c_str());
    logDebugMessage(0, "  pos = %s\n", oldPosition.tostring().c_str());
    logDebugMessage(0, "  vel = %s\n", oldVelocity.tostring().c_str());
    logDebugMessage(0, "  angle  = %f\n", oldAzimuth);
    logDebugMessage(0, "  angvel = %f\n", oldAngVel);
  }

  // prepare new state
  fvec3 newVelocity(oldVelocity);
  float newAngVel = 0.0f;

  if (!headless) {
    clearRemoteSounds();
  }
  World* world = World::getWorld();
  if (!world) {
    return; /* no world, no motion */
  }

  // if was teleporting and exceeded teleport time then not teleporting anymore
  static BZDB_float bzdbTeleTime("_teleportTime"); // aka BZDB_TELEPORTTIME
  if (isTeleporting() && ((lastTime - getTeleportTime()) >= bzdbTeleTime)) {
    setStatus(getStatus() & ~short(PlayerState::Teleporting));
  }

  // phased means we can pass through buildings
  const bool phased = ((location == Dead) || (location == Exploding) ||
		       (getFlag() == Flags::OscillationOverthruster) ||
		       isPhantomZoned());

  float groundLimit = computeGroundLimit(getFlag());

  // get linear and angular speed at start of time step
  if (!NEAR_ZERO(dt, ZERO_TOLERANCE)) {
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
      newVelocity.z += BZDBCache::gravity * dt;
      newAngVel = 0.0f;	// or oldAngVel to spin while exploding
    } else if ((location == OnGround) || (location == OnBuilding) ||
	       ((location == InBuilding) && (oldPosition.z == groundLimit))) {
      // full control
      float speed = desiredSpeed;

      // angular velocity
      newAngVel = getNewAngVel(oldAngVel, desiredAngVel, dt);

      // limit acceleration
      doMomentum(dt, speed, newAngVel);

      // compute velocity so far
      const float angle = oldAzimuth + 0.5f * dt * newAngVel;
      newVelocity.x = speed * cosf(angle);
      newVelocity.y = speed * sinf(angle);
      newVelocity.z = 0.0f;

      // now friction, if any
      doFriction(dt, oldVelocity, newVelocity);

      // reset our flap count if we have wings
      if (getFlag() == Flags::Wings) {
	wingsFlapCount = (int) BZDB.eval(StateDatabase::BZDB_WINGSJUMPCOUNT);
      }

      if ((oldPosition.z < 0.0f) && (getFlag() == Flags::Burrow)) {
	newVelocity.z += 4 * BZDBCache::gravity * dt;
      } else if (oldPosition.z > groundLimit) {
	newVelocity.z += BZDBCache::gravity * dt;
      }

      // save speed for next update
      lastSpeed = speed;
    }
    else {
      // can't control motion in air unless have wings
      if (getFlag() == Flags::Wings) {
	float speed = desiredSpeed;

	// angular velocity
	newAngVel = getNewAngVel(oldAngVel, desiredAngVel, dt);

	// compute horizontal velocity so far
	const float slideTime = BZDB.eval(StateDatabase::BZDB_WINGSSLIDETIME);
	if (slideTime > 0.0) {
	  doSlideMotion(dt, slideTime, newAngVel, newVelocity);
	} else {
	  const float angle = oldAzimuth + 0.5f * dt * newAngVel;
	  newVelocity.x = speed * cosf(angle);
	  newVelocity.y = speed * sinf(angle);
	}

	newVelocity.z += BZDB.eval(StateDatabase::BZDB_WINGSGRAVITY) * dt;
	lastSpeed = speed;
      } else if (getFlag() == Flags::LowGravity) {
  	newVelocity.z += BZDB.eval(StateDatabase::BZDB_LGGRAVITY) * dt;
	newAngVel = oldAngVel;
      } else {
	newVelocity.z += BZDBCache::gravity * dt;
	newAngVel = oldAngVel;
      }
    }

    // now apply outside forces
    doForces(dt, newVelocity, newAngVel);

    // below the ground: however I got there, creep up
    if (oldPosition.z < groundLimit) {
      newVelocity.z = std::max(newVelocity.z, -oldPosition.z / 2.0f + 0.5f);
    }
  }

  // jump here, we allow a little change in horizontal motion
  if (wantJump) {
    doJump();
    if (!wantJump) {
      newVelocity.z = oldVelocity.z;
      if ((lastObstacle != NULL) && !lastObstacle->isFlatTop()
	  && BZDB.isTrue(StateDatabase::BZDB_NOCLIMB)) {
	newVelocity.x = 0.0f;
	newVelocity.y = 0.0f;
      }
    }
  }

  // do the physics driver stuff
  const int driverId = getPhysicsDriver();
  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driverId);
  if (phydrv != NULL) {
    const fvec3& v = phydrv->getLinearVel();

    newVelocity.z += v.z;

    if (phydrv->getIsSlide()) {
      const float slideTime = phydrv->getSlideTime();
      doSlideMotion(dt, slideTime, newAngVel, newVelocity);
    } else {
      // adjust the horizontal velocity
      newVelocity.x += v.x;
      newVelocity.y += v.y;

      const float av = phydrv->getAngularVel();
      const fvec2& ap = phydrv->getAngularPos();

      if (av != 0.0f) {
	// the angular velocity is in radians/sec
	newAngVel += av;
	const float dx = oldPosition.x - ap.x;
	const float dy = oldPosition.y - ap.y;
	newVelocity.x -= av * dy;
	newVelocity.y += av * dx;
      }
    }
  }
  lastObstacle = NULL;

  // get new position so far (which is just the current position)
  fvec3 newPos(oldPosition);
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
    // then only any 0.5 seconds while stuck, take an action

    // try to see if we are stuck on a building
    obstacle = getHitBuilding(newPos, newAzimuth, newPos, newAzimuth,
			      phased, expel);

    if (obstacle && expel) {
      // just got stuck?
      if (!stuck) {
	stuckStartTime = TimeKeeper::getCurrent();
	stuck = true;
      }
    } else {
      // weee, we're free
      stuckStartTime = TimeKeeper::getNullTime();
      stuck = false;
    }

    // unstick if stuck for more than a half a second
    if (obstacle && stuck && (TimeKeeper::getCurrent() - stuckStartTime > 0.5)) {
      // reset stuckStartTime in order to have this if-construct being executed
      // more than 1 time as often more iterations are needed to get unstuck
      stuckStartTime = TimeKeeper::getCurrent();
      // we are using a maximum value on time for frame to avoid lagging problem
      setDesiredSpeed(0.25f);
      float delta = (dt > 0.1f) ? 0.1f : dt;
      fvec3 normalStuck;
      obstacle->getNormal(newPos, normalStuck);
      // use all the given speed to exit
      float movementMax = desiredSpeed * delta;

      newVelocity.x = movementMax * normalStuck.x;
      newVelocity.y = movementMax * normalStuck.y;
      if (canJump())
	newVelocity.z = movementMax * normalStuck.z;
      else
	newVelocity.z = 0.0f;

      // exit will be in the normal direction
      newPos += newVelocity;
      // compute time for all other kind of movements
      timeStep -= delta;
    }
  }

  hitWall = false;
  for (int numSteps = 0; numSteps < MaxSteps; numSteps++) {
    // record position at beginning of time step
    fvec3 tmpPos = newPos;
    float tmpAzimuth = newAzimuth;

    // get position at end of time step
    newAzimuth = tmpAzimuth + timeStep * newAngVel;
    newPos = tmpPos + (timeStep * newVelocity);
    if ((newPos.z < groundLimit) && (newVelocity.z < 0)) {
      // Hit lower limit, stop falling
      newPos.z = groundLimit;
      if (location == Exploding) {
	// tank pieces reach the ground, friction
	// stop them, & mainly player view
	newPos.x = tmpPos.x;
	newPos.y = tmpPos.y;
      }
    }

    // see if we hit anything.  if not then we're done.
    obstacle = getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth,
			      phased, expel);

    if (!obstacle || !expel) {
      break;
    }

    float obstacleTop = obstacle->getPosition().z + obstacle->getHeight();
    if ((oldLocation != InAir) && obstacle->isFlatTop() &&
	(obstacleTop != tmpPos.z) &&
	(obstacleTop < (tmpPos.z + BZDB.eval(StateDatabase::BZDB_MAXBUMPHEIGHT)))) {
      newPos.x = oldPosition.x;
      newPos.y = oldPosition.y;
      newPos.z = obstacleTop;

      // drive over bumps
      const Obstacle* bumpObstacle = getHitBuilding(newPos, tmpAzimuth,
						    newPos, newAzimuth,
						    phased, expel);
      if (!bumpObstacle) {
	move(newPos, getAngle());
	static BZDB_float bumpSpeedFactor("_bumpSpeedFactor");
	newPos.xy() += newVelocity.xy() * (dt * bumpSpeedFactor);
	break;
      }
    }

    hitWall = true;

    // record position when hitting
    fvec3 hitPos     = newPos;
    float hitAzimuth = newAzimuth;

    // find the latest time before the collision
    float searchTime = 0.0f, searchStep = 0.5f * timeStep;
    for (int i = 0; searchStep > MinSearchStep && i < MaxSearchSteps;
	 searchStep *= 0.5f, i++) {
      // get intermediate position
      const float t = searchTime + searchStep;
      newAzimuth = tmpAzimuth + (t * newAngVel);
      newPos = tmpPos + (t * newVelocity);
      if ((newPos.z < groundLimit) && (newVelocity.z < 0)) {
	newPos.z = groundLimit;
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
	hitPos = newPos;
      }
    }

    // get position just before impact
    newAzimuth = tmpAzimuth + (searchTime * newAngVel);
    newPos = tmpPos + (searchTime * newVelocity);
    if (oldPosition.z < groundLimit) {
      newVelocity.z = std::max(newVelocity.z, -oldPosition.z / 2.0f + 0.5f);
    }


    // record how much time is left in time step
    timeStep -= searchTime;

    // get normal at intersection.  sometimes fancy test says there's
    // no intersection but we're expecting one so, in that case, fall
    // back to simple normal calculation.
    fvec3 normal;
    if (!getHitNormal(obstacle, newPos, newAzimuth,
                                hitPos, hitAzimuth, normal)) {
      obstacle->getNormal(newPos, normal);
    }

    // check for being on a building
    if ((newPos.z > 0.0f) && (normal.z > 0.001f)) {
      if (location != Dead && location != Exploding && expel) {
	location = OnBuilding;
	lastObstacle = obstacle;
      }
      newVelocity.z = 0.0f;
    }
    else {
      // get component of velocity in normal direction (in horizontal plane)
      float mag = (normal.x * newVelocity.x) +
		  (normal.y * newVelocity.y);

      // handle upward normal component to prevent an upward force
      if (!NEAR_ZERO(normal.z, ZERO_TOLERANCE)) {
	// if going down then stop falling
	if (newVelocity.z < 0.0f && newVelocity.z -
	    (mag + normal.z * newVelocity.z) * normal.z > 0.0f) {
	  newVelocity.z = 0.0f;
	}

	// normalize force magnitude in horizontal plane
	float horNormal = normal.x * normal.x + normal.y * normal.y;
	if (!NEAR_ZERO(horNormal, ZERO_TOLERANCE)) {
	  mag /= horNormal;
	}
      }

      // cancel out component in normal direction (if velocity and
      // normal point in opposite directions).  also back off a tiny
      // amount to prevent a spurious collision against the same
      // obstacle.
      if (mag < 0.0f) {
	newVelocity.x -= mag * normal.x;
	newVelocity.y -= mag * normal.y;
	if (!(getStatus() & PlayerState::BackedOff)) {
	  newPos.x -= TinyDistance * mag * normal.x;
	  newPos.y -= TinyDistance * mag * normal.y;
	  setStatus(getStatus() | PlayerState::BackedOff);
	}
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
    else if (newPos.z > 0.0f) {
      location = InAir;
    }
  }

  // see if we're crossing a wall
  if ((location == InBuilding) &&
      (getFlag() == Flags::OscillationOverthruster)) {
    if (obstacle->isCrossing(newPos, newAzimuth,
			     0.5f * BZDBCache::tankLength,
			     0.5f * BZDBCache::tankWidth,
			     BZDBCache::tankHeight, NULL)) {
      setStatus(getStatus() | int(PlayerState::CrossingWall));
    } else {
      setStatus(getStatus() & ~int(PlayerState::CrossingWall));
    }
  }
  else {
    const MeshFace* face =
      world->crossingTeleporter(newPos, newAzimuth,
                                0.5f * BZDBCache::tankLength,
                                0.5f * BZDBCache::tankWidth,
                                BZDBCache::tankHeight);
    if (face) {
      crossingPlane = face->getPlane(); // used in checkHit
      setStatus(getStatus() | int(PlayerState::CrossingWall));
    } else {
      setStatus(getStatus() & ~int(PlayerState::CrossingWall));
    }
  }

  // compute actual velocities.  do this before teleportation.
  if (!NEAR_ZERO(dt, ZERO_TOLERANCE)) {
    const float oodt = 1.0f / dt;
    newAngVel = (newAzimuth - oldAzimuth) * oodt;
    newVelocity = (newPos - oldPosition) * oodt;
  }

  // teleportation
  const bool teleported = tryTeleporting(oldPosition, newPos,
                                         oldVelocity, newVelocity,
                                         oldAzimuth,  newAzimuth,
                                         oldAngVel,   newAngVel,
                                         phased, expel);

  // setup the physics driver
  setPhysicsDriver(-1);
  if ((lastObstacle != NULL) && (lastObstacle->getTypeID() == faceType)) {
    const MeshFace* meshFace = (const MeshFace*) lastObstacle;
    int driverID = meshFace->getPhysicsDriver();
    const PhysicsDriver* phydriver = PHYDRVMGR.getDriver(driverID);
    if (phydriver != NULL) {
      setPhysicsDriver(driverID);
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
    land();
    setLandingSpeed(oldVelocity.z);
    if (!headless) {
      EFFECTS.addLandEffect(getColor(),newPos,getAngle());
    }
  }

  if (gettingSound) {
    const PhysicsDriver* phydriver = PHYDRVMGR.getDriver(getPhysicsDriver());
    if ((phydriver != NULL) && (phydriver->getLinearVel().z > 0.0f)) {
      SOUNDSYSTEM.play(SFX_BOUNCE);
      addRemoteSound(PlayerState::BounceSound);
    } else if (justLanded && !entryDrop) {
      SOUNDSYSTEM.play(SFX_LAND);
    } else if ((location == OnGround) &&
	       (oldPosition.z == 0.0f) && (newPos.z < 0.f)) {
      SOUNDSYSTEM.play(SFX_BURROW);
    }
  }

  // set falling status
  if (location == OnGround || location == OnBuilding ||
      (location == InBuilding && newPos.z == 0.0f)) {
    setStatus(getStatus() & ~short(PlayerState::Falling));
    setStatus(getStatus() & ~PlayerState::BackedOff);
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
    case Exploding: {
      firingStatus = Deceased;
      break;
    }
    case InBuilding: {
      firingStatus = (getFlag() == Flags::PhantomZone) ? Zoned : Sealed;
      break;
    }
    default: {
      if (isPhantomZoned())
        firingStatus = Zoned;
      else if ((getReloadTime() > 0.0f) && (getFlag() != Flags::TriggerHappy))
        firingStatus = Loading;
      else
        firingStatus = Ready;
      break;
    }
  }

  // burrowed and oscillating tanks get some resistance in their joystick
  // if they have ff on
  if ((location == InBuilding) || (newPos.z < -0.5f))
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
  // If we are at or below the water level, send a player update now
  if (newPos.z <= world->getWaterLevel()) {
    server->sendPlayerUpdate(this);
  }

  // see if I'm over my antidote
  if (antidoteFlag && location == OnGround) {
    const float distSq = (flagAntidotePos.xy() - newPos.xy()).lengthSq();
    const float twoRads = getRadius() + BZDBCache::flagRadius;
    if (distSq < (twoRads * twoRads)) {
      server->sendDropFlag(getPosition());
      setShotType(StandardShot);
    }
  }

  if ((getFlag() == Flags::Bouncy) &&
      ((location == OnGround) || (location == OnBuilding))) {
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
    if ((oldPosition.x != newPos.x) || (oldPosition.y != newPos.y) ||
	(oldPosition.z != newPos.z) || (oldAzimuth != newAzimuth)) {
      SOUNDSYSTEM.setReceiver(newPos.x, newPos.y, newPos.z, newAzimuth,
			NEAR_ZERO(dt, ZERO_TOLERANCE) ||
			(teleported && (getFlag() != Flags::PhantomZone)));
    }
    if (NEAR_ZERO(dt, ZERO_TOLERANCE)) {
      SOUNDSYSTEM.setReceiverVec(newVelocity.x, newVelocity.y, newVelocity.z);
    } else {
      SOUNDSYSTEM.setReceiverVec((newPos.x - oldPosition.x) / dt,
                                 (newPos.y - oldPosition.y) / dt,
                                 (newPos.z - oldPosition.z) / dt);
    }
  }
}


//============================================================================//

bool LocalPlayer::tryTeleporting(const fvec3& oldPos,   fvec3& newPos,
                                 const fvec3& oldVel,   fvec3& newVel,
                                 const float oldAngle,  float& newAngle,
                                 const float oldAngVel, float& newAngVel,
                                 bool phased, bool& expel)
{
  if (!isAlive()) {
    return false;
  }

  // see if we crossed a link source
  World* world = World::getWorld();
  const MeshFace* linkSrc = world->crossesTeleporter(oldPos, newPos);
  if (linkSrc == NULL) {
    return false;
  }

  // PhantomZone tanks just change phase
  if (getFlag() == Flags::PhantomZone) {
    // change zoned state
    setStatus(getStatus() ^ PlayerState::PhantomZoned);
    setStatus(getStatus() ^ PlayerState::FlagActive);
    if (gettingSound) {
      SOUNDSYSTEM.play(SFX_PHANTOM);
    }
    return false; // just a phase change
  }

  // get the destination link, the link IDs, and the link physics
  int linkSrcID, linkDstID;
  const LinkPhysics* linkPhysics;
  const MeshFace* linkDst = linkManager.getTankLinkDst(linkSrc,
                                                       linkSrcID, linkDstID,
                                                       linkPhysics,
                                                       newPos, newVel,
                                                       getTeam(), getFlag());
  // no link, no love; bail out
  if (!linkDst) {
    const std::string& failMsg = linkSrc->getSpecialData()->linkSrcTankFail;
    if (!failMsg.empty()) {
      addMessage(NULL, failMsg);
    }
    return false;
  }

  // get the destination's coordinates
  linkSrc->teleportTank(*linkDst, *linkPhysics,
                        newPos,    newPos,
                        newVel,    newVel,
                        newAngle,  newAngle,
                        newAngVel, newAngVel);

  // check if the destination is blocked by an obstacle
  const Obstacle* teleBlocker =
    getHitBuilding(newPos, newAngle, newPos, newAngle, phased, expel);
  if (teleBlocker) {
    // try again, but with a little more Z
    newPos.z += 0.001f;
    teleBlocker =
      getHitBuilding(newPos, newAngle, newPos, newAngle, phased, expel);
  }

  // check if the destination is blocked by the ground
  bool teleGrounded = false;
  if (!teleBlocker) {
    const float groundLimit = computeGroundLimit(getFlag());
    const float groundDiff = newPos.z - groundLimit;
    if (groundDiff < 0.0f) {
      if (groundDiff > -0.001f) {
        newPos.z = groundLimit; // bump it up
      } else {
        teleGrounded = true;
      }
    }
  }

  // revert or rebound when blocked
  if ((teleBlocker != NULL) || teleGrounded) {
    if (debugTele >= 1) {
      if (teleBlocker) {
        logDebugMessage(0, "teleport blocked by %s '%s'\n",
                        teleBlocker->getType(), teleBlocker->getName().c_str());
      } else {
        logDebugMessage(0, "teleport blocked by the ground\n");
      }
    }

    if (!linkSrc->linkSrcRebound()) {
      // revert / block
      newPos    = oldPos;
      newVel    = fvec3(0.0f, 0.0f, oldVel.z);
      newAngle  = oldAngle;
      newAngVel = oldAngVel;
      if (debugTele >= 1) {
        logDebugMessage(0, "  reverted\n");
      }
    }
    else {
      // rebound
      newPos    = oldPos;
      newAngle  = oldAngle;
      newAngVel = oldAngVel;
      const fvec3& plane = linkSrc->getPlane().xyz();
      newVel = oldVel - (2.0f * plane * fvec3::dot(plane, oldVel));
      if (debugTele >= 1) {
        logDebugMessage(0, "  rebounded: vel = %s\n",
                        newVel.tostring().c_str());
      }
    }
    return false;
  }

  // adjust the 'location'
  if (newVel.z != 0.0f) {
    if (newPos.z <= 0.0f) {
      newVel.z = 0.0f;
      newPos.z = 0.0f;
      location = OnGround;
    } else {
      location = InAir;
    }
  }

  // set teleport info
  setTeleport(lastTime, linkSrcID, linkDstID);

  // send teleport info
  server->sendTeleport(linkSrcID, linkDstID);

  // play the music, if desired
  if (gettingSound && !linkSrc->linkSrcNoSound()) {
    SOUNDSYSTEM.play(SFX_TELEPORT);
  }

  return true;
}


//============================================================================//

bool LocalPlayer::canJump() const
{
  // does the world exist
  World *world = World::getWorld();
  if (!world) {
    return false;
  }

  // are we allowed by the server to jump?
  if (!Player::canJump()) {
    return false;
  }

  // can't jump while burrowed
  if (getPosition().z < 0.0f) {
    return false;
  }

  if (world->allowJumping()) {
    // if all tanks may jump, then you may jump unless you hold NJ
    if (getFlag() == Flags::NoJumping)
      return false;
    else
      return true;
  } else {
    // otherwise, you may not jump unless you hold J, WG, B, or LG
    if ((getFlag() == Flags::Jumping) || (getFlag() == Flags::Wings)
        || (getFlag() == Flags::Bouncy) || (getFlag() == Flags::LowGravity))
      return true;
    else
      return false;
  }
}


const Obstacle* LocalPlayer::getHitBuilding(const fvec3& p, float a,
					    bool phased, bool& expel) const
{
  const bool hasOOflag = getFlag() == Flags::OscillationOverthruster;
  const fvec3& dims = getDimensions();
  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }
  const Obstacle* obstacle = world->hitBuilding(p, a, dims.x, dims.y, dims.z);

  expel = (obstacle != NULL);
  if (expel && phased) {
    // set expel
    if (obstacle->getTypeID() == wallType) {
      expel = true;
    }
    else if ((obstacle->getTypeID() == faceType) &&
             (((const MeshFace*)obstacle)->isLinkFace())) {
      expel = true;
    }
    else if (hasOOflag && (desiredSpeed < 0.0f) &&
             NEAR_ZERO(p.z, ZERO_TOLERANCE)) {
      expel = true;
    }
    else {
      expel = false;
    }
  }
  return obstacle;
}


const Obstacle* LocalPlayer::getHitBuilding(const fvec3& oldP, float oldA,
					    const fvec3& p, float a,
					    bool phased, bool& expel)
{
  const bool hasOOflag = getFlag() == Flags::OscillationOverthruster;
  const fvec3& dims = getDimensions();
  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }
  const Obstacle* obstacle = world->hitBuilding(oldP, oldA, p, a, dims.x, dims.y, dims.z, !hasOOflag);

  expel = (obstacle != NULL);
  if (expel && phased) {
    // set expel
    if (obstacle->getTypeID() == wallType) {
      expel = true;
    }
    else if ((obstacle->getTypeID() == faceType) &&
             (((const MeshFace*)obstacle)->isLinkFace())) {
      expel = true;
    }
    else if (hasOOflag && (desiredSpeed < 0.0f) &&
             NEAR_ZERO(p.z, ZERO_TOLERANCE)) {
      expel = true;
    }
    else {
      expel = false;
    }
  }

  if (obstacle != NULL) {
    if (obstacle->getTypeID() == faceType) {
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
			       const fvec3& pos1, float azimuth1,
			       const fvec3& pos2, float azimuth2,
			       fvec3& normal) const
{
  const fvec3& dims = getDimensions();
  return o->getHitNormal(pos1, azimuth1, pos2, azimuth2,
			 dims.x, dims.y, dims.z, normal);
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
  const fvec3& pos = getPosition();
  const float angle = getAngle();
  const fvec3& dims = getDimensions();

  // get the list of possible inside buildings
  const ObsList* olist =
    COLLISIONMGR.boxTest (pos, angle, dims.x, dims.y, dims.z);

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    if (obs->inBox(pos, angle, dims.x, dims.y, dims.z)) {
      if (obs->getTypeID() == faceType) {
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
	if (!ClientIntangibilityManager::instance().getWorldObjectTangibility(obs) &&
	    notInObstacleList(mesh, insideBuildings)) {
	  insideBuildings.push_back(mesh);
	}
      }
      else if (!ClientIntangibilityManager::instance().getWorldObjectTangibility(obs)) {
	if (obs->getTypeID() == meshType) {
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


float LocalPlayer::getFlagShakingTime() const
{
  return flagShakingTime;
}


int LocalPlayer::getFlagShakingWins() const
{
  return flagShakingWins;
}


const fvec3* LocalPlayer::getAntidoteLocation() const
{
  return (antidoteFlag ? &antidoteFlag->getCenter() : NULL);
}


void LocalPlayer::restart(const fvec3& pos, float _azimuth)
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
  static const fvec3 zero(0.0f, 0.0f, 0.0f);
  location = (pos.z > 0.0f) ? OnBuilding : OnGround;
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


void LocalPlayer::setTeam(TeamColor _team)
{
  changeTeam(_team);
}


void LocalPlayer::setDesiredSpeed(float fracOfMaxSpeed)
{
  FlagType* flag = getFlag();

  // If we aren't allowed to move, then the desired speed is 0.
  if (
	  (!canMoveForward() && fracOfMaxSpeed > 0) ||
	  (!canMoveBackward() && fracOfMaxSpeed < 0)) {
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
  } else if ((flag == Flags::Burrow) && (getPosition().z < 0.0f)) {
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


void LocalPlayer::setDesiredAngVel(float fracOfMaxAngVel)
{
  FlagType* flag = getFlag();

  // limit turn speed to maximum
  if (fracOfMaxAngVel > 1.0f) fracOfMaxAngVel = 1.0f;
  else if (fracOfMaxAngVel < -1.0f) fracOfMaxAngVel = -1.0f;

  // further limit turn speed for certain flags or when we aren't allowed to turn
  if (fracOfMaxAngVel < 0.0f && (getFlag() == Flags::LeftTurnOnly || !canTurnLeft()))
    fracOfMaxAngVel = 0.0f;
  else if (fracOfMaxAngVel > 0.0f && (getFlag() == Flags::RightTurnOnly || !canTurnRight()))
    fracOfMaxAngVel = 0.0f;

  // boost turn speed for other flags
  if (flag == Flags::QuickTurn) {
    fracOfMaxAngVel *= BZDB.eval(StateDatabase::BZDB_ANGULARAD);
  } else if ((flag == Flags::Burrow) && (getPosition().z < 0.0f)) {
    fracOfMaxAngVel *= BZDB.eval(StateDatabase::BZDB_BURROWANGULARAD);
  }

  // apply handicap advantage to tank speed
  fracOfMaxAngVel *= (1.0f + (handicap * (BZDB.eval(StateDatabase::BZDB_HANDICAPANGAD) - 1.0f)));

  // set desired turn speed
  desiredAngVel = fracOfMaxAngVel * BZDB.eval(StateDatabase::BZDB_TANKANGVEL);
  Player::setUserAngVel(desiredAngVel);

  return;
}


void LocalPlayer::setDeadStop(void)
{
  setVelocity(fvec3(0.0f, 0.0f, 0.0f));
  setAngularVelocity(0.0);
}


void LocalPlayer::setPause(bool pause)
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


void LocalPlayer::requestAutoPilot(bool autopilot)
{
  requestedAutopilot = autopilot;
  server->sendAutoPilot(autopilot);
}


bool LocalPlayer::fireShot()
{
  if (! (firingStatus == Ready || firingStatus == Zoned))
    return false;

  if (!canShoot())
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
  firingInfo.timeSent = (float)syncedClock.GetServerSeconds();
  firingInfo.shotType = getShotType();
  firingInfo.shot.player = getId();
  firingInfo.shot.id     = uint16_t(i + getSalt());
  prepareShotInfo(firingInfo);
  // make shot and put it in the table
  addShot(new LocalShotPath(firingInfo,syncedClock.GetServerSeconds()), firingInfo);

  // Insert timestamp, useful for dead reckoning jitter fixing
  firingInfo.timeSent = (float)syncedClock.GetServerSeconds();

  // always send a player-update message. To synchronize movement and
  // shot start. They should generally travel on the same frame, when
  // flushing the output queues.
  server->sendPlayerUpdate(this);
  server->sendBeginShot(firingInfo);

  if (BZDB.isTrue("enableLocalShotEffect") && SceneRenderer::instance().useQuality() >= _MEDIUM_QUALITY)
    EFFECTS.addShotEffect(getColor(), firingInfo.shot.pos, getAngle(), getVelocity());

  if (gettingSound) {
    switch (firingInfo.shotType) {
      case ShockWaveShot: {
        SOUNDSYSTEM.play(SFX_SHOCK);
        ForceFeedback::shockwaveFired();
        break;
      }
      case LaserShot: {
        SOUNDSYSTEM.play(SFX_LASER);
        ForceFeedback::laserFired();
        break;
      }
      case GMShot: {
        SOUNDSYSTEM.play(SFX_MISSILE);
        ForceFeedback::shotFired();
        break;
      }
      case ThiefShot: {
        SOUNDSYSTEM.play(SFX_THIEF);
        ForceFeedback::shotFired();
        break;
      }
      default: {
        SOUNDSYSTEM.play(SFX_FIRE);
        ForceFeedback::shotFired();
        break;
      }
    }
  }

  if (getFlag() == Flags::TriggerHappy) {
    // make sure all the shots don't go off at once
    forceReload(BZDB.eval(StateDatabase::BZDB_RELOADTIME) / numShots);
  }
  return true;
}


bool LocalPlayer::doEndShot(int ident, bool isHit, fvec3& pos)
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
  pos = shots[index]->getPosition();
  shots[index]->setExpired();
  return true;
}


void LocalPlayer::setJump()
{
  wantJump = jumpPressed;
  jumpPressed = false;
}


void LocalPlayer::setJumpPressed(bool value)
{
  if (onSolidSurface() || hasWings())
    jumpPressed = value;
}


void LocalPlayer::doJump()
{
  wantJump = false;

  FlagType* flag = getFlag();
  World *world = World::getWorld();
  if (!world) {
    return;
  }

  // check to see if it's possible for us to jump
  // i.e. appropriate flags, world settings, permissions
  if (!canJump()) {
    return;
  }

  if (hasWings()) {
    if (wingsFlapCount <= 0) {
      return;
    }
    wingsFlapCount--;
  } else if (!onSolidSurface()) {
    // can't jump without wings unless on the ground or a building
    if (flag != Flags::Wings)
      return;
    if (wingsFlapCount <= 0)
      return;
    wingsFlapCount--;
  }

  // add jump velocity (actually, set the vertical component since you
  // can only jump if resting on something)
  const fvec3& oldVelocity = getVelocity();
  fvec3 newVelocity;
  newVelocity.x = oldVelocity.x;
  newVelocity.y = oldVelocity.y;
  if (flag == Flags::Wings) {
    newVelocity.z = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
  } else if (flag == Flags::Bouncy) {
    const float factor = 0.25f + ((float)bzfrand() * 0.75f);
    newVelocity.z = factor * BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
  }  else {
    newVelocity.z = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
  }

  /* better realism .. make it so that if you're falling, wings will
   * just slow you down.
   */
  if ((flag == Flags::Wings) && (oldVelocity.z < 0))
    newVelocity.z += oldVelocity.z;

  setVelocity(newVelocity);
  location = InAir;

  // setup the graphics
  fireJumpJets();

  // setup the sound
  if (gettingSound) {
    if (hasWings()) {
      SOUNDSYSTEM.play(SFX_FLAP);
      addRemoteSound(PlayerState::WingsSound);
    } else {
      SOUNDSYSTEM.play(SFX_JUMP);
      addRemoteSound(PlayerState::JumpSound);
    }
  }
}


void LocalPlayer::setTarget(const Player* _target)
{
  target = _target;
}


void LocalPlayer::setNemesis(const Player* _nemesis)
{
  if ((_nemesis == NULL) || _nemesis->getPlayerType() == TankPlayer)
    nemesis = _nemesis;
}


void LocalPlayer::setRecipient(const Player* _recipient)
{
  if ((_recipient == NULL) || (_recipient->getId() <= LastRealPlayer))
    recipient = _recipient;
}


void LocalPlayer::explodeTank()
{
  if (location == Dead || location == Exploding) return;
  const float gravity    = BZDBCache::gravity;
  const float explodeTim = BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
  // Limiting max height increment to this value (the old default value)
  const float zMax  = 49.0f;
  setExplode(TimeKeeper::getTick());
  const fvec3& oldVelocity = getVelocity();
  fvec3 newVelocity;
  float maxSpeed;
  newVelocity.x = oldVelocity.x;
  newVelocity.y = oldVelocity.y;
  if (gravity < 0.0f) {
    // comparing 2 speed:
    //   to have a simmetric path (ending at same height as starting)
    //   to reach the acme of parabola, under the max height established
    // take the less
    newVelocity.z = - 0.5f * gravity * explodeTim;
    maxSpeed       = sqrtf(- 2.0f * zMax * gravity);
    if (newVelocity.z > maxSpeed)
      newVelocity.z = maxSpeed;
  } else {
    newVelocity.z = oldVelocity.z;
  }
  setVelocity(newVelocity);
  location = Exploding;
  target = NULL;		// lose lock when dead
}


void LocalPlayer::doMomentum(float dt,float& speed, float& angVel)
{
  computeMomentum(dt, getFlag(), speed,angVel,lastSpeed,getAngularVelocity());
}


void LocalPlayer::doFriction(float dt, const fvec3& oldVel, fvec3& newVel)
{
  computeFriction(dt, getFlag(), oldVel, newVel);
}


void LocalPlayer::doForces(float /*dt*/, fvec3& /*velocity*/, float& /*angVel*/)
{
  // apply external forces
  // do nothing -- no external forces right now
}


// NOTE -- minTime should be initialized to Infinity by the caller
bool LocalPlayer::checkHit(const Player* source,
                           const ShotPath*& hit, float& minTime) const
{
  bool goodHit = false;

  // if firing tank is paused then it doesn't count
  if (source->isPaused()) {
    return goodHit;
  }

  const int maxShots = source->getMaxShots();
  for (int i = 0; i < maxShots; i++) {
    const ShotPath* shot = source->getShot(i);
    if (!shot || shot->isExpired()) {
      continue;
    }
    ShotType _shotType = shot->getShotType();

    // my own shock wave cannot kill me
    if ((source == this) && ((_shotType == ShockWaveShot) ||
                             (_shotType == ThiefShot))) {
      continue;
    }

    // if no team kills, shots of my team can't kill me  (except my own)
    if ((source != this) && !World::getWorld()->allowTeamKills() &&
        (shot->getTeam() != RogueTeam) && (shot->getTeam() == getTeam())) {
      continue;
    }

    // short circuit test if shot can't possibly hit.
    // only superbullet or shockwave can kill zoned dude
    if (isPhantomZoned()             &&
        (_shotType != SuperShot)     &&
        (_shotType != PhantomShot)   &&
        (_shotType != ShockWaveShot)) {
      continue;
    }

    // laser can't hit a cloaked tank
    if ((getFlag() == Flags::Cloaking) && (_shotType == LaserShot)) {
      continue;
    }

    // zoned shots only kill zoned tanks
    if ((_shotType == PhantomShot) && !isPhantomZoned()) {
      continue;
    }

    // test myself against shot
    fvec3 position;

    ShotCollider collider;
    collider.position = getPosition();
    collider.angle    = getAngle();
    collider.length   = BZDBCache::tankLength;
    collider.motion   = getLastMotion();
    collider.radius   = this->getRadius();
    collider.size     = getDimensions();
    collider.test2D   = (this->getFlag() == Flags::Narrow);
    collider.bbox     = bbox;
    collider.testLastSegment = (getId() == shot->getPlayer());

    const float t = shot->checkHit(collider, position);
    if (t >= minTime) {
      continue;
    }

    // test if shot hit a part of my tank that's through a teleporter.
    // hit is no good if hit point is behind crossing plane.
    if (isCrossingWall() && (crossingPlane.planeDist(position) < 0.0f)) {
      continue;
    }

    // okay, shot hit
    goodHit = true;
    hit = shot;
    minTime = t;
  }
  return goodHit;
}


bool LocalPlayer::checkCollision(const Player* otherTank)
{
  if (!otherTank) {
    return false;
  }

  TimeKeeper current = TimeKeeper::getTick();
  // Don't flood the network with MsgCollide
  if (current - lastCollisionTime < BZDBCache::collisionLimit) {
    return false;
  }

  const fvec3& myPosition = getPosition();
  const fvec3& otherPosition = otherTank->getPosition();

  const float distSq2D = (otherPosition.xy() - myPosition.xy()).lengthSq();

  const float r = BZDBCache::freezeTagRadius;

  if (distSq2D < (r * r)) {
    server->sendCollide(getId(), otherTank->getId(), myPosition);
    lastCollisionTime = current;
    return true;
  } else {
    return false;
  }
}


void LocalPlayer::setFlag(FlagType* flag)
{
  Player::setFlag(flag);
  World *world = World::getWorld();
  if (!world) {
    return;
  }

  const float worldSize = BZDBCache::worldSize;

  // if it's bad then reset countdowns and set antidote flag
  if ((getFlag() != Flags::Null) && (getFlag()->endurance == FlagSticky)) {

    if (world->allowShakeTimeout()) {
      flagShakingTime = world->getFlagShakeTimeout();
    }

    if (world->allowShakeWins()) {
      flagShakingWins = world->getFlagShakeWins();
    }

    if (world->allowAntidote()) {
      float tankRadius = BZDBCache::tankRadius;
      float baseSize = BZDB.eval(StateDatabase::BZDB_BASESIZE);

      do {
	if (world->allowTeamFlags()) {
	  flagAntidotePos.x = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
	  flagAntidotePos.y = 0.5f * worldSize * ((float)bzfrand() - 0.5f);
	  flagAntidotePos.z = 0.0f;
	} else {
	  flagAntidotePos.x = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	  flagAntidotePos.y = (worldSize - baseSize) * ((float)bzfrand() - 0.5f);
	  flagAntidotePos.z = 0.0f;
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


void LocalPlayer::changeScore(float newRank,
                              short newWins, short newLosses, short newTks)
{
  Player::changeScore(newRank, newWins, newLosses, newTks);
  World *world = World::getWorld();
  if (!world) {
    return;
  }
  if (newWins > 0 && world->allowShakeWins() &&
      flagShakingWins > 0) {
    flagShakingWins -= newWins;
    if (flagShakingWins <= 0) {
      flagShakingWins = 0;
      server->sendDropFlag(getPosition());
	  setShotType(StandardShot);
    }
  }
}


void LocalPlayer::addAntidote(SceneDatabase* scene)
{
  if (antidoteFlag)
    scene->addDynamicNode(antidoteFlag);
}


std::string LocalPlayer::getInputMethodName(InputMethod whatInput)
{
  switch (whatInput) {
    case Keyboard: { return std::string("Keyboard");       }
    case Mouse:    { return std::string("Mouse");          }
    case Joystick: { return std::string("Joystick");       }
    default:       { return std::string("Unnamed Device"); }
  }
}


void LocalPlayer::setKey(int button, bool pressed)
{
  switch (button) {
    case BzfKeyEvent::Left:  { left  = pressed; break; }
    case BzfKeyEvent::Right: { right = pressed; break; }
    case BzfKeyEvent::Up:    { up    = pressed; break; }
    case BzfKeyEvent::Down:  { down  = pressed; break; }
  }
}


std::string LocalPlayer::getLocationString(Location location)
{
  switch (location) {
    case Dead:       { return "Dead";       }
    case Exploding:  { return "Exploding";  }
    case OnGround:   { return "OnGround";   }
    case InBuilding: { return "InBuilding"; }
    case OnBuilding: { return "OnBuilding"; }
    case InAir:      { return "InAir";      }
    default:         { return "Unknown";    }
  }
  return "Unknown";
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
