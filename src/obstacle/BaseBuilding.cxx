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

#include "common.h"
#include <math.h>
#include "global.h"
#include "Pack.h"
#include "BaseBuilding.h"
#include "Intersect.h"
#include "MeshTransform.h"


const char*		BaseBuilding::typeName = "BaseBuilding";

BaseBuilding::BaseBuilding()
{
}

BaseBuilding::BaseBuilding(const float *p, float rotation,
	const float *size, int _team) :
		Obstacle(p, rotation, size[0], size[1], size[2]),
		team(_team)
{
}

BaseBuilding::~BaseBuilding()
{
  // do nothing
}

Obstacle* BaseBuilding::copyWithTransform(const MeshTransform& xform) const
{
  float newPos[3], newSize[3], newAngle;
  memcpy(newPos, pos, sizeof(float[3]));
  memcpy(newSize, size, sizeof(float[3]));
  newAngle = angle;

  MeshTransform::Tool tool(xform);
  tool.modifyOldStyle(newPos, newSize, newAngle);
  
  BaseBuilding* copy = new BaseBuilding(newPos, newAngle, newSize, team);
  
  return copy;
}

const char*		BaseBuilding::getType() const
{
  return typeName;
}

const char*		BaseBuilding::getClassName()
{
  return typeName;
}

float			BaseBuilding::intersect(const Ray &r) const
{
  return timeRayHitsBlock(r, getPosition(), getRotation(),
			  getWidth(), getBreadth(), getHeight());
}

void			BaseBuilding::getNormal(const float *p, float *n) const
{
  getNormalRect(p, getPosition(), getRotation(), getWidth(), getBreadth(), n);
}

void			BaseBuilding::get3DNormal(const float* p, float* n) const
{
  // This bit of cruft causes bullets to bounce of buildings in the z direction
  if (fabs(p[2] - getPosition()[2]) < Epsilon) {
    n[0] = 0.0f;
    n[1] = 0.0f;
    n[2] = -1.0f;
  }
  else if (fabs(p[2] - (getPosition()[2] + getHeight())) < Epsilon) {
    n[0] = 0.0f;
    n[1] = 0.0f;
    n[2] = 1.0f;
  } // end cruftiness
  else
    getNormal(p, n);
}

bool			BaseBuilding::inCylinder(const float *p, float radius, float height) const
{
  return (p[2] < (getPosition()[2] + getHeight()))
  &&     ((p[2]+height) > getPosition()[2])
  &&     testRectCircle(getPosition(), getRotation(), getWidth(), getBreadth(), p, radius);
}

bool			BaseBuilding::inBox(const float *p, float angle,
			float dx, float dy, float height) const
{
  return (p[2] < (getPosition()[2] + getHeight()))
  &&     ((p[2]+height) >= getPosition()[2])
  &&     testRectRect(getPosition(), getRotation(), getWidth(), getBreadth(), p, angle, dx, dy);
}

bool			BaseBuilding::inMovingBox(const float* oldP, float,
					       const float *p, float angle,
			float dx, float dy, float height) const
{
  float topBaseHeight = getPosition()[2] + getHeight();
  float higherZ;
  float lowerZ;
  // if a base is just the ground (z == 0 && height == 0) no collision
  // ground is already handled
  if (topBaseHeight <= 0.0)
    return false;
  if (oldP[2] > p[2]) {
    higherZ = oldP[2];
    lowerZ  = p[2];
  } else {
    higherZ = p[2];
    lowerZ  = oldP[2];
  }
  if (lowerZ >= topBaseHeight)
    return false;
  if ((higherZ + height) < getPosition()[2])
    return false;
  return testRectRect(getPosition(), getRotation(), getWidth(), getBreadth(),
		      p, angle, dx, dy);
}

bool			BaseBuilding::isCrossing(const float *p, float angle,
			float dx, float dy, float height,
			float *plane) const
{
  // if not inside or contained, then not crossing
  if (!inBox(p, angle, dx, dy, height) ||
      testRectInRect(getPosition(), getRotation(),
	getWidth(), getBreadth(), p, angle, dx, dy))
    return false;
  if(!plane) return true;

  // it's crossing -- choose which wall is being crossed (this
  // is a guestimate, should really do a careful test). Just
  // see which wall the point is closest to
  const float *p2 = getPosition();
  const float a2  = getRotation();
  const float c   = cosf(-a2), s = sinf(-a2);
  const float x   = c * (p[0] - p2[0]) - s * (p[1] - p2[1]);
  const float y   = c * (p[1] - p2[1]) - s * (p[0] - p2[0]);
  float pw[2];
  if(fabsf(fabsf(x) - getWidth()) < fabsf(fabsf(y) - getBreadth())) {
    plane[0] = ((x < 0.0) ? -cosf(a2) : cosf(a2));
    plane[1] = ((x < 0.0) ? -sinf(a2) : sinf(a2));
    pw[0] = p2[0] + getWidth() * plane[0];
    pw[1] = p2[1] + getWidth() * plane[1];
  } else {
    plane[0] = ((y < 0.0) ? sinf(a2) : -sinf(a2));
    plane[1] = ((y < 0.0) ? cosf(a2) : -cosf(a2));
    pw[0] = p2[0] + getBreadth() * plane[0];
    pw[1] = p2[1] + getBreadth() * plane[1];
  }

  // now finish off plane equation
  plane[2] = 0.0;
  plane[3] = -(plane[0] * pw[0] + plane[1] * pw[1]);
  return true;
}

bool			BaseBuilding::getHitNormal(const float *pos1, float azimuth1,
			const float *pos2, float azimuth2,
			float halfWidth, float halfBreadth, float,
			float *normal) const
{
  return Obstacle::getHitNormal(pos1, azimuth1, pos2, azimuth2, halfWidth, halfBreadth,
			getPosition(), getRotation(), getWidth(), getBreadth(),
			getHeight(), normal) >= 0.0f;
}

void			BaseBuilding::getCorner(int index, float *pos) const
{
  const float *base = getPosition();
  const float c = cosf(getRotation());
  const float s = sinf(getRotation());
  const float w = getWidth();
  const float b = getBreadth();
  switch(index & 3) {
    case 0:
      pos[0] = base[0] + c * w - s * b;
      pos[1] = base[1] + s * w + c * b;
      break;
    case 1:
      pos[0] = base[0] - c * w - s * b;
      pos[1] = base[1] - s * w + c * b;
      break;
    case 2:
      pos[0] = base[0] - c * w + s * b;
      pos[1] = base[1] - s * w - c * b;
      break;
    case 3:
      pos[0] = base[0] + c * w + s * b;
      pos[1] = base[1] + s * w - c * b;
      break;
  }
  pos[2] = base[2];
  if(index >= 4) pos[2] += getHeight();
}

int	BaseBuilding::getTeam() const {
  return team;
}

bool			BaseBuilding::isFlatTop() const
{
  return true;
}


void* BaseBuilding::pack(void* buf) const
{
  buf = nboPackUShort(buf, (uint16_t) team);
  
  buf = nboPackVector(buf, pos);
  buf = nboPackFloat(buf, angle);
  buf = nboPackVector(buf, size);

  unsigned char stateByte = 0;
  stateByte |= isDriveThrough() ? _DRIVE_THRU : 0;
  stateByte |= isShootThrough() ? _SHOOT_THRU : 0;
  buf = nboPackUByte(buf, stateByte);

  return buf;
}


void* BaseBuilding::unpack(void* buf)
{
  uint16_t shortTeam;
  buf = nboUnpackUShort(buf, shortTeam);
  team = (int)shortTeam;
  
  buf = nboUnpackVector(buf, pos);
  buf = nboUnpackFloat(buf, angle);
  buf = nboUnpackVector(buf, size);

  unsigned char stateByte;
  buf = nboUnpackUByte(buf, stateByte);
  driveThrough = (stateByte & _DRIVE_THRU) != 0;
  shootThrough = (stateByte & _SHOOT_THRU) != 0;

  return buf;
}


int BaseBuilding::packSize() const
{
  int fullSize = 0;
  fullSize += sizeof(uint16_t); // team
  fullSize += sizeof(float[3]); // pos
  fullSize += sizeof(float);    // rotation
  fullSize += sizeof(float[3]); // size
  fullSize += sizeof(uint8_t);  // state bits
  return fullSize;
}


void BaseBuilding::print(std::ostream& out, const std::string& indent) const
{
  out << indent << "base" << std::endl;
  const float *pos = getPosition();
  out << indent << "  position " << pos[0] << " " << pos[1] << " " 
                                 << pos[2] << std::endl;
  out << indent << "  size " << getWidth() << " " << getBreadth() 
                             << " " << getHeight() << std::endl;
  out << indent << "  rotation " << ((getRotation() * 180.0) / M_PI)
                                 << std::endl;
  out << indent << "  color " << getTeam() << std::endl;
  if (isDriveThrough() && isShootThrough()) {
    out << indent << "  passable" << std::endl;
  } else {
    if (isDriveThrough()) {
      out << indent << "  drivethrough" << std::endl;
    }
    if (isShootThrough()) {
      out << indent << "  shootthrough" << std::endl;
    }
  }
  out << indent << "end" << std::endl << std::endl;
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

