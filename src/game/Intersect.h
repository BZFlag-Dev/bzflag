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

/*
 * Utilities for doing intersection calculations.
 */

#ifndef BZF_INTERSECT_H
#define BZF_INTERSECT_H

#include "Ray.h"
#include "common.h"
#include "vectors.h"
#include "game/Frustum.h"

class Extents;


namespace Intersect {

  enum IntersectLevel {
    Outside,
    Partial,
    Contained
  };

  // returns normal to 2d rect (size 2dx x 2dy) by point p
  void getNormalRect(const fvec3& p,
                     const fvec3& boxPos, float boxAngle,
                     float dx, float dy, fvec3& n);

  // true iff 2d rect (size 2dx x 2dy) intersects circle (in z = const plane)
  bool testRectCircle(const fvec3& boxPos, float boxAngle, float dx, float dy,
                      const fvec3& circPos, float circRadius);

  // ray r1 started at time t1 minus ray r2 started at time t2
  Ray rayMinusRay(const Ray& r1, float t1, const Ray& r2, float t2);

  // return t at which ray passes through sphere at origin of given radius
  float rayAtDistanceFromOrigin(const Ray& r, float radius);

  // return t at which ray intersects origin box (size 2dx x 2dy x dz)
  // (-1 if never, 0 if starts inside).
  float timeRayHitsOrigBox(const Ray& r, float dx, float dy, float dz);
  float timeRayHitsOrigBox(const fvec3& rayOrig, const fvec3& rayVel,
                           float dx, float dy, float dz);
  // return t at which ray intersects box (size 2dx x 2dy x dz)
  // (-1 if never, 0 if starts inside).
  float timeRayHitsBlock(const Ray& r, const fvec3& boxPos, float boxAngle,
                         float dx, float dy, float dz);

  // return t at which ray intersects pyramid (size 2dx x 2dy x dz)
  // (-1 if never, 0 if starts inside).
  float timeRayHitsPyramids(const Ray& r, const fvec3& pyrPos, float pyrAngle,
                            float dx, float dy, float dz, bool flipZ);

  // true if rectangles intersect (in z = const plane)
  bool testRectRect(const fvec3& p1, float angle1, float dx1, float dy1,
                    const fvec3& p2, float angle2, float dx2, float dy2);

  // true if first rectangle contains second intersect (in z = const plane)
  bool testRectInRect(const fvec3& bigPos,   float angle1, float dx1, float dy1,
                      const fvec3& smallPos, float angle2, float dx2, float dy2);

  // return t at which ray intersects 2d rect (size 2dx x 2dy) and side
  // of intersection.  0,1,2,3 for east, north, west, south;  -1 if never;
  // -2 if starts inside.
  float timeAndSideRayHitsOrigRect(const fvec3& rayOrigin, const fvec3& rayDir,
                                   float dx, float dy, int& side);
  float timeAndSideRayHitsRect(const Ray& r, const fvec3& boxPos, float boxAngle,
                               float dx, float dy, int& side);

  // return true if polygon touches the axis aligned box
  bool testPolygonInAxisBox(int pointCount, const fvec3* points,
                            const fvec4& plane, const Extents& extents);

  // return level of axis box intersection with Frumstum
  // possible values are Outside, Partial, and Contained.
  // the frustum plane normals point inwards
  IntersectLevel testAxisBoxInFrustum(const Extents& extents,
                                      const Frustum* frustum);

  // return true if the axis aligned bounding box
  // is contained within all of the planes.
  // the occluder plane normals point inwards
  IntersectLevel testAxisBoxOcclusion(const Extents& extents,
                                      const fvec4* planes, int planeCount);

  // return true if the ray will intersect with the
  // axis aligned bounding box defined by the mins
  // and maxs. it will also fill in enterTime and
  // leaveTime if there is an intersection.
  bool textRayInAxisBox(const Ray& ray, const Extents& extents,
                        float& enterTime, float& leaveTime);


  // return true if the ray hits the box
  // if it does hit, set the inTime value
  bool testRayHitsAxisBox(const Ray* ray, const Extents& extents,
                          float* inTime);

  // return true if the ray hits the box
  // if it does hit, set the inTime and outTime values
  bool testRayHitsAxisBox(const Ray* ray, const Extents& extents,
                          float* inTime, float* outTime);
}

#endif // BZF_INTERSECT_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
