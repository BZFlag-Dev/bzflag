/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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
#include "PyramidBuilding.h"
#include "Intersect.h"
#include "TriWallSceneNode.h"
#include "QuadWallSceneNode.h"

std::string		PyramidBuilding::typeName("PyramidBuilding");

PyramidBuilding::PyramidBuilding(const float* p, float a,
				float w, float b, float h) :
				Obstacle(p, a, w, b, h)
{
  // do nothing
}

PyramidBuilding::~PyramidBuilding()
{
  // do nothing
}

std::string		PyramidBuilding::getType() const
{
  return typeName;
}

std::string		PyramidBuilding::getClassName() // const
{
  return typeName;
}

float			PyramidBuilding::intersect(const Ray& r) const
{
  const float s = shrinkFactor(r.getOrigin()[2]);
  return timeRayHitsBlock(r, getPosition(), getRotation(),
			s * getWidth(), s * getBreadth(), getHeight());
}

void			PyramidBuilding::getNormal(const float* p,
							float* n) const
{

  // get normal in z = const plane
  const float s = shrinkFactor(p[2]);
  getNormalRect(p, getPosition(), getRotation(),
			s * getWidth(), s * getBreadth(), n);

  // now angle it due to slope of wall
  // FIXME -- this assumes the pyramid has a square base!
  const float h = 1.0f / hypotf(getHeight(), getWidth());
  n[0] *= h * getHeight();
  n[1] *= h * getHeight();
  n[2] = h * getWidth();
}

bool			PyramidBuilding::isInside(const float* p,
						float radius) const
{
  // really rough -- doesn't decrease size with height
  return p[2] <= getHeight() && testRectCircle(getPosition(), getRotation(),
					getWidth(), getBreadth(), p, radius)
					&& p[2] >= getPosition()[2];
}

bool			PyramidBuilding::isInside(const float* p, float a,
						float dx, float dy) const
{
  const float s = shrinkFactor(p[2]);
  return s > 0.0 && testRectRect(getPosition(), getRotation(),
					s * getWidth(), s * getBreadth(),
					p, a, dx, dy)
					&& p[2] >= getPosition()[2];
}

bool			PyramidBuilding::isCrossing(const float* p, float a,
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

  // now finish off plane equation (FIXME -- assumes a square base)
  const float h = 1.0f / hypotf(getHeight(), getWidth());
  plane[0] *= h * getHeight();
  plane[1] *= h * getHeight();
  plane[2] = h * getWidth();
  plane[3] = -(plane[0] * pw[0] + plane[1] * pw[1]);
  return true;
}

bool			PyramidBuilding::getHitNormal(
				const float* pos1, float,
				const float*, float,
				float, float,
				float* normal) const
{
  getNormal(pos1, normal);
  return true;
}

ObstacleSceneNodeGenerator*	PyramidBuilding::newSceneNodeGenerator() const
{
  return new PyramidSceneNodeGenerator(this);
}

void			PyramidBuilding::getCorner(int index,
						float* pos) const
{
  const float* base = getPosition();
  const float c = cosf(getRotation());
  const float s = sinf(getRotation());
  const float w = getWidth();
  const float h = getBreadth();
  switch (index) {
    case 0:
      pos[0] = base[0] + c * w - s * h;
      pos[1] = base[1] + s * w + c * h;
      pos[2] = base[2];
      break;
    case 1:
      pos[0] = base[0] - c * w - s * h;
      pos[1] = base[1] - s * w + c * h;
      pos[2] = base[2];
      break;
    case 2:
      pos[0] = base[0] - c * w + s * h;
      pos[1] = base[1] - s * w - c * h;
      pos[2] = base[2];
      break;
    case 3:
      pos[0] = base[0] + c * w + s * h;
      pos[1] = base[1] + s * w - c * h;
      pos[2] = base[2];
      break;
    case 4:
      pos[0] = base[0];
      pos[1] = base[1];
      pos[2] = getHeight() + getPosition()[2];
      break;
  }
}

float			PyramidBuilding::shrinkFactor(float z) const
{
  const float *pos = getPosition();
  z -= pos[2];

  if (z < 0.0f)
    return 1.0f;
  if (z > getHeight())
    return 0.0f;

  return (getHeight() - z) / getHeight();
}

//
// PyramidSceneNodeGenerator
//

PyramidSceneNodeGenerator::PyramidSceneNodeGenerator(
				const PyramidBuilding* _pyramid) :
				pyramid(_pyramid)
{
  // do nothing
}

PyramidSceneNodeGenerator::~PyramidSceneNodeGenerator()
{
  // do nothing
}

WallSceneNode*		PyramidSceneNodeGenerator::getNextNode(
				float uRepeats, float vRepeats, bool lod)
{

	bool isSquare = false;

	if (getNodeNumber() == 5) return NULL;

  GLfloat base[3], sCorner[3], tCorner[3];
  switch (incNodeNumber()) {
    case 1:
      pyramid->getCorner(0, base);
      pyramid->getCorner(1, sCorner);
      pyramid->getCorner(4, tCorner);
	  isSquare = false;
      break;
    case 2:
      pyramid->getCorner(1, base);
      pyramid->getCorner(2, sCorner);
      pyramid->getCorner(4, tCorner);
	  isSquare = false;
      break;
    case 3:
      pyramid->getCorner(2, base);
      pyramid->getCorner(3, sCorner);
      pyramid->getCorner(4, tCorner);
	  isSquare = false;
      break;
    case 4:
      pyramid->getCorner(3, base);
      pyramid->getCorner(0, sCorner);
      pyramid->getCorner(4, tCorner);
	  isSquare = false;
      break;
    case 5:
      pyramid->getCorner(0, base);
      pyramid->getCorner(3, sCorner);
      pyramid->getCorner(1, tCorner);
	  isSquare = true;
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

  if(isSquare != true)
	return new TriWallSceneNode(base, sEdge, tEdge, uRepeats, vRepeats, lod);
  else
	return new QuadWallSceneNode(base, sEdge, tEdge, uRepeats, vRepeats, lod);

}
// ex: shiftwidth=2 tabstop=8
