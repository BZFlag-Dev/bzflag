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

/*
 *
 */

//#define SHOOTING_FIX

#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "RobotPlayer.h"
//#include "World.h"
#include "ServerLink.h"
#include "BZDBCache.h"

std::vector<BzfRegion*>* RobotPlayer::obstacleList = NULL;

RobotPlayer::RobotPlayer(const PlayerId& _id, const char* _name,
				ServerLink* _server,
				const char* _email = "anonymous") :
				LocalPlayer(_id, _name, _email),
				target(NULL),
				pathIndex(0),
				timeSinceShot(0.0f),
				timerForShot(0.0f),
				drivingForward(true)
{
  gettingSound = false;
  server       = _server;
}

void			RobotPlayer::doUpdate(float dt)
{
  LocalPlayer::doUpdate(dt);

  float tankRadius = BZDBCache::tankRadius;
  float shotRange  = BZDB.eval(StateDatabase::BZDB_SHOTRANGE);

  timerForShot  -= dt;
  if (timerForShot < 0.0f)
    timerForShot = 0.0f;
  const float azimuth = getAngle();
  // Allow shooting only if angle is near and timer has elapsed
  if ((int)path.size() != 0 && timerForShot <= 0.0f) {
    const float* p1     = target->getPosition();
    const float* p2     = getPosition();
    float shootingRange = hypotf(p1[0] - p2[0], p1[1] - p2[1]);
    float shootingAngle = atan2f(p1[1] - p2[1], p1[0] - p2[0]);
    if (shootingAngle < 0.0f)
      shootingAngle += 2.0f * M_PI;
    float azimuthDiff   = shootingAngle - azimuth;
    if (azimuthDiff > M_PI)
      azimuthDiff -= 2.0f * M_PI;
    else
      if (azimuthDiff < -M_PI)
	azimuthDiff += 2.0f * M_PI;
    if ((shootingRange < tankRadius)
	|| (shootingRange <= shotRange
	    && fabs(azimuthDiff) < tankRadius / shootingRange))
      if (fireShot())
	// separate shot by 0.4 sec (experimental value)
	timerForShot = 0.4f;
  }
}

void			RobotPlayer::doUpdateMotion(float dt)
{
  if (isAlive()) {
    // record previous position
    const float oldAzimuth = getAngle();
    const float* oldPosition = getPosition();
    float position[3];
    position[0] = oldPosition[0];
    position[1] = oldPosition[1];
    position[2] = oldPosition[2];
    float azimuth = oldAzimuth;
    float tankAngVel = BZDB.eval(StateDatabase::BZDB_TANKANGVEL);
    float tankSpeed = BZDB.eval(StateDatabase::BZDB_TANKSPEED);
    if (dt > 0.0 && pathIndex < (int)path.size()) {
      float distance;
      float v[2];
      const float* endPoint = path[pathIndex].get();
      // find how long it will take to get to next path segment
      v[0] = endPoint[0] - position[0];
      v[1] = endPoint[1] - position[1];
      distance = hypotf(v[0], v[1]);
      float tankRadius = BZDBCache::tankRadius;
      // smooth path a little by turning early at corners, might get us stuck, though
      if (distance <= 2.5f * tankRadius)
	pathIndex++;

      float segmentAzimuth = atan2f(v[1], v[0]);
      float azimuthDiff = segmentAzimuth - azimuth;
      if (azimuthDiff > M_PI) azimuthDiff -= 2.0f * M_PI;
      else if (azimuthDiff < -M_PI) azimuthDiff += 2.0f * M_PI;
      if (fabs(azimuthDiff) > 0.01f) {
	// drive backward when target is behind, try to stick to last direction
	if (drivingForward)
	  drivingForward = fabs(azimuthDiff) < M_PI/2*0.9 ? true : false;
	else
	  drivingForward = fabs(azimuthDiff) < M_PI/2*0.3 ? true : false;
	setDesiredSpeed(drivingForward ? 1.0f : -1.0f);
	// set desired turn speed
	if (azimuthDiff >= dt * tankAngVel) {
	  setDesiredAngVel(1.0f);
	} else if (azimuthDiff <= -dt * tankAngVel) {
	  setDesiredAngVel(-1.0f);
	} else {
	  setDesiredAngVel(azimuthDiff / dt / tankAngVel);
	}
      } else {
	drivingForward = true;
	// tank doesn't turn while moving forward
	setDesiredAngVel(0.0f);
	// find how long it will take to get to next path segment
	if (distance <= dt * tankSpeed) {
	  pathIndex++;
	  // set desired speed
	  setDesiredSpeed(distance / dt / tankSpeed);
	} else {
	  setDesiredSpeed(1.0f);
	}
      }
    }
  }
  LocalPlayer::doUpdateMotion(dt);
}

void			RobotPlayer::explodeTank()
{
  LocalPlayer::explodeTank();
  target = NULL;
  path.clear();
}

void			RobotPlayer::restart(const float* pos, float _azimuth)
{
  LocalPlayer::restart(pos, _azimuth);
  // no target
  path.clear();
  target = NULL;
  pathIndex = 0;

}

float			RobotPlayer::getTargetPriority(const
							Player* _target) const
{
  // don't target teammates or myself
  if (!this->validTeamTarget(_target))
    return 0.0f;

  // go after closest player
  // FIXME -- this is a pretty stupid heuristic
  const float worldSize = BZDB.eval(StateDatabase::BZDB_WORLDSIZE);
  const float* p1 = getPosition();
  const float* p2 = _target->getPosition();

  float basePriority = 1.0f;
  // give bonus to non-paused player
  if (!_target->isPaused())
    basePriority += 2.0f;
  // give bonus to non-deadzone targets
  if (obstacleList) {
    float nearest[2];
    const BzfRegion* targetRegion = findRegion (p2, nearest);
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
  target = _target;
  if (!target) return;

  // work backwards (from target to me)
  const float* p1 = target->getPosition();
  const float* p2 = getPosition();
  float q1[2], q2[2];
  BzfRegion* headRegion = findRegion(p1, q1);
  BzfRegion* tailRegion = findRegion(p2, q2);
  if (!headRegion || !tailRegion) {
    // if can't reach target then forget it
    return;
  }

  mailbox++;
  headRegion->setPathStuff(0.0f, NULL, q1, mailbox);
  RegionPriorityQueue queue;
  queue.insert(headRegion, 0.0f);
  BzfRegion* next;
  while (!queue.isEmpty() && (next = queue.remove()) != tailRegion)
    findPath(queue, next, tailRegion, q2, mailbox);

  // get list of points to go through to reach the target
  next = tailRegion;
  do {
    p1 = next->getA();
    path.push_back(p1);
    next = next->getTarget();
  } while (next && next != headRegion);
  if (next || tailRegion == headRegion)
    path.push_back(q1);
  else
    path.clear();
  pathIndex = 0;
}

BzfRegion*		RobotPlayer::findRegion(const float p[2],
						float nearest[2]) const
{
  nearest[0] = p[0];
  nearest[1] = p[1];
  const int count = obstacleList->size();
  for (int o = 0; o < count; o++)
    if ((*obstacleList)[o]->isInside(p))
      return (*obstacleList)[o];

  // point is outside: find nearest region
  float      distance      = maxDistance;
  BzfRegion* nearestRegion = NULL;
  for (int i = 0; i < count; i++) {
    float currNearest[2];
    float currDistance = (*obstacleList)[i]->getDistance(p, currNearest);
    if (currDistance < distance) {
      nearestRegion = (*obstacleList)[i];
      distance = currDistance;
      nearest[0] = currNearest[0];
      nearest[1] = currNearest[1];
    }
  }
  return nearestRegion;
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

  float vect = d[0] * b[1] - d[1] * b[0];
  float t    = 0.0f;  // safe value
  if (fabs(vect) > ZERO_TOLERANCE) {
    // compute intersection along (p1,d) with (a,b)
    t = (a[0] * b[1] - a[1] * b[0] - p1[0] * b[1] + p1[1] * b[0]) / vect;
    if (t > 1.0f)
      t = 1.0f;
    else if (t < 0.0f)
      t = 0.0f;
  }

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


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

