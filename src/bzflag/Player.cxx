/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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
#include "common.h"
#include "Player.h"
#include "World.h"
#include "TankSceneNode.h"
#include "SphereSceneNode.h"
#include "SceneDatabase.h"
#include "OpenGLMaterial.h"
#include "playing.h"

// for dead reckoning
static const float	PositionTolerance = 0.01f;	// meters
static const float	AngleTolerance = 0.01f;		// radians
static const float	MaxUpdateTime = 1.0f;		// seconds

//
// Player
//

OpenGLTexture*		Player::tankTexture = NULL;
int			Player::totalCount = 0;

Player::Player(const PlayerId& _id, TeamColor _team,
		const char* name, const char* _email, const PlayerType _type) :
				notResponding(false),
				autoPilot(false),
				hunted(false),
				id(_id),
				team(_team),
				type(_type),
				flag(Flags::Null),
				fromTeleporter(0),
				toTeleporter(0),
				teleporterProximity(0.0f),
				wins(0),
				losses(0),
				tks(0),
				localWins(0),
				localLosses(0),
				localTks(0)
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
    pausedSphere = new SphereSceneNode(state.pos, 1.5f * BZDB->eval(StateDatabase::BZDB_TANKRADIUS));
    pausedSphere->setColor(0.0f, 0.0f, 0.0f, 0.5f);
    totalCount++;
  }

}

Player::~Player()
{
  if (id != ServerPlayer) {
    delete tankIDLNode;
    delete tankNode;
    delete pausedSphere;
    if (--totalCount == 0) {
      delete tankTexture;
      tankTexture = NULL;
    }
  }
}

float			Player::getRadius() const
{
  float tankRadius = BZDB->eval(StateDatabase::BZDB_TANKRADIUS);
  if (flag == Flags::Obesity) return tankRadius * BZDB->eval(StateDatabase::BZDB_OBESEFACTOR);
  if (flag == Flags::Tiny)    return tankRadius * BZDB->eval(StateDatabase::BZDB_TINYFACTOR);
  if (flag == Flags::Thief)   return tankRadius * BZDB->eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  return tankRadius;
}

void			Player::getMuzzle(float* m) const
{
  // okay okay, I should really compute the up vector instead of using [0,0,1]
  float front = BZDB->eval(StateDatabase::BZDB_MUZZLEFRONT);
  if (flag == Flags::Obesity) front *= BZDB->eval(StateDatabase::BZDB_OBESEFACTOR);
  else if (flag == Flags::Tiny) front *= BZDB->eval(StateDatabase::BZDB_TINYFACTOR);
  else if (flag == Flags::Thief) front *= BZDB->eval(StateDatabase::BZDB_THIEFTINYFACTOR);
  m[0] = state.pos[0] + front * forward[0];
  m[1] = state.pos[1] + front * forward[1];
  m[2] = state.pos[2] + front * forward[2] + BZDB->eval(StateDatabase::BZDB_MUZZLEHEIGHT);
}

void			Player::move(const float* _pos, float _azimuth)
{
  // assumes _forward is normalized
  state.pos[0] = _pos[0];
  state.pos[1] = _pos[1];
  state.pos[2] = _pos[2];
  state.azimuth = _azimuth;

  // limit angle
  if (state.azimuth < 0.0f) state.azimuth = 2.0f * M_PI - fmodf(-state.azimuth, 2.0f * M_PI);
  else if (state.azimuth >= 2.0f * M_PI) state.azimuth = fmodf(state.azimuth, 2.0f * M_PI);

  // update forward vector (always in horizontal plane)
  forward[0] = cosf(state.azimuth);
  forward[1] = sinf(state.azimuth);
  forward[2] = 0.0f;

  // compute teleporter proximity
  if (World::getWorld())
    teleporterProximity = World::getWorld()->getProximity(state.pos, BZDB->eval(StateDatabase::BZDB_TANKRADIUS));
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

void			Player::setTexture(const OpenGLTexture& _texture)
{
  if (!tankTexture)
    tankTexture = new OpenGLTexture;
  *tankTexture = _texture;
}

void			Player::changeTeam(TeamColor _team)
{
  static const GLfloat	tankSpecular[3] = { 0.1f, 0.1f, 0.1f };
  static const GLfloat	tankEmissive[3] = { 0.0f, 0.0f, 0.0f };

  // set team
  team = _team;

  // change color of tank
  const float* _color = Team::getTankColor(team);
  color[0] = _color[0];
  color[1] = _color[1];
  color[2] = _color[2];
  color[3] = 1.0f;
  tankNode->setColor(color);
  tankNode->setMaterial(OpenGLMaterial(tankSpecular, tankEmissive, 20.0f));
  tankNode->setTexture(*tankTexture);
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

void			Player::setFlag(FlagDesc* _flag)
{
  flag = _flag;
}

void			Player::endShot(int index,
				bool isHit, bool showExplosion)
{
  float pos[3];
  if (doEndShot(index, isHit, pos) && showExplosion)
    addShotExplosion(pos);
}

void			Player::updateSparks(float /*dt*/)
{
  if (flag != Flags::PhantomZone || !isFlagActive()) {
	  teleporterProximity = World::getWorld()->getProximity(state.pos, BZDB->eval(StateDatabase::BZDB_TANKRADIUS));
    if (teleporterProximity == 0.0f) {
      color[3] = 1.0f;
      tankNode->setColor(color);
      return;
    }
  }

  if (flag == Flags::PhantomZone && isFlagActive()) {
    // almost totally transparent
    color[3] = 0.25f;
  }
  else {
    // transparency depends on proximity
    color[3] = 1.0f - 0.75f * teleporterProximity;
  }
  tankNode->setColor(color);
}

void			Player::addPlayer(SceneDatabase* scene,
						const float* colorOverride,
						bool showIDL)
{
  if (!isAlive() && !isExploding()) return;
  tankNode->move(state.pos, forward);
  tankNode->setColorOverride(colorOverride);
  if (isAlive()) {
    if (flag == Flags::Obesity) tankNode->setObese();
    else if (flag == Flags::Tiny) tankNode->setTiny();
    else if (flag == Flags::Narrow) tankNode->setNarrow();
    else if (flag == Flags::Thief) tankNode->setThief();
    else tankNode->setNormal();
    tankNode->setExplodeFraction(0.0f);
    scene->addDynamicNode(tankNode);

    if (isCrossingWall()) {
      // get which plane to compute IDL against
      GLfloat plane[4];
      float tankLength = BZDB->eval(StateDatabase::BZDB_TANKLENGTH);
      float tankWidth = BZDB->eval(StateDatabase::BZDB_TANKWIDTH);
      const GLfloat a = atan2f(forward[1], forward[0]);
      const Obstacle* obstacle = World::getWorld()->hitBuilding(state.pos, a,
					0.5f * tankLength, 0.5f * tankWidth);
      if (obstacle && obstacle->isCrossing(state.pos, a,
				0.5f * tankLength, 0.5f * tankWidth, plane) ||
		World::getWorld()->crossingTeleporter(state.pos, a,
				0.5f * tankLength, 0.5f * tankWidth, plane)) {
	// stick in interdimensional lights node
	if (showIDL) {
	  tankIDLNode->move(plane);
	  scene->addDynamicNode(tankIDLNode);
	}

	// add clipping plane to tank node
	tankNode->setClipPlane(plane);
      }
    }
    else if ((getFlag() == Flags::Burrow) && (getPosition()[2] < 0.0f)) {
      GLfloat plane[4];
      plane[0] = plane[1] = 0.0f;
      plane[2] = 1.0f;
      plane[3] = 0.0f;
      tankNode->setClipPlane(plane);
    }
    else {
      tankNode->setClipPlane(NULL);
    }
  }
  else if (isExploding()) {
    float t = (TimeKeeper::getTick() - explodeTime) / BZDB->eval(StateDatabase::BZDB_EXPLODETIME);
    if (t > 1.0f) {
// FIXME
//      setStatus(DeadStatus);
      t = 1.0f;
    }
    else if (t < 0.0f) {
      // shouldn't happen but why take chances
      t = 0.0f;
    }
    tankNode->setExplodeFraction(t);
    scene->addDynamicNode(tankNode);
  }
  if (isAlive() && (isPaused() || isNotResponding())) {
    pausedSphere->move(state.pos, 1.5f * BZDB->eval(StateDatabase::BZDB_TANKRADIUS));
    scene->addDynamicSphere(pausedSphere);
  }
}

void			Player::setHidden(bool hidden)
{
  tankNode->setHidden(hidden);
}

void			Player::setInvisible(bool invisible)
{
  tankNode->setInvisible(invisible);
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

void*			Player::unpack(void* buf)
{
  buf = state.unpack(buf);
  setDeadReckoning();
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
  ((Player*)this)->inputPrevTime = TimeKeeper::getTick();
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
  }
  else if (inputStatus & PlayerState::Falling) {
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
    ((Player*)this)->inputZSpeed += BZDB->eval(StateDatabase::BZDB_GRAVITY) * (dt - dt2);
    ((Player*)this)->inputPos[2] += inputZSpeed * (dt - dt2);
  }
  else {
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
    }

    else {
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
  return (dt < 3.5f * MaxUpdateTime);
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
    groundLimit = BZDB->eval(StateDatabase::BZDB_BURROWDEPTH);
  if (predictedPos[2] < groundLimit) return true;

  // client side throttling
  const int throttleRate = 30; // should be configurable
  const float minUpdateTime = throttleRate > 0 ? 1.0f / throttleRate : 0.0f;
  if (TimeKeeper::getTick() - inputTime < minUpdateTime) return false;

  // see if position and azimuth are close enough
  if (fabsf(state.pos[0] - predictedPos[0]) > PositionTolerance) return true;
  if (fabsf(state.pos[1] - predictedPos[1]) > PositionTolerance) return true;
  if (fabsf(state.pos[2] - predictedPos[2]) > PositionTolerance) return true;
  if (fabsf(state.azimuth - predictedAzimuth) > AngleTolerance) return true;

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
    groundLimit = BZDB->eval(StateDatabase::BZDB_BURROWDEPTH);

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

// ex: shiftwidth=2 tabstop=8
