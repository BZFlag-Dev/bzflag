/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
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
#include "SceneNodeGroup.h"
#include "SceneNodeTransform.h"
#include "SceneManager.h"
#include "ShotPath.h"
#include "playing.h"

// for dead reckoning
static const float		PositionTolerance = 0.01f;		// meters
static const float		AngleTolerance = 0.01f;				// radians
static const float		MaxUpdateTime = 1.0f;				// seconds

//
// Player
//

Player::Player(PlayerId _id, TeamColor _team,
				const char* name, const char* _email) :
								notResponding(false),
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

	// make nodes
	transformSceneNode   = new SceneNodeTransform;
	teamPlayerSceneNode  = NULL;
	roguePlayerSceneNode = NULL;

	// make scene nodes
	changeTeam(team);
}

Player::~Player()
{
	unrefNodes();
	transformSceneNode->unref();
}

void					Player::setId(PlayerId newID)
{
	// crs 1/1/02 -- not even sure why this is needed;  the id shouldn't
	// change but there's some code in playing.cxx that wants to set it.
	id = newID;
}

float					Player::getRadius() const
{
	if (flag == ObesityFlag)
		return TankRadius * ObeseFactor;
	if (flag == TinyFlag)
		return TankRadius * TinyFactor;
	return TankRadius;
}

void					Player::getMuzzle(float* m) const
{
	// okay okay, I should really compute the up vector instead of using [0,0,1]
	float front = MuzzleFront;
	if (flag == ObesityFlag)
		front *= ObeseFactor;
	else if (flag == TinyFlag)
		front *= TinyFactor;
	m[0] = pos[0] + front * forward[0];
	m[1] = pos[1] + front * forward[1];
	m[2] = pos[2] + front * forward[2] + MuzzleHeight;
}

void					Player::move(const float* _pos, float _azimuth)
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

void					Player::setVelocity(const float* _velocity)
{
	velocity[0] = _velocity[0];
	velocity[1] = _velocity[1];
	velocity[2] = _velocity[2];
}

void					Player::setAngularVelocity(float _angVel)
{
	angVel = _angVel;
}

void					Player::unrefNodes()
{
	transformSceneNode->clearChildren();
	if (teamPlayerSceneNode != NULL) {
		teamPlayerSceneNode->unref();
		teamPlayerSceneNode = NULL;
	}
	if (roguePlayerSceneNode != NULL) {
		roguePlayerSceneNode->unref();
		roguePlayerSceneNode = NULL;
	}
}

void					Player::changeTeam(TeamColor _team)
{
	static const char* teamSuffix = "yrgbp";

	// set team
	team = _team;

	// get scene nodes
	unrefNodes();
	teamPlayerSceneNode = SCENEMGR->find(string_util::format(
								"player-%c", teamSuffix[team]));
	roguePlayerSceneNode = SCENEMGR->find("player-y");
	// FIXME -- error if not found
}

void					Player::setStatus(short _status)
{
	status = _status;
}

void					Player::setExplode(const TimeKeeper& t)
{
	if (!isAlive()) return;
	explodeTime = t;
	setStatus((getStatus() | short(Exploding) | short(Falling)) &
						~(short(Alive) | short(Paused)));
}

void					Player::setTeleport(const TimeKeeper& t,
												short from, short to)
{
	if (!isAlive()) return;
	teleportTime = t;
	fromTeleporter = from;
	toTeleporter = to;
	setStatus(getStatus() | short(Teleporting));
}

void					Player::changeScore(short deltaWins, short deltaLosses)
{
	wins += deltaWins;
	losses += deltaLosses;
}

void					Player::changeLocalScore(short dWins, short dLosses)
{
	localWins += dWins;
	localLosses += dLosses;
}

void					Player::setFlag(FlagId _flag)
{
	flag = _flag;
}

void					Player::endShot(int index,
								bool isHit, bool showExplosion)
{
	float pos[3];
	if (doEndShot(index, isHit, pos) && showExplosion)
		addShotExplosion(pos);
}

void					Player::updateSparks(float /*dt*/)
{
	// FIXME -- need animated alpha on tanks
	if (flag != PhantomZoneFlag || !isFlagActive()) {
		teleporterProximity = World::getWorld()->getProximity(pos, TankRadius);
		if (teleporterProximity == 0.0f) {
			// FIXME -- alpha = 1.0f;
			return;
		}
	}

	if (flag == PhantomZoneFlag && isFlagActive()) {
		// almost totally transparent
		// FIXME -- alpha = 0.25f;
	}
	else {
		// transparency depends on proximity
		// FIXME -- alpha = 1.0f - 0.75f * teleporterProximity;
	}
}

void					Player::addPlayerSceneNode(
								SceneNodeGroup* group, bool colorblind)
{
	// skip player that's dead
	if (!isAlive() && !isExploding())
		return;

	// do explosion update
	if (isExploding()) {
		// FIXME -- should set explosion time parameter
	}

	// compute transformation
	transformSceneNode->translate.clear();
	transformSceneNode->rotate.clear();
	transformSceneNode->scale.clear();
	transformSceneNode->translate.push(pos[0], pos[1], pos[2]);
	transformSceneNode->rotate.push(0.0f, 0.0f, 1.0f, azimuth * 180.0f / M_PI);
	if (flag == ObesityFlag)
		transformSceneNode->scale.push(ObeseFactor, ObeseFactor, 1.0f);
	else if (flag == TinyFlag)
		transformSceneNode->scale.push(TinyFactor, TinyFactor, 1.0f);
	else if (flag == NarrowFlag)
		transformSceneNode->scale.push(1.0f, 0.01f, 1.0f);

	if (isCrossingWall()) {
		// FIXME -- should set clipping plane
	}

	// choose player model
	transformSceneNode->clearChildren();
	if (colorblind)
		transformSceneNode->pushChild(roguePlayerSceneNode);
	else
		transformSceneNode->pushChild(teamPlayerSceneNode);

	// add player
	group->pushChild(transformSceneNode);

	// add black cap
	if (isAlive() && (isPaused() || isNotResponding())) {
//    pausedSphere->move(pos, 1.5f * TankRadius);
//    group->pushChild(pauseSceneNode);
	}
}

void					Player::addShotsSceneNodes(
								SceneNodeGroup* group, bool colorblind)
{
	// add shots
	const int count = World::getWorld()->getMaxShots();
	for (int i = 0; i < count; i++) {
		ShotPath* shot = getShot(i);
		if (shot && !shot->isExpiring() && !shot->isExpired())
			shot->addShot(group, colorblind);
	}
}

void*					Player::pack(void* buf) const
{
	((Player*)this)->setDeadReckoning();
	buf = nboPackShort(buf, int16_t(status));
	buf = nboPackVector(buf, pos);
	buf = nboPackVector(buf, velocity);
	buf = nboPackFloat(buf, azimuth);
	buf = nboPackFloat(buf, angVel);
	return buf;
}

void*					Player::unpack(void* buf)
{
	int16_t inStatus;
	buf = nboUnpackShort(buf, inStatus);
	buf = nboUnpackVector(buf, pos);
	buf = nboUnpackVector(buf, velocity);
	buf = nboUnpackFloat(buf, azimuth);
	buf = nboUnpackFloat(buf, angVel);
	status = short(inStatus);
	setDeadReckoning();
	return buf;
}

void					Player::setDeadReckoning()
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

bool					Player::getDeadReckoning(
								float* predictedPos, float* predictedAzimuth,
								float* predictedVel) const
{
	// see if predicted position and orientation (only) are close enough
	const float dt2 = inputPrevTime - inputTime;
	((Player*)this)->inputPrevTime = TimeKeeper::getTick();
	const float dt = inputPrevTime - inputTime;

	if (inputStatus & AutoPilot) {
		// AutoPilot bot code here
		predictedPos[0] = inputPos[0];
		predictedPos[1] = inputPos[1];
		predictedPos[2] = inputPos[2];
		predictedVel[0] = fabsf(inputSpeed) * cosf(inputSpeedAzimuth);
		predictedVel[1] = fabsf(inputSpeed) * sinf(inputSpeedAzimuth);
		predictedVel[2] = 0.0f;
		*predictedAzimuth = inputAzimuth;
	}
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

bool					Player::isDeadReckoningWrong() const
{
	// always send a new packet when some kinds of status change
	if ((status & (Alive | Paused | Falling)) !=
      (inputStatus & (Alive | Paused | Falling)))
		return true;

	// never send a packet when dead
	if (!(status & Alive)) return false;

	// otherwise always send at least one packet per second
	if (TimeKeeper::getTick() - inputTime >= MaxUpdateTime) return true;

	// get predicted state
	float predictedPos[3], predictedAzimuth, predictedVel[3];
	getDeadReckoning(predictedPos, &predictedAzimuth, predictedVel);

	// always send a new packet on reckoned touchdown
	if (predictedPos[2] < 0.0f) return true;

	// see if position and azimuth are close enough
	if (fabsf(pos[0] - predictedPos[0]) > PositionTolerance) return true;
	if (fabsf(pos[1] - predictedPos[1]) > PositionTolerance) return true;
	if (fabsf(pos[2] - predictedPos[2]) > PositionTolerance) return true;
	if (fabsf(azimuth - predictedAzimuth) > AngleTolerance) return true;

	// prediction is good enough
	return false;
}

void					Player::doDeadReckoning()
{
	// get predicted state
	float predictedPos[3], predictedAzimuth, predictedVel[3];
	notResponding = !getDeadReckoning(predictedPos, &predictedAzimuth,
																predictedVel);
	if (!isAlive()) notResponding = false;

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

