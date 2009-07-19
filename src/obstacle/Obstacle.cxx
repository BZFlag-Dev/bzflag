/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include <string.h>
#include <iostream>
#include "Obstacle.h"
#include "Intersect.h"
#include "StateDatabase.h"


// limits the maximum extent of any obstacle
const float Obstacle::maxExtent = 1.0e30f;

// for counting OBJ file objects
int Obstacle::objCounter = 0;


Obstacle::Obstacle()
: pos(0.0f, 0.0f, 0.0f)
, size(0.0f, 0.0f, 0.0f)
, angle(0.0f)
, driveThrough(0)
, shootThrough(0)
, ricochet(false)
, zFlip(false)
, source(WorldSource)
, listID(0)
, insideNodeCount(0)
, insideNodes(NULL)
{
  // do nothing
}


Obstacle::Obstacle(const fvec3& _pos, float _angle,
		   float _width, float _breadth, float _height,
		   unsigned char drive, unsigned char shoot, bool rico)
: pos(_pos)
, size(_width, _breadth, _height)
, angle(_angle)
, driveThrough(drive)
, shootThrough(shoot)
, ricochet(rico)
, zFlip(false)
, source(WorldSource)
, listID(0)
, insideNodeCount(0)
, insideNodes(NULL)
{
  // do nothing
}


Obstacle::~Obstacle()
{
  delete[] insideNodes;
  return;
}


bool Obstacle::isValid() const
{
  for (int a = 0; a < 3; a++) {
    if ((extents.mins[a] < -maxExtent) || (extents.maxs[a] > maxExtent)) {
      return false;
    }
  }
  return true;
}


void Obstacle::setExtents()
{
  const float abs_cos = fabsf(cosf(angle));
  const float abs_sin = fabsf(sinf(angle));
  const float xspan = (abs_cos * size.x) + (abs_sin * size.y);
  const float yspan = (abs_cos * size.y) + (abs_sin * size.x);
  extents.mins.x = pos.x - xspan;
  extents.maxs.x = pos.x + xspan;
  extents.mins.y = pos.y - yspan;
  extents.maxs.y = pos.y + yspan;
  extents.mins.z = pos.z;
  extents.maxs.z = pos.z + size.z;
  return;
}


bool Obstacle::isFlatTop() const
{
  return false;
}


void Obstacle::setZFlip()
{
  zFlip = true;
}


bool Obstacle::isCrossing(const fvec3&, float, float, float, float, fvec4*) const
{
  // never crossing by default
  return false;
}


float Obstacle::getHitNormal(const fvec3& pos1, float azimuth1,
                             const fvec3& pos2, float azimuth2,
                             float width, float breadth,
                             const fvec3& oPos, float oAzimuth,
                             float oWidth, float oBreadth, float oHeight,
                             fvec3& normal) const
{
  static const fvec2 square[4] = {
    fvec2(+1.0f, +1.0f),
    fvec2(-1.0f, +1.0f),
    fvec2(-1.0f, -1.0f),
    fvec2(+1.0f, -1.0f)
  };

  // construct a ray between the old and new positions of each corner
  // of the moving object.  test each ray against object and record
  // the minimum valid intersection time.  valid times are [0,1].
  float c1 = cosf(azimuth1), s1 = sinf(azimuth1);
  float c2 = cosf(azimuth2), s2 = sinf(azimuth2);

  int i, bestSide = -1;
  float minTime = 1.0f;
  for (i = 0; i < 4; i++) {
    fvec3 p, d;
    p.x = pos1.x + (square[i].x * c1 * width) - (square[i].y * s1 * breadth);
    p.y = pos1.y + (square[i].x * s1 * width) + (square[i].y * c1 * breadth);
    p.z = 0;
    d.x = pos2.x + (square[i].x * c2 * width) - (square[i].y * s2 * breadth) - p.x;
    d.y = pos2.y + (square[i].x * s2 * width) + (square[i].y * c2 * breadth) - p.y;
    d.z = 0;
    int side;
    const float t = Intersect::timeAndSideRayHitsRect(Ray(p, d),
				                      oPos, oAzimuth,
				                      oWidth, oBreadth, side);
    if (side >= 0 && t <= minTime) {
      minTime = t;
      bestSide = side;
    }
  }

  // check time to intersect roof (on the way down;  don't care about way up)
  if (pos2.z < pos1.z) {
    const float t = (pos1.z - oHeight - oPos.z) / (pos1.z - pos2.z);
    if (t >= 0.0f && t <= minTime) {
      minTime = t;
      bestSide = 4;
    }
  } else if (pos2.z == pos1.z) {
    if (pos1.z == (oHeight + oPos.z)) {
      minTime = 0.0f;
      bestSide = 4;
    }
  }

  // now do the same with obstacle's corners against moving object.
  // we must transform the building into moving object's space.
  bool isObstacle = false;
  c1 = cosf(oAzimuth);
  s1 = sinf(oAzimuth);
  for (i = 0; i < 4; i++) {
    fvec3 v, p, p2, d; // FIXME - change to fvec2 with Intersect is fixed
    v.x = oPos.x + square[i].x * c1 * oWidth - square[i].y * s1 * oBreadth;
    v.y = oPos.y + square[i].x * s1 * oWidth + square[i].y * c1 * oBreadth;

    p.x = (v.x - pos1.x) * cosf(-azimuth1) - (v.y - pos1.y) * sinf(-azimuth1);
    p.y = (v.x - pos1.x) * sinf(-azimuth1) + (v.y - pos1.y) * cosf(-azimuth1);

    p2.x = (v.x - pos2.x) * cosf(-azimuth2) - (v.y - pos2.y) * sinf(-azimuth2);
    p2.y = (v.x - pos2.x) * sinf(-azimuth2) + (v.y - pos2.y) * cosf(-azimuth2);

    d.x = p2.x - p.x;
    d.y = p2.y - p.y;
    int side;
    const float t = Intersect::timeAndSideRayHitsOrigRect(p, d,
                                                          width, breadth, side);
    if (side >= 0 && t <= minTime) {
      minTime = t;
      bestSide = side;
      isObstacle = true;
    }
  }

  // get normal
  if (bestSide == -1) {
    return -1.0f;
  }

  if (bestSide == 4) {
    normal.x = 0.0f;
    normal.y = 0.0f;
    normal.z = 1.0f;
  }
  else if (!isObstacle) {
    const float _angle = (float)(0.5 * M_PI * (float)bestSide + oAzimuth);
    normal.x = cosf(_angle);
    normal.y = sinf(_angle);
    normal.z = 0.0f;
  }
  else {
    const float _angle = (float)(0.5 * M_PI * (float)bestSide +
			minTime * (azimuth2 - azimuth1) + azimuth1);
    normal.x = -cosf(_angle);
    normal.y = -sinf(_angle);
    normal.z = 0.0f;
  }
  return minTime;
}


void Obstacle::addInsideSceneNode(SceneNode* node)
{
  insideNodeCount++;
  SceneNode** tmp = new SceneNode*[insideNodeCount];
  memcpy(tmp, insideNodes, (insideNodeCount - 1) * sizeof(SceneNode*));
  delete[] insideNodes;
  insideNodes = tmp;
  insideNodes[insideNodeCount - 1] = node;
}


void Obstacle::freeInsideSceneNodeList()
{
  insideNodeCount = 0;
  delete[] insideNodes;
  insideNodes = NULL;
  return;
}


int Obstacle::getInsideSceneNodeCount() const
{
  return insideNodeCount;
}


SceneNode** Obstacle::getInsideSceneNodeList() const
{
  return insideNodes;
}


Obstacle* Obstacle::copyWithTransform(MeshTransform const&) const
{
  std::cout << "ERROR: Obstacle::copyWithTransform()" << std::endl;
  exit(1);
  // umm, yeah...make the compiler happy...
  return NULL;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
