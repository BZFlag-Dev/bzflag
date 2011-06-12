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

#include "common.h"

#include <math.h>

#include "game/global.h"
#include "net/Pack.h"
#include "WallObstacle.h"
#include "game/Intersect.h"

const char*   WallObstacle::typeName = "WallObstacle";

WallObstacle::WallObstacle() {
}

WallObstacle::WallObstacle(const fvec3& p, float a, float b, float h, bool rico)
  : Obstacle(p, a, 0.0, b, h, false, false, rico) {
  finalize();
}

void WallObstacle::finalize() {
  // compute normal
  const fvec3& p = getPosition();
  const float a = getRotation();
  plane.x = cosf(a);
  plane.y = sinf(a);
  plane.z = 0.0;
  plane.w = -fvec3::dot(p, plane.xyz());

  return;
}


WallObstacle::~WallObstacle() {
  // do nothing
}


const char* WallObstacle::getType() const {
  return typeName;
}


const char* WallObstacle::getClassName() { // const
  return typeName;
}


float WallObstacle::intersect(const Ray& r) const {
  const fvec3& o = r.getOrigin();
  const fvec3& d = r.getDirection();
  const float dot = -fvec3::dot(d, plane.xyz());
  if (dot == 0.0f) {
    return -1.0f;
  }
  float t = plane.planeDist(o) / dot;
  return t;
}


void WallObstacle::getNormal(const fvec3&, fvec3& n) const {
  n = plane.xyz();
}


bool WallObstacle::inCylinder(const fvec3& p, float r, float /* height */) const {
  return plane.planeDist(p) < r;
}


bool WallObstacle::inBox(const fvec3& p, float _angle,
                         float halfWidth, float halfBreadth,
                         float /* height */) const {
  const float xWidth = cosf(_angle);
  const float yWidth = sinf(_angle);
  const float xBreadth = -yWidth;
  const float yBreadth = xWidth;
  fvec3 corner;
  corner.z = p.z;

  // check to see if any corner is inside negative half-space
  corner.x = p.x - (xWidth * halfWidth) - (xBreadth * halfBreadth);
  corner.y = p.y - (yWidth * halfWidth) - (yBreadth * halfBreadth);
  if (inCylinder(corner, 0.0f, 0.0f)) { return true; }
  corner.x = p.x + (xWidth * halfWidth) - (xBreadth * halfBreadth);
  corner.y = p.y + (yWidth * halfWidth) - (yBreadth * halfBreadth);
  if (inCylinder(corner, 0.0f, 0.0f)) { return true; }
  corner.x = p.x - (xWidth * halfWidth) + (xBreadth * halfBreadth);
  corner.y = p.y - (yWidth * halfWidth) + (yBreadth * halfBreadth);
  if (inCylinder(corner, 0.0f, 0.0f)) { return true; }
  corner.x = p.x + (xWidth * halfWidth) + (xBreadth * halfBreadth);
  corner.y = p.y + (yWidth * halfWidth) + (yBreadth * halfBreadth);
  if (inCylinder(corner, 0.0f, 0.0f)) { return true; }

  return false;
}


bool WallObstacle::inMovingBox(const fvec3& /* oldP */, float /* oldAngle */,
                               const fvec3& p, float _angle,
                               float halfWidth, float halfBreadth, float height) const

{
  return inBox(p, _angle, halfWidth, halfBreadth, height);
}


bool WallObstacle::getHitNormal(const fvec3&, float,
                                const fvec3&, float,
                                float, float, float,
                                fvec3& normal) const {
  getNormal(fvec3(), normal);
  return true;
}


void* WallObstacle::pack(void* buf) const {
  buf = nboPackFVec3(buf, pos);
  buf = nboPackFloat(buf, angle);
  buf = nboPackFloat(buf, size.y);
  buf = nboPackFloat(buf, size.z);

  unsigned char stateByte = 0;
  stateByte |= canRicochet() ? _RICOCHET : 0;
  buf = nboPackUInt8(buf, stateByte);

  return buf;
}


void* WallObstacle::unpack(void* buf) {
  buf = nboUnpackFVec3(buf, pos);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackFloat(buf, size.y);
  buf = nboUnpackFloat(buf, size.z);

  unsigned char stateByte;
  buf = nboUnpackUInt8(buf, stateByte);
  ricochet = (stateByte & _RICOCHET) != 0;

  finalize();

  return buf;
}


int WallObstacle::packSize() const {
  int fullSize = 0;
  fullSize += sizeof(fvec3); // pos
  fullSize += sizeof(float);    // rotation
  fullSize += sizeof(float);  // breadth
  fullSize += sizeof(float);  // height
  fullSize += sizeof(uint8_t);  // state bits
  return fullSize;
}


void WallObstacle::print(std::ostream& /*out*/,
                         const std::string& /*indent*/) const {
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
