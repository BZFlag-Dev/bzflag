/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string.h>
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
		const char* name, const char* _email) :
				notResponding(False),
				id(_id),
				team(_team),
				flag(NoFlag),
				fromTeleporter(0),
				toTeleporter(0),
				teleporterProximity(0.0f),
				wins(0),
				losses(0),
				localWins(0),
				localLosses(0),
				status(DeadStatus)
{
  // initialize position, etc.
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

  // make scene nodes
  tankNode = new TankSceneNode(pos, forward);
  tankIDLNode = new TankIDLSceneNode(tankNode);
  changeTeam(team);
  pausedSphere = new SphereSceneNode(pos, 1.5f * TankRadius);
  pausedSphere->setColor(0.0f, 0.0f, 0.0f, 0.5f);

  totalCount++;
}

Player::~Player()
{
  delete tankIDLNode;
  delete tankNode;
  delete pausedSphere;
  if (--totalCount == 0) {
    delete tankTexture;
    tankTexture = NULL;
  }
}

float			Player::getRadius() const
{
  if (flag == ObesityFlag) return TankRadius * ObeseFactor;
  if (flag == TinyFlag) return TankRadius * TinyFactor;
  return TankRadius;
}

void			Player::getMuzzle(float* m) const
{
  // okay okay, I should really compute the up vector instead of using [0,0,1]
  float front = MuzzleFront;
  if (flag == ObesityFlag) front *= ObeseFactor;
  else if (flag == TinyFlag) front *= TinyFactor;
  m[0] = pos[0] + front * forward[0];
  m[1] = pos[1] + front * forward[1];
  m[2] = pos[2] + front * forward[2] + MuzzleHeight;
}

void			Player::move(const float* _pos, float _azimuth)
{
  // assumes _forward is normalized
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
  azimuth = _azimuth;

  // limit angle
  if (azimuth < 0.0f) azimuth = 2.0f * M_PI - fmodf(-azimuth, 2.0f * M_PI);
  else if (azimuth >= 2.0f * M_PI) azimuth = fmodf(azimuth, 2.0f * M_PI);

  // update forward vector (always in horizontal plane)
  forward[0] = cosf(azimuth);
  forward[1] = sinf(azimuth);
  forward[2] = 0.0f;

  // compute teleporter proximity
  if (World::getWorld())
    teleporterProximity = World::getWorld()->getProximity(pos, TankRadius);
}

void			Player::setVelocity(const float* _velocity)
{
  velocity[0] = _velocity[0];
  velocity[1] = _velocity[1];
  velocity[2] = _velocity[2];
}

void			Player::setAngularVelocity(float _angVel)
{
  angVel = _angVel;
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
  static const float	rogueColor[3] = { 0.25f, 0.25f, 0.25f };

  // set team
  team = _team;

  // change color of tank
  const float* _color = (team == RogueTeam) ? rogueColor :
						Team::getTankColor(team);
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
  status = _status;
}

void			Player::setExplode(const TimeKeeper& t)
{
  if (!isAlive()) return;
  explodeTime = t;
  setStatus((getStatus() | short(Exploding) | short(Falling)) &
			~(short(Alive) | short(Paused)));
}

void			Player::setTeleport(const TimeKeeper& t,
						short from, short to)
{
  if (!isAlive()) return;
  teleportTime = t;
  fromTeleporter = from;
  toTeleporter = to;
  setStatus(getStatus() | short(Teleporting));
}

void			Player::changeScore(short deltaWins, short deltaLosses)
{
  wins += deltaWins;
  losses += deltaLosses;
}

void			Player::changeLocalScore(short dWins, short dLosses)
{
  localWins += dWins;
  localLosses += dLosses;
}

void			Player::setFlag(FlagId _flag)
{
  flag = _flag;
}

void			Player::endShot(int index,
				boolean isHit, boolean showExplosion)
{
  float pos[3];
  if (doEndShot(index, isHit, pos) && showExplosion)
    addShotExplosion(pos);
}

void			Player::updateSparks(float /*dt*/)
{
  if (flag != PhantomZoneFlag || !isFlagActive()) {
    teleporterProximity = World::getWorld()->getProximity(pos, TankRadius);
    if (teleporterProximity == 0.0f) {
      color[3] = 1.0f;
      tankNode->setColor(color);
      return;
    }
  }

  if (flag == PhantomZoneFlag && isFlagActive()) {
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
						boolean colorblind,
						boolean showIDL)
{
  if (!isAlive() && !isExploding()) return;
  tankNode->move(pos, forward);
  tankNode->setColorblind(colorblind);
  if (isAlive()) {
    if (flag == ObesityFlag) tankNode->setObese();
    else if (flag == TinyFlag) tankNode->setTiny();
    else if (flag == NarrowFlag) tankNode->setNarrow();
    else tankNode->setNormal();
    tankNode->setExplodeFraction(0.0f);
    scene->addDynamicNode(tankNode);

    if (isCrossingWall()) {
      // get which plane to compute IDL against
      GLfloat plane[4];
      const GLfloat a = atan2f(forward[1], forward[0]);
      const Obstacle* obstacle = World::getWorld()->hitBuilding(pos, a,
					0.5f * TankLength, 0.5f * TankWidth);
      if (obstacle && obstacle->isCrossing(pos, a,
				0.5f * TankLength, 0.5f * TankWidth, plane) ||
		World::getWorld()->crossingTeleporter(pos, a,
				0.5f * TankLength, 0.5f * TankWidth, plane)) {
	// stick in interdimensional lights node
	if (showIDL) {
	  tankIDLNode->move(plane);
	  scene->addDynamicNode(tankIDLNode);
	}

	// add clipping plane to tank node
	tankNode->setClipPlane(plane);
      }
    }
    else {
      tankNode->setClipPlane(NULL);
    }
  }
  else if (isExploding()) {
    float t = (TimeKeeper::getTick() - explodeTime) / ExplodeTime;
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
    pausedSphere->move(pos, 1.5f * TankRadius);
    scene->addDynamicSphere(pausedSphere);
  }
}

void			Player::setHidden(boolean hidden)
{
  tankNode->setHidden(hidden);
}

void			Player::setInvisible(boolean invisible)
{
  tankNode->setInvisible(invisible);
}

void			Player::addShots(SceneDatabase* scene,
					boolean colorblind) const
{
  const int count = World::getWorld()->getMaxShots();
  for (int i = 0; i < count; i++) {
    ShotPath* shot = getShot(i);
    if (shot && !shot->isExpiring() && !shot->isExpired())
      shot->addShot(scene, colorblind);
  }
}

void*			Player::pack(void* buf) const
{
  ((Player*)this)->setDeadReckoning();
  buf = nboPackShort(buf, int16_t(status));
  buf = nboPackFloat(buf, pos[0]);
  buf = nboPackFloat(buf, pos[1]);
  buf = nboPackFloat(buf, pos[2]);
  buf = nboPackFloat(buf, velocity[0]);
  buf = nboPackFloat(buf, velocity[1]);
  buf = nboPackFloat(buf, velocity[2]);
  buf = nboPackFloat(buf, azimuth);
  buf = nboPackFloat(buf, angVel);
  return buf;
}

void*			Player::unpack(void* buf)
{
  int16_t inStatus;
  buf = nboUnpackShort(buf, inStatus);
  buf = nboUnpackFloat(buf, pos[0]);
  buf = nboUnpackFloat(buf, pos[1]);
  buf = nboUnpackFloat(buf, pos[2]);
  buf = nboUnpackFloat(buf, velocity[0]);
  buf = nboUnpackFloat(buf, velocity[1]);
  buf = nboUnpackFloat(buf, velocity[2]);
  buf = nboUnpackFloat(buf, azimuth);
  buf = nboUnpackFloat(buf, angVel);
  status = short(inStatus);
  setDeadReckoning();
  return buf;
}

void			Player::setDeadReckoning()
{
  // save stuff for dead reckoning
  inputTime = TimeKeeper::getTick();
  inputPrevTime = inputTime;
  inputStatus = status;
  inputPos[0] = pos[0];
  inputPos[1] = pos[1];
  inputPos[2] = pos[2];
  inputSpeed = hypotf(velocity[0], velocity[1]);
  if (cosf(azimuth) * velocity[0] + sinf(azimuth) * velocity[1] < 0.0f)
    inputSpeed = -inputSpeed;
  if (inputSpeed != 0.0f)
    inputSpeedAzimuth = atan2f(velocity[1], velocity[0]);
  else
    inputSpeedAzimuth = 0.0f;
  inputZSpeed = velocity[2];
  inputAzimuth = azimuth;
  inputAngVel = angVel;
}

boolean			Player::getDeadReckoning(
				float* predictedPos, float* predictedAzimuth,
				float* predictedVel) const
{
  // see if predicted position and orientation (only) are close enough
  const float dt2 = inputPrevTime - inputTime;
  ((Player*)this)->inputPrevTime = TimeKeeper::getTick();
  const float dt = inputPrevTime - inputTime;

  if (inputStatus & Paused) {
    // don't move when paused
    predictedPos[0] = inputPos[0];
    predictedPos[1] = inputPos[1];
    predictedPos[2] = inputPos[2];
    predictedVel[0] = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
    predictedVel[1] = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);
    predictedVel[2] = 0.0f;
    *predictedAzimuth = inputAzimuth;
  }
  else if (inputStatus & Falling) {
    // no control when falling
    predictedVel[0] = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
    predictedVel[1] = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);

    // follow a simple parabola
    predictedPos[0] = inputPos[0] + dt * predictedVel[0];
    predictedPos[1] = inputPos[1] + dt * predictedVel[1];

    // only turn if alive
    if (inputStatus & Alive)
      *predictedAzimuth = inputAzimuth + dt * inputAngVel;
    else
      *predictedAzimuth = inputAzimuth;

    // update z with Newtownian integration (like LocalPlayer)
    ((Player*)this)->inputZSpeed += Gravity * (dt - dt2);
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

boolean			Player::isDeadReckoningWrong() const
{
  // always send a new packet when some kinds of status change
  if ((status & (Alive | Paused | Falling)) !=
      (inputStatus & (Alive | Paused | Falling)))
    return True;

  // never send a packet when dead
  if (!(status & Alive)) return False;

  // otherwise always send at least one packet per second
  if (TimeKeeper::getTick() - inputTime >= MaxUpdateTime) return True;

  // get predicted state
  float predictedPos[3], predictedAzimuth, predictedVel[3];
  getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel);

  // always send a new packet on reckoned touchdown
  if (predictedPos[2] < 0.0f) return True;

  // see if position and azimuth are close enough
  if (fabsf(pos[0] - predictedPos[0]) > PositionTolerance) return True;
  if (fabsf(pos[1] - predictedPos[1]) > PositionTolerance) return True;
  if (fabsf(pos[2] - predictedPos[2]) > PositionTolerance) return True;
  if (fabsf(azimuth - predictedAzimuth) > AngleTolerance) return True;

  // prediction is good enough
  return False;
}

void			Player::doDeadReckoning()
{
  // get predicted state
  float predictedPos[3], predictedAzimuth, predictedVel[3];
  notResponding = !getDeadReckoning(predictedPos, &predictedAzimuth,
								predictedVel);
  if (!isAlive()) notResponding = False;

  // if hit ground then update input state (since we don't want to fall
  // anymore)
  if (predictedPos[2] < 0.0f) {
    predictedPos[2] = 0.0f;
    predictedVel[2] = 0.0f;
    inputStatus &= ~Falling;
    inputZSpeed = 0.0f;
    inputSpeedAzimuth = inputAzimuth;
  }

  move(predictedPos, predictedAzimuth);
  setVelocity(predictedVel);
}

