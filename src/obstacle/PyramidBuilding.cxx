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

#include <math.h>
#include "common.h"
#include "global.h"
#include "PyramidBuilding.h"
#include "Intersect.h"
#include "TriWallSceneNode.h"
#include "QuadWallSceneNode.h"
#include "BZDBCache.h"

std::string		PyramidBuilding::typeName("PyramidBuilding");

PyramidBuilding::PyramidBuilding(const float* p, float a,
				float w, float b, float h, bool drive, bool shoot) :
				Obstacle(p, a, w, b, h,drive,shoot)
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
  return timeRayHitsPyramids(r, getPosition(), getRotation(),
			     getWidth(), getBreadth(), getHeight(),
			     getZFlip());
}

void			PyramidBuilding::getNormal(const float* p,
							float* n) const
{
  // get normal in z = const plane
  const float s = shrinkFactor(p[2]);
  getNormalRect(p, getPosition(), getRotation(),
			s * getWidth(), s * getBreadth(), n);

  // make sure we are not way above or way below it
  // above is good so we can drive on it when it's fliped
  float top =  getPosition()[2]+getHeight();
  float bottom = getPosition()[2];

  if (s ==0){
	  if (this->getZFlip()){
		  if (p[2] >= top){
			  n[0] = n[1] = 0;
			  n[2] = 1;
			  return;
		  }
	  }else{
		  if (p[2] <= bottom){
			  n[0] = n[1] = 0;
			  n[2] = -1;
			  return;
		  }
	  }
  }

  // now angle it due to slope of wall
  // FIXME -- this assumes the pyramid has a square base!
  const float h = 1.0f / hypotf(getHeight(), getWidth());
  n[0] *= h * getHeight();
  n[1] *= h * getHeight();
  n[2] = h * getWidth();

  if (this->getZFlip())
	  n[2] *= -1;
}

void			PyramidBuilding::get3DNormal(const float* p,
						     float* n) const
{
  const float epsilon = ZERO_TOLERANCE;

  // get normal in z = const plane
  const float s = shrinkFactor(p[2]);
  getNormalRect(p, getPosition(), getRotation(),
		s * getWidth(), s * getBreadth(), n);

  // make sure we are not way above or way below it
  // above is good so we can drive on it when it's fliped
  float top =  getPosition()[2]+getHeight();
  float bottom = getPosition()[2];

  if (s == 0) {
    if (getZFlip()) {
      if (p[2] >= top) {
	n[0] = n[1] = 0;
	n[2] = 1;
	return;
      }
    } else {
      if (p[2] <= bottom) {
	n[0] = n[1] = 0;
	n[2] = -1;
	return;
      }
    }
  }

  if (s >= 1.0f - epsilon) {
    n[0] = n[1] = 0;
    if (getZFlip()) {
      n[2] = 1;
    } else {
      n[2] = -1;
    }
    return;
  }

  // now angle it due to slope of wall
  // FIXME -- this assumes the pyramid has a square base!
  const float h = 1.0f / hypotf(getHeight(), getWidth());
  n[0] *= h * getHeight();
  n[1] *= h * getHeight();
  n[2]  = h * getWidth();

  if (this->getZFlip())
    n[2] *= -1;
}

bool			PyramidBuilding::isInside(const float* p,
						float radius) const
{
  // really rough -- doesn't decrease size with height
  return (p[2] <= getHeight())
  &&     ((p[2]+BZDBCache::tankHeight) >= getPosition()[2])
  &&     testRectCircle(getPosition(), getRotation(), getWidth(), getBreadth(), p, radius);
}

bool			PyramidBuilding::isInside(const float* p, float a,
						float dx, float dy) const
{
  // Tank is below pyramid ?
  if (p[2] + BZDBCache::tankHeight < getPosition()[2])
    return false;
  // Tank is above pyramid ?
  if (p[2] >= getPosition()[2] + getHeight())
    return false;
  // Could be inside. Then check collision with the rectangle at object height
  // This is a rectangle reduced by shrinking but pass the height that we are
  // not so sure where collision can be
  const float s = shrinkFactor(p[2], BZDBCache::tankHeight);
  return testRectRect(getPosition(), getRotation(),
		      s * getWidth(), s * getBreadth(), p, a, dx, dy);
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
				const float* pos2, float,
				float, float, float height,
				float* normal) const
{
  // pyramids height and flipping
  // normalize height sign and report that in flip
  float oHeight = getHeight();
  bool  flip    = getZFlip();
  if (oHeight < 0) {
    flip    = !flip;
    oHeight = -oHeight;
  }

  // get Bottom and Top of building
  float oBottom = getPosition()[2];
  float oTop    = oBottom + oHeight;

  // get higher and lower point of base of colliding object
  float objHigh = pos1[2];
  float objLow  = pos2[2];
  if (objHigh < objLow) {
    float temp = objHigh;
    objHigh    = objLow;
    objLow     = temp;
  }

  normal[0] = normal[1] = 0;
  if (flip && objHigh > oTop) {
    // base of higher object is over the plateau
    normal[2] = 1;
    return true;
  } else if (!flip && objLow + height < oBottom) {
    // top of lower object is below the base
    normal[2] = -1;
    return true;
  }

  // get normal in z = const plane
  const float s = shrinkFactor(pos1[2], height);

  getNormalRect(pos1, getPosition(), getRotation(),
		s * getWidth(), s * getBreadth(), normal);

  // now angle it due to slope of wall
  // FIXME -- this assumes the pyramid has a square base!
  const float h = 1.0f / hypotf(oHeight, getWidth());
  normal[0] *= h * oHeight;
  normal[1] *= h * oHeight;
  normal[2]  = h * getWidth();

  if (flip)
    normal[2] = -normal[2];
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
  const float top  = getHeight() + base[2];
  switch (index) {
    case 0:
      pos[0] = base[0] + c * w - s * h;
      pos[1] = base[1] + s * w + c * h;
	  if (getZFlip())
		 pos[2] = top;
	  else
		 pos[2] = base[2];
      break;
    case 1:
      pos[0] = base[0] - c * w - s * h;
      pos[1] = base[1] - s * w + c * h;
	  if (getZFlip())
		 pos[2] = top;
	  else
		 pos[2] = base[2];
      break;
    case 2:
      pos[0] = base[0] - c * w + s * h;
      pos[1] = base[1] - s * w - c * h;
	  if (getZFlip())
		 pos[2] = top;
	  else
		 pos[2] = base[2];
      break;
    case 3:
      pos[0] = base[0] + c * w + s * h;
      pos[1] = base[1] + s * w - c * h;
	  if (getZFlip())
		 pos[2] = top;
	  else
		 pos[2] = base[2];
      break;
    case 4:
      pos[0] = base[0];
      pos[1] = base[1];
	  if (getZFlip())
		 pos[2] = base[2];
	  else
		 pos[2] = top;
      break;
  }
}

float			PyramidBuilding::shrinkFactor(float z,
						      float height) const
{
  float shrink;

  // Normalize Height and flip to have height > 0
  float oHeight = getHeight();
  bool  flip    = getZFlip();
  if (oHeight < 0) {
    flip    = !flip;
    oHeight = - oHeight;
  }

 // Remove heights bias
  const float *pos = getPosition();
  z -= pos[2];
  if (oHeight <= ZERO_TOLERANCE) {
    shrink = 1.0f;
  } else {
  // Normalize heights
  z /= oHeight;

  // if flipped the bigger intersection is at top of obiect
  if (flip) {
    // Normalize the object height, we have not done yet
    z += height / oHeight;
  }

  // shrink is that
  if (flip) {
    shrink = z;
  } else {
    shrink = 1.0f - z;
  }
  }

  // clamp in 0 .. 1
  if (shrink < 0.0)
    shrink = 0.0;
  else if (shrink > 1.0)
    shrink = 1.0;

  return shrink;
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
  if (pyramid->getZFlip()){
  switch (incNodeNumber()) {
    case 1:
      pyramid->getCorner(4, base);
      pyramid->getCorner(1, sCorner);
      pyramid->getCorner(0, tCorner);
	  isSquare = false;
      break;
    case 2:
      pyramid->getCorner(4, base);
      pyramid->getCorner(2, sCorner);
      pyramid->getCorner(1, tCorner);
	  isSquare = false;
      break;
    case 3:
      pyramid->getCorner(4, base);
      pyramid->getCorner(3, sCorner);
      pyramid->getCorner(2, tCorner);
	  isSquare = false;
      break;
    case 4:
      pyramid->getCorner(4, base);
      pyramid->getCorner(0, sCorner);
      pyramid->getCorner(3, tCorner);
	  isSquare = false;
      break;
    case 5:
      pyramid->getCorner(0, base);
      pyramid->getCorner(1, sCorner);
      pyramid->getCorner(3, tCorner);
	  isSquare = true;
      break;
  }
  }else{
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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

