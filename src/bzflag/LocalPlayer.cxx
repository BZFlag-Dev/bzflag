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

#include "LocalPlayer.h"
#include "World.h"
#include "SceneNodeBillboard.h"
#include "SceneNodeGeometry.h"
#include "SceneNodeGroup.h"
#include "SceneNodeTransform.h"
#include "SceneManager.h"
#include "ServerLink.h"
#include "sound.h"
#include "Flag.h"
#include "StateDatabase.h"
#include "BzfEvent.h"
#include <stdlib.h>
#include <math.h>

//
// BaseLocalPlayer
//

BaseLocalPlayer::BaseLocalPlayer(PlayerId id,
								const char* name, const char* email) :
								Player(id, RogueTeam, name, email),
								lastTime(TimeKeeper::getTick()),
								salt(0)
{
	lastPosition[0] = 0.0f;
	lastPosition[1] = 0.0f;
	lastPosition[2] = 0.0f;
	bbox[0][0] = bbox[1][0] = 0.0f;
	bbox[0][1] = bbox[1][1] = 0.0f;
	bbox[0][2] = bbox[1][2] = 0.0f;
}

BaseLocalPlayer::~BaseLocalPlayer()
{
	// do nothing
}

int						BaseLocalPlayer::getSalt()
{
	salt = (salt + 1) & 127;
	return salt << 8;
}

void					BaseLocalPlayer::update()
{
	// save last position
	const float* oldPosition = getPosition();
	lastPosition[0] = oldPosition[0];
	lastPosition[1] = oldPosition[1];
	lastPosition[2] = oldPosition[2];

	// update by time step
	float dt = TimeKeeper::getTick() - lastTime;
	lastTime = TimeKeeper::getTick();
	if (dt < 0.001f) dt = 0.001f;
	doUpdateMotion(dt);

	// compute motion's bounding box around center of tank
	const float* newVelocity = getVelocity();
	bbox[0][0] = bbox[1][0] = oldPosition[0];
	bbox[0][1] = bbox[1][1] = oldPosition[1];
	bbox[0][2] = bbox[1][2] = oldPosition[2];
	if (newVelocity[0] > 0.0f)
		bbox[1][0] += dt * newVelocity[0];
	else
		bbox[0][0] += dt * newVelocity[0];
	if (newVelocity[1] > 0.0f)
		bbox[1][1] += dt * newVelocity[1];
	else
		bbox[0][1] += dt * newVelocity[1];
	if (newVelocity[2] > 0.0f)
		bbox[1][2] += dt * newVelocity[2];
	else
		bbox[0][2] += dt * newVelocity[2];

	// expand bounding box to include entire tank
	float size = TankRadius;
	if (getFlag() == ObesityFlag)
		size *= ObeseFactor;
	else if (getFlag() == TinyFlag)
		size *= TinyFactor;
	bbox[0][0] -= size;
	bbox[1][0] += size;
	bbox[0][1] -= size;
	bbox[1][1] += size;
	bbox[1][2] += TankHeight;

	// do remaining update stuff
	doUpdate(dt);
}

Ray						BaseLocalPlayer::getLastMotion() const
{
	return Ray(lastPosition, getVelocity());
}

#ifdef __MWERKS__
	const float				(*BaseLocalPlayer::getLastMotionBBox() )[3] const
#else
	const float				(*BaseLocalPlayer::getLastMotionBBox() const)[3]
#endif
{
	return bbox;
}

//
// LocalPlayer
//

LocalPlayer*			LocalPlayer::mainPlayer = NULL;

LocalPlayer::LocalPlayer(PlayerId id,
								const char* name, const char* email) :
								BaseLocalPlayer(id, name, email),
								RoamView(RoamViewFree),
								roamTrackTank(0),
								roamTrackFlag(0),
								roamPos(0.0, 0.0, MuzzleHeight),
								roamDPos(0.0, 0.0, 0.0),
								roamTheta(0.0),
								roamDTheta(0.0),
								roamPhi(0.0),
								roamDPhi(0.0),
								roamZoom(60.0),
								roamDZoom(0.0),
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
								nemesis(NULL)
{
	if(BZDB->get("playerType") == "observer") {
		setStatus(Alive);
	}
	// initialize shots array to no shots fired
	const int numShots = World::getWorld()->getMaxShots();
	shots = new LocalShotPath*[numShots];
	for (int i = 0; i < numShots; i++)
		shots[i] = NULL;
	keyboardMoving = false;
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
	if (antidoteFlag != NULL)
		antidoteFlag->unref();
}

LocalPlayer*				LocalPlayer::getMyTank()
{
	return mainPlayer;
}

std::string				LocalPlayer::getRoamingStatus(RemotePlayer** players, World* world) {
	switch(RoamView) {
		case RoamViewTrack:
			return std::string("Tracking ") + players[roamTrackTank]->getCallSign();
		case RoamViewFollow:
			return std::string("Following ") + players[roamTrackTank]->getCallSign();
		case RoamViewFP:
			return std::string("Driving with ") + players[roamTrackTank]->getCallSign();
		case RoamViewFlag:
			return std::string("Tracking ") + Flag::getName(world->getFlag(roamTrackFlag).id) + " Flag";
		case RoamViewFree:
		default:
			return std::string("Roaming");
	}
}

void					LocalPlayer::setMyTank(LocalPlayer* player)
{
	mainPlayer = player;
}

void					LocalPlayer::doUpdate(float dt)
{
	const bool hadShotActive = anyShotActive;
	const int numShots = World::getWorld()->getMaxShots();

	// if paused then boost the reload times by dt (so that, effectively,
	// reloading isn't performed)
	int i;
	if (isPaused())
		for (i = 0; i < numShots; i++)
			if (shots[i])
				shots[i]->boostReloadTime(dt);

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
		getFlag() != NoFlag && Flag::getType(getFlag()) == FlagSticky &&
		flagShakingTime > 0.0f) {
		flagShakingTime -= dt;
		if (flagShakingTime <= 0.0f) {
			flagShakingTime = 0.0f;
			ServerLink::getServer()->sendDropFlag(DropReasonTimeout, getPosition());
		}
	}
}

void					LocalPlayer::doUpdateMotion(float dt)
{
	if(BZDB->get("playerType") == "observer")
		return;
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
	if (isTeleporting() && getTeleportTime() - lastTime >= TeleportTime)
		setStatus(getStatus() & ~short(Teleporting));

	// phased means we can pass through buildings
	const bool phased = (location == Dead || location == Exploding ||
						(getFlag() == OscOverthrusterFlag) ||
						(getFlag() == PhantomZoneFlag && isFlagActive()));

	// get linear and angular speed at start of time step
	if (dt != 0.0f) {
		if (location == Dead || isPaused()) {
			// can't move if paused or dead -- set dt to zero instead of
			// clearing velocity and newAngVel for when we resume (if paused)
			dt = 0.0f;
			newAngVel = oldAngVel;
		}
		else if (location == Exploding) {
			// see if explosing time has expired
			if (lastTime - getExplodeTime() >= ExplodeTime) {
				dt -= (lastTime - getExplodeTime()) - ExplodeTime;
				if (dt < 0.0f) dt = 0.0f;
				setStatus(DeadStatus);
				location = Dead;
			}

			// can't control explosion motion
			newVelocity[2] += Gravity * dt;
			newAngVel = 0.0f;	// or oldAngVel to spin while exploding
		}
		else {
			// full control
			float speed = desiredSpeed;
			newAngVel = desiredAngVel;

			// limit acceleration
			doMomentum(dt, speed, newAngVel);

			// compute velocity so far
			if (location == OnGround || location == OnBuilding ||
				(location == InBuilding && oldPosition[2] == 0.0f)) {
				newVelocity[0] = speed * cosf(oldAzimuth + 0.5f * dt * newAngVel);
				newVelocity[1] = speed * sinf(oldAzimuth + 0.5f * dt * newAngVel);
				newVelocity[2] = 0.0f;
				if (oldPosition[2] != 0.0f) newVelocity[2] += Gravity * dt;
			}
			else {
				// can't control motion in air
				newVelocity[2] += Gravity * dt;
				newAngVel = oldAngVel;
			}

			// now apply outside forces
			doForces(dt, newVelocity, newAngVel);

			// save speed for next update
			lastSpeed = speed;
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
	if (location != Dead && location != Exploding) location = OnGround;
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
		if (newPos[2] < 0.0f) newPos[2] = 0.0f;

		// see if we hit anything.  if not then we're done.
		obstacle = getHitBuilding(newPos, newAzimuth, phased, expelled);
		if (!obstacle || !expelled) break;

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
			if (newPos[2] < 0.0f) newPos[2] = 0.0f;

			// see if we hit anything
			bool searchExpelled;
			const Obstacle* searchObstacle =
				getHitBuilding(newPos, newAzimuth, phased, searchExpelled);

			// if no hit then search latter half of time step
			if (!searchObstacle || !searchExpelled) searchTime = t;

			// if we hit a building then record which one and where
			else if (searchObstacle) {
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
		if (newPos[2] < 0.0f) newPos[2] = 0.0f;

		// record how much time is left in time step
		timeStep -= searchTime;

		// get normal at intersection.  sometimes fancy test says there's
		// no intersection but we're expecting one so, in that case, fall
		// back to simple normal calculation.
		float normal[3];
		if (!getHitNormal(obstacle, newPos, newAzimuth, hitPos, hitAzimuth, normal))
			obstacle->getNormal(newPos, normal);

		// check for being on a building
		if (newPos[2] > 0.0f && normal[2] > 0.001f) {
			if (location != Dead && location != Exploding && expelled)
				location = OnBuilding;
			newVelocity[2] = 0.0f;
		}

		else {
			// get component of velocity in normal direction (in horizontal plane)
			float mag = normal[0] * newVelocity[0] + normal[1] * newVelocity[1];

			// handle upward normal component to prevent an upward force
			if (normal[2] != 0.0f) {
				// if going down then stop falling
				if (newVelocity[2] < 0.0f && newVelocity[2] -
				(mag + normal[2] * newVelocity[2]) * normal[2] > 0.0f)
					newVelocity[2] = 0.0f;

				// normalize force magnitude in horizontal plane
				mag /= normal[0] * normal[0] + normal[1] * normal[1];
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
		if (obstacle && !expelled) {
			location = InBuilding;
		}
		else if (newPos[2] > 0.0f) {
			location = InAir;
		}
	}
	insideBuilding = (const Obstacle*)(location == InBuilding ? obstacle : NULL);

	// see if we're crossing a wall
	if (location == InBuilding && getFlag() == OscOverthrusterFlag) {
		if (insideBuilding->isCrossing(newPos, newAzimuth,
						0.5f * TankLength, 0.5f * TankWidth, NULL))
			setStatus(getStatus() | int(CrossingWall));
		else
			setStatus(getStatus() & ~int(CrossingWall));
	}
	else if (World::getWorld()->crossingTeleporter(newPos, newAzimuth,
						0.5f * TankLength, 0.5f * TankWidth, crossingPlane)) {
		setStatus(getStatus() | int(CrossingWall));
	}
	else {
		setStatus(getStatus() & ~int(CrossingWall));
	}

	// compute actual velocities.  do this before teleportation.
	if (dt != 0.0f) {
		newAngVel = (newAzimuth - oldAzimuth) / dt;
		newVelocity[0] = (newPos[0] - oldPosition[0]) / dt;
		newVelocity[1] = (newPos[1] - oldPosition[1]) / dt;
		newVelocity[2] = (newPos[2] - oldPosition[2]) / dt;
	}

	// see if we teleported
	int face;
	const Teleporter* teleporter = (const Teleporter*)(!isAlive() ? NULL :
		World::getWorld()->crossesTeleporter(oldPosition, newPos, face));
	if (teleporter) {
		if (getFlag() == PhantomZoneFlag) {
			// change zoned state
			setStatus(getStatus() ^ FlagActive);
			if (getStatus() & FlagActive)
		      playLocalSound( SFX_PHANTOM );
		}
		else {
			// teleport
			const int source = World::getWorld()->getTeleporter(teleporter, face);
			const int target = World::getWorld()->getTeleportTarget(source);
			int outFace;
			const Teleporter* outPort = World::getWorld()->
										getTeleporter(target, outFace);
			teleporter->getPointWRT(*outPort, face, outFace,
										newPos, newVelocity, newAzimuth,
										newPos, newVelocity, &newAzimuth);

			// save teleport info
			setTeleport(lastTime, source, target);
			ServerLink::getServer()->sendTeleport(source, target);
			playLocalSound(SFX_TELEPORT);
		}
	}

	// play landing sound if we weren't on something and now we are
	if (oldLocation == InAir && (location == OnGround || location == OnBuilding))
		playLocalSound(SFX_LAND);

	// set falling status
	if (location == OnGround || location == OnBuilding ||
		(location == InBuilding && newPos[2] == 0.0f))
		setStatus(getStatus() & ~short(Falling));
	else if (location == InAir || location == InBuilding)
		setStatus(getStatus() | short(Falling));

	// compute firing status
	switch (location) {
		case Dead:
		case Exploding:
			firingStatus = Deceased;
			break;
		case InBuilding:
			firingStatus = (getFlag() == PhantomZoneFlag) ? Zoned : Sealed;
			break;
		default:
			if (getFlag() == PhantomZoneFlag && isFlagActive())
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
		if (dist < (getRadius() + FlagRadius) * (getRadius() + FlagRadius))
			ServerLink::getServer()->sendDropFlag(DropReasonTimeout, getPosition());
	}

	if (oldPosition[0] != newPos[0] || oldPosition[1] != newPos[1] ||
		oldPosition[2] != newPos[2] || oldAzimuth != newAzimuth)
		moveSoundReceiver(newPos[0], newPos[1], newPos[2], newAzimuth,
		(dt == 0.0f || teleporter != NULL && getFlag() != PhantomZoneFlag));
	if (dt == 0.0f)
		speedSoundReceiver(newVelocity[0], newVelocity[1], newVelocity[2]);
	else
		speedSoundReceiver((newPos[0] - oldPosition[0]) / dt,
						(newPos[1] - oldPosition[1]) / dt,
						(newPos[2] - oldPosition[2]) / dt);
}

const Obstacle*			LocalPlayer::getHitBuilding(const float* p, float a,
								bool phased, bool& expelled) const
{
	float length = 0.5f * TankLength;
	float width = 0.5f * TankWidth;
	if (getFlag() == ObesityFlag) {
		length *= ObeseFactor;
		width *= 2.0f * ObeseFactor;
	}
	else if (getFlag() == TinyFlag) {
		length *= TinyFactor;
		width *= 2.0f * TinyFactor;
	}
	else if (getFlag() == NarrowFlag) {
		width = 0.0f;
	}

	const Obstacle* obstacle = World::getWorld()->
								hitBuilding(p, a, length, width);
	expelled = (obstacle != NULL);
	if (expelled && phased)
		expelled = (obstacle->getType() == WallObstacle::getClassName() ||
				obstacle->getType() == Teleporter::getClassName() ||
				(getFlag() == OscOverthrusterFlag && desiredSpeed < 0.0f &&
				p[2] == 0.0f));
	return obstacle;
}

bool					LocalPlayer::getHitNormal(const Obstacle* o,
								const float* pos1, float azimuth1,
								const float* pos2, float azimuth2,
								float* normal) const
{
	float length = 0.5f * TankLength;
	float width = 0.5f * TankWidth;
	if (getFlag() == ObesityFlag) {
		length *= ObeseFactor;
		width *= 2.0f * ObeseFactor;
	}
	else if (getFlag() == TinyFlag) {
		length *= TinyFactor;
		width *= 2.0f * TinyFactor;
	}
	else if (getFlag() == NarrowFlag) {
		width = 0.0f;
	}

	return o->getHitNormal(pos1, azimuth1, pos2, azimuth2, length, width, normal);
}

float					LocalPlayer::getReloadTime() const
{
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
	if (minTime < 0.0f) minTime = 0.0f;
	return minTime;
}

float					LocalPlayer::getFlagShakingTime() const
{
	return flagShakingTime;
}

int						LocalPlayer::getFlagShakingWins() const
{
	return flagShakingWins;
}

const float*				LocalPlayer::getAntidoteLocation() const
{
	return antidoteFlag ? flagAntidotePos : NULL;
}

ShotPath*				LocalPlayer::getShot(int index) const
{
	return shots[index & 255];
}

void					LocalPlayer::restart(const float* pos, float _azimuth)
{
	// put me in limbo
	setStatus(short(DeadStatus));

	// can't have a flag
	setFlag(NoFlag);

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
	location = OnGround;
	lastSpeed = 0.0f;
	desiredSpeed = 0.0f;
	desiredAngVel = 0.0f;
	move(pos, _azimuth);
	setVelocity(zero);
	setAngularVelocity(0.0f);
	setKeyboardSpeed(0.0f);
	setKeyboardAngVel(0.0f);
	setSlowKeyboard(false);
	resetKey();
	doUpdateMotion(0.0f);

	// make me alive now
	setStatus(getStatus() | short(Alive));
}

void					LocalPlayer::setTeam(TeamColor team)
{
	changeTeam(team);
}

void					LocalPlayer::setDesiredSpeed(float fracOfMaxSpeed)
{
	// can't go faster forward than at top speed, and backward at half speed
	if (fracOfMaxSpeed > 1.0f) fracOfMaxSpeed = 1.0f;
	else if (fracOfMaxSpeed < -0.5f) fracOfMaxSpeed = -0.5f;

	// oscillation overthruster tank in building can't back up
	if (fracOfMaxSpeed < 0.0f && getLocation() == InBuilding &&
								getFlag() == OscOverthrusterFlag)
		fracOfMaxSpeed = 0.0f;

	// boost speed for certain flags
	if (getFlag() == VelocityFlag)
		fracOfMaxSpeed *= VelocityAd;

	// set desired speed
	desiredSpeed = fracOfMaxSpeed * TankSpeed;
}

void					LocalPlayer::setDesiredAngVel(float fracOfMaxAngVel)
{
	// limit turn speed to maximum
	if (fracOfMaxAngVel > 1.0f) fracOfMaxAngVel = 1.0f;
	else if (fracOfMaxAngVel < -1.0f) fracOfMaxAngVel = -1.0f;

	// further limit turn speed for certain flags
	if (fracOfMaxAngVel < 0.0f && getFlag() == LeftTurnOnlyFlag)
		fracOfMaxAngVel = 0.0f;
	else if (fracOfMaxAngVel > 0.0f && getFlag() == RightTurnOnlyFlag)
		fracOfMaxAngVel = 0.0f;

	// boost turn speed for other flags
	if (getFlag() == QuickTurnFlag)
		fracOfMaxAngVel *= AngularAd;

	// set desired turn speed
	desiredAngVel = fracOfMaxAngVel * TankAngVel;
}

void					LocalPlayer::setPause(bool pause)
{
	if (isAlive()) {
		if (pause && !isPaused()) {
			setStatus(getStatus() | short(Paused));
		}
		else if (!pause && isPaused()) {
			setStatus(getStatus() & ~short(Paused));
		}
	}
}

bool					LocalPlayer::fireShot()
{
	if(BZDB->get("playerType") == "observer")
		return true;
	// find an empty slot
	const int numShots = World::getWorld()->getMaxShots();
	int i;
	for (i = 0; i < numShots; i++)
		if (!shots[i])
			break;
	if (i == numShots) return false;

	// make sure we're allowed to shoot
	if (!isAlive() || isPaused() || location == InBuilding ||
		(getFlag() == PhantomZoneFlag && isFlagActive()))
		return false;

	// prepare shot
	FiringInfo firingInfo(*this, i + getSalt());
	if (firingInfo.flag == ShockWaveFlag) {
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
		// comment following line if you want shots to have the same vertical
		// velocity as the tank when fired.  keeping shots moving horizontally
		// makes the game more playable, though.
		firingInfo.shot.vel[2] = 0.0f;
	}

	// make shot and put it in the table
	shots[i] = new LocalShotPath(firingInfo);

	ServerLink::getServer()->sendBeginShot(firingInfo);
	if (firingInfo.flag == ShockWaveFlag)
		playLocalSound(SFX_SHOCK);
	else if (firingInfo.flag == LaserFlag)
		playLocalSound(SFX_LASER);
	else if (firingInfo.flag == GuidedMissileFlag)
		playLocalSound(SFX_MISSILE);
	else
		playLocalSound(SFX_FIRE);

	return true;
}

bool					LocalPlayer::doEndShot(
								int id, bool isHit, float* pos)
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

void					LocalPlayer::jump()
{
	// can't jump unless on the ground or a building
	if (location != OnGround && location != OnBuilding)
		return;

	// can only jump with a jumping flag or if jumping is allowed for all
	if (getFlag() != JumpingFlag && !World::getWorld()->allowJumping())
		return;

	// add jump velocity (actually, set the vertical component since you
	// can only jump if resting on something)
	const float* oldVelocity = getVelocity();
	float newVelocity[3];
	newVelocity[0] = oldVelocity[0];
	newVelocity[1] = oldVelocity[1];
	newVelocity[2] = /*oldVelocity[2] +*/ JumpVelocity;
	setVelocity(newVelocity);
	location = InAir;
	playLocalSound(SFX_JUMP);
}

void					LocalPlayer::setTarget(const Player* _target)
{
	target = _target;
}

void					LocalPlayer::setNemesis(const Player* _nemesis)
{
	nemesis = _nemesis;
}


void					LocalPlayer::explodeTank()
{
	if (location == Dead || location == Exploding) return;
	setExplode(TimeKeeper::getTick());
	const float* oldVelocity = getVelocity();
	float newVelocity[3];
	newVelocity[0] = oldVelocity[0];
	newVelocity[1] = oldVelocity[1];
	newVelocity[2] = -0.5f * Gravity * ExplodeTime;
	setVelocity(newVelocity);
	location = Exploding;
	target = NULL;			// lose lock when dead
}

void					LocalPlayer::doMomentum(float dt,
												float& speed, float& angVel)
{
	// get maximum linear and angular accelerations
	const float linearAcc = (getFlag() == MomentumFlag) ? MomentumLinAcc :
								World::getWorld()->getLinearAcceleration();
	const float angularAcc = (getFlag() == MomentumFlag) ? MomentumAngAcc :
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

void					LocalPlayer::doForces(float /*dt*/,
												float* /*velocity*/,
												float& /*angVel*/)
{
	// apply external forces
	// do nothing -- no external forces right now
}

// NOTE -- minTime should be initialized to Infinity by the caller
bool					LocalPlayer::checkHit(const Player* source,
								const ShotPath*& hit, float& minTime) const
{
	bool goodHit = false;

	// if firing tank is paused then it doesn't count
	if (source->isPaused()) return goodHit;

	const int maxShots = World::getWorld()->getMaxShots();
	for (int i = 0; i < maxShots; i++) {
		// get shot
		const ShotPath* shot = source->getShot(i);
		if (!shot || shot->isExpired()) continue;

		// my own shock wave cannot kill me
		if (source == this && shot->getFlag() == ShockWaveFlag) continue;

		// short circuit test if shot can't possibly hit.
		// only superbullet or shockwave can kill zoned dude
		const FlagId shotFlag = shot->getFlag();
		if (getFlag() == PhantomZoneFlag && isFlagActive() &&
				shotFlag != SuperBulletFlag && shotFlag != ShockWaveFlag)
			continue;
		// laser can't hit a cloaked tank
		if (getFlag() == CloakingFlag && shotFlag == LaserFlag)
			continue;

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

void					LocalPlayer::setFlag(FlagId id)
{
	Player::setFlag(id);

	// discard previous antidote flag, if any
	if (antidoteFlag != NULL) {
		antidoteFlag->unref();
		antidoteFlag = NULL;
	}
	flagShakingTime = 0.0f;
	flagShakingWins = 0;

	// if it's bad then reset countdowns and set antidote flag
	if (getFlag() != NoFlag && Flag::getType(getFlag()) == FlagSticky) {
		if (World::getWorld()->allowShakeTimeout())
			flagShakingTime = World::getWorld()->getFlagShakeTimeout();
		if (World::getWorld()->allowShakeWins())
			flagShakingWins = World::getWorld()->getFlagShakeWins();
		if (World::getWorld()->allowAntidote()) {
			do {
				if (World::getWorld()->allowTeamFlags()) {
					flagAntidotePos[0] = 0.5f * WorldSize * ((float)bzfrand() - 0.5f);
					flagAntidotePos[1] = 0.5f * WorldSize * ((float)bzfrand() - 0.5f);
					flagAntidotePos[2] = 0.0f;
				}
				else {
					flagAntidotePos[0] = (WorldSize - BaseSize) * ((float)bzfrand() - 0.5f);
					flagAntidotePos[1] = (WorldSize - BaseSize) * ((float)bzfrand() - 0.5f);
					flagAntidotePos[2] = 0.0f;
				}
			} while (World::getWorld()->inBuilding(flagAntidotePos, TankRadius));

			// make scene graph
			SceneNodeTransform* xform = new SceneNodeTransform;
			SceneNodeBillboard* billboard = new SceneNodeBillboard;
			SceneNodeGeometry* geometry = new SceneNodeGeometry;
			xform->pushChild(billboard);
			billboard->pushChild(geometry);
			geometry->pushChild(SCENEMGR->find("flag"));
			geometry->unref();
			billboard->unref();
			antidoteFlag = xform;

			// prep scene graph
			xform->translate.push(flagAntidotePos[0],
								flagAntidotePos[1],
								flagAntidotePos[2]);
			billboard->axis.push(0.0f, 0.0f, 1.0f);
			geometry->pushBundle();
			geometry->color->push(1.0f, 1.0f, 0.0f, 1.0f);
		}
	}
}

void					LocalPlayer::changeScore(short deltaWins,
												short deltaLosses)
{
	Player::changeScore(deltaWins, deltaLosses);
	ServerLink::getServer()->sendNewScore(getWins(), getLosses());
	if (deltaWins > 0 && World::getWorld()->allowShakeWins() &&
												flagShakingWins > 0) {
		flagShakingWins -= deltaWins;
		if (flagShakingWins <= 0) {
			flagShakingWins = 0;
			ServerLink::getServer()->sendDropFlag(DropReasonTimeout, getPosition());
		}
	}
}

void					LocalPlayer::addAntidoteSceneNode(SceneNodeGroup* group)
{
	if (antidoteFlag != NULL) {
		group->pushChild(antidoteFlag);
	}
}
// ex: shiftwidth=4 tabstop=4
