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
#include "global.h"
#include "BoxBuilding.h"
#include "Intersect.h"
#include "QuadWallSceneNode.h"
#include "BZDBCache.h"

std::string		BoxBuilding::typeName("BoxBuilding");

BoxBuilding::BoxBuilding() : Obstacle(), noNodes(false)
{
  // do nothing
}

BoxBuilding::BoxBuilding(const float* p, float a, float w, float b, float h, 
			 bool drive, bool shoot, bool invisible) :
  Obstacle(p, a, w, b, h,drive,shoot), noNodes(invisible)
{
  // do nothing
}

BoxBuilding::~BoxBuilding()
{
  // do nothing
}

std::string		BoxBuilding::getType() const
{
  return typeName;
}

std::string		BoxBuilding::getClassName() // const
{
  return typeName;
}

float			BoxBuilding::intersect(const Ray& r) const
{
  return timeRayHitsBlock(r, getPosition(), getRotation(),
			getWidth(), getBreadth(), getHeight());
}

void			BoxBuilding::getNormal(const float* p, float* n) const
{
  getNormalRect(p, getPosition(), getRotation(), getWidth(), getBreadth(), n);
}

void			BoxBuilding::get3DNormal(const float* p, float* n) const
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

bool			BoxBuilding::isInside(const float* p,
						float radius) const
{
  return (p[2] < (getPosition()[2] + getHeight()))
  && ((p[2]+BZDBCache::tankHeight) >= getPosition()[2])
  &&     testRectCircle(getPosition(), getRotation(), getWidth(), getBreadth(), p, radius);
}

bool			BoxBuilding::isInside(const float* p, float a,
						float dx, float dy) const
{
  return (p[2] < (getPosition()[2] + getHeight()))
  &&     ((p[2]+BZDBCache::tankHeight) >= getPosition()[2])
  &&     testRectRect(getPosition(), getRotation(), getWidth(), getBreadth(), p, a, dx, dy);
}

bool			BoxBuilding::isInside(const float* oldP, float,
					      const float* p, float a,
					      float dx, float dy) const
{
  float higherZ;
  float lowerZ;
  if (oldP[2] > p[2]) {
    higherZ = oldP[2];
    lowerZ  = p[2];
  } else {
    higherZ = p[2];
    lowerZ  = oldP[2];
  }
  if (lowerZ >= (getPosition()[2] + getHeight()))
    return false;
  if ((higherZ + BZDBCache::tankHeight) < getPosition()[2])
    return false;
  return testRectRect(getPosition(), getRotation(), getWidth(), getBreadth(),
		      p, a, dx, dy);
}

bool			BoxBuilding::isCrossing(const float* p, float a,
					float dx, float dy, float* plane) const
{
  // if not inside or contained then not crossing
  if (!isInside(p, a, dx, dy) ||
	testRectInRect(getPosition(), getRotation(),
			getWidth(), getBreadth(), p, a, dx, dy))
    return false;
  if (!plane) return true;

  // it's crossing -- choose which wall is being crossed (this
  // is a guestimate, should really do a careful test).  just
  // see which wall the point is closest to.
  const float* p2 = getPosition();
  const float a2 = getRotation();
  const float c = cosf(-a2), s = sinf(-a2);
  const float x = c * (p[0] - p2[0]) - s * (p[1] - p2[1]);
  const float y = c * (p[1] - p2[1]) + s * (p[0] - p2[0]);
  float pw[2];
  if (fabsf(fabsf(x) - getWidth()) < fabsf(fabsf(y) - getBreadth())) {
    plane[0] = ((x < 0.0) ? -cosf(a2) : cosf(a2));
    plane[1] = ((x < 0.0) ? -sinf(a2) : sinf(a2));
    pw[0] = p2[0] + getWidth() * plane[0];
    pw[1] = p2[1] + getWidth() * plane[1];
  }
  else {
    plane[0] = ((y < 0.0) ? sinf(a2) : -sinf(a2));
    plane[1] = ((y < 0.0) ? -cosf(a2) : cosf(a2));
    pw[0] = p2[0] + getBreadth() * plane[0];
    pw[1] = p2[1] + getBreadth() * plane[1];
  }

  // now finish off plane equation
  plane[2] = 0.0;
  plane[3] = -(plane[0] * pw[0] + plane[1] * pw[1]);
  return true;
}

bool			BoxBuilding::getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float width, float breadth,
				float* normal) const
{
  return Obstacle::getHitNormal(pos1, azimuth1, pos2, azimuth2, width, breadth,
			getPosition(), getRotation(), getWidth(), getBreadth(),
			getHeight(), normal) >= 0.0f;
}

ObstacleSceneNodeGenerator*	BoxBuilding::newSceneNodeGenerator() const
{
  if (noNodes)
    return new EmptySceneNodeGenerator();
  else
    return new BoxSceneNodeGenerator(this);
}

void			BoxBuilding::getCorner(int index, float* pos) const
{
  const float* base = getPosition();
  const float c = cosf(getRotation());
  const float s = sinf(getRotation());
  const float w = getWidth();
  const float h = getBreadth();
  switch (index & 3) {
    case 0:
      pos[0] = base[0] + c * w - s * h;
      pos[1] = base[1] + s * w + c * h;
      break;
    case 1:
      pos[0] = base[0] - c * w - s * h;
      pos[1] = base[1] - s * w + c * h;
      break;
    case 2:
      pos[0] = base[0] - c * w + s * h;
      pos[1] = base[1] - s * w - c * h;
      break;
    case 3:
      pos[0] = base[0] + c * w + s * h;
      pos[1] = base[1] + s * w - c * h;
      break;
  }
  pos[2] = base[2];
  if (index >= 4) pos[2] += getHeight();
}

//
// BoxSceneNodeGenerator
//

BoxSceneNodeGenerator::BoxSceneNodeGenerator(const BoxBuilding* _box) :
				box(_box)
{
  // do nothing
}

BoxSceneNodeGenerator::~BoxSceneNodeGenerator()
{
  // do nothing
}

WallSceneNode*		BoxSceneNodeGenerator::getNextNode(
				float uRepeats, float vRepeats, bool lod)
{
  if (getNodeNumber() == 6) return NULL;

  GLfloat base[3], sCorner[3], tCorner[3];
  switch (incNodeNumber()) {
    case 1:
      box->getCorner(0, base);
      box->getCorner(1, sCorner);
      box->getCorner(4, tCorner);
      break;
    case 2:
      box->getCorner(1, base);
      box->getCorner(2, sCorner);
      box->getCorner(5, tCorner);
      break;
    case 3:
      box->getCorner(2, base);
      box->getCorner(3, sCorner);
      box->getCorner(6, tCorner);
      break;
    case 4:
      box->getCorner(3, base);
      box->getCorner(0, sCorner);
      box->getCorner(7, tCorner);
      break;
    case 5:							//This is the top polygon
      box->getCorner(4, base);
      box->getCorner(5, sCorner);
      box->getCorner(7, tCorner);
      break;
    case 6:							//This is the bottom polygon
      box->getCorner(0, base);
      box->getCorner(3, sCorner);
      box->getCorner(1, tCorner);
      break;
  }

  GLfloat sEdge[3];
  GLfloat tEdge[3];
  sEdge[0] = sCorner[0] - base[0];
  sEdge[1] = sCorner[1] - base[1];
  sEdge[2] = sCorner[2] - base[2];
  tEdge[0] = tCorner[0] - base[0];
  tEdge[1] = tCorner[1] - base[1];
  tEdge[2] = tCorner[2] - base[2];
  return new QuadWallSceneNode(base, sEdge, tEdge, uRepeats, vRepeats, lod);
}

/* ex: shiftwidth=2 tabstop=8
 * Local Variables: ***
 * mode:C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 */

