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

#include "Pack.h"
#include "WallObstacle.h"
#include "Intersect.h"

const char*		WallObstacle::typeName = "WallObstacle";

WallObstacle::WallObstacle()
{
  iType = wallType;
}

WallObstacle::WallObstacle(const float* p, float a, float b, float h) :
				Obstacle(p, a, 0.0, b, h)
{
  iType = wallType;
  finalize();
}

void WallObstacle::finalize()
{
  // compute normal
  const float* p = getPosition();
  const float a = getRotation();
  plane[0] = cosf(a);
  plane[1] = sinf(a);
  plane[2] = 0.0;
  plane[3] = -(p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2]);

  return;
}

WallObstacle::~WallObstacle()
{
  // do nothing
}

const char*		WallObstacle::getType() const
{
  return typeName;
}

const char*		WallObstacle::getClassName() // const
{
  return typeName;
}

float			WallObstacle::intersect(const Ray& r) const
{
  const float* o = r.getOrigin();
  const float* d = r.getDirection();
  const float dot = -(d[0] * plane[0] + d[1] * plane[1] + d[2] * plane[2]);
  if (dot == 0.0f) return -1.0f;
  float t = (o[0] * plane[0] + o[1] * plane[1] + o[2] * plane[2] +
							plane[3]) / dot;
  return t;
}

void			WallObstacle::getNormal(const float*, float* n) const
{
  n[0] = plane[0];
  n[1] = plane[1];
  n[2] = plane[2];
}

bool			WallObstacle::inCylinder(const float* p, float r, float /* height */) const
{
  return p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2] + plane[3] < r;
}

bool			WallObstacle::inBox(const float* p, float _angle,
					    float halfWidth, float halfBreadth,
					    float /* height */) const
{
  const float xWidth = cosf(_angle);
  const float yWidth = sinf(_angle);
  const float xBreadth = -yWidth;
  const float yBreadth = xWidth;
  float corner[3];
  corner[2] = p[2];

  // check to see if any corner is inside negative half-space
  corner[0] = p[0] - xWidth * halfWidth - xBreadth * halfBreadth;
  corner[1] = p[1] - yWidth * halfWidth - yBreadth * halfBreadth;
  if (inCylinder(corner, 0.0f, 0.0f)) return true;
  corner[0] = p[0] + xWidth * halfWidth - xBreadth * halfBreadth;
  corner[1] = p[1] + yWidth * halfWidth - yBreadth * halfBreadth;
  if (inCylinder(corner, 0.0f, 0.0f)) return true;
  corner[0] = p[0] - xWidth * halfWidth + xBreadth * halfBreadth;
  corner[1] = p[1] - yWidth * halfWidth + yBreadth * halfBreadth;
  if (inCylinder(corner, 0.0f, 0.0f)) return true;
  corner[0] = p[0] + xWidth * halfWidth + xBreadth * halfBreadth;
  corner[1] = p[1] + yWidth * halfWidth + yBreadth * halfBreadth;
  if (inCylinder(corner, 0.0f, 0.0f)) return true;

  return false;
}

bool			WallObstacle::inMovingBox(const float* /* oldP */, float /* oldAngle */,
				       const float* p, float _angle,
				       float halfWidth, float halfBreadth, float height) const

{
  return inBox (p, _angle, halfWidth, halfBreadth, height);
}

bool			WallObstacle::getHitNormal(
				const float*, float,
				const float*, float,
				float, float, float,
				float* normal) const
{
  getNormal(NULL, normal);
  return true;
}


void* WallObstacle::pack(void* buf) const
{
  buf = nboPackVector(buf, pos);
  buf = nboPackFloat(buf, angle);
  buf = nboPackFloat(buf, size[1]);
  buf = nboPackFloat(buf, size[2]);

  return buf;
}


void* WallObstacle::unpack(void* buf)
{
  buf = nboUnpackVector(buf, pos);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackFloat(buf, size[1]);
  buf = nboUnpackFloat(buf, size[2]);

  finalize();

  return buf;
}


int WallObstacle::packSize() const
{
  int fullSize = 0;
  fullSize += sizeof(float[3]); // pos
  fullSize += sizeof(float);    // rotation
  fullSize += sizeof(float);	// breadth
  fullSize += sizeof(float);	// height
  return fullSize;
}


void WallObstacle::print(std::ostream& /*out*/,
			 const std::string& /*indent*/) const
{
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
