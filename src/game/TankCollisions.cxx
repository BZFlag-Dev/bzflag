/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "TankCollisions.h"

bool			TankCollisions::test(const ShotCollider& tank, const ShotPath* shotPath) const
{
  SegmentedShotStrategy* strategy = (SegmentedShotStrategy*) shotPath->getStrategy();

  float minTime = Infinity;
  // expired shot can't hit anything
  if (shotPath->isExpired()) return minTime;

  // get tank radius
  const float radius2 = tank.radius * tank.radius;

  // tank is positioned from it's bottom so shift position up by
  // half a tank height.
  const float tankHeight = tank.size[2];
  float lastTankPositionRaw[3];
  lastTankPositionRaw[0] = tank.motion.getOrigin()[0];
  lastTankPositionRaw[1] = tank.motion.getOrigin()[1];
  lastTankPositionRaw[2] = tank.motion.getOrigin()[2] + 0.5f * tankHeight;
  Ray tankLastMotion(lastTankPositionRaw, tank.motion.getDirection());

  // if bounding box of tank and entire shot doesn't overlap then no hit
  const float (*tankBBox)[3] = tank.bbox;
  if (!strategy->isOverlapping((*strategy).bbox, tankBBox)) return minTime;

  float shotRadius = BZDB.eval(StateDatabase::BZDB_SHOTRADIUS);

  // check each segment in interval (prevTime,currentTime]
  const float dt = float(TimeKeeper::getCurrent().getSeconds() - strategy->getPreviousTime());
  const int numSegments = (const int)(*strategy).segments.size();
  for (int i = (*strategy).lastSegment; i <= (*strategy).segment && i < numSegments; i++)
  {
    // can never hit your own first laser segment
    if (i == 0 && shotPath->getShotType() == LaserShot && tank.testLastSegment)
      continue;

/*
    // skip segments that don't overlap in time with current interval
    if (segments[i].end <= prevTime) continue;
    if (currentTime <= segments[i].start) break;
*/

    // if shot segment and tank bboxes don't overlap then no hit, or if it's a shot that is out of the world boundry
    const ShotPathSegment& s = (*strategy).segments[i];
    if (!strategy->isOverlapping(s.bbox, tankBBox) || s.reason == ShotPathSegment::Boundary) continue;

    // construct relative shot ray:  origin and velocity relative to
    // my tank as a function of time (t=0 is start of the interval).
    Ray relativeRay(rayMinusRay(s.ray, float((*strategy).prevTime - s.start), tankLastMotion, 0.0f));

    // get hit time
    float t;
    if (tank.test2D)
    {
      // find closest approach to narrow box around tank.  width of box
      // is shell radius so you can actually hit narrow tank head on.
      static float tankBase[3] = { 0.0f, 0.0f, -0.5f * tankHeight };
      t = timeRayHitsBlock(relativeRay, tankBase, tank.angle,
			0.5f * tank.length, shotRadius, tankHeight);
    }
    else {
      // find time when shot hits sphere around tank
      t = rayAtDistanceFromOrigin(relativeRay, 0.99f * tank.radius);
    }

    // short circuit if time is greater then smallest time so far
    if (t > minTime) continue;

    // make sure time falls within segment
    if (t < 0.0f || t > dt) continue;
    if (t > s.end - ((*strategy).prevTime)) continue;

    // check if shot hits tank -- get position at time t, see if in radius
    float closestPos[3];
    relativeRay.getPoint(t, closestPos);
    if (closestPos[0] * closestPos[0] +
	closestPos[1] * closestPos[1] +
	closestPos[2] * closestPos[2] < radius2) {
      // save best time so far
      minTime = t;

      // compute location of tank at time of hit
      float tankPos[3];
      tank.motion.getPoint(t, tankPos);

      // compute position of intersection
      //position[0] = tankPos[0] + closestPos[0];
      //position[1] = tankPos[1] + closestPos[1];
      //position[2] = tankPos[2] + closestPos[2];
      //printf("%u:%u %u:%u\n", tank->getId().port, tank->getId().number, getPath().getPlayer().port, getPath().getPlayer().number);
    }
  }
  return minTime;
}
