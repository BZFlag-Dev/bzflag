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

#include <math.h>
#include "Intersect.h"

boolean			testCircleCircle(const float* o1, float r1,
					const float* o2, float r2)
{
  const float x = o1[0] - o2[0];
  const float y = o1[1] - o2[1];
  const float r = r1 + r2;
  return (x * x + y * y <= r * r);
}

boolean			testSphereSphere(const float* o1, float r1,
					const float* o2, float r2)
{
  const float x = o1[0] - o2[0];
  const float y = o1[1] - o2[1];
  const float z = o1[2] - o2[2];
  const float r = r1 + r2;
  return (x * x + y * y + z * z <= r * r);
}

// get angle of normal vector to axis aligned rect centered at origin by point p
static float		getNormalOrigRect(const float* p, float dx, float dy)
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
      return M_PI;

  if (p[1] > dy)					// north of box
    return 0.5f * M_PI;

  if (p[1] < -dy)					// south of box
    return 1.5f * M_PI;

  // inside box
  if (p[0] > 0.0f)					// inside east
    if (p[1] > 0.0f)					//  inside ne quadrant
      if (dy * p[0] > dx * p[1])			//   east wall
	return 0.0f;
      else						//   north wall
	return 0.5f * M_PI;
    else						//  inside se quadrant
      if (dy * p[0] > -dx * p[1])			//   east wall
	return 0.0f;
      else						//   south wall
	return 1.5f * M_PI;

  else							// inside west
    if (p[1] > 0.0f)					//  inside nw quadrant
      if (dy * p[0] < -dx * p[1])			//   west wall
	return M_PI;
      else						//   north wall
	return 0.5f * M_PI;
    else						//  inside sw quadrant
      if (dy * p[0] < dx * p[1])			//   west wall
	return M_PI;
      else						//   south wall
	return 1.5f * M_PI;
}

void			getNormalRect(const float* p1, const float* p2,
						float angle, float dx,
						float dy, float* n)
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

// True iff axis aligned rect centered at origin intersects circle
boolean			testOrigRectCircle(float dx, float dy,
					const float* p, float r)
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

  return True;						// circle origin in rect
}

boolean			testRectCircle(const float* p1, float angle,
					float dx, float dy,
					const float* p2, float r)
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
Ray			rayMinusRay(const Ray& r1, float t1,
					const Ray& r2, float t2)
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

float			rayClosestToOrigin(const Ray& r)
{
  const float* d = r.getDirection();
  if (d[0] == 0.0 && d[1] == 0.0 && d[2] == 0.0) return 0.0;

  const float* p = r.getOrigin();
  return -(p[0] * d[0] + p[1] * d[1] + p[2] * d[2]) /
		(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
}

float			rayAtDistanceFromOrigin(const Ray& r, float radius)
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
static float		timeRayHitsOrigBox(const float* p, const float* v,
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

float			timeRayHitsBlock(const Ray& r, const float* p1,
					float angle, float dx,
					float dy, float dz)
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

// rect covers interval x=[-dx,dx], y=[-dy,dy]
float			timeAndSideRayHitsOrigRect(
				const float* p, const float* v,
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

float			timeAndSideRayHitsRect(const Ray& r,
					const float* p1, float angle,
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

static boolean		testOrigRectRect(const float* p, float angle,
					float dx1, float dy1,
					float dx2, float dy2)
{
  static const float	box[4][2] =	{ {  1.0,  1.0 }, {  1.0, -1.0 },
					  { -1.0, -1.0 }, { -1.0,  1.0 } };
  const float c = cosf(angle), s = sinf(angle);
  float corner1[4][2];
  int	i, region[4][2];

  // get corners of first rect and classify according to position with
  // respect to second rect, return True iff any lies inside second.
  for (i = 0; i < 4; i++) {
    corner1[i][0] = p[0] + c * dx1 * box[i][0] - s * dy1 * box[i][1];
    corner1[i][1] = p[1] + s * dx1 * box[i][0] + c * dy1 * box[i][1];
    region[i][0] = corner1[i][0] < -dx2 ? -1 : (corner1[i][0] > dx2 ? 1 : 0);
    region[i][1] = corner1[i][1] < -dy2 ? -1 : (corner1[i][1] > dy2 ? 1 : 0);
    if (!region[i][0] && !region[i][1]) return True;
  }

  // check each edge of rect1
  for (i = 0; i < 4; i++) {
    float corner2[2], e[2];
    int j = (i + 1) % 4;

    // if the edge lies completely to one side of rect2 then continue
    // if it crosses the center then return True
    if (region[i][0] == region[j][0])
      if (region[i][0] == 0 && region[i][1] != region[j][1])
	return True;
      else
	continue;
    else if (region[i][1] == region[j][1])
      if (region[i][1] == 0)
	return True;
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
      return True;
  }
  return False;
}

boolean			testRectRect(const float* p1, float angle1,
					float dx1, float dy1,
					const float* p2, float angle2,
					float dx2, float dy2)
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

boolean			testRectInRect(const float* p1, float angle1,
					float dx1, float dy1,
					const float* p2, float angle2,
					float dx2, float dy2)
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
    if (fabsf(x) > dx1 || fabsf(y) > dy1) return False;
  }
  return True;
}
