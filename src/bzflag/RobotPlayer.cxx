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

/*
 *
 */

//#define SHOOTING_FIX

#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "RobotPlayer.h"
#include "World.h"
#include "ServerLink.h"

std::vector<BzfRegion*>* RobotPlayer::obstacleList = NULL;

RobotPlayer::RobotPlayer(const PlayerId& _id, const char* _name,
				ServerLink* _server,
				const char* _email = "anonymous") :
				BaseLocalPlayer(_id, _name, _email),
				server(_server),
				target(NULL),
				pathIndex(0),
				timeSinceShot(0.0f),
				timerForShot(0.0f)
{
  // NOTE -- code taken directly from LocalPlayer
  // initialize shots array to no shots fired
  const int numShots = World::getWorld()->getMaxShots();
  shots = new LocalShotPath*[numShots];
  for (int i = 0; i < numShots; i++)
    shots[i] = NULL;
}

RobotPlayer::~RobotPlayer()
{
  // NOTE -- code taken directly from LocalPlayer
  // free shots
  const int numShots = World::getWorld()->getMaxShots();
  for (int i = 0; i < numShots; i++)
    if (shots[i])
      delete shots[i];
  delete[] shots;
}

// NOTE -- code taken directly from LocalPlayer
ShotPath*		RobotPlayer::getShot(int index) const
{
  return shots[index & 255];
}

// NOTE -- code taken directly from LocalPlayer
bool			RobotPlayer::doEndShot(
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

// NOTE -- code mostly taken from LocalPlayer::doUpdate()
void			RobotPlayer::doUpdate(float dt)
{
  // reap dead (reloaded) shots
  const int numShots = World::getWorld()->getMaxShots();
  int i;
  for (i = 0; i < numShots; i++)
    if (shots[i] && shots[i]->isReloaded()) {
      if (!shots[i]->isExpired())
	shots[i]->setExpired();
      delete shots[i];
      shots[i] = NULL;
    }

  // fire shot if any available
  timeSinceShot += dt;
  float reloadTime = BZDB->eval(StateDatabase::BZDB_RELOADTIME);
#if !defined(SHOOTING_FIX)
  timeSinceShot += reloadTime / numShots;
#endif
  // separate shot by at least 0.4 sec
  timerForShot  -= dt;
  if (timerForShot < 0.0f)
    timerForShot = 0.0f;
  bool        shoot   = true;
  const float azimuth = getAngle();
  // Allow shooting only if angle is near and timer has elapsed
  if ((int)path.size() == 0 || timerForShot > 0.0f)
    shoot = false;
  else
    for (i = pathIndex; i < (int)path.size(); i++) {
      float azimuthDiff = pathAzimuth[i] - azimuth;
      if (azimuthDiff > M_PI)
	azimuthDiff -= 2.0f * M_PI;
      else
	if (azimuthDiff < -M_PI)
	  azimuthDiff += 2.0f * M_PI;
      if (fabs(azimuthDiff) > 0.01f) {
	shoot = false;
	break;
      }
    }
  if (isAlive() && timeSinceShot > reloadTime / numShots && shoot) {
    timeSinceShot -= reloadTime / numShots;
    for (i = 0; i < numShots; i++)
      if (!shots[i]) {
	// separate shot by 0.4 sec (experimental value)
	timerForShot = 0.4f;
	FiringInfo firingInfo(*this, i + getSalt());
	firingInfo.shot.vel[2] = 0.0f;
	shots[i] = new LocalShotPath(firingInfo);
	server->sendBeginShot(firingInfo);
	break;
      }
  }

  // update shots
  for (i = 0; i < numShots; i++)
    if (shots[i])
      shots[i]->update(dt);

}

void			RobotPlayer::doUpdateMotion(float dt)
{
  // record time
  const float dt0 = dt;

  // record previous position
  const float oldAzimuth = getAngle();
  const float* oldPosition = getPosition();
  float position[3];
  position[0] = oldPosition[0];
  position[1] = oldPosition[1];
  position[2] = oldPosition[2];
  float azimuth = oldAzimuth;
  float tankAngVel = BZDB->eval(StateDatabase::BZDB_TANKANGVEL);
  if (isAlive()) {
    while (dt > 0.0 && pathIndex < (int)path.size()) {
      float azimuthDiff = pathAzimuth[pathIndex] - azimuth;
      if (azimuthDiff > M_PI) azimuthDiff -= 2.0f * M_PI;
      else if (azimuthDiff < -M_PI) azimuthDiff += 2.0f * M_PI;
      if (fabs(azimuthDiff) > 0.01f) {
	// tank doesn't move forward while turning
	if (azimuthDiff >= dt * tankAngVel) {
	  azimuth += dt * tankAngVel;
	  dt = 0.0f;
	}
	else if (azimuthDiff <= -dt * tankAngVel) {
	  azimuth -= dt * tankAngVel;
	  dt = 0.0f;
	}
	else {
	  azimuth += azimuthDiff;
	  dt -= fabsf(azimuthDiff / tankAngVel);
	}
      }
      else {
	// tank doesn't turn while moving forward
	// find how long it will take to get to next path segment
	const float* endPoint = path[pathIndex].get();
	float v[2];
	v[0] = endPoint[0] - position[0];
	v[1] = endPoint[1] - position[1];
	const float distance = hypotf(v[0], v[1]);
	float t;
	float tankSpeed = BZDB->eval(StateDatabase::BZDB_TANKSPEED);
	if (distance <= dt * tankSpeed) {
	  pathIndex++;
	  t = distance / tankSpeed;
	}
	else {
	  t = dt;
	}
	dt -= t;
	position[0] += t * tankSpeed * cosf(azimuth);
	position[1] += t * tankSpeed * sinf(azimuth);
      }
    }
  }
  else if (isExploding()) {
    if (lastTime - getExplodeTime() >= BZDB->eval(StateDatabase::BZDB_EXPLODETIME))
      setStatus(PlayerState::DeadStatus);
  }

  float velocity[3];
  velocity[0] = (position[0] - oldPosition[0]) / dt0;
  velocity[1] = (position[1] - oldPosition[1]) / dt0;
  velocity[2] = getVelocity()[2] + BZDB->eval(StateDatabase::BZDB_GRAVITY) * dt0;
  position[2] += dt0 * velocity[2];
  if (position[2] <= 0.0f) {
    position[2] = 0.0f;
    velocity[2] = 0.0f;
  }

  move(position, azimuth);
  setVelocity(velocity);
  setAngularVelocity((getAngle() - oldAzimuth) / dt0);
}

// NOTE -- code taken directly from LocalPlayer
// NOTE -- minTime should be initialized to Infinity by the caller
bool			RobotPlayer::checkHit(const Player* source,
						const ShotPath*& hit,
						float& minTime) const
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
    if (source == this && shot->getFlag() == Flags::ShockWave) continue;

    // short circuit test if shot can't possibly hit.
    // only superbullet or shockwave can kill zoned dude
    const FlagDesc* shotFlag = shot->getFlag();
    if (getFlag() == Flags::PhantomZone && isFlagActive() &&
		shotFlag != Flags::SuperBullet && shotFlag != Flags::ShockWave)
      continue;
    // laser can't hit a cloaked tank
    if (getFlag() == Flags::Cloaking && shotFlag == Flags::Laser)
      continue;

    // test myself against shot
    float position[3];
    const float t = shot->checkHit(this, position);
    if (t >= minTime) continue;

/* robots don't have this yet
    // test if shot hit a part of my tank that's through a teleporter.
    // hit is no good if hit point is behind crossing plane.
    if (isCrossingWall() && position[0] * crossingPlane[0] +
		position[1] * crossingPlane[1] +
		position[2] * crossingPlane[2] + crossingPlane[3] < 0.0f)
      continue;
*/

    // okay, shot hit
    goodHit = true;
    hit = shot;
    minTime = t;
  }
  return goodHit;
}

void			RobotPlayer::explodeTank()
{
  // NOTE -- code taken directly from LocalPlayer
  setExplode(TimeKeeper::getTick());
  const float* oldVelocity = getVelocity();
  float newVelocity[3];
  newVelocity[0] = oldVelocity[0];
  newVelocity[1] = oldVelocity[1];
  newVelocity[2] = -0.5f * BZDB->eval(StateDatabase::BZDB_GRAVITY) * BZDB->eval(StateDatabase::BZDB_EXPLODETIME);
  setVelocity(newVelocity);
  target = NULL;
  path.clear();
}

void			RobotPlayer::setTeam(TeamColor team)
{
  changeTeam(team);
}

void			RobotPlayer::changeScore(short deltaWins,
						short deltaLosses,
						short deltaTeamKills)
{
  Player::changeScore(deltaWins, deltaLosses, deltaTeamKills);
}

void			RobotPlayer::restart()
{
  // pick a random location and position to join at
  // FIXME -- doesn't handle starting on team base or capture the flag
  // style play.  this code should be merged with restartPlaying() in
  // playing.c++ and moved elsewhere (maybe World).
  float startPoint[3];
  startPoint[2] = 0.0f;
  float tankRadius = BZDB->eval(StateDatabase::BZDB_TANKRADIUS);
  float worldSize = BZDB->eval(StateDatabase::BZDB_WORLDSIZE);
  do {
    startPoint[0] = (worldSize - 2.0f * tankRadius) * ((float)bzfrand() - 0.5f);
    startPoint[1] = (worldSize - 2.0f * tankRadius) * ((float)bzfrand() - 0.5f);
  } while (World::getWorld()->inBuilding(startPoint, 2.0f * tankRadius));

  // NOTE -- code taken directly from LocalPlayer
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

  // no target
  path.clear();
  pathAzimuth.clear();
  target = NULL;
  pathIndex = 0;

  // initialize position/speed state
  static const float zero[3] = { 0.0f, 0.0f, 0.0f };
  move(startPoint, 2.0f * M_PI * (float)bzfrand());
  setVelocity(zero);
  setAngularVelocity(0.0f);
  doUpdateMotion(0.0f);

  // make me alive now
  setStatus(getStatus() | short(PlayerState::Alive));
}

float			RobotPlayer::getTargetPriority(const
							Player* _target) const
{
  // don't target teammates or myself
  if (!this->validTeamTarget(_target))
    return 0.0f;

  // go after closest player
  // FIXME -- this is a pretty stupid heuristic
  const float worldSize = BZDB->eval(StateDatabase::BZDB_WORLDSIZE);
  const float* p1 = getPosition();
  const float* p2 = _target->getPosition();

  float basePriority = 1.0f;
  // give bonus to non-deadzone targets
  if (obstacleList) {
    const BzfRegion* targetRegion = findRegion (p2);
    if (targetRegion && targetRegion->isInside(p2))
      basePriority += 1.0f;
  }
  return basePriority
    - 0.5f * hypotf(p2[0] - p1[0], p2[1] - p1[1]) / worldSize;
}

void                    RobotPlayer::setObstacleList(std::vector<BzfRegion*>*
						     _obstacleList)
{
  obstacleList = _obstacleList;
};

const Player*		RobotPlayer::getTarget() const
{
  return target;
}

void			RobotPlayer::setTarget(const Player* _target)
{
  static int mailbox = 0;

  path.clear();
  pathAzimuth.clear();
  target = _target;
  if (!target) return;

  // work backwards (from target to me)
  const float* p1 = target->getPosition();
  const float* p2 = getPosition();
  BzfRegion* headRegion = findRegion(p1);
  BzfRegion* tailRegion = findRegion(p2);
  if (!headRegion || !tailRegion) {
    // if can't reach target then forget it
    return;
  }

  mailbox++;
  headRegion->setPathStuff(0.0f, NULL, p1, mailbox);
  RegionPriorityQueue queue;
  queue.insert(headRegion, 0.0f);
  BzfRegion* next;
  while (!queue.isEmpty() && (next = queue.remove()) != tailRegion)
    findPath(queue, next, tailRegion, p2, mailbox);

  // get list of points to go through to reach the target
  next = tailRegion;
  do {
    p1 = next->getA();
    float segmentAzimuth = atan2f(p1[1] - p2[1], p1[0] - p2[0]);
    if (segmentAzimuth < 0.0f) segmentAzimuth += 2.0f * M_PI;
    p2 = p1;
    pathAzimuth.push_back(segmentAzimuth);
    path.push_back(p1);
    next = next->getTarget();
  } while (next && next != headRegion);
  pathIndex = 0;
}

BzfRegion*		RobotPlayer::findRegion(const float p[2]) const
{
  const int count = obstacleList->size();
  for (int i = 0; i < count; i++)
    if ((*obstacleList)[i]->isInside(p))
      return (*obstacleList)[i];
  return NULL;
}

float			RobotPlayer::getRegionExitPoint(
				const float p1[2], const float p2[2],
				const float a[2], const float targetPoint[2],
				float mid[2], float& priority)
{
  float b[2];
  b[0] = targetPoint[0] - a[0];
  b[1] = targetPoint[1] - a[1];
  float d[2];
  d[0] = p2[0] - p1[0];
  d[1] = p2[1] - p1[1];

  // safe value in mid
  mid[0]   = p1[0];
  mid[1]   = p1[1];
  priority = 1.0e6;

  if (!d[0] && !d[1])
    return 1.0e6;
    
  // restrict exit zone avoiding tankRadius each side. 
  // there are possibly obstacles at both end points
  const float tankRadius = BZDB->eval(StateDatabase::BZDB_TANKRADIUS);
  const float minT = tankRadius / hypotf(d[0], d[1]);
  const float maxT = 1.0f - minT;

  if (minT > maxT)
    return 1.0e6;

  // compute intersection along (p1,d) with (a,b)
  float t = (a[0] * b[1] - a[1] * b[0] - p1[0] * b[1] + p1[1] * b[0]) /
		(d[0] * b[1] - d[1] * b[0]);
  if (t > maxT) t = maxT;
  else if (t < minT) t = minT;
  mid[0] = p1[0] + t * d[0];
  mid[1] = p1[1] + t * d[1];

  const float distance = hypotf(a[0] - mid[0], a[1] - mid[1]);
  // priority is to minimize distance traveled and distance left to go
  priority = distance + hypotf(targetPoint[0] - mid[0], targetPoint[1] - mid[1]);
  // return distance traveled
  return distance;
}

void			RobotPlayer::findPath(RegionPriorityQueue& queue,
					BzfRegion* region,
					BzfRegion* targetRegion,
					const float targetPoint[2],
					int mailbox)
{
  const int numEdges = region->getNumSides();
  for (int i = 0; i < numEdges; i++) {
    BzfRegion* neighbor = region->getNeighbor(i);
    if (!neighbor) continue;

    const float* p1 = region->getCorner(i).get();
    const float* p2 = region->getCorner((i+1)%numEdges).get();
    float mid[2], priority;
    float total = getRegionExitPoint(p1, p2, region->getA(),
					targetPoint, mid, priority);
    priority += region->getDistance();
    if (neighbor == targetRegion)
      total += hypotf(targetPoint[0] - mid[0], targetPoint[1] - mid[1]);
    total += region->getDistance();
    if (neighbor->test(mailbox) || total < neighbor->getDistance()) {
      neighbor->setPathStuff(total, region, mid, mailbox);
      queue.insert(neighbor, priority);
    }
  }
}

// ex: shiftwidth=2 tabstop=8
