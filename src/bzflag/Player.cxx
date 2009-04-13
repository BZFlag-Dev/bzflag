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
#include "Player.h"

// common interface headers
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "CollisionManager.h"
#include "ObstacleMgr.h"
#include "PhysicsDriver.h"
#include "ObstacleList.h"
#include "WallObstacle.h"
#include "ClientIntangibilityManager.h"
#include "MotionUtils.h"
#include "EventHandler.h"

// local implementation headers
#include "playing.h"
#include "World.h"
#include "TrackMarks.h"
#include "sound.h"
#include "EffectsRenderer.h"
#include "Roaming.h"
#include "SyncClock.h"

// for dead reckoning
static const float MaxUpdateTime = 1.0f; // seconds

//
// Player
//


int Player::tankTexture = -1;


Player::Player(const PlayerId& _id, TeamColor _team,
	       const char* name, const PlayerType _type)
: lastObstacle(NULL)
, handicap(0.0f)
, notResponding(false)
, hunted(false)
, autoHuntLevel(0)
, id(_id)
, admin(false)
, registered(false)
, verified(false)
, playerList(false)
, gfxBlock(GfxBlock::Tank, id, true)
, lastVisualTeam(NoTeam)
, team(_team)
, type(_type)
, flagType(Flags::Null)
, fromTeleporter(0)
, toTeleporter(0)
, teleporterProximity(0.0f)
, rank(0.42f)			// rank is received from the server on join
, wins(0)
, losses(0)
, tks(0)
, allow(0)
, localWins(0)
, localLosses(0)
, localTks(0)
, autoPilot(false)
, deltaTime(0.0)
, offset(0.0)
, deadReckoningState(0)
, oldStatus(0)
, oldZSpeed(0.0f)
{
  static const fvec3 zero(0.0f, 0.0f, 0.0f);
  move(zero, 0.0f);
  setVelocity(zero);
  setAngularVelocity(0.0f);
  setPhysicsDriver(-1);
  setDeadReckoning(syncedClock.GetServerSeconds());
  setRelativeMotion();
  setUserSpeed(0.0f);
  setUserAngVel(0.0f);

  lastLanding = TimeKeeper::getCurrent().getSeconds();

  // set call sign
  ::strncpy(callSign, name, CallSignLen);
  callSign[CallSignLen-1] = '\0';

  // only get an avtar if it can be drawn
  if (id != ServerPlayer && !headless)
    avatar = getPlayerAvatar(id,state.pos,forward);
  else
    avatar = NULL;

  // setup the dimension properties
  dimensions[0] = 0.5f * BZDBCache::tankLength;
  dimensions[1] = 0.5f * BZDBCache::tankWidth;
  dimensions[2] = BZDBCache::tankHeight;

  for (int i = 0; i < 3; i++) {
    dimensionsRate[i] = 0.0f;
    dimensionsScale[i] = 1.0f;
    dimensionsTarget[i] = 1.0f;
  }
  useDimensions = false;

  // setup alpha properties
  alpha = 1.0f;
  alphaRate = 0.0f;
  alphaTarget = 1.0f;
  teleAlpha = 1.0f;

  haveIpAddr = false; // no IP address yet
  lastTrackDraw = TimeKeeper::getCurrent();

  spawnTime = TimeKeeper::getCurrent();

  shotType = StandardShot;

  return;
}

Player::~Player()
{
  // free shots
  const int numShots = getMaxShots();
  for (int i = 0; i < numShots; i++) {
    if (shots[i])
      delete shots[i];
  }
  freePlayerAvatar (avatar);
}

float Player::getTKRatio() const
{
  if (wins == 0)
    return (float)tks/1.0f;    // well, we have to return *something* if they have no wins
  else
    return (float)tks/(float)wins;
}

// returns a value between 1.0 and -1.0
float Player::getNormalizedScore() const
{
  return ((float)wins - losses) / ((wins+losses>20) ? wins+losses : 20);
}

void Player::forceReload(float time)
{
  jamTime = TimeKeeper::getCurrent();
  jamTime+= time;
}

float Player::getReloadTime() const
{
  World *world = World::getWorld();
  if (!world)
    return 0.0f;

  const int numShots = world->getMaxShots();
  if (numShots <= 0)
    return 0.0f;

  float time = float(jamTime - TimeKeeper::getCurrent());
  if (time > 0.0f)
    return time;

  // look for an empty slot
  int i;
  for (i = 0; i < numShots; i++) {
    if (!shots[i])
      return 0.0f;
  }

  // look for the shot fired least recently
  float minTime = float(shots[0]->getReloadTime() - (shots[0]->getCurrentTime() - shots[0]->getStartTime()));
  for (i = 1; i < numShots; i++) {
    const float t = float(shots[i]->getReloadTime() - (shots[i]->getCurrentTime() - shots[i]->getStartTime()));
    if (t < minTime)
      minTime = t;
  }

  if (minTime < 0.0f)
    minTime = 0.0f;

  return minTime;
}

float Player::getLocalNormalizedScore() const
{
  return ((float)localWins - localLosses)
    / ((localWins+localLosses>5) ? localWins+localLosses : 5);
}

float Player::getRabbitScore() const
{
  return rank;
}


float Player::getRadius() const
{
  // NOTE: this encompasses everything but Narrow
  //       the Obese, Tiny, and Thief flags adjust
  //       the radius, but Narrow does not.
  return (dimensionsScale[0] * BZDBCache::tankRadius);
}

float Player::getMaxSpeed ( void ) const
{
  // BURROW and AGILITY will not be taken into account
  const FlagType* flag = getFlag();
  float maxSpeed = BZDBCache::tankSpeed;
  if (flag == Flags::Velocity)
    maxSpeed *= BZDB.eval(StateDatabase::BZDB_VELOCITYAD);
  else if (flag == Flags::Thief)
    maxSpeed *= BZDB.eval(StateDatabase::BZDB_THIEFVELAD);
  return maxSpeed;
}


void Player::getMuzzle(float* m) const
{
  // NOTE: like getRadius(), we only use dimensionsScale[0].
  //       as well, we do not use BZDB_MUZZLEFRONT, but the
  //       0.1f value listed in global.cxx is added on to the
  //       scaled version of tankRadius.
  float front = (dimensionsScale[0] * BZDBCache::tankRadius);
  if (dimensionsRate[0] > 0.0f)
    front = front + (dimensionsRate[0] * 0.1f);

  front = front + 0.1f;

  m[0] = state.pos[0] + (front * forward[0]);
  m[1] = state.pos[1] + (front * forward[1]);

  const float height = BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
  m[2] = state.pos[2] + (height * dimensionsScale[2]);
  return;
}


float Player::getMuzzleHeight() const
{
  return (dimensionsScale[2] * BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT));
}


void Player::move(const fvec3& _pos, float _azimuth)
{
  // update the speed of the state
  float currentTime = (float)TimeKeeper::getCurrent().getSeconds();
  if ( state.lastUpdateTime >= 0 ) {
    float delta = currentTime - state.lastUpdateTime;
    state.apparentVelocity[0] = (_pos[0]-state.pos[0])/delta;
    state.apparentVelocity[1] = (_pos[1]-state.pos[1])/delta;
    state.apparentVelocity[2] = (_pos[2]-state.pos[2])/delta;
  }
  state.lastUpdateTime = currentTime;

  // assumes _forward is normalized
  state.pos[0] = _pos[0];
  state.pos[1] = _pos[1];
  state.pos[2] = _pos[2];
  state.azimuth = _azimuth;

  // limit angle
  if (state.azimuth < 0.0f)
    state.azimuth = (float)((2.0 * M_PI) - fmodf(-state.azimuth, (float)(2.0 * M_PI)));
  else if (state.azimuth >= (2.0f * M_PI))
    state.azimuth = fmodf(state.azimuth, (float)(2.0 * M_PI));

  // update forward vector (always in horizontal plane)
  forward[0] = cosf(state.azimuth);
  forward[1] = sinf(state.azimuth);
  forward[2] = 0.0f;

  // compute teleporter proximity
  if (World::getWorld())
    teleporterProximity = World::getWorld()->getProximity(state.pos, BZDBCache::tankRadius);
}


void Player::setVelocity(const fvec3& _velocity)
{
  state.velocity[0] = _velocity[0];
  state.velocity[1] = _velocity[1];
  state.velocity[2] = _velocity[2];
}


void Player::setAngularVelocity(float _angVel)
{
  state.angVel = _angVel;
}


void Player::setPhysicsDriver(int driver)
{
  state.phydrv = driver;

  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(driver);
  if (phydrv != NULL)
    state.status |= PlayerState::OnDriver;
  else
    state.status &= ~PlayerState::OnDriver;

  return;
}


void Player::setRelativeMotion()
{
  bool falling = (state.status & short(PlayerState::Falling)) != 0;
  if (falling && (getFlag() != Flags::Wings))
    return;    // no adjustments while falling

  // set 'relativeSpeed' and 'relativeAngVel'
  float relativeVel[2];
  Player::calcRelativeMotion(relativeVel, relativeSpeed, relativeAngVel);

  return;
}


void Player::setUserSpeed(float speed)
{
  state.userSpeed = speed;
  return;
}


void Player::setUserAngVel(float angVel)
{
  state.userAngVel = angVel;
  return;
}


void Player::calcRelativeMotion(float vel[2], float& speed, float& angVel)
{
  vel[0] = state.velocity[0];
  vel[1] = state.velocity[1];

  angVel = state.angVel;

  const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(state.phydrv);
  if (phydrv != NULL) {
    const fvec3& v = phydrv->getLinearVel();
    const float av = phydrv->getAngularVel();
    const fvec2& ap = phydrv->getAngularPos();

    // adjust for driver velocity
    vel[0] -= v[0];
    vel[1] -= v[1];

    // adjust for driver angular velocity
    if (av != 0.0f) {
      const float dx = state.pos[0] - ap[0];
      const float dy = state.pos[1] - ap[1];
      vel[0] += av * dy;
      vel[1] -= av * dx;
      angVel = state.angVel - av;
    }
  }

  // speed relative to the tank's direction
  // (could use forward[] instead of re-doing the trig, but this is
  //  used in the setDeadReckoning(), when forward[] is not yet set)
  speed = (vel[0] * cosf(state.azimuth)) + (vel[1] * sinf(state.azimuth));

  return;
}

void Player::changeTeam(TeamColor _team)
{
  const TeamColor oldTeam = team;

  // set team
  team = _team;

  // set the scene node
  if (!headless) {
    setVisualTeam(team);
  }

  if (team != oldTeam) {
    eventHandler.PlayerTeamChange(*this, (int)oldTeam);
  }
}


void Player::setStatus(short _status)
{
  state.status = _status;
}


void Player::setExplode(const TimeKeeper& t)
{
  if (!isAlive()) {
    return;
  }

  explodeTime = t;

  const short setBits = short(PlayerState::Exploding)
                      | short(PlayerState::Falling);
  const short clearBits = short(PlayerState::Alive)
                        | short(PlayerState::Paused);
  setStatus((getStatus() | setBits) & ~clearBits);

  if (avatar) {
    avatar->explode();
    updateFlagEffect(Flags::Null);    // setup the flag effect to revert to normal
  }
}


void Player::setTeleport(const TimeKeeper& t, short from, short to)
{
  if (!isAlive()) return;
  teleportTime = t;
  fromTeleporter = from;
  toTeleporter = to;
  setStatus(getStatus() | short(PlayerState::Teleporting));
}


void Player::updateTank(float dt, bool local)
{
  updateDimensions(dt, local);
  updateTranslucency(dt);
  updateTreads(dt);
  updateJumpJets(dt);
  updateTrackMarks();
  return;
}


void Player::updateJumpJets(float dt)
{
  float jumpVel = computeJumpVelocity(getFlag());
  const float jetTime = 0.5f * (jumpVel / -BZDBCache::gravity);
  state.jumpJetsScale -= (dt / jetTime);
  if (state.jumpJetsScale < 0.0f) {
    state.jumpJetsScale = 0.0f;
    state.status &= ~PlayerState::JumpJets;
  }
  return;
}


void Player::updateTrackMarks()
{
  const float minSpeed = 0.1f; // relative speed slop

  if (isAlive() && !isFalling() && !isPhantomZoned()) {
    const float lifeTime = float(TimeKeeper::getCurrent() - lastTrackDraw);
    if (lifeTime > TrackMarks::updateTime) {
      bool drawMark = true;
      fvec3 markPos;
      markPos[2] = state.pos[2];
      // FIXME - again, this should be pulled for TankGeometryMgr
      const float fullLength = 6.0f;
      const float treadHeight = 1.2f;
      const float dist =
	dimensions[0] * ((fullLength - treadHeight) / fullLength);

      if (relativeSpeed > +minSpeed) {
	// draw the mark at the back of the treads
	markPos[0] = state.pos[0] - (forward[0] * dist);
	markPos[1] = state.pos[1] - (forward[1] * dist);
      } else if (relativeSpeed < -minSpeed) {
	// draw the mark at the front of the treads
	markPos[0] = state.pos[0] + (forward[0] * dist);
	markPos[1] = state.pos[1] + (forward[1] * dist);
      } else {
	drawMark = false;
      }

      if (drawMark) {
	TrackMarks::addMark(markPos, dimensionsScale[1],
			    state.azimuth, state.phydrv);
	lastTrackDraw = TimeKeeper::getCurrent();
      }
    }
  }
  return;
}


void Player::updateDimensions(float dt, bool local)
{
  // copy the current information
  const fvec3 oldRates      = dimensionsRate;
  const fvec3 oldScales     = dimensionsScale;
  const fvec3 oldDimensions = dimensions;

  // update the dimensions
  bool resizing = false;
  for (int i = 0; i < 3; i++) {
    if (dimensionsRate[i] != 0.0f) {
      resizing = true;
      dimensionsScale[i] += (dt * dimensionsRate[i]);
      if (dimensionsRate[i] < 0.0f) {
	if (dimensionsScale[i] < dimensionsTarget[i]) {
	  dimensionsScale[i] = dimensionsTarget[i];
	  dimensionsRate[i] = 0.0f;
	}
      } else {
	if (dimensionsScale[i] > dimensionsTarget[i]) {
	  dimensionsScale[i] = dimensionsTarget[i];
	  dimensionsRate[i] = 0.0f;
	}
      }
    } else {
      // safety play, should not be required
      dimensionsScale[i] = dimensionsTarget[i];
    }
  }

  // set the actual dimensions based on the scale
  dimensions[0] = dimensionsScale[0] * (0.5f * BZDBCache::tankLength);
  dimensions[1] = dimensionsScale[1] * (0.5f * BZDBCache::tankWidth);
  dimensions[2] = dimensionsScale[2] * BZDBCache::tankHeight;

  // do not resize if it will cause a collision
  // only checked for the local player, remote is computationally expensive
  if (local) {
    // also do not bother with collision checking if we are not resizing
    if (resizing && hitObstacleResizing()) {
      // copy the old information
      dimensions      = oldDimensions;
      dimensionsScale = oldScales;
      dimensionsRate  = oldRates;
    }
  }

  // check if the dimensions are at a steady state
  if ((dimensionsScale[0] == dimensionsTarget[0]) &&
      (dimensionsScale[1] == dimensionsTarget[1]) &&
      (dimensionsScale[2] == dimensionsTarget[2])) {
    useDimensions = false;
  } else {
    useDimensions = true;
  }

  return;
}


bool Player::hitObstacleResizing()
{
  const fvec3& dims = dimensions;

  // check walls
  const World* world = World::getWorld();
  if (!world) {
    return false;
  }

  const ObstacleList& walls = OBSTACLEMGR.getWalls();
  for (unsigned int i = 0; i < walls.size(); i++) {
    const WallObstacle* wall = (const WallObstacle*) walls[i];
    if (wall->inBox(getPosition(), getAngle(), dims[0], dims[1], dims[2])) {
      return true;
    }
  }

  // check everything else
  const ObsList* olist =
    COLLISIONMGR.boxTest(getPosition(), getAngle(), dims[0], dims[1], dims[2]);
  if (!olist) {
    return false;
  }

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const bool onTop = obs->isFlatTop() &&
      ((obs->getPosition()[2] + obs->getHeight()) <= getPosition()[2]);
    if (ClientIntangibilityManager::instance().getWorldObjectTangibility(obs) == 0 && !onTop &&
	obs->inBox(getPosition(), getAngle(), dims[0], dims[1], dims[2])) {
      return true;
    }
  }

  return false;
}

bool Player::getHitCorrection(const fvec3& startPos, const float startAzimuth,
                              const fvec3& /*endPos*/, const float /*endAzimuth*/,
                              const fvec3& startVelocity, double dt,
                              float groundLimit, fvec3& velocity, fvec3& position,
                              float* azimuth) 
{ 
  // constants
  static const float MinSearchStep = 0.0001f;
  static const int MaxSearchSteps = 7;
  static const int MaxSteps = 4;
  static const float TinyDistance = 0.0001f;

  fvec3 newPos;
  fvec3 newVelocity;
  float newAzimuth;
  float newAngVel;
  bool hitWall;
  const Obstacle* obstacle;
  bool expel;
  float timeStep = (float)dt;
  
  newPos[0] = startPos[0];
  newPos[1] = startPos[1];
  newPos[2] = startPos[2];
  newVelocity[0] = startVelocity[0];
  newVelocity[1] = startVelocity[1];
  newVelocity[2] = startVelocity[2];
  newAngVel = state.angVel;
  newAzimuth = startAzimuth;
  hitWall = false;

  bool phased = !isSolid();

  for (int numSteps = 0; numSteps < MaxSteps; numSteps++) {
    // record position at beginning of time step
    fvec3 tmpPos; 
	float tmpAzimuth;
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
	  if ((state.status & PlayerState::Exploding) != 0) {
	    // tank pieces reach the ground, friction
	    // stop them, & mainly player view
	    newPos[0] = tmpPos[0];
	    newPos[1] = tmpPos[1];
      }
    }

    // see if we hit anything.  if not then we're done.
    obstacle = getHitBuilding(tmpPos, tmpAzimuth, newPos, newAzimuth,
			      phased, expel);

    if (!obstacle || !expel)
      break;

    float obstacleTop = obstacle->getPosition()[2] + obstacle->getHeight();
	if (((inputStatus & PlayerState::Falling) == 0) && obstacle->isFlatTop() &&
	(obstacleTop != tmpPos[2]) &&
	(obstacleTop < (tmpPos[2] + BZDB.eval(StateDatabase::BZDB_MAXBUMPHEIGHT)))) {
      newPos[0] = startPos[0];
      newPos[1] = startPos[1];
      newPos[2] = obstacleTop;

      // drive over bumps
      const Obstacle* bumpObstacle = getHitBuilding(newPos, tmpAzimuth,
						    newPos, newAzimuth,
						    phased, expel);
      if (!bumpObstacle) {
	    move(newPos, getAngle());
	    const float speedFactor = BZDB.eval("_bumpSpeedFactor");
	    newPos[0] += newVelocity[0] * ((float)dt * speedFactor);
	    newPos[1] += newVelocity[1] * ((float)dt * speedFactor);
	    break;
      }
    }

    hitWall = true;

    // record position when hitting
    fvec3 hitPos;
	float hitAzimuth;
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
    if (startPos[2] < groundLimit) {
      newVelocity[2] = std::max(newVelocity[2], -startPos[2] / 2.0f + 0.5f);
    }


    // record how much time is left in time step
    timeStep -= searchTime;

    // get normal at intersection.  sometimes fancy test says there's
    // no intersection but we're expecting one so, in that case, fall
    // back to simple normal calculation.
    fvec3 normal;
    if (!getHitNormal(obstacle, newPos, newAzimuth, hitPos, hitAzimuth, normal)) {
      obstacle->getNormal(newPos, normal);
    }

    // check for being on a building
    if ((newPos[2] > 0.0f) && (normal[2] > 0.001f)) {
		if (((state.status & PlayerState::DeadStatus) == 0) && 
			((state.status & PlayerState::Exploding) == 0) && expel) {
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
	    if (!(getStatus() & PlayerState::BackedOff)) {
	      newPos[0] -= TinyDistance * mag * normal[0];
	      newPos[1] -= TinyDistance * mag * normal[1];
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

  velocity[0] = newVelocity[0];
  velocity[1] = newVelocity[1];
  velocity[2] = newVelocity[2];
  position[0] = newPos[0]; 
  position[1] = newPos[1]; 
  position[2] = newPos[2]; 
  *azimuth = newAzimuth;

  return hitWall;
}

const Obstacle* Player::getHitBuilding(const fvec3& oldP, float oldA,
					    const fvec3& p, float a,
					    bool phased, bool& expel)
{
  const bool hasOOflag = getFlag() == Flags::OscillationOverthruster;
  const fvec3& dims = getDimensions();
  World *world = World::getWorld();
  if (!world) {
    return NULL;
  }
  const Obstacle* obstacle = world->hitBuilding(oldP, oldA, p, a,
                                                dims.x, dims.y, dims.z,
                                                !hasOOflag);

  expel = (obstacle != NULL);
  if (expel && phased)
    expel = ((obstacle->getType() == WallObstacle::getClassName()) ||
             (obstacle->getType() == Teleporter::getClassName()));

  return obstacle;
}


bool Player::getHitNormal(const Obstacle* o,
                          const fvec3& pos1, float azimuth1,
                          const fvec3& pos2, float azimuth2,
                          fvec3& normal) const
{
  const fvec3& dims = getDimensions();
  return o->getHitNormal(pos1, azimuth1, pos2, azimuth2,
			 dims.x, dims.y, dims.z, normal);
}


void Player::updateTranslucency(float dt)
{
  // update the alpha value
  if (alphaRate != 0.0f) {
    alpha += dt * alphaRate;
    if (alphaRate < 0.0f) {
      if (alpha < alphaTarget) {
	alpha = alphaTarget;
	alphaRate = 0.0f;
      }
    } else {
      if (alpha > alphaTarget) {
	alpha = alphaTarget;
	alphaRate = 0.0f;
      }
    }
  }

  // set the tankNode color
  if (isPhantomZoned()) {
    teleAlpha = 1.0f;
    color[3] = 0.25f; // barely visible, regardless of teleporter proximity
  } else {
    World *world = World::getWorld();
    if (!world) {
      return;
    }
    teleporterProximity =
      world->getProximity(state.pos, BZDBCache::tankRadius);
    teleAlpha = (1.0f - (0.75f * teleporterProximity));

    if (alpha == 0.0f) {
      color[3] = 0.0f; // not trusting FP accuracy
    } else {
      color[3] = teleAlpha * alpha;
    }
  }

  return;
}


void Player::updateTreads(float dt)
{
  float speedFactor;
  float angularFactor;

  if ((state.status & PlayerState::UserInputs) != 0) {
    speedFactor = state.userSpeed;
    angularFactor = state.userAngVel;
  } else {
    speedFactor = relativeSpeed;
    angularFactor = relativeAngVel;
  }

  // setup the linear component
  if (dimensionsScale[0] > 1.0e-6f) {
    speedFactor = speedFactor / dimensionsScale[0];
  } else {
    speedFactor = speedFactor * 1.0e6f;
  }

  // setup the angular component
  const float angularScale = 4.0f; // spin factor (at 1.0, the edges line up)
  angularFactor *= angularScale;
  const float halfWidth = 0.5f * BZDBCache::tankWidth;
  // not using dimensions[1], because it may be set to 0.001 by a Narrow flag
  angularFactor *= dimensionsScale[0] * halfWidth;

  const float leftOff = dt * (speedFactor - angularFactor);
  const float rightOff = dt * (speedFactor + angularFactor);

  if (avatar)
    avatar->setTurnOffsets(leftOff,rightOff);

  return;
}


void Player::changeScore(float newRank,
			 short newWins,
			 short newLosses,
			 short newTeamKills)
{
  rank   = newRank;
  wins   = newWins;
  losses = newLosses;
  tks    = newTeamKills;

  eventHandler.PlayerScoreChange(*this);
}


void Player::changeLocalScore(short dWins, short dLosses, short dTeamKills)
{
  localWins += dWins;
  localLosses += dLosses;
  localTks += dTeamKills;
}


void Player::setFlag(FlagType* _flag)
{
  // set the type
  flagType = _flag;
  if (_flag == NULL)
    setShotType(StandardShot);
  updateFlagEffect(flagType);
  return;
}


void Player::updateFlagEffect(FlagType* effectFlag)
{
  float FlagEffectTime = BZDB.eval(StateDatabase::BZDB_FLAGEFFECTTIME);
  if (FlagEffectTime <= 0.0f) {
    FlagEffectTime = 0.001f; // safety
  }

  // set the dimension targets
  dimensionsTarget[0] = 1.0f;
  dimensionsTarget[1] = 1.0f;
  dimensionsTarget[2] = 1.0f;
  if (effectFlag == Flags::Obesity) {
    const float factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    dimensionsTarget[0] = factor;
    dimensionsTarget[1] = factor;
  }
  else if (effectFlag == Flags::Tiny) {
    const float factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    dimensionsTarget[0] = factor;
    dimensionsTarget[1] = factor;
  }
  else if (effectFlag == Flags::Thief) {
    const float factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    dimensionsTarget[0] = factor;
    dimensionsTarget[1] = factor;
  }
  else if (effectFlag == Flags::Narrow) {
    dimensionsTarget[1] = 0.001f;
  }

  // set the dimension rates
  for (int i = 0; i < 3; i++) {
    if (dimensionsTarget[i] != dimensionsScale[i]) {
      dimensionsRate[i] = dimensionsTarget[i] - dimensionsScale[i];
      dimensionsRate[i] = dimensionsRate[i] / FlagEffectTime;
    }
  }

  // set the alpha target
  if (effectFlag == Flags::Cloaking) {
    alphaTarget = 0.0f;
  } else {
    alphaTarget = 1.0f;
  }

  // set the alpha rate
  if (alphaTarget != alpha) {
    alphaRate = (alphaTarget - alpha) / FlagEffectTime;
  }

  return;
}


void Player::endShot(int index, bool isHit, bool showExplosion)
{
  fvec3 pos;
  if (doEndShot(index, isHit, pos) && showExplosion) {
    addShotExplosion(pos);
  }
  return;
}


void Player::setVisualTeam (TeamColor visualTeam)
{
  color.rgb() = Team::getTankColor(visualTeam).rgb();

  if (avatar) {
    avatar->setVisualTeam(visualTeam, color);
  }
}


void Player::fireJumpJets()
{
  state.jumpJetsScale = 1.0f;
  state.status |= PlayerState::JumpJets;
  return;
}


const std::string & Player::getCustomField ( const std::string & key )const
{
  static std::string emptyField;

  std::map<std::string,std::string>::const_iterator itr = customData.find( key);
  if (itr != customData.end())
      return itr->second;

  return emptyField;
}


void Player::clearRemoteSounds()
{
  state.sounds = PlayerState::NoSounds;
  state.status &= ~PlayerState::PlaySound;
  return;
}


void Player::addRemoteSound(int sound)
{
  state.sounds |= sound;
  if (state.sounds != PlayerState::NoSounds)
    state.status |= PlayerState::PlaySound;
  return;
}


void Player::addToScene(SceneDatabase* scene, TeamColor effectiveTeam,
			bool inCockpit, bool seerView,
			bool showTreads, bool showIDL, bool thirdPerson)
{
  const fvec4 groundPlane(0.0f, 0.0f, 1.0f, 0.0f);

  if (gfxBlock.blocked()) {
    return; // don't draw anything
  }

  if (!isAlive() && !isExploding()) {
    return; // don't draw anythinge
  }

  World *world = World::getWorld();
  if (!world) {
    return; // no world, shouldn't add to scene
  }

  if (!avatar) {
    return;
  }

  // place the tank
  avatar->move(state.pos, forward);

  // only use dimensions if we aren't at steady state.
  // this is done because it's more expensive to use
  // GL_NORMALIZE then to use precalculated normals.
  if (useDimensions)
    avatar->setScale(dimensionsScale);
  else {
    teAvatarScaleModes mode = eNormal;	// TODO, have the server set a player scale for these things, so it's not up to the client

    if (flagType == Flags::Obesity)
      mode = eFat;
    else if (flagType == Flags::Tiny)
      mode = eTiny;
    else if (flagType == Flags::Narrow)
      mode = eThin;
    else if (flagType == Flags::Thief)
      mode = eThief;
    avatar->setScale(mode);
  }

  // is this tank fully cloaked?
  bool cloaked = (flagType == Flags::Cloaking) && (color[3] == 0.0f);

  // in third person we draw like the radar does. cloak is visible, and ST is not
  if (thirdPerson)
    cloaked = (flagType == Flags::Stealth);

  if (cloaked && !seerView)
    return; // don't draw anything

  avatar->setVisualMode(inCockpit,showTreads);

  // adjust alpha for seerView
  if (seerView) {
    if (isPhantomZoned()) {
      color[3] = 0.25f;
    } else {
      color[3] = teleAlpha;
    }
  }

  setVisualTeam(effectiveTeam);

  if (isAlive()) {
    avatar->setAnimationValues(0,state.jumpJetsScale);

    std::vector<SceneNode*> nodeList = avatar->getSceneNodes();
    for ( int i = 0; i < (int)nodeList.size(); i++ ) {
      if (nodeList[i])
	scene->addDynamicNode(nodeList[i]);
    }

    if (isCrossingWall()) {
      // get which plane to compute IDL against
      fvec4 plane;
      const float a = atan2f(forward[1], forward[0]);
      const Obstacle* obstacle =
	world->hitBuilding(state.pos, a,
			   dimensions[0], dimensions[1],
			   dimensions[2]);
      if ((obstacle && obstacle->isCrossing(state.pos, a,
					   dimensions[0], dimensions[1],
					   dimensions[2], &plane)) ||
	  world->crossingTeleporter(state.pos, a,
				    dimensions[0], dimensions[1],
				    dimensions[2], &plane)) {
	// stick in interdimensional lights node
	if (showIDL && GfxBlockMgr::halos.notBlocked()) {
	  avatar->moveIDL(plane);
	  nodeList = avatar->getIDLSceneNodes();
	  for ( int i = 0; i < (int)nodeList.size(); i++ ) {
	    if (nodeList[i])
	      scene->addDynamicNode(nodeList[i]);
	  }
	}

	// add clipping plane to tank node
	if (!inCockpit) {
	  avatar->setClippingPlane(&plane);
	}
      }
    } else if (getPosition()[2] < 0.0f) {
      // this should only happen with Burrow flags
      avatar->setClippingPlane(&groundPlane);
    } // isCrossingWall()
  } else if (isExploding() && (state.pos[2] > ZERO_TOLERANCE)) {
    // isAlive()
    float t = float((TimeKeeper::getTick() - explodeTime) /
		    BZDB.eval(StateDatabase::BZDB_EXPLODETIME));
    if (t > 1.0f) {
      // FIXME - setStatus(DeadStatus);
      t = 1.0f;
    } else if (t < 0.0f) {
      // shouldn't happen but why take chances
      t = 0.0f;
    }
    // fade at the end of the explosion
    const float fadeRatio = 0.8f;
    if (t > fadeRatio) {
      fvec4 newColor = color;
      const float fadeFactor = (1.0f - t) / (1.0f - fadeRatio);
      newColor[3] = color[3] * fadeFactor;
      avatar->setColor(newColor);
    }
    avatar->setAnimationValues(t,0);
    avatar->setClippingPlane(&groundPlane); // shadows are not clipped

    std::vector<SceneNode*> nodeList = avatar->getSceneNodes();
    for ( int i = 0; i < (int)nodeList.size(); i++ ) {
      if (nodeList[i])
	scene->addDynamicNode(nodeList[i]);
    }
  }

  if (isAlive() && (isPaused() || isNotResponding())) {
    avatar->movePause(state.pos, 1.5f * BZDBCache::tankRadius * dimensionsScale[0]);

    std::vector<SceneNode*> nodeList = avatar->getPauseSceneNodes();
    for ( int i = 0; i < (int)nodeList.size(); i++ ) {
      if ( nodeList[i] )
	scene->addDynamicNode(nodeList[i]);
    }
  }
}


void Player::setLandingSpeed(float velocity)
{
  eventHandler.PlayerLanded(*this, velocity);

  float squishiness = BZDB.eval(StateDatabase::BZDB_SQUISHFACTOR);
  if (squishiness < 0.001f) {
    return;
  }
  float squishTime = BZDB.eval(StateDatabase::BZDB_SQUISHTIME);
  if (squishTime < 0.001) {
    return;
  }
  const float gravity = BZDBCache::gravity;
  if (velocity > 0.0f) {
    velocity = 0.0f;
  }

  // Setup so that a drop height of BZDB_GRAVITY squishes
  // by a factor of 1/11, when BZDB_SQUISHFACTOR is set to 1
  //
  // G = gravity;  V = velocity;  D = fall distance; K = factor
  //
  // V = sqrt (2 * D * G)
  // V = sqrt(2) * G  { @ D = G)
  // scale = 1 / (1 + (K * V^2))
  // scale = 1 / (1 + (K * 2 * G^2))
  // set: (K * 2 * G^2) = 0.1
  // K = 0.1 / (2 * G^2)
  //
  float k = 0.1f / (2.0f * gravity * gravity);
  k = k * squishiness;
  if (flagType == Flags::Bouncy) {
    k = k * 4.0f;
  }
  const float newZscale = 1.0f / (1.0f + (k * (velocity * velocity)));

  // don't update if the tank is still recovering
  // from a spawn effect or a larger fall.
  if (newZscale < dimensionsScale[2]) {
    dimensionsScale[2] = newZscale;
    // use a fixed decompression rate
    dimensionsRate[2] = 1.0f / squishTime;
  }

  return;
}


void Player::spawnEffect()
{
  const float squishiness = BZDB.eval(StateDatabase::BZDB_SQUISHFACTOR);
  if (squishiness > 0.0f) {
    const float effectTime = BZDB.eval(StateDatabase::BZDB_FLAGEFFECTTIME);
    const float factor = 1.0f / effectTime;
    for (int i = 0; i < 3; i++) {
      dimensionsRate[i] = factor;
      dimensionsScale[i] = 0.01f;
    }
  }
  spawnTime = TimeKeeper::getCurrent();
  return;
}


void Player::addShots(SceneDatabase* scene, bool colorblind ) const
{
  const int count = getMaxShots();
  for (int i = 0; i < count; i++) {
    ShotPath* shot = getShot(i);
    if (shot && !shot->isExpiring() && !shot->isExpired()) {
      if (shot->getGfxBlock().notBlocked()) {
        shot->addShot(scene, colorblind);
      }
    }
  }
}


void* Player::unpack(void* buf, uint16_t code)
{
  double timestamp;
  PlayerId ident;

  buf = nboUnpackUInt8(buf, ident);
  buf = nboUnpackDouble(buf, timestamp);
  buf = state.unpack(buf, code);

  setDeadReckoning(timestamp);
  setRelativeMotion();

  return buf;
}


bool Player::validTeamTarget(const Player *possibleTarget) const
{
  TeamColor myTeam = getTeam();
  TeamColor targetTeam = possibleTarget->getTeam();
  if (myTeam != targetTeam) {
    return true;
  }

  if (myTeam != RogueTeam) {
    return false;
  }

  World *world = World::getWorld();
  return (world && !world->allowRabbit());
}


void Player::getDeadReckoning(fvec3& predictedPos, float& predictedAzimuth,
                              fvec3& predictedVel, float dt) const
{
  predictedAzimuth = inputAzimuth;

  if (inputStatus & PlayerState::Paused) {
    // don't move when paused
    predictedPos[0] = inputPos[0];
    predictedPos[1] = inputPos[1];
    predictedPos[2] = inputPos[2];
    predictedVel[0] = 0.0f;
    predictedVel[1] = 0.0f;
    predictedVel[2] = 0.0f;
  }
  else if (inputStatus & PlayerState::Falling) {
    // no control when falling
    predictedVel[0] = inputVel[0];
    predictedVel[1] = inputVel[1];
    predictedPos[0] = inputPos[0] + (dt * inputVel[0]);
    predictedPos[1] = inputPos[1] + (dt * inputVel[1]);
    // only turn if alive
    if (inputStatus & PlayerState::Alive) {
      predictedAzimuth += (dt * inputAngVel);
    }
    // following the parabola
    predictedVel[2] = inputVel[2] + (BZDBCache::gravity * dt);
    predictedPos[2] = inputPos[2] + (inputVel[2] * dt) +
      (0.5f * BZDBCache::gravity * dt * dt);
  } else {
    // velocity[2] is zero when not falling, except for Burrow flag
    predictedVel[2] = inputVel[2];
    predictedPos[2] = inputPos[2] + (inputVel[2] * dt);

    // different algorithms for tanks moving in
    // a straight line vs. turning in a circle
    if (!inputTurning) {
      // move straight
      predictedVel[0] = inputRelVel[0];
      predictedVel[1] = inputRelVel[1];
      predictedPos[0] = inputPos[0] + (dt * inputRelVel[0]);
      predictedPos[1] = inputPos[1] + (dt * inputRelVel[1]);
    } else {
      // make a sweeping arc
      const float angle = (dt * inputRelAngVel);
      predictedAzimuth += angle;
      const float cos_val = cosf(angle);
      const float sin_val = sinf(angle);
      const float* tc = inputTurnCenter;
      const float* tv = inputTurnVector;
      predictedPos[0] = tc[0] + ((tv[0] * cos_val) - (tv[1] * sin_val));
      predictedPos[1] = tc[1] + ((tv[1] * cos_val) + (tv[0] * sin_val));
      const float* rv = inputRelVel;
      predictedVel[0] = (rv[0] * cos_val) - (rv[1] * sin_val);
      predictedVel[1] = (rv[1] * cos_val) + (rv[0] * sin_val);
    }

    // make the physics driver adjustments
    const PhysicsDriver* phydrv = PHYDRVMGR.getDriver(inputPhyDrv);
    if (phydrv != NULL) {
      if (phydrv->getIsSlide()) {
	predictedVel[0] = inputRelVel[0];
	predictedVel[1] = inputRelVel[1];
	predictedPos[0] = inputPos[0] + (dt * inputRelVel[0]);
	predictedPos[1] = inputPos[1] + (dt * inputRelVel[1]);
      } else {
	// angular velocity adjustment
	const float pdAngVel = phydrv->getAngularVel();
	if (pdAngVel != 0.0f) {
	  const float angle = (dt * pdAngVel);
	  predictedAzimuth += angle;
	  const fvec2& pdAngPos = phydrv->getAngularPos();
	  const float dx = predictedPos[0] - pdAngPos[0];
	  const float dy = predictedPos[1] - pdAngPos[1];
	  const float cos_val = cosf(angle);
	  const float sin_val = sinf(angle);
	  predictedPos[0] = pdAngPos[0] + ((dx * cos_val) - (dy * sin_val));
	  predictedPos[1] = pdAngPos[1] + ((dy * cos_val) + (dx * sin_val));
	  predictedVel[0] += (-dy * pdAngVel);
	  predictedVel[1] += (+dx * pdAngVel);
	}
	// linear velocity adjustment
	const fvec3& pdVel = phydrv->getLinearVel();
	predictedPos[0] += (dt * pdVel[0]);
	predictedPos[1] += (dt * pdVel[1]);
	predictedVel[0] += pdVel[0];
	predictedVel[1] += pdVel[1];
      }
    }
  }

  return;
}


bool Player::isDeadReckoningWrong() const
{
  const uint16_t checkStates =
    (PlayerState::Alive | PlayerState::Paused | PlayerState::Falling);
  // always send a new packet when some kinds of status change
  if ((state.status & checkStates) != (inputStatus & checkStates)) {
    return true;
  }

  // never send a packet when dead
  if ((state.status & PlayerState::Alive) == 0) {
    return false;
  }

  //  send a packet if we've made some noise
  if (state.sounds != PlayerState::NoSounds) {
    return true;
  }

  //  send a packet if we've crossed a physics driver boundary
  if (state.phydrv != inputPhyDrv) {
    return true;
  }

  // time since setdeadreckoning
  const double dt = syncedClock.GetServerSeconds() - updateTimeStamp;

  // otherwise always send at least one packet per second
  if (dt >= MaxUpdateTime) {
    return true;
  }

  // get predicted state
  fvec3 predictedPos;
  fvec3 predictedVel;
  float predictedAzimuth;
  getDeadReckoning(predictedPos, predictedAzimuth, predictedVel, (float)dt);

  // always send a new packet on reckoned touchdown
  float groundLimit = 0.0f;
  if (getFlag() == Flags::Burrow) {
    groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);
  }
  if (predictedPos[2] < groundLimit) {
    return true;
  }

  // client side throttling
  const int throttleRate = int(BZDB.eval(StateDatabase::BZDB_UPDATETHROTTLERATE));
  const float minUpdateTime = (throttleRate > 0) ? (1.0f / throttleRate) : 0.0f;
  if (dt < minUpdateTime) {
    return false;
  }

  // see if position and azimuth are close enough
  float positionTolerance = BZDB.eval(StateDatabase::BZDB_POSITIONTOLERANCE);
  if ((fabsf(state.pos[0] - predictedPos[0]) > positionTolerance) ||
      (fabsf(state.pos[1] - predictedPos[1]) > positionTolerance) ||
      (fabsf(state.pos[2] - predictedPos[2]) > positionTolerance)) {
    if (debugLevel >= 4) {
      if (fabsf(state.pos[0] - predictedPos[0]) > positionTolerance) {
	printf ("state.pos[0] = %f, predictedPos[0] = %f\n",
		state.pos[0], predictedPos[0]);
      }
      if (fabsf(state.pos[1] - predictedPos[1]) > positionTolerance) {
	printf ("state.pos[1] = %f, predictedPos[1] = %f\n",
		state.pos[1], predictedPos[1]);
      }
      if (fabsf(state.pos[2] - predictedPos[2]) > positionTolerance) {
	printf ("state.pos[2] = %f, predictedPos[2] = %f\n",
		state.pos[2], predictedPos[2]);
      }
    }
    return true;
  }

  float angleTolerance = BZDB.eval(StateDatabase::BZDB_ANGLETOLERANCE);
  if (fabsf(state.azimuth - predictedAzimuth) > angleTolerance) {
    logDebugMessage(4,"state.azimuth = %f, predictedAzimuth = %f\n",
		    state.azimuth, predictedAzimuth);
    return true;
  }

  // prediction is good enough
  return false;
}


void Player::doDeadReckoning()
{
  if (!isAlive() && !isExploding())
    return;

  // get predicted state
  fvec3 predictedPos;
  fvec3 predictedVel;
  float predictedAzimuth;

  double dt = syncedClock.GetServerSeconds() - updateTimeStamp;
  getDeadReckoning(predictedPos, predictedAzimuth, predictedVel, (float)dt);

  // setup notResponding
  if (!isAlive())
    notResponding = false;
  else
    notResponding = (dt > BZDB.eval(StateDatabase::BZDB_NOTRESPONDINGTIME));

  bool hitWorld = false;
  bool ZHit = false;
  
  // if the tanks hits something in Z then update input state (we don't want to fall anymore)
  float zLimit = 0.0f;
  if (getFlag() == Flags::Burrow )
    zLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);

  // check for collisions and correct accordingly
  World *world = World::getWorld();
  if (world) {
	bool expel;
    const Obstacle* obstacle = getHitBuilding(inputPos, inputAzimuth, 
		  predictedPos, predictedAzimuth, !isSolid(), expel);

    // did they hit something?
    if (obstacle && expel) {
      fvec3 hitPos;
      fvec3 hitVelocity;
	  fvec3 hitNormal;
	  float hitAzimuth;

	  // get the normal for the collision
      if (!getHitNormal(obstacle, inputPos, inputAzimuth, 
		  predictedPos, predictedAzimuth, hitNormal)) {
        obstacle->getNormal(inputPos, hitNormal);
      }

	  // get the corrected position
	  // FIXME: Azimuth isn't corrected properly
      hitWorld = getHitCorrection(inputPos, inputAzimuth, predictedPos, 
		  predictedAzimuth, inputVel, dt, zLimit, hitVelocity, hitPos, 
		  &hitAzimuth);

      if (hitWorld) {
		// there was a collision, copy corrected position and velocity
		predictedPos[0] = hitPos[0];
		predictedPos[1] = hitPos[1];
		predictedPos[2] = hitPos[2];
		predictedVel[0] = hitVelocity[0];
		predictedVel[1] = hitVelocity[1];
		predictedVel[2] = hitVelocity[2];
		//predictedAzimuth = hitAzimuth; 

		// check if the collision was in the Z direction
		if (hitNormal[2] > 0.001f)
		{
		  zLimit = hitPos[2];
		  ZHit = true;
		}
	  }
    }
  }
  

  // the velocity check is for when a Burrow flag is dropped
  if (ZHit || ((predictedPos[2] <= zLimit) 
	  && (predictedVel[2] <= 0.0f))) {
    predictedPos[2] = zLimit;
    predictedVel[2] = 0.0f;
    inputStatus &= ~PlayerState::Falling;
    inputVel[2] = 0.0f;
	hitWorld = true;
  }

  // setup remote players' landing sounds and graphics, and jumping sounds
  if (isAlive() && !headless) {
    // the importance level of the remote sounds
    const bool soundImportance = false;
    const bool localSound = (ROAM.isRoaming() && (ROAM.getMode() == Roaming::roamViewFP) && (ROAM.getTargetTank() == this));

    // check for a landing
    if (((oldStatus & PlayerState::Falling) != 0) && ((inputStatus & PlayerState::Falling) == 0)) {
      setLandingSpeed(oldZSpeed);    // setup the squish effect

      EFFECTS.addLandEffect(getColor(),predictedPos,state.azimuth);    // make it "land"

      // setup the sound
      if (BZDB.isTrue("remoteSounds")){
        if ((getFlag() != Flags::Burrow) || (predictedPos[2] > 0.0f))
          SOUNDSYSTEM.play(SFX_LAND, state.pos, soundImportance, localSound);
        else    // probably never gets played
          SOUNDSYSTEM.play(SFX_BURROW, state.pos, soundImportance, localSound);
      }

      // play jumping type sounds, and then clear them
      if (state.sounds != PlayerState::NoSounds){
        if (BZDB.isTrue("remoteSounds")) {
          if ((state.sounds & PlayerState::JumpSound) != 0)
            SOUNDSYSTEM.play(SFX_JUMP, state.pos, soundImportance, localSound);
          if ((state.sounds & PlayerState::WingsSound) != 0)
            SOUNDSYSTEM.play(SFX_FLAP, state.pos, soundImportance, localSound);
          if ((state.sounds & PlayerState::BounceSound) != 0)
            SOUNDSYSTEM.play(SFX_BOUNCE, state.pos, soundImportance, localSound);
        }
        state.sounds = PlayerState::NoSounds;
      }
    }
  }

  // copy some old state
  oldZSpeed = state.velocity[2];
  oldStatus = inputStatus;

  move(predictedPos, predictedAzimuth);
  setVelocity(predictedVel);
  setRelativeMotion();

  if (hitWorld) { // if we hit something, then we want to DR from here now, instead of from the starting point
    setDeadReckoning(syncedClock.GetServerSeconds());
  }

  return;
}


// How long does the filter takes to be considered "initialized"
const int   DRStateStable      = 100;
const float maxToleratedJitter = 1.0f;

void Player::setDeadReckoning(double timestamp)
{
  // set the current state
  inputTime = TimeKeeper::getTick();

#ifdef DEBUG
  double currentServerTime = syncedClock.GetServerSeconds();
  if (currentServerTime <= timestamp)
    printf("Player::setDeadReckoning(): currentServerTime %11.4f ought to be greater than timestamp %11.4f\n", currentServerTime, timestamp);
#endif
  updateTimeStamp = timestamp;

  // copy stuff for dead reckoning
  inputStatus = state.status;
  inputAzimuth = state.azimuth;
  inputAngVel = state.angVel;
  inputPos = state.pos;
  inputVel = state.velocity;
  inputPhyDrv = state.phydrv;

  //
  // pre-calculate some stuff for dead reckoning
  //

  // the relative motion information (with respect to the physics drivers)
  calcRelativeMotion(inputRelVel, inputRelSpeed, inputRelAngVel);

  // setup the turning parameters
  inputTurning = false;
  if (fabsf(inputRelAngVel) > 0.001f) {
    inputTurning = true;
    const float radius = (inputRelSpeed / inputRelAngVel);
    inputTurnVector[0] = +sinf(inputAzimuth) * radius;
    inputTurnVector[1] = -cosf(inputAzimuth) * radius;
    inputTurnCenter[0] = inputPos[0] - inputTurnVector[0];
    inputTurnCenter[1] = inputPos[1] - inputTurnVector[1];
  }

  setRelativeMotion ();
}


void Player::renderRadar() const
{
  if (!isAlive() && !isExploding()) {
    return; // don't draw anything
  }
  if (avatar) {
    avatar->renderRadar();
  }
}


bool Player::getIpAddress(Address& addr)
{
  if (!haveIpAddr) {
    return false;
  }
  addr = ipAddr;
  return true;
}


void Player::setIpAddress(const Address& addr)
{
  ipAddr = addr;
  haveIpAddr = true;
}

ShotPath* Player::getShot(int index) const
{
  index &= 0x00FF;
  if (index >= (int)shots.size())
    return NULL;
  return shots[index];
}

void Player::prepareShotInfo(FiringInfo &firingInfo)
{
  firingInfo.shot.dt = 0.0f;
  firingInfo.lifetime = BZDB.eval(StateDatabase::BZDB_RELOADTIME);

  firingInfo.flagType = getFlag();
  // wee bit o hack -- if phantom flag but not phantomized
  // the shot flag is normal -- otherwise FiringInfo will have
  // to be changed to add a real bitwise status variable
  if (getFlag() == Flags::PhantomZone && !isFlagActive())
    firingInfo.shotType = StandardShot;

  firingInfo.shot.team = getTeam();

  if (firingInfo.shotType == ShockWaveShot) {
    // move shot origin under tank and make it stationary
    const fvec3& pos = getPosition();
    firingInfo.shot.pos[0] = pos[0];
    firingInfo.shot.pos[1] = pos[1];
    firingInfo.shot.pos[2] = pos[2];
    firingInfo.shot.vel[0] = 0.0f;
    firingInfo.shot.vel[1] = 0.0f;
    firingInfo.shot.vel[2] = 0.0f;
  } else {
    getMuzzle(firingInfo.shot.pos);

    const fvec3& dir     = getForward();
    const fvec3& tankVel = getVelocity();
    float shotSpeed      = BZDB.eval(StateDatabase::BZDB_SHOTSPEED);

    if (handicap > 0.0f) {
      // apply any handicap advantage to shot speed
      const float speedAd = 1.0f + (handicap * (BZDB.eval(StateDatabase::BZDB_HANDICAPSHOTAD) - 1.0f));
      shotSpeed *= speedAd;
    }

    firingInfo.shot.vel[0] = tankVel[0] + shotSpeed * dir[0];
    firingInfo.shot.vel[1] = tankVel[1] + shotSpeed * dir[1];
    firingInfo.shot.vel[2] = tankVel[2] + shotSpeed * dir[2];

    // Set _shotsKeepVerticalVelocity on the server if you want shots
    // to have the same vertical velocity as the tank when fired.
    // keeping shots moving horizontally makes the game more playable.
    if (!BZDB.isTrue(StateDatabase::BZDB_SHOTSKEEPVERTICALV))
      firingInfo.shot.vel[2] = 0.0f;
  }
}

void Player::addShot(ShotPath *shot, const FiringInfo &info)
{
  int shotNum = int(shot->getShotId() & 255);

  if (shotNum >= (int)shots.size())
    shots.resize(shotNum+1);
  else if (shots[shotNum] != NULL)
    delete shots[shotNum];

  shots[shotNum] = shot;
  shotStatistics.recordFire(info.flagType,getForward(),shot->getVelocity());
}

void Player::updateShot ( FiringInfo &info, int shotID, double time )
{
  // kill the old shot
  if (shotID < (int)shots.size()) {
    if ( shots[shotID] != NULL ) {
      delete shots[shotID];
      shots[shotID] = NULL;
    }
  }
  else
    return;

  // build a new shot with the new info
  prepareShotInfo(info);
  shots[shotID] = new LocalShotPath(info,time);
}


void Player::setHandicap(float _handicap)
{
  handicap = _handicap;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
