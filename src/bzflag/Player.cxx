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


// for dead reckoning
static const float	MaxUpdateTime = 1.0f;		// seconds

//
// Player
//

int		Player::tankTexture = -1;

Player::Player(const PlayerId& _id, TeamColor _team,
	       const char* name, const char* _email, const PlayerType _type) :
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
  setDeadReckoning();

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
  dimensions[0] = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  dimensions[1] = 0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  dimensions[2] = BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
  memcpy (oldDimensions, dimensions, sizeof(float[3]));
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

short		Player::getRabbitScore() const
{
  return (int)(rabbitRank(wins, losses) * 100.0);
}

float			Player::getRadius() const
{
  float tankRadius = BZDBCache::tankRadius;
  if (flagType == Flags::Obesity) {
    return tankRadius * BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  }
  else if (flagType == Flags::Tiny) {
    return tankRadius * BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  }
  else if (flagType == Flags::Thief) {
    return tankRadius * BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  }
  else {
    return tankRadius;
  }
}

void			Player::getMuzzle(float* m) const
{
  // okay okay, I should really compute the up vector instead of using [0,0,1]
  float front = BZDB.eval(StateDatabase::BZDB_MUZZLEFRONT);
  if (flagType == Flags::Obesity) front *= BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
  else if (flagType == Flags::Tiny) front *= BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
  else if (flagType == Flags::Thief) front *= BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  m[0] = state.pos[0] + front * forward[0];
  m[1] = state.pos[1] + front * forward[1];
  m[2] = state.pos[2] + front * forward[2] + BZDB.eval(StateDatabase::BZDB_MUZZLEHEIGHT);
}

void			Player::move(const float* _pos, float _azimuth)
{
  // assumes _forward is normalized
  state.pos[0] = _pos[0];
  state.pos[1] = _pos[1];
  state.pos[2] = _pos[2];
  state.azimuth = _azimuth;

  // limit angle
  if (state.azimuth < 0.0f) state.azimuth = 2.0f * M_PI - fmodf(-state.azimuth, 2.0f * (float)M_PI);
  else if (state.azimuth >= 2.0f * M_PI) state.azimuth = fmodf(state.azimuth, 2.0f * (float)M_PI);

  // update forward vector (always in horizontal plane)
  forward[0] = cosf(state.azimuth);
  forward[1] = sinf(state.azimuth);
  forward[2] = 0.0f;

  // compute teleporter proximity
  if (World::getWorld())
    teleporterProximity = World::getWorld()
      ->getProximity(state.pos, BZDBCache::tankRadius);
}

void			Player::setVelocity(const float* _velocity)
{
  state.velocity[0] = _velocity[0];
  state.velocity[1] = _velocity[1];
  state.velocity[2] = _velocity[2];
}

void			Player::setAngularVelocity(float _angVel)
{
  state.angVel = _angVel;
}

void			Player::changeTeam(TeamColor _team)
{
  // set team
  team = _team;

  // set the scene node
  setVisualTeam(team);
}

void			Player::setStatus(short _status)
{
  state.status = _status;
}

void			Player::setExplode(const TimeKeeper& t)
{
  if (!isAlive()) return;
  explodeTime = t;
  setStatus((getStatus() | short(PlayerState::Exploding) | short(PlayerState::Falling)) &
	    ~(short(PlayerState::Alive) | short(PlayerState::Paused)));
}

void			Player::setTeleport(const TimeKeeper& t,
					    short from, short to)
{
  if (!isAlive()) return;
  teleportTime = t;
  fromTeleporter = from;
  toTeleporter = to;
  setStatus(getStatus() | short(PlayerState::Teleporting));
}

void			Player::updateFlagProperties(float dt)
{
  // copy the current dimensions to the old dimensions
  memcpy (oldDimensions, dimensions, sizeof(float[3]));

  // update the dimensions
  for (int i = 0; i < 3; i++) {
    if (dimensionsRate[i] != 0.0f) {
      dimensionsScale[i] += dt * dimensionsRate[i];
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
  
  // set the actual dimensions based on the scale
  dimensions[0] = dimensionsScale[0] *
                  0.5f * BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
  dimensions[1] = dimensionsScale[1] * 
                  0.5f * BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
  dimensions[2] = dimensionsScale[2] *
                  BZDB.eval(StateDatabase::BZDB_TANKHEIGHT);
                  
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

void			Player::changeScore(short deltaWins, short deltaLosses, short deltaTeamKills)
{
  wins += deltaWins;
  losses += deltaLosses;
  tks += deltaTeamKills;
}

void			Player::changeLocalScore(short dWins, short dLosses, short dTeamKills)
{
  localWins += dWins;
  localLosses += dLosses;
  localTks += dTeamKills;
}

void			Player::setFlag(FlagType* _flag)
{
  // set the type
  flagType = _flag;
  
  float FlagEffectTime = BZDB.eval(StateDatabase::BZDB_FLAGEFFECTTIME);
  if (FlagEffectTime <= 0.0f) {
    FlagEffectTime = 0.001f; // safety
  }
  
  // set the dimension targets
  dimensionsTarget[0] = 1.0f;
  dimensionsTarget[1] = 1.0f;
  dimensionsTarget[2] = 1.0f;
  if (flagType == Flags::Obesity) {
    const float factor = BZDB.eval(StateDatabase::BZDB_OBESEFACTOR);
    dimensionsTarget[0] = factor;
    dimensionsTarget[1] = factor;
  }
  else if (flagType == Flags::Tiny) {
    const float factor = BZDB.eval(StateDatabase::BZDB_TINYFACTOR);
    dimensionsTarget[0] = factor;
    dimensionsTarget[1] = factor;
  }
  else if (flagType == Flags::Thief) {
    const float factor = BZDB.eval(StateDatabase::BZDB_THIEFTINYFACTOR);
    dimensionsTarget[0] = factor;
    dimensionsTarget[1] = factor;
  }
  else if (flagType == Flags::Narrow) {
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
  if (flagType == Flags::Cloaking) {
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

void			Player::endShot(int index,
					bool isHit, bool showExplosion)
{
  float pos[3];
  if (doEndShot(index, isHit, pos) && showExplosion)
    addShotExplosion(pos);
}

void			Player::setVisualTeam (TeamColor visualTeam)
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

  // change color of tank
  const float* _color = Team::getTankColor(visualTeam);
  if (BZDBCache::texture && tankTexture >= 0) {  // color is in the image
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
  } else {
    // we are the hunter, we are orange..
    // TODO this is cheap, just untill a "hunter" team is made
    if (hunter) {
      color[0] = 1.0f;
      color[1] = 0.5f;
      color[2] = 0.0f;
    } else {
      color[0] = _color[0];
      color[1] = _color[1];
      color[2] = _color[2];
    }
  }
  color[3] = (getFlag() == Flags::PhantomZone) && isFlagActive() ? 0.5f : 1.0f;
  tankNode->setColor(color);
  tankNode->setMaterial(OpenGLMaterial(tankSpecular, emissive, shininess));
  tankNode->setTexture(tankTexture);
}


void			Player::addToScene(SceneDatabase* scene,
					  TeamColor effectiveTeam,
					  bool showIDL)
{
  if (!isAlive() && !isExploding()) {
    return;
  }

  tankNode->move(state.pos, forward);
  setVisualTeam(effectiveTeam);
  
  if (isAlive()) {
    if (flagType == Flags::Obesity) tankNode->setObese();
    else if (flagType == Flags::Tiny) tankNode->setTiny();
    else if (flagType == Flags::Narrow) tankNode->setNarrow();
    else if (flagType == Flags::Thief) tankNode->setThief();
    else tankNode->setNormal();
    // only use dimensions if we aren't at steady state.
    // this is done because it's more expensive to use
    // GL_NORMALIZE then to use precalculated normals.
    if (useDimensions) {
      tankNode->setDimensions(dimensionsScale);
    } else {
      tankNode->ignoreDimensions();
    }
    
    tankNode->setExplodeFraction(0.0f);
    scene->addDynamicNode(tankNode);

    if (isCrossingWall()) {
      // get which plane to compute IDL against
      GLfloat plane[4];
      float tankLength = BZDB.eval(StateDatabase::BZDB_TANKLENGTH);
      float tankWidth = BZDB.eval(StateDatabase::BZDB_TANKWIDTH);
      const GLfloat a = atan2f(forward[1], forward[0]);
      const Obstacle* obstacle = World::getWorld()->hitBuilding(state.pos, a,
								0.5f * tankLength, 0.5f * tankWidth,
								BZDBCache::tankHeight);
      if (obstacle && obstacle->isCrossing(state.pos, a,
					   0.5f * tankLength, 0.5f * tankWidth,
					   BZDBCache::tankHeight, plane) ||
	  World::getWorld()->crossingTeleporter(state.pos, a,
						0.5f * tankLength, 0.5f * tankWidth,
						BZDBCache::tankHeight, plane)) {
	// stick in interdimensional lights node
	if (showIDL) {
	  tankIDLNode->move(plane);
	  scene->addDynamicNode(tankIDLNode);
	}

	// add clipping plane to tank node
	tankNode->setClipPlane(plane);
      }
    } else if ((getFlag() == Flags::Burrow) && (getPosition()[2] < 0.0f)) {
      GLfloat plane[4];
      plane[0] = plane[1] = 0.0f;
      plane[2] = 1.0f;
      plane[3] = 0.0f;
      tankNode->setClipPlane(plane);
    } else {
      tankNode->setClipPlane(NULL);
    }
  } else if (isExploding() && state.pos[2] > ZERO_TOLERANCE) {
    float t = (TimeKeeper::getTick() - explodeTime) / BZDB.eval(StateDatabase::BZDB_EXPLODETIME);
    if (t > 1.0f) {
      // FIXME
      //      setStatus(DeadStatus);
      t = 1.0f;
    } else if (t < 0.0f) {
      // shouldn't happen but why take chances
      t = 0.0f;
    }
    tankNode->setExplodeFraction(t);
    scene->addDynamicNode(tankNode);
  }
  
  if (isAlive() && (isPaused() || isNotResponding())) {
    pausedSphere->move(state.pos, 1.5f * BZDBCache::tankRadius);
    scene->addDynamicSphere(pausedSphere);
  }
}


void			Player::setHidden(bool hidden)
{
  tankNode->setHidden(hidden);
}

void			Player::setCloaked(bool invisible)
{
  tankNode->setCloaked(invisible);
}

int			Player::getMaxShots() const
{
  return World::getWorld()->getMaxShots();
}

void			Player::addShots(SceneDatabase* scene,
					 bool colorblind) const
{
  const int count = getMaxShots();
  for (int i = 0; i < count; i++) {
    ShotPath* shot = getShot(i);
    if (shot && !shot->isExpiring() && !shot->isExpired())
      shot->addShot(scene, colorblind);
  }
}

void*			Player::unpack(void* buf, uint16_t code)
{
  float timestamp;
  PlayerId id;

  buf = nboUnpackFloat(buf, timestamp);
  buf = nboUnpackUByte(buf, id);
  buf = state.unpack(buf, code);
  setDeadReckoning(timestamp);
  return buf;
}

bool			Player::validTeamTarget(const Player *possibleTarget) const
{
  TeamColor myTeam = getTeam();
  TeamColor targetTeam = possibleTarget->getTeam();
  if (myTeam != targetTeam)
    return true;

  if (myTeam != RogueTeam)
    return false;

  return !World::getWorld()->allowRabbit();
}

bool			Player::getDeadReckoning(
						 float* predictedPos, float* predictedAzimuth,
						 float* predictedVel) const
{
  // see if predicted position and orientation (only) are close enough
  const float dt2 = inputPrevTime - inputTime;
  inputPrevTime = TimeKeeper::getTick();
  const float dt = inputPrevTime - inputTime;

  if (inputStatus & PlayerState::Paused) {
    // don't move when paused
    predictedPos[0] = inputPos[0];
    predictedPos[1] = inputPos[1];
    predictedPos[2] = inputPos[2];
    predictedVel[0] = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
    predictedVel[1] = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);
    predictedVel[2] = 0.0f;
    *predictedAzimuth = inputAzimuth;
  } else if (inputStatus & PlayerState::Falling) {
    // no control when falling
    predictedVel[0] = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
    predictedVel[1] = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);

    // follow a simple parabola
    predictedPos[0] = inputPos[0] + dt * predictedVel[0];
    predictedPos[1] = inputPos[1] + dt * predictedVel[1];

    // only turn if alive
    if (inputStatus & PlayerState::Alive)
      *predictedAzimuth = inputAzimuth + dt * inputAngVel;
    else
      *predictedAzimuth = inputAzimuth;

    // update z with Newtownian integration (like LocalPlayer)
    inputZSpeed += BZDB.eval(StateDatabase::BZDB_GRAVITY) * (dt - dt2);
    inputPos[2] += inputZSpeed * (dt - dt2);
  } else {
    // azimuth changes linearly
    *predictedAzimuth = inputAzimuth + dt * inputAngVel;

    // different algorithms for tanks moving in a straight line vs
    // turning in a circle
    if (inputAngVel == 0.0f) {

      // straight ahead
      predictedVel[0] = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
      predictedVel[1] = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);
      predictedPos[0] = inputPos[0] + dt * predictedVel[0];
      predictedPos[1] = inputPos[1] + dt * predictedVel[1];

    } else {

      // need dt2 because velocity is based on previous time step
      const float tmpAzimuth = inputAzimuth + dt2 * inputAngVel;
      predictedVel[0] = inputSpeed * cosf(tmpAzimuth);
      predictedVel[1] = inputSpeed * sinf(tmpAzimuth);

      // find current position on circle:
      // tank with constant angular and linear velocity moves in a circle
      // with radius = (linear velocity/angular velocity).  circle turns
      // to the left (counterclockwise) when the ratio is positive.
      const float radius = inputSpeed / inputAngVel;
      const float offAzimuth = inputAzimuth - 0.5f * M_PI;
      const float angle = offAzimuth + dt * inputAngVel;
      predictedPos[0] = inputPos[0] + radius * (cosf(angle) - cosf(offAzimuth));
      predictedPos[1] = inputPos[1] + radius * (sinf(angle) - sinf(offAzimuth));

    }

    // inputZSpeed will be zero when not falling
  }

  predictedVel[2] = inputZSpeed;
  predictedPos[2] = inputPos[2];

  // return false if we haven't gotten an update in a while
  return (dt < BZDB.eval(StateDatabase::BZDB_NOTRESPONDINGTIME));
}

bool			Player::isDeadReckoningWrong() const
{
  // always send a new packet when some kinds of status change
  if ((state.status & (PlayerState::Alive | PlayerState::Paused | PlayerState::Falling)) !=
      (inputStatus & (PlayerState::Alive | PlayerState::Paused | PlayerState::Falling)))
    return true;

  // never send a packet when dead
  if (!(state.status & PlayerState::Alive)) return false;

  // otherwise always send at least one packet per second
  if (TimeKeeper::getTick() - inputTime >= MaxUpdateTime) return true;

  // get predicted state
  float predictedPos[3], predictedAzimuth, predictedVel[3];
  getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel);

  // always send a new packet on reckoned touchdown
  float groundLimit = 0.0f;
  if (getFlag() == Flags::Burrow)
    groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);
  if (predictedPos[2] < groundLimit) return true;

  // client side throttling
  const int throttleRate = int(BZDB.eval(StateDatabase::BZDB_UPDATETHROTTLERATE));
  const float minUpdateTime = throttleRate > 0 ? 1.0f / throttleRate : 0.0f;
  if (TimeKeeper::getTick() - inputTime < minUpdateTime) return false;

  // see if position and azimuth are close enough
  float positionTolerance = BZDB.eval(StateDatabase::BZDB_POSITIONTOLERANCE);
  if (fabsf(state.pos[0] - predictedPos[0]) > positionTolerance) return true;
  if (fabsf(state.pos[1] - predictedPos[1]) > positionTolerance) return true;
  if (fabsf(state.pos[2] - predictedPos[2]) > positionTolerance) return true;

  float angleTolerance = BZDB.eval(StateDatabase::BZDB_ANGLETOLERANCE);
  if (fabsf(state.azimuth - predictedAzimuth) > angleTolerance) return true;

  // prediction is good enough
  return false;
}

void			Player::doDeadReckoning()
{
  if (!isAlive() && !isExploding())
    return;

  // get predicted state
  float predictedPos[3], predictedAzimuth, predictedVel[3];
  notResponding = !getDeadReckoning(predictedPos, &predictedAzimuth,
				    predictedVel);

  if (!isAlive()) notResponding = false;

  // if hit ground then update input state (since we don't want to fall
  // anymore)
  float groundLimit = 0.0f;
  if (getFlag() == Flags::Burrow)
    groundLimit = BZDB.eval(StateDatabase::BZDB_BURROWDEPTH);

  if (predictedPos[2] < groundLimit) {
    predictedPos[2] = groundLimit;
    predictedVel[2] = 0.0f;
    inputStatus &= ~PlayerState::Falling;
    inputZSpeed = 0.0f;
    inputSpeedAzimuth = inputAzimuth;
  }

  move(predictedPos, predictedAzimuth);
  setVelocity(predictedVel);
}

// How long does the filter takes to be considered "initialized"
const int   DRStateStable      = 10;
const float maxToleratedJitter = 1.0f;

void			Player::setDeadReckoning(float timestamp)
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
    } else if (offset > 0) {
      // fast alignment to the packet that take less travel time
      // that's for trying to have less lag
      alpha = 1.0f;
    }
  }
  // alpha filtering
  deltaTime = deltaTime + offset * alpha;
  if (discardUpdate)
    return;
  // when alpha is 1, that really means we are re-initializing deltaTime
  // so offset should be zero
  if (alpha == 1.0f)
    offset = 0.0f;
  if (deadReckoningState < DRStateStable)
    ++deadReckoningState;

  setDeadReckoning();

  // Future the state based on offset
  if (inputStatus & PlayerState::Paused) {
    // don't move when paused
  } else if (inputStatus & PlayerState::Falling) {
    // no control when falling
    float vx = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
    float vy = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);
    // follow a simple parabola
    inputPos[0] -= offset * vx;
    inputPos[1] -= offset * vy;

    // only turn if alive
    if (inputStatus & PlayerState::Alive)
       inputAzimuth -= offset * inputAngVel;

    // update z
    float deltaSpeed      = BZDB.eval(StateDatabase::BZDB_GRAVITY) * offset;
    inputZSpeed          -= deltaSpeed;
    inputPos[2]          -= (deltaSpeed / 2.0f + inputZSpeed) * offset;
  } else {
    // different algorithms for tanks moving in a straight line vs
    // turning in a circle
    if (inputAngVel == 0.0f) {

      // straight ahead
      float vx = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
      float vy = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);
      inputPos[0] -= offset * vx;
      inputPos[1] -= offset * vy;

    } else {

      // find current position on circle:
      // tank with constant angular and linear velocity moves in a circle
      // with radius = (linear velocity/angular velocity).  circle turns
      // to the left (counterclockwise) when the ratio is positive.
      const float radius = inputSpeed / inputAngVel;

      // Rotation center coordinate
      inputPos[0]       -= radius * sin(inputAzimuth);
      inputPos[1]       += radius * cos(inputAzimuth);

      // azimuth changes linearly
      inputAzimuth      -= offset * inputAngVel;

      inputPos[0]       += radius * sin(inputAzimuth);
      inputPos[1]       -= radius * cos(inputAzimuth);

    }
  }
}

void			Player::setDeadReckoning()
{
  // save stuff for dead reckoning
  inputTime = TimeKeeper::getTick();
  inputPrevTime = inputTime;
  inputStatus = state.status;
  inputPos[0] = state.pos[0];
  inputPos[1] = state.pos[1];
  inputPos[2] = state.pos[2];
  inputSpeed = hypotf(state.velocity[0], state.velocity[1]);
  if (cosf(state.azimuth) * state.velocity[0] + sinf(state.azimuth) * state.velocity[1] < 0.0f)
    inputSpeed = -inputSpeed;
  if (inputSpeed != 0.0f)
    inputSpeedAzimuth = atan2f(state.velocity[1], state.velocity[0]);
  else
    inputSpeedAzimuth = 0.0f;
  inputZSpeed = state.velocity[2];
  inputAzimuth = state.azimuth;
  inputAngVel = state.angVel;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
