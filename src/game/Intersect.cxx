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

#include "common.h"
#include <math.h>
#include "Intersect.h"
#include "Extents.h"


// get angle of normal vector to axis aligned rect centered at origin by point p
static float getNormalOrigRect(const float* p, float dx, float dy)
{
  if (p[0] > dx)					// east of box
    if (p[1] > dy)					//  ne corner
      return atan2f(p[1] - dy, p[0] - dx);
    else if (p[1] < -dy)				//  se corner
      return atan2f(p[1] + dy, p[0] - dx);
    else						//  east side
      return 0.0f;

  if (p[0] < -dx)					// west of box
    if (p[1] > dy)					//  nw corner
      return atan2f(p[1] - dy, p[0] + dx);
    else if (p[1] < -dy)				//  sw corner
      return atan2f(p[1] + dy, p[0] + dx);
    else						//  west side
      return (float)M_PI;

  if (p[1] > dy)					// north of box
    return (float)(0.5 * M_PI);

  if (p[1] < -dy)					// south of box
    return (float)(1.5 * M_PI);

  // inside box
  if (p[0] > 0.0f)					// inside east
    if (p[1] > 0.0f)					//  inside ne quadrant
      if (dy * p[0] > dx * p[1])			//   east wall
	return 0.0f;
      else						//   north wall
	return (float)(0.5 * M_PI);
    else						//  inside se quadrant
      if (dy * p[0] > -dx * p[1])			//   east wall
	return 0.0f;
      else						//   south wall
	return (float)(1.5 * M_PI);

  else							// inside west
    if (p[1] > 0.0f)					//  inside nw quadrant
      if (dy * p[0] < -dx * p[1])			//   west wall
	return (float)M_PI;
      else						//   north wall
	return (float)(0.5 * M_PI);
    else						//  inside sw quadrant
      if (dy * p[0] < dx * p[1])			//   west wall
	return (float)M_PI;
      else						//   south wall
	return (float)(1.5 * M_PI);
}


void getNormalRect(const float* p1, const float* p2,
		   float angle, float dx, float dy, float* n)
{
  // translate origin
  float pa[2];
  pa[0] = p1[0] - p2[0];
  pa[1] = p1[1] - p2[1];

  // rotate
  float pb[2];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];

  // get angle
  const float normAngle = getNormalOrigRect(pb, dx, dy) + angle;

  // make normal
  n[0] = cosf(normAngle);
  n[1] = sinf(normAngle);
  n[2] = 0.0f;
}


// true iff axis aligned rect centered at origin intersects circle
bool testOrigRectCircle(float dx, float dy, const float* p, float r)
{
  // Algorithm from Graphics Gems, pp51-53.
  const float rr = r * r, rx = -p[0], ry = -p[1];
  if (rx + dx < 0.0)					// west of rect
    if (ry + dy < 0.0)					//  sw corner
      return (rx + dx) * (rx + dx) + (ry + dy) * (ry + dy) < rr;
    else if (ry - dy > 0.0)				//  nw corner
      return (rx + dx) * (rx + dx) + (ry - dy) * (ry - dy) < rr;
    else						//  due west
      return rx + dx > -r;

  else if (rx - dx > 0.0)				// east of rect
    if (ry + dy < 0.0)					//  se corner
      return (rx - dx) * (rx - dx) + (ry + dy) * (ry + dy) < rr;
    else if (ry - dy > 0.0)				//  ne corner
      return (rx - dx) * (rx - dx) + (ry - dy) * (ry - dy) < rr;
    else						//  due east
      return rx - dx < r;

  else if (ry + dy < 0.0)				// due south
    return ry + dy > -r;

  else if (ry - dy > 0.0)				// due north
    return ry - dy < r;

  return true;						// circle origin in rect
}


bool testRectCircle(const float* p1, float angle,
		    float dx, float dy, const float* p2, float r)
{
  // translate origin
  float pa[2];
  pa[0] = p2[0] - p1[0];
  pa[1] = p2[1] - p1[1];

  // rotate
  float pb[2];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];

  // do test
  return testOrigRectCircle(dx, dy, pb, r);
}


// ray r1 started at time t1 minus ray r2 started at time t2
Ray rayMinusRay(const Ray& r1, float t1, const Ray& r2, float t2)
{
  // get points at respective times
  float p1[3], p2[3];
  r1.getPoint(t1, p1);
  r2.getPoint(t2, p2);

  // construct new ray
  float p[3], d[3];
  p[0] = p1[0] - p2[0];
  p[1] = p1[1] - p2[1];
  p[2] = p1[2] - p2[2];
  d[0] = r1.getDirection()[0] - r2.getDirection()[0];
  d[1] = r1.getDirection()[1] - r2.getDirection()[1];
  d[2] = r1.getDirection()[2] - r2.getDirection()[2];
  return Ray(p, d);
}


float rayAtDistanceFromOrigin(const Ray& r, float radius)
{
  const float* d = r.getDirection();
  if (d[0] == 0.0 && d[1] == 0.0 && d[2] == 0.0) return 0.0f;

  const float* p = r.getOrigin();
  const float a = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
  const float b = -(p[0] * d[0] + p[1] * d[1] + p[2] * d[2]);
  const float c = p[0] * p[0] + p[1] * p[1] + p[2] * p[2] - radius * radius;
  const float disc = b * b - a * c;
  if (disc < 0.0f) return -1.0f;		// misses sphere
  const float d1_2 = sqrtf(disc);
  const float t0 = b + d1_2;
  const float t1 = b - d1_2;
  if (t0 < t1)
    if (t0 < 0.0f) return t1 / a;
    else return t0 / a;
  else
    if (t1 < 0.0) return t0 / a;
    else return t1 / a;
}


// block covers interval x=[-dx,dx], y=[-dy,dy], z=[0.0,dz]
static float timeRayHitsOrigBox(const float* p, const float* v,
				float dx, float dy, float dz)
{
  float tx, ty, tz;

  // there's gotta be a better way to do this whole thing

  if (fabsf(p[0]) <= dx && fabsf(p[1]) <= dy && p[2] >= 0.0 && p[2] <= dz)
    return 0.0;						// inside

  if (p[0] > dx)					// to east
    if (v[0] >= 0.0) return -1.0f;			//  going east
    else tx = (dx - p[0]) / v[0];			//  get east wall hit
  else if (p[0] < -dx)					// to west
    if (v[0] <= 0.0) return -1.0f;			//  going west
    else tx = -(dx + p[0]) / v[0];			//  get west wall hit
  else tx = -1.0f;					// doesn't matter

  if (p[1] > dy)					// to north
    if (v[1] >= 0.0) return -1.0f;			//  going north
    else ty = (dy - p[1]) / v[1];			//  get north wall hit
  else if (p[1] < -dy)					// to south
    if (v[1] <= 0.0) return -1.0f;			//  going south
    else ty = -(dy + p[1]) / v[1];			//  get north wall hit
  else ty = -1.0f;					// doesn't matter

  if (p[2] > dz)					// above
    if (v[2] >= 0.0) return -1.0f;			//  going up
    else tz = (dz - p[2]) / v[2];			//  get ceiling hit
  else if (p[2] < 0.0)					// below
    if (v[2] <= 0.0) return -1.0f;			//  going down
    else tz = -p[2] / v[2];				//  get floor hit
  else tz = -1.0f;					// doesn't matter

  // throw out solutions < 0.0 or that intersect outside box
  if (tx < 0.0 ||
	fabsf(p[1] + tx * v[1]) > dy ||
	p[2] + tx * v[2] < 0.0 ||
	p[2] + tx * v[2] > dz)
    tx = -1.0f;
  if (ty < 0.0 ||
	fabsf(p[0] + ty * v[0]) > dx ||
	p[2] + ty * v[2] < 0.0 ||
	p[2] + ty * v[2] > dz)
    ty = -1.0f;
  if (tz < 0.0 ||
	fabsf(p[0] + tz * v[0]) > dx ||
	fabsf(p[1] + tz * v[1]) > dy)
    tz = -1.0f;

  if (tx < 0.0 && ty < 0.0 && tz < 0.0) return -1.0f;	// no hits

  // pick closest valid solution
  if (tx < 0.0f) {
    if (ty < 0.0f) return tz;
    if (tz < 0.0f || ty < tz) return ty;
    return tz;
  }
  else if (ty < 0.0f) {
    if (tz < 0.0 || tx < tz) return tx;
    return tz;
  }
  else if (tz < 0.0f) {
    if (tx < ty) return tx;
    return ty;
  }
  else {
    if (tx < ty && tx < tz) return tx;
    if (ty < tz) return ty;
    return tz;
  }
}


float timeRayHitsBlock(const Ray& r, const float* p1,
		       float angle, float dx, float dy, float dz)
{
  // get names for ray info
  const float* p2 = r.getOrigin();
  const float* d = r.getDirection();

  // translate origin
  float pa[2];
  pa[0] = p2[0] - p1[0];
  pa[1] = p2[1] - p1[1];

  // rotate
  float pb[3], db[3];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];
  pb[2] = p2[2] - p1[2];
  db[0] = c * d[0] - s * d[1];
  db[1] = c * d[1] + s * d[0];
  db[2] = d[2];

  // find t
  return timeRayHitsOrigBox(pb, db, dx, dy, dz);
}


/** Computing ray travel time to the plane described by 3 points
 */
static float timeRayHitsPlane(const float pb[3], const float db[3],
			      const float x1[3], const float x2[3],
			      const float x3[3])
{
  float u[3], v[3], d[3];
  int i;

  // Compute the 2 vectors describing the plane
  for (i = 0; i < 3; i++) {
    u[i] = x2[i] - x1[i];
  }
  for (i = 0; i < 3; i++) {
    v[i] = x3[i] - x1[i];
  }
  // Thats a distance vector: a vector from the plane to the ray beginning
  for (i = 0; i < 3; i++) {
    d[i] = pb[i] - x1[i];
  }

  // plane versor unnormalized
  float n[3];
  n[0] = u[1] * v[2] - u[2] * v[1];
  n[1] = u[2] * v[0] - u[0] * v[2];
  n[2] = u[0] * v[1] - u[1] * v[0];

  // computing unnormalized distance projecting the distance on versor
  float distance = 0.0;
  for (i = 0; i < 3; i++) {
    distance += n[i] * d[i];
  }

  // if distance is negative, plane is already passed
  if (distance <= 0.0f)
    return 0.0f;

  // project velocity vector on the plan versor unnormalized
  float velocity = 0.0f;
  for (i = 0; i < 3; i++) {
    velocity += n[i] * db[i];
  }

  // if velocity is greater than 0 no way to trespass the plane
  if (velocity > 0.0f)
    return -1.0f;

  // time is ... that is normalized
  return - distance / velocity;
}


float timeRayHitsPyramids(const Ray& r, const float* p1, float angle,
			  float dx, float dy, float dz, bool flipZ)
{

  const float epsilon = 1.0e-3f;
  // get names for ray info
  int i;
  const float* p2 = r.getOrigin();
  const float* d  = r.getDirection();

  // translate origin
  float pa[2];
  pa[0] = p2[0] - p1[0];
  pa[1] = p2[1] - p1[1];

  // rotate
  float pb[3], db[3];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];
  pb[2] = p2[2] - p1[2];
  db[0] = c * d[0] - s * d[1];
  db[1] = c * d[1] + s * d[0];
  db[2] = d[2];

  if (dx < 0)
    dx = - dx;
  if (dy < 0)
    dy = - dy;
  if (dz < 0) {
    dz = - dz;
  }
  if (flipZ) {
    pb[2] = dz - pb[2];
    db[2] = - db[2];
  }

  float residualTime = 0.0f;

  float x1[3], x2[3], x3[3];
  float residualTemp;

  x1[2] = 0.0f;
  x2[2] = 0.0f;
  x3[0] = 0.0f;
  x3[1] = 0.0f;
  x3[2] = dz;

  // trying to get to the pyramid removing half space at time
  // start with the 4 faces, and end with the base
  x1[0] = - dx;
  x1[1] = - dy;
  x2[0] =   dx;
  x2[1] = - dy;
  residualTemp = timeRayHitsPlane(pb, db, x1, x2, x3);
  if (residualTemp < -0.5f)
    return residualTemp;
  for (i = 0; i < 3; i++) {
    pb[i] += residualTemp * db[i];
  }
  residualTime += residualTemp;

  x1[0] = - x1[0];
  x1[1] = - x1[1];
  residualTemp = timeRayHitsPlane(pb, db, x2, x1, x3);
  if (residualTemp < -0.5f)
    return residualTemp;
  for (i = 0; i < 3; i++) {
    pb[i] += residualTemp * db[i];
  }
  residualTime += residualTemp;

  x2[0] = - x2[0];
  x2[1] = - x2[1];
  residualTemp = timeRayHitsPlane(pb, db, x1, x2, x3);
  if (residualTemp < -0.5f)
    return residualTemp;
  for (i = 0; i < 3; i++) {
    pb[i] += residualTemp * db[i];
  }
  residualTime += residualTemp;

  x1[0] = - x1[0];
  x1[1] = - x1[1];
  residualTemp = timeRayHitsPlane(pb, db, x2, x1, x3);
  if (residualTemp < -0.5f)
    return residualTemp;
  for (i = 0; i < 3; i++) {
    pb[i] += residualTemp * db[i];
  }
  residualTime += residualTemp;

  x3[0] = dx;
  x3[1] = dy;
  x3[2] = 0.0f;
  residualTemp = timeRayHitsPlane(pb, db, x1, x2, x3);
  if (residualTemp < -0.5f)
    return residualTemp;
  for (i = 0; i < 3; i++) {
    pb[i] += residualTemp * db[i];
  }
  residualTime += residualTemp;

  // No way to move further. See if inside
  // first bounding box
  pb[0] = fabsf(pb[0]);
  pb[1] = fabsf(pb[1]);
  if (pb[0] > dx + epsilon * dx)
    return -1.0f;
  if (pb[1] > dy + epsilon * dy)
    return -1.0f;
  if (pb[2] < 0.0f - epsilon * dz)
    return -1.0f;
  // now shrink
  float scaledDistance;
  if (pb[0] * dy > pb[1] * dx)
    scaledDistance = dz * (dx - pb[0]) - pb[2] * dx;
  else
    scaledDistance = dz * (dy - pb[1]) - pb[2] * dy;
  if (scaledDistance < - epsilon * dz)
    residualTime = -1.0f;

  return residualTime;
}


// rect covers interval x=[-dx,dx], y=[-dy,dy]
float timeAndSideRayHitsOrigRect(const float* p, const float* v,
				 float dx, float dy, int& side)
{
  // check if inside
  if (fabsf(p[0]) <= dx && fabsf(p[1]) <= dy) {
    side = -2;
    return 0.0f;
  }

  // assume it won't hit
  side = -1;

  float tx, ty;
  if (p[0] > dx)					// to east
    if (v[0] >= 0.0f) return -1.0f;			//  going east
    else tx = (dx - p[0]) / v[0];			//  get east wall hit
  else if (p[0] < -dx)					// to west
    if (v[0] <= 0.0f) return -1.0f;			//  going west
    else tx = -(dx + p[0]) / v[0];			//  get west wall hit
  else tx = -1.0f;					// doesn't matter

  if (p[1] > dy)					// to north
    if (v[1] >= 0.0f) return -1.0f;			//  going north
    else ty = (dy - p[1]) / v[1];			//  get north wall hit
  else if (p[1] < -dy)					// to south
    if (v[1] <= 0.0f) return -1.0f;			//  going south
    else ty = -(dy + p[1]) / v[1];			//  get north wall hit
  else ty = -1.0f;					// doesn't matter

  // throw out solutions < 0.0 or that intersect outside box
  if (fabsf(p[1] + tx * v[1]) > dy) tx = -1.0f;
  if (fabsf(p[0] + ty * v[0]) > dx) ty = -1.0f;
  if (tx < 0.0f && ty < 0.0f) return -1.0f;		// no hits

  // pick closest valid solution
  if (tx < 0.0f || (ty >= 0.0f && ty < tx)) {
    side = (p[1] > dy) ? 1 : 3;
    return ty;
  }
  side = (p[0] > dx) ? 0 : 2;
  return tx;
}


float timeAndSideRayHitsRect(const Ray& r, const float* p1, float angle,
			     float dx, float dy, int& side)
{
  // get names for ray info
  const float* p2 = r.getOrigin();
  const float* d = r.getDirection();

  // translate origin
  float pa[2];
  pa[0] = p2[0] - p1[0];
  pa[1] = p2[1] - p1[1];

  // rotate
  float pb[3], db[3];
  const float c = cosf(-angle), s = sinf(-angle);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];
  pb[2] = p2[2] - p1[2];
  db[0] = c * d[0] - s * d[1];
  db[1] = c * d[1] + s * d[0];
  db[2] = d[2];

  // find t and side
  return timeAndSideRayHitsOrigRect(pb, db, dx, dy, side);
}


static bool testOrigRectRect(const float* p, float angle,
			     float dx1, float dy1, float dx2, float dy2)
{
  static const float	box[4][2] =	{ {  1.0,  1.0 }, {  1.0, -1.0 },
					  { -1.0, -1.0 }, { -1.0,  1.0 } };
  const float c = cosf(angle), s = sinf(angle);
  float corner1[4][2];
  int	i, region[4][2];

  // return true if the second rectangle is within the first
  // hint:  cos(+a) = cos(-a)  and  -sin(a) = sin(-a)
  float sx = (c * p[0]) + (s * p[1]);
  float sy = (c * p[1]) - (s * p[0]);
  if ((fabsf(sx) < dx1) && (fabsf(sy) < dy1)) {
    return true;
  }

  // get corners of first rect and classify according to position with
  // respect to second rect, return true iff any lies inside second.
  for (i = 0; i < 4; i++) {
    corner1[i][0] = p[0] + c * dx1 * box[i][0] - s * dy1 * box[i][1];
    corner1[i][1] = p[1] + s * dx1 * box[i][0] + c * dy1 * box[i][1];
    region[i][0] = corner1[i][0] < -dx2 ? -1 : (corner1[i][0] > dx2 ? 1 : 0);
    region[i][1] = corner1[i][1] < -dy2 ? -1 : (corner1[i][1] > dy2 ? 1 : 0);
    if (!region[i][0] && !region[i][1]) return true;
  }

  // check each edge of rect1
  for (i = 0; i < 4; i++) {
    float corner2[2], e[2];
    int j = (i + 1) % 4;

    // if the edge lies completely to one side of rect2 then continue
    // if it crosses the center then return true
    if (region[i][0] == region[j][0])
      if (region[i][0] == 0 && region[i][1] != region[j][1])
	return true;
      else
	continue;
    else if (region[i][1] == region[j][1])
      if (region[i][1] == 0)
	return true;
      else
	continue;

    // determine corners of rect2 the edge might pass between
    if (region[i][0] == 0) {
      corner2[0] = region[j][0] * dx2;
      corner2[1] = region[i][1] * dy2;
    }
    else if (region[j][0] == 0) {
      corner2[0] = region[i][0] * dx2;
      corner2[1] = region[j][1] * dy2;
    }
    else if (region[i][1] == 0) {
      corner2[0] = region[i][0] * dx2;
      corner2[1] = region[j][1] * dy2;
    }
    else {
      corner2[0] = region[j][0] * dx2;
      corner2[1] = region[i][1] * dy2;
    }

    // see if edge crosses the rectangle
    e[0] = corner1[j][0] - corner1[i][0];
    e[1] = corner1[j][1] - corner1[i][1];
    if ((e[1]*(corner2[0]-corner1[i][0])-e[0]*(corner2[1]-corner1[i][1])) *
	(e[1]*(corner2[0]+corner1[i][0])-e[0]*(corner2[1]+corner1[i][1])) > 0.0)
      return true;
  }
  return false;
}


bool testRectRect(const float* p1, float angle1, float dx1, float dy1,
		  const float* p2, float angle2, float dx2, float dy2)
{
  // translate origin
  float pa[2];
  pa[0] = p2[0] - p1[0];
  pa[1] = p2[1] - p1[1];

  // rotate
  float pb[2];
  const float c = cosf(-angle1), s = sinf(-angle1);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];

  // do test
  return testOrigRectRect(pb, angle2 - angle1, dx2, dy2, dx1, dy1);
}


bool testRectInRect(const float* p1, float angle1, float dx1, float dy1,
		    const float* p2, float angle2, float dx2, float dy2)
{
  static const float	box[4][2] =	{ {  1.0,  1.0 }, {  1.0, -1.0 },
					  { -1.0, -1.0 }, { -1.0,  1.0 } };

  // translate origin
  float pa[2];
  pa[0] = p2[0] - p1[0];
  pa[1] = p2[1] - p1[1];

  // rotate
  float pb[2];
  const float c = cosf(-angle1), s = sinf(-angle1);
  pb[0] = c * pa[0] - s * pa[1];
  pb[1] = c * pa[1] + s * pa[0];

  // see if corners of second rectangle are inside first
  const float c2 = cosf(angle2 - angle1), s2 = sinf(angle2 - angle1);
  for (int i = 0; i < 4; i++) {
    const float x = pb[0] + c2 * dx2 * box[i][0] - s2 * dy2 * box[i][1];
    const float y = pb[1] + s2 * dx2 * box[i][0] + c2 * dy2 * box[i][1];
    if (fabsf(x) > dx1 || fabsf(y) > dy1) return false;
  }
  return true;
}


static inline void projectAxisBox(const float* dir, const Extents& extents,
				  float* minDist, float* maxDist)
{
  static float i[3];
  static float o[3];

  // find the extreme corners
  for (int t = 0; t < 3; t++) {
    if (dir[t] > 0.0f) {
      i[t] = extents.maxs[t];
      o[t] = extents.mins[t];
    } else {
      i[t] = extents.mins[t];
      o[t] = extents.maxs[t];
    }
  }

  float idist = (i[0] * dir[0]) + (i[1] * dir[1]) + (i[2] * dir[2]);
  float odist = (o[0] * dir[0]) + (o[1] * dir[1]) + (o[2] * dir[2]);

  if (idist < odist) {
    *minDist = idist;
    *maxDist = odist;
  } else {
    *minDist = odist;
    *maxDist = idist;
  }

  return;
}


static inline void projectPolygon(const float* dir,
				  int count, const float (*points)[3],
				  float* minDist, float* maxDist)
{
  float mind = MAXFLOAT;
  float maxd = -MAXFLOAT;

  for (int i = 0; i < count; i++) {
    const float* p = points[i];
    float dist = (p[0] * dir[0]) + (p[1] * dir[1]) + (p[2] * dir[2]);
    if (dist < mind) {
      mind = dist;
    }
    if (dist > maxd) {
      maxd = dist;
    }
  }

  *minDist = mind;
  *maxDist = maxd;

  return;
}


// return true if polygon touches the axis aligned box
// *** assumes that an extents test has already been done ***
bool testPolygonInAxisBox(int pointCount, const float (*points)[3],
			  const float* plane, const Extents& extents)
{
  int t;
  static float i[3]; // inside point  (assuming partial)
  static float o[3]; // outside point (assuming partial)

  // test the plane
  for (t = 0; t < 3; t++) {
    if (plane[t] > 0.0f) {
      i[t] = extents.maxs[t];
      o[t] = extents.mins[t];
    } else {
      i[t] = extents.mins[t];
      o[t] = extents.maxs[t];
    }
  }
  const float icross = (plane[0] * i[0]) +
		       (plane[1] * i[1]) +
		       (plane[2] * i[2]) + plane[3];
  const float ocross = (plane[0] * o[0]) +
		       (plane[1] * o[1]) +
		       (plane[2] * o[2]) + plane[3];
  if ((icross * ocross) > 0.0f) {
    // same polarity means that the plane doesn't cut the box
    return false;
  }

  // test the edges
  const float axisNormals[3][3] =
    {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
  for (t = 0; t < pointCount; t++) {
    int next = (t + 1) % pointCount;
    float edge[3];
    edge[0] = points[next][0] - points[t][0];
    edge[1] = points[next][1] - points[t][1];
    edge[2] = points[next][2] - points[t][2];
    for (int a = 0; a < 3; a++) {
      float cross[3];
      const float* axis = axisNormals[a];
      cross[0] = (edge[1] * axis[2]) - (edge[2] * axis[1]);
      cross[1] = (edge[2] * axis[0]) - (edge[0] * axis[2]);
      cross[2] = (edge[0] * axis[1]) - (edge[1] * axis[0]);
      const float length =
	(cross[0] * cross[0]) + (cross[1] * cross[1]) + (cross[2] * cross[2]);
      if (length < 0.001f) {
	continue;
      }
      // find the projected distances
      float boxMinDist, boxMaxDist;
      float polyMinDist, polyMaxDist;
      projectAxisBox(cross, extents, &boxMinDist, &boxMaxDist);
      projectPolygon(cross, pointCount, points, &polyMinDist, &polyMaxDist);
      // check if this is a separation axis
      if ((boxMinDist > polyMaxDist) || (boxMaxDist < polyMinDist)) {
	return false;
      }
    }
  }

  return true;
}


// return level of axis box intersection with Frumstum
// possible values are Outside, Partial, and Contained.
// the frustum plane normals point inwards
IntersectLevel testAxisBoxInFrustum(const Extents& extents,
				    const Frustum* frustum)
{
  // FIXME - use a sphere vs. cone test first?

  static int s, t;
  static float i[3]; // inside point  (assuming partial)
  static float o[3]; // outside point (assuming partial)
  static float len;
  static const float* p; // the plane
  IntersectLevel result = Contained;

  // FIXME - 0 is the near clip plane, not that useful really?
  //	 OpenGL should easily clip the few items sneak in

  const int planeCount = frustum->getPlaneCount();

  for (s = 1 /* NOTE: not 0 */; s < planeCount; s++) {

    p = frustum->getSide(s);

    // setup the inside/outside corners
    // this can be determined easily based
    // on the normal vector for the plane
    for (t = 0; t < 3; t++) {
      if (p[t] > 0.0f) {
	i[t] = extents.maxs[t];
	o[t] = extents.mins[t];
      } else {
	i[t] = extents.mins[t];
	o[t] = extents.maxs[t];
      }
    }
    // check the inside length
    len = (p[0] * i[0]) + (p[1] * i[1]) + (p[2] * i[2]) + p[3];
    if (len < -1.0f) {
      return Outside; // box is fully outside the frustum
    }

    // check the outside length
    len = (p[0] * o[0]) + (p[1] * o[1]) + (p[2] * o[2]) + p[3];
    if (len < -1.0f) {
      result = Partial; // partial containment at best
    }
  }

  return result;
}


// return true if the axis aligned bounding box
// is contained within all of the planes.
// the occluder plane normals point inwards
IntersectLevel testAxisBoxOcclusion(const Extents& extents,
				    const float (*planes)[4], int planeCount)
{
  static int s, t;
  static float i[3]; // inside point  (assuming partial)
  static float o[3]; // outside point (assuming partial)
  static float len;
  static const float* p; // the plane
  IntersectLevel result = Contained;

  for (s = 0; s < planeCount; s++) {

    p = planes[s];

    // setup the inside/outside corners
    // this can be determined easily based
    // on the normal vector for the plane
    for (t = 0; t < 3; t++) {
      if (p[t] > 0.0f) {
	i[t] = extents.maxs[t];
	o[t] = extents.mins[t];
      } else {
	i[t] = extents.mins[t];
	o[t] = extents.maxs[t];
      }
    }

    // check the inside length
    len = (p[0] * i[0]) + (p[1] * i[1]) + (p[2] * i[2]) + p[3];
    if (len < +0.1f) {
      return Outside; // box is fully outside the occluder
    }

    // FIXME - if we don't do occlusion by SceneNode,
    //	 then ditch the partial test. This will
    //	 save an extra dot product, and reduce
    //	 the likely number of loops

    // check the outside length
    len = (p[0] * o[0]) + (p[1] * o[1]) + (p[2] * o[2]) + p[3];
    if (len < +0.1f) {
      result =  Partial; // partial containment at best
    }
  }

  return result;
}

// return true if the ray hits the box
// if it does hit, set the inTime value
bool testRayHitsAxisBox(const Ray* ray, const Extents& exts,
			float* inTime)
{
  int a;
  const float* const o = ray->getOrigin();
  const float* const v = ray->getDirection();
  const float* extents[2] = { exts.mins, exts.maxs };
  int zone[3];
  bool inside = true;

  // setup the zones
  for (a = 0; a < 3; a++) {
    if (o[a] < exts.mins[a]) {
      if (v[a] <= 0.0f) {
	return false;
      }
      zone[a] = 0;
      inside = false;
    }
    else if (o[a] > exts.maxs[a]) {
      if (v[a] >= 0.0f) {
	return false;
      }
      zone[a] = 1;
      inside = false;
    }
    else {
      zone[a] = -1;
    }
  }

  int hitPlane;
  float hitTime[3];

  if (inside) {
    *inTime = 0.0f;
  }
  else {
    // calculate the hitTimes
    for (a = 0; a < 3; a++) {
      if (zone[a] < 0) {
	hitTime[a] = -1.0f;
      } else {
	hitTime[a] = (extents[zone[a]][a] - o[a]) / v[a];
      }
    }

    // use the largest hitTime
    hitPlane = 0;
    if (hitTime[1] > hitTime[0]) {
      hitPlane = 1;
    }
    if (hitTime[2] > hitTime[hitPlane]) {
      hitPlane = 2;
    }

    // check the hitPlane
    const float useTime = hitTime[hitPlane];
    if (useTime < 0.0f) {
      return false;
    }
    for (a = 0; a < 3; a++) {
      if (a != hitPlane) {
	const float hitDist = o[a] + (useTime * v[a]);
	if ((hitDist < exts.mins[a]) || (hitDist > exts.maxs[a])) {
	  return false;
	}
      }
    }
    *inTime = useTime;
  }

  return true;
}


// return true if the ray hits the box
// if it does hit, set the inTime and outTime values
bool testRayHitsAxisBox(const Ray* ray, const Extents& extents,
			float* inTime, float* outTime)
{
  if (!testRayHitsAxisBox(ray, extents, inTime)) {
    return false;
  }

  int a;
  const float* const o = ray->getOrigin();
  const float* const v = ray->getDirection();

  // calculate the hitTimes for the outTime
  float hitTime[3];
  for (a = 0; a < 3; a++) {
    if (v[a] == 0.0f) {
      hitTime[a] = MAXFLOAT;
    }
    else if (v[a] < 0.0f) {
      hitTime[a] = (extents.mins[a] - o[a]) / v[a];
    }
    else {
      hitTime[a] = (extents.maxs[a] - o[a]) / v[a];
    }
  }

  // use the smallest hitTime
  int hitPlane = 0;
  if (hitTime[1] < hitTime[0]) {
    hitPlane = 1;
  }
  if (hitTime[2] < hitTime[hitPlane]) {
    hitPlane = 2;
  }
  *outTime = hitTime[hitPlane];

  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
