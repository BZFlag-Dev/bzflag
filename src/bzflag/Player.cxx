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

#include <string.h>
#include <string>
#include "common.h"
#include "playing.h"
#include "Player.h"
#include "World.h"
#include "TankSceneNode.h"
#include "SphereSceneNode.h"
#include "SceneDatabase.h"
#include "OpenGLMaterial.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "ShotPath.h"
#include "ShotStatistics.h"
#include "CollisionManager.h"
#include "Obstacle.h"
#include "PhysicsDriver.h"
#include "TrackMarks.h"
#include "sound.h"


// for dead reckoning
static const float	MaxUpdateTime = 1.0f;		// seconds

//
// Player
//

int		Player::tankTexture = -1;

Player::Player(const PlayerId& _id, TeamColor _team,
	       const char* name, const char* _email, const PlayerType _type) :
  lastObstacle(NULL),
  notResponding(false),
  autoPilot(false),
  hunted(false),
  id(_id),
  lastVisualTeam(NoTeam),
  team(_team),
  type(_type),
  flagType(Flags::Null),
  fromTeleporter(0),
  toTeleporter(0),
  teleporterProximity(0.0f),
  wins(0),
  losses(0),
  tks(0),
  localWins(0),
  localLosses(0),
  localTks(0),
  deltaTime(0.0),
  offset(0.0),
  deadReckoningState(0)
{
  static const float zero[3] = { 0.0f, 0.0f, 0.0f };
  move(zero, 0.0f);
  setVelocity(zero);
  setAngularVelocity(0.0f);
  setPhysicsDriver(-1);
  setDeadReckoning();
  setRelativeMotion();
  setUserSpeed(0.0f);
  setUserAngVel(0.0f);
  
  // set call sign
  ::strncpy(callSign, name, CallSignLen);
  callSign[CallSignLen-1] = '\0';

  // set email address
  ::strncpy(email, _email, EmailLen);
  email[EmailLen-1] = '\0';

  if (id != ServerPlayer) {
    // make scene nodes
    tankNode = new TankSceneNode(state.pos, forward);
    tankIDLNode = new TankIDLSceneNode(tankNode);
    changeTeam(team);
    pausedSphere = new SphereSceneNode(state.pos, 1.5f * BZDBCache::tankRadius);
    pausedSphere->setColor(0.0f, 0.0f, 0.0f, 0.5f);
  }

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

  lastTrackDraw = TimeKeeper::getCurrent();
  
  return;
}

Player::~Player()
{
  if (id != ServerPlayer) {
    delete tankIDLNode;
    delete tankNode;
    delete pausedSphere;
  }
}

// Take into account the quality of player wins/(wins+loss)
// Try to penalize winning casuality
static float rabbitRank (int wins, int losses) {
  // otherwise do score-based ranking
  int sum = wins + losses;
  if (sum == 0)
    return 0.5;
  float average = (float)wins/(float)sum;
  // IIRC that is how wide is the gaussian
  float penalty = (1.0f - 0.5f / sqrt((float)sum));
  return average * penalty;
}


short Player::getRabbitScore() const
{
  return (int)(rabbitRank(wins, losses) * 100.0);
}


float Player::getRadius() const
{
  // NOTE: this encompasses everything but Narrow
  //       the Obese, Tiny, and Thief flags adjust
  //       the radius, but Narrow does not.
  return (dimensionsScale[0] * BZDBCache::tankRadius);
}


void Player::getMuzzle(float* m) const
{
  // NOTE: like getRadius(), we only use dimensionsScale[0].
  //       as well, we do not use BZDB_MUZZLEFRONT, but the
  //       0.1f value listed in global.cxx is added on to the
  //       scaled version of tankRadius.
  float front = (dimensionsScale[0] * BZDBCache::tankRadius);
  if (dimensionsRate[0] > 0.0f) {
    front = front + (dimensionsRate[0] * 0.1f);
  }
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


void Player::move(const float* _pos, float _azimuth)
{
  // assumes _forward is normalized
  state.pos[0] = _pos[0];
  state.pos[1] = _pos[1];
  state.pos[2] = _pos[2];
  state.azimuth = _azimuth;

  // limit angle
  if (state.azimuth < 0.0f) {
    state.azimuth = (2.0f * M_PI) - fmodf(-state.azimuth, (2.0f * M_PI));
  }
  else if (state.azimuth >= (2.0f * M_PI)) {
    state.azimuth = fmodf(state.azimuth, (2.0f * M_PI));
  }

  // update forward vector (always in horizontal plane)
  forward[0] = cosf(state.azimuth);
  forward[1] = sinf(state.azimuth);
  forward[2] = 0.0f;

  // compute teleporter proximity
  if (World::getWorld()) {
    teleporterProximity =
      World::getWorld()->getProximity(state.pos, BZDBCache::tankRadius);
  }
}


void Player::setVelocity(const float* _velocity)
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
  if (phydrv != NULL) {
    state.status |= PlayerState::OnDriver;
  } else {
    state.status &= ~PlayerState::OnDriver;
  }

  return;
}


void Player::setRelativeMotion()
{
  bool falling = (state.status & short(PlayerState::Falling)) != 0;
  if (falling && (getFlag() != Flags::Wings)) {
    // no adjustments while falling
    return;
  }

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
    const float* v = phydrv->getVelocity();
    const float av = phydrv->getAngularVel();
    const float* ap = phydrv->getAngularPos();
    
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
  // set team
  team = _team;

  // set the scene node
  setVisualTeam(team);
}


void Player::setStatus(short _status)
{
  state.status = _status;
}


void Player::setExplode(const TimeKeeper& t)
{
  if (!isAlive()) return;
  explodeTime = t;
  setStatus((getStatus() | short(PlayerState::Exploding) | short(PlayerState::Falling)) &
	    ~(short(PlayerState::Alive) | short(PlayerState::Paused)));
  tankNode->rebuildExplosion();
  // setup the flag effect to revert to normal
  updateFlagEffect(Flags::Null);
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
  float jumpVel;
  if (getFlag() == Flags::Wings) {
    jumpVel = BZDB.eval(StateDatabase::BZDB_WINGSJUMPVELOCITY);
  } else {
    jumpVel = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
  }
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
  
  if (isAlive() && !isFalling() && 
      ((getFlag() != Flags::PhantomZone) || !isFlagActive())) {
    const float lifeTime = TimeKeeper::getCurrent() - lastTrackDraw;
    if (lifeTime > TrackMarks::updateTime) {
      bool drawMark = true;
      float markPos[3];
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
  float oldRates[3];
  float oldScales[3];
  float oldDimensions[3];
  memcpy (oldRates, dimensionsRate, sizeof(float[3]));
  memcpy (oldScales, dimensionsScale, sizeof(float[3]));
  memcpy (oldDimensions, dimensions, sizeof(float[3]));

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
      memcpy (dimensions, oldDimensions, sizeof(float[3]));
      memcpy (dimensionsScale, oldScales, sizeof(float[3]));
      memcpy (dimensionsRate, oldRates, sizeof(float[3]));
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
  const float* dims = dimensions;

  // check walls
  const World* world = World::getWorld();
  if (world) {
    const std::vector<WallObstacle*>& walls = world->getWalls();
    std::vector<WallObstacle*>::const_iterator wallScan = walls.begin();
    while (wallScan != walls.end()) {
      const WallObstacle& wall = **wallScan;
      if (wall.inBox(getPosition(), getAngle(), dims[0], dims[1], dims[2])) {
	return true;
      }
      wallScan++;
    }
  }

  // check everything else
  const ObsList* olist =
    COLLISIONMGR.boxTest(getPosition(), getAngle(), dims[0], dims[1], dims[2]);

  for (int i = 0; i < olist->count; i++) {
    const Obstacle* obs = olist->list[i];
    const bool onTop = obs->isFlatTop() &&
      ((obs->getPosition()[2] + obs->getHeight()) <= getPosition()[2]);
    if (!obs->isDriveThrough() && !onTop &&
	obs->inBox(getPosition(), getAngle(), dims[0], dims[1], dims[2])) {
      return true;
    }
  }

  return false;
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
  if ((flagType == Flags::PhantomZone) && isFlagActive()) {
    color[3] = 0.25f; // barely visible, regardless of teleporter proximity
  }
  else if (alpha == 0.0f) {
    color[3] = 0.0f;
  }
  else {
    teleporterProximity =
      World::getWorld()->getProximity(state.pos, BZDBCache::tankRadius);
    color[3] = alpha * (1.0f - (0.75f * teleporterProximity));
  }
  tankNode->setColor(color);

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
  const float angularScale = 2.0f; // spin factor (at 1.0, the edges line up)
  angularFactor *= angularScale;
  const float halfWidth = 0.5f * BZDBCache::tankWidth;
  // not using dimensions[1], because it may be set to 0.001 by a Narrow flag
  angularFactor *= dimensionsScale[0] * halfWidth;
    
  const float leftOff = dt * (speedFactor - angularFactor);
  const float rightOff = dt * (speedFactor + angularFactor);

  tankNode->addTreadOffsets(leftOff, rightOff);

  return;
}


void Player::changeScore(short deltaWins, short deltaLosses, short deltaTeamKills)
{
  wins += deltaWins;
  losses += deltaLosses;
  tks += deltaTeamKills;
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
  float pos[3];
  if (doEndShot(index, isHit, pos) && showExplosion) {
    addShotExplosion(pos);
  }
  return;
}


void Player::setVisualTeam (TeamColor visualTeam)
{
  // only do all this junk when the effective team color actually changes
  if (visualTeam == lastVisualTeam)
    return;
  lastVisualTeam = visualTeam;

  static const GLfloat	tankSpecular[3] = { 0.1f, 0.1f, 0.1f };
  static GLfloat	tankEmissive[3] = { 0.0f, 0.0f, 0.0f };
  static float		tankShininess = 20.0f;
  static GLfloat	rabbitEmissive[3] = { 0.0f, 0.0f, 0.0f };
  static float		rabbitShininess = 100.0f;

  GLfloat *emissive;
  GLfloat shininess;

  if (visualTeam == RabbitTeam) {
    emissive = rabbitEmissive;
    shininess = rabbitShininess;
  } else {
    emissive = tankEmissive;
    shininess = tankShininess;
  }

  // get the texture each time, since it's just a refrence
  const bool hunter = World::getWorld()->allowRabbit() && visualTeam != RabbitTeam;

  TextureManager &tm = TextureManager::instance();
  std::string texName;
  if (hunter)
    texName = BZDB.get("hunterTeamPrefix");
  else
    texName = Team::getImagePrefix(visualTeam);

  texName += BZDB.get("tankTexture");

  // now after we did all that, see if they have a user texture
  tankTexture = -1;
  if (userTexture.size())
    tankTexture = tm.getTextureID(userTexture.c_str(),false);

  // if the user load failed try our calculated texture
  if (tankTexture < 0)
    tankTexture = tm.getTextureID(texName.c_str(),false);

  // we are the hunter, we are orange..
  // TODO this is cheap, just untill a "hunter" team is made
  if (hunter) {
    color[0] = 1.0f;
    color[1] = 0.5f;
    color[2] = 0.0f;
  } else {
    const float* _color = Team::getTankColor(visualTeam);
    color[0] = _color[0];
    color[1] = _color[1];
    color[2] = _color[2];
  }
  color[3] = (getFlag() == Flags::PhantomZone) && isFlagActive() ? 0.5f : 1.0f;
  tankNode->setColor(color);
  tankNode->setMaterial(OpenGLMaterial(tankSpecular, emissive, shininess));
  tankNode->setTexture(tankTexture);
  
  int jumpJetsTexture = tm.getTextureID("jumpjets", false);
  tankNode->setJumpJetsTexture(jumpJetsTexture);
}


void Player::fireJumpJets()
{
  state.jumpJetsScale = 1.0f;
  state.status |= PlayerState::JumpJets;
  return;
}


void Player::addToScene(SceneDatabase* scene, TeamColor effectiveTeam,
			bool inCockpit, bool showIDL)
{
  if (!isAlive() && !isExploding()) {
    return;
  }

  tankNode->move(state.pos, forward);
  setVisualTeam(effectiveTeam);

  // only use dimensions if we aren't at steady state.
  // this is done because it's more expensive to use
  // GL_NORMALIZE then to use precalculated normals.
  if (!useDimensions) {
    tankNode->ignoreDimensions();
    if (flagType == Flags::Obesity) tankNode->setObese();
    else if (flagType == Flags::Tiny) tankNode->setTiny();
    else if (flagType == Flags::Narrow) tankNode->setNarrow();
    else if (flagType == Flags::Thief) tankNode->setThief();
    else tankNode->setNormal();
  }
  else {
    tankNode->setDimensions(dimensionsScale);
  }

  tankNode->setInTheCockpit(inCockpit);

  // reset the clipping plane
  tankNode->setClipPlane(NULL);

  tankNode->setJumpJets(0.0f);
  
  if (isAlive()) {
    tankNode->setExplodeFraction(0.0f);
    tankNode->setJumpJets(state.jumpJetsScale);
    scene->addDynamicNode(tankNode);

    if (isCrossingWall()) {
      // get which plane to compute IDL against
      GLfloat plane[4];
      const GLfloat a = atan2f(forward[1], forward[0]);
      const Obstacle* obstacle =
	World::getWorld()->hitBuilding(state.pos, a,
				       dimensions[0], dimensions[1],
				       dimensions[2]);
      if (obstacle && obstacle->isCrossing(state.pos, a,
					   dimensions[0], dimensions[1],
					   dimensions[2], plane) ||
	  World::getWorld()->crossingTeleporter(state.pos, a,
						dimensions[0], dimensions[1],
						dimensions[2], plane)) {
	// stick in interdimensional lights node
	if (showIDL) {
	  tankIDLNode->move(plane);
	  scene->addDynamicNode(tankIDLNode);
	}

	// add clipping plane to tank node
	if (!inCockpit) {
	  tankNode->setClipPlane(plane);
	}
      }
    }
    else if (getPosition()[2] < 0.0f) {
      // this should only happen with Burrow flags
      GLfloat plane[4];
      plane[0] = plane[1] = 0.0f;
      plane[2] = 1.0f;
      plane[3] = 0.0f;
      tankNode->setClipPlane(plane);
    } // isCrossingWall()
  }   // isAlive()
  else if (isExploding() && state.pos[2] > ZERO_TOLERANCE) {
    float t = (TimeKeeper::getTick() - explodeTime) /
	      BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
    if (t > 1.0f) {
      // FIXME - setStatus(DeadStatus);
      t = 1.0f;
    } else if (t < 0.0f) {
      // shouldn't happen but why take chances
      t = 0.0f;
    }
    tankNode->setExplodeFraction(t);
    scene->addDynamicNode(tankNode);
  }

  if (isAlive() && (isPaused() || isNotResponding())) {
    pausedSphere->move(state.pos,
		       1.5f * BZDBCache::tankRadius * dimensionsScale[0]);
    scene->addDynamicSphere(pausedSphere);
  }
}


bool Player::needsToBeRendered(bool cloaked, bool showTreads)
{
  if (cloaked && !showTreads) {
    return false;
  }

  // setup the visibility properties
  if (cloaked) {
    tankNode->setCloaked(true); // show the fading effect
  }
  else if (!showTreads) {
    tankNode->setHidden(true); // just shadows
  }
  else {
    tankNode->setHidden(false); // show all
  }

  return true;
}


void Player::setLandingSpeed(float velocity)
{
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
  return;
}


int Player::getMaxShots() const
{
  return World::getWorld()->getMaxShots();
}


void Player::addShots(SceneDatabase* scene, bool colorblind) const
{
  const int count = getMaxShots();
  for (int i = 0; i < count; i++) {
    ShotPath* shot = getShot(i);
    if (shot && !shot->isExpiring() && !shot->isExpired())
      shot->addShot(scene, colorblind);
  }
}


void* Player::unpack(void* buf, uint16_t code)
{
  float timestamp;
  PlayerId id;
  
  buf = nboUnpackFloat(buf, timestamp);
  buf = nboUnpackUByte(buf, id);
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

  return !World::getWorld()->allowRabbit();
}


void Player::getDeadReckoning(float* predictedPos, float* predictedAzimuth,
			      float* predictedVel, float dt) const
{
  *predictedAzimuth = inputAzimuth;
  
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
      *predictedAzimuth += (dt * inputAngVel);
    }
    // following the parabola
    predictedVel[2] = inputVel[2] + (BZDBCache::gravity * dt);
    predictedPos[2] = inputPos[2] + (inputVel[2] * dt) +
                      (0.5f * BZDBCache::gravity * dt * dt);
  }
  else {
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
      *predictedAzimuth += angle;
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
      }
      else {
        // angular velocity adjustment
        const float pdAngVel = phydrv->getAngularVel();
        if (pdAngVel != 0.0f) {
          const float angle = (dt * pdAngVel);
          *predictedAzimuth += angle;
          const float* pdAngPos = phydrv->getAngularPos();
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
        const float* pdVel = phydrv->getVelocity();
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

  //  send a packet if we've crossed a physics driver boundary
  if (state.phydrv != inputPhyDrv) {
    return true;
  }
  
  // otherwise always send at least one packet per second
  if ((TimeKeeper::getTick() - inputTime) >= MaxUpdateTime) {
    return true;
  }

  // get predicted state
  float predictedPos[3];
  float predictedVel[3];
  float predictedAzimuth;
  float dt = TimeKeeper::getTick() - inputTime;
  getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel, dt);

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
  if (TimeKeeper::getTick() - inputTime < minUpdateTime) {
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
    DEBUG4 ("state.azimuth = %f, predictedAzimuth = %f\n",
            state.azimuth, predictedAzimuth);
    return true;
  }

  // prediction is good enough
  return false;
}


void Player::doDeadReckoning()
{
  if (!isAlive() && !isExploding()) {
    return;
  }

  // get predicted state
  float predictedPos[3];
  float predictedVel[3];
  float predictedAzimuth;
  float dt = TimeKeeper::getTick() - inputTime;
  getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel, dt);

  // setup notResponding
  if (!isAlive()) {
    notResponding = false;
  } else {
    notResponding = (dt > BZDB.eval(StateDatabase::BZDB_NOTRESPONDINGTIME));
  }

  // if hit ground then update input state (we don't want to fall anymore)
  float groundLimit = 0.0f;
  if (getFlag() == Flags::Burrow) {
    groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);
  }
  // the velocity check is for when a Burrow flag is dropped
  if ((predictedPos[2] < groundLimit) && (predictedVel[2] <= 0.0f)) {
    predictedPos[2] = groundLimit;
    predictedVel[2] = 0.0f;
    inputStatus &= ~PlayerState::Falling;
    inputVel[2] = 0.0f;
  }

  // setup remote players' landing sounds and graphics, and jumping sounds
  if (isAlive()) {
    // the importance level of the remote sounds
    const bool remoteImportant = false;

    // check for a landing
    if (((oldStatus & PlayerState::Falling) != 0) &&
        ((inputStatus & PlayerState::Falling) == 0)) {
      // setup the squish effect
      setLandingSpeed(oldZSpeed);
      // setup the sound
      if (BZDB.isTrue("remoteSounds")) {
        if ((getFlag() != Flags::Burrow) || (predictedPos[2] > 0.0f)) {
          playWorldSound(SFX_LAND, state.pos, remoteImportant);
        } else  {
          // probably never gets played
          playWorldSound(SFX_BURROW,  state.pos, remoteImportant);
        }
      }
    }

    // FIXME - this still needs work      
    //         also calculate a more accurate landing speed?
    
    // check for a jump
    if ((state.velocity[2] > oldZSpeed) && (state.velocity[2] > 0.0f) &&
        (state.pos[2] > 0.0f)) {
  //    if (((oldStatus & PlayerState::Falling) == 0) &&
  //        ((inputStatus & PlayerState::Falling) != 0) &&
  //        (predictedVel[2] > 0.0f)) {
      // setup the sound
      if (BZDB.isTrue("remoteSounds")) {
        if (state.jumpJetsScale > 0.0f) {
          if (getFlag() == Flags::Wings) {
            playWorldSound(SFX_FLAP, state.pos, remoteImportant);
          } else {
            playWorldSound(SFX_JUMP, state.pos, remoteImportant);
          }
        } else {
          playWorldSound(SFX_BOUNCE, state.pos, remoteImportant);
        }
      }
    }
  }
  
  // copy some old state
  oldZSpeed = state.velocity[2];
  oldStatus = inputStatus;

  move(predictedPos, predictedAzimuth);
  setVelocity(predictedVel);
  setRelativeMotion();

  return;
}


// How long does the filter takes to be considered "initialized"
const int   DRStateStable      = 10;
const float maxToleratedJitter = 1.0f;

void Player::setDeadReckoning(float timestamp)
{
  // offset should be the time packet has been delayed above average
  offset = timestamp - (TimeKeeper::getTick() - TimeKeeper::getNullTime())
    - deltaTime;

  bool discardUpdate = false;
  // at first stage, Delta time is computed as the average of the last
  // differences in time (local & remote) the values is then updated
  // with the new samples, smoothed with the old values
  float alpha = 1.0f / float(deadReckoningState + 1);
  if (deadReckoningState >= DRStateStable) {
    if (fabs(offset) > maxToleratedJitter) {
      // Put a threshold on untimed measurement
      offset = (offset > 0) ? maxToleratedJitter : -maxToleratedJitter;
      // and discard, but before adjust delta a little
      discardUpdate = true;
    }
    else if (offset > 0) {
      // fast alignment to the packet that take less travel time
      // that's for trying to have less lag
      alpha = 1.0f;
    }
  }
  // alpha filtering
  deltaTime = deltaTime + (offset * alpha);
  if (discardUpdate) {
    return;
  }
  // when alpha is 1, that really means we are re-initializing deltaTime
  // so offset should be zero
  if (alpha == 1.0f) {
    offset = 0.0f;
  }
  if (deadReckoningState < DRStateStable) {
    deadReckoningState++;
  }

  // set the current state
  setDeadReckoning();
  
  // adjust for the time offset  
  if (deadReckoningState >= DRStateStable) {

    // FIXME - untested
    
    // get predicted offset state
    float predictedPos[3];
    float predictedVel[3];
    float predictedAzimuth;
    getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel, -offset);
    move(predictedPos, predictedAzimuth);
    setVelocity(predictedVel);
    // set the current state, again
    setDeadReckoning();
  }

  setRelativeMotion();
  
  return;
}


void Player::setDeadReckoning()
{
  inputTime = TimeKeeper::getTick();
  
  // copy stuff for dead reckoning
  inputStatus = state.status;
  inputAzimuth = state.azimuth;
  inputAngVel = state.angVel;
  memcpy(inputPos, state.pos, sizeof(float[3]));
  memcpy(inputVel, state.velocity, sizeof(float[3]));
  inputPhyDrv = state.phydrv;

  //
  // pre-calculate some stuff for dead reckoning  
  //
  
  // the relative motion information (with respect to the physics drivers)
  calcRelativeMotion(inputRelVel, inputRelSpeed, inputRelAngVel);

  // setup the turning pararmeters  
  inputTurning = false;
  if (fabsf(inputRelAngVel) > 0.001f) {
    inputTurning = true;
    const float radius = (inputRelSpeed / inputRelAngVel);
    inputTurnVector[0] = +sinf(inputAzimuth) * radius;
    inputTurnVector[1] = -cosf(inputAzimuth) * radius;
    inputTurnCenter[0] = inputPos[0] - inputTurnVector[0];
    inputTurnCenter[1] = inputPos[1] - inputTurnVector[1];
  }
  
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
