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

#include <math.h>
#include "common.h"
#include "BaseBuilding.h"
#include "global.h"
#include "Intersect.h"
#include "QuadWallSceneNode.h"

std::string		BaseBuilding::typeName("BaseBuilding");

BaseBuilding::BaseBuilding(const float *p, float rotation,
	const float *size, int _team) :
		Obstacle(p, rotation, size[0], size[1], size[2]),
		team(_team)
{
  if(pos[2] != 0) {
    height = 1.0;
  } else {
    height = 0.0;
  }
}

BaseBuilding::~BaseBuilding()
{
  // do nothing
}

std::string		BaseBuilding::getType() const
{
  return typeName;
}

std::string		BaseBuilding::getClassName()
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

bool			BaseBuilding::isInside(const float *p, float radius) const
{
  return (p[2] < (getPosition()[2] + getHeight())) 
  &&     ((p[2]+TankHeight) > getPosition()[2])
  &&     testRectCircle(getPosition(), getRotation(), getWidth(), getBreadth(), p, radius);
}

bool			BaseBuilding::isInside(const float *p, float angle,
			float dx, float dy) const
{
  return (p[2] < (getPosition()[2] + getHeight()))
  &&     ((p[2]+TankHeight) >= getPosition()[2])
  &&     testRectRect(getPosition(), getRotation(), getWidth(), getBreadth(), p, angle, dx, dy);
}

bool			BaseBuilding::isCrossing(const float *p, float angle,
			float dx, float dy,
			float *plane) const
{
  // if not inside or contained, then not crossing
  if (!isInside(p, angle, dx, dy) ||
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
			float halfWidth, float halfBreadth,
			float *normal) const
{
  return Obstacle::getHitNormal(pos1, azimuth1, pos2, azimuth2, halfWidth, halfBreadth,
      			getPosition(), getRotation(), getWidth(), getBreadth(),
			getHeight(), normal) >= 0.0f;
}

ObstacleSceneNodeGenerator* BaseBuilding::newSceneNodeGenerator() const
{
  return new BaseSceneNodeGenerator(this);
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

const int	BaseBuilding::getTeam() const {
  return team;
}

BaseSceneNodeGenerator::BaseSceneNodeGenerator(const BaseBuilding* _base) : base(_base)
{
  // do nothing
}

BaseSceneNodeGenerator::~BaseSceneNodeGenerator()
{
  // do nothing
}

WallSceneNode*	BaseSceneNodeGenerator::getNextNode(float uRepeats, float vRepeats, bool lod)
{
  const GLfloat *pos = base->getPosition();
  if(getNodeNumber() >= 1 && pos[2] == 0) return NULL;
  if(getNodeNumber() >= 6) return NULL;
  GLfloat bPoint[3], sCorner[3], tCorner[3];
  if(base->getPosition()[2] == 0) {
    incNodeNumber();
    base->getCorner(0, bPoint);
    base->getCorner(3, tCorner);
    base->getCorner(1, sCorner);
  } else {
    switch(incNodeNumber()) {
      case 1:
	base->getCorner(4, bPoint);
	base->getCorner(5, sCorner);
	base->getCorner(7, tCorner);
	break;
      case 2:
	base->getCorner(0, bPoint);
	base->getCorner(3, sCorner);
	base->getCorner(1, tCorner);
	break;
      case 3:
	base->getCorner(0, bPoint);
	base->getCorner(1, sCorner);
	base->getCorner(4, tCorner);
	break;
      case 4:
	base->getCorner(1, bPoint);
	base->getCorner(2, sCorner);
	base->getCorner(5, tCorner);
	break;
      case 5:
	base->getCorner(2, bPoint);
	base->getCorner(3, sCorner);
	base->getCorner(6, tCorner);
	break;
      case 6:
	base->getCorner(3, bPoint);
	base->getCorner(0, sCorner);
	base->getCorner(7, tCorner);
	break;
    }
  }
  GLfloat color[4];
  switch(base->getTeam()) {
    case 1:
      color[0] = 0.7f; color[1] = 0.0f; color[2] = 0.0f;
      break;
    case 2:
      color[0] = 0.0f; color[1] = 0.7f; color[2] = 0.0f;
      break;
    case 3:
      color[0] = 0.0f; color[1] = 0.0f; color[2] = 0.7f;
      break;
    case 4:
      color[0] = 0.7f; color[1] = 0.0f; color[2] = 0.7f;
      break;
  }
  color[3] = 1.0;

  GLfloat sEdge[3];
  GLfloat tEdge[3];
  sEdge[0] = sCorner[0] - bPoint[0];
  sEdge[1] = sCorner[1] - bPoint[1];
  sEdge[2] = sCorner[2] - bPoint[2];
  tEdge[0] = tCorner[0] - bPoint[0];
  tEdge[1] = tCorner[1] - bPoint[1];
  tEdge[2] = tCorner[2] - bPoint[2];

  WallSceneNode *retval = new QuadWallSceneNode(bPoint, sEdge, tEdge, uRepeats, vRepeats, lod);
  retval->setColor(color);
  return retval;
}
// ex: shiftwidth=2 tabstop=8
