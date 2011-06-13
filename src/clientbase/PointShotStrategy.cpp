/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "PointShotStrategy.h"

/* system implementation headers */
#include <assert.h>

/* common implementation headers */
#include "game/BZDBCache.h"
#include "game/Intersect.h"
#include "obstacle/WallObstacle.h"
/* local implementation headers */
#include "LocalPlayer.h"
#include "playing.h"


PointShotStrategy::PointShotStrategy(ShotPath* _path, bool useGMtest)
  : ShotStrategy(_path)
  , gmTest(useGMtest) {
}



PointShotStrategy::~PointShotStrategy() {
}


static bool RayTest(const ShotCollider& tank, const Ray& relativeRay,
                    fvec3& hitPos, float& minTime, const ShotPathSegment& s,
                    const float dt, const BzTime& prevTime, const float shotRadius) {
  static const fvec3 tankBase(0.0f, 0.0f, -shotRadius);

  float t = Intersect::timeRayHitsBlock(
              relativeRay, tankBase, tank.angle,
              tank.size.x + shotRadius,
              tank.size.y + shotRadius,
              tank.size.z + (shotRadius * 2.0f)
            );

  if (t > minTime) {
    return false;
  }

  // make sure time falls within segment
  if ((t < 0.0f) || (t > dt)) {
    return false;
  }
  if (t > (s.end - prevTime)) {
    return false;
  }

  // save best time so far
  minTime = t;

  // compute position of intersection
  hitPos = relativeRay.getPoint(t) +
           tank.motion.getPoint(t);
  // compensate for the relativeRay offset
  hitPos.z -= tank.zshift;

  return true;
}


float PointShotStrategy::checkShotHit(const ShotCollider& tank,
                                      fvec3& hitPos, float shotRadius) const {
  float minTime = Infinity;

  // expired shot can't hit anything
  if (getPath().isExpired()) {
    return minTime;
  }

  // shift the tank position to the bottom of its extents
  fvec3 lastTankPositionRaw = tank.motion.getOrigin();
  lastTankPositionRaw.z -= tank.zshift;
  Ray tankLastMotion(lastTankPositionRaw, tank.motion.getDirection());

  const Extents& tankBBox = tank.bbox;

  // if bounding box of tank and entire shot doesn't overlap then no hit
  // we only do this for shots that keep the bbox updated
  if (!gmTest && !bbox.touches(tankBBox)) {
    return minTime;
  }

  // check each segment in interval (prevTime,currentTime]
  const float dt = float(currentTime - prevTime);
  const int numSegments = (int)segments.size();

  int i = 0;
  if (gmTest) {
    // only test most recent segment if shot is from my tank
    if ((numSegments > 1) && tank.testLastSegment) {
      i = numSegments - 1;
    }

    for (; i < numSegments; i++)  {
      const Ray& ray = segments[i].ray;

      Ray relativeRay(Intersect::rayMinusRay(ray, 0.0, tankLastMotion, 0.0));

      if (!RayTest(tank, relativeRay, hitPos, minTime,
                   segments[i], dt, prevTime, shotRadius)) {
        continue;
      }
    }
  }
  else {
    for (i = lastSegment; i <= segment && i < numSegments; i++) {
      // can never hit your own first laser segment
      if ((i == 0) && tank.testLastSegment &&
          (getPath().getShotType() == LaserShot)) {
        continue;
      }

      /*
        // skip segments that don't overlap in time with current interval
        if (segments[i].end <= prevTime) continue;
        if (currentTime <= segments[i].start) break;
      */

      const ShotPathSegment& s = segments[i];

      // if shot segment and tank bboxes don't overlap then no hit,
      // or if it's a shot that is out of the world boundary
      if (!s.bbox.touches(tankBBox) ||
          (s.reason == ShotPathSegment::Boundary)) {
        continue;
      }

      // construct relative shot ray:  origin and velocity relative to
      // my tank as a function of time (t=0 is start of the interval).
      const float diffTime = (float)(prevTime - s.start);
      Ray relativeRay(Intersect::rayMinusRay(s.ray, diffTime,
                                             tankLastMotion, 0.0f));

      if (!RayTest(tank, relativeRay, hitPos, minTime,
                   s, dt, prevTime, shotRadius)) {
        continue;
      }
    }
  }

  return minTime;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
