/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
{
  memset(pos, 0, sizeof(float) * 3);
  memset(size, 0, sizeof(float) * 3);
  angle = 0;
  driveThrough = false;
  shootThrough = false;
  ZFlip = false;
  source = WorldSource;

  insideNodeCount = 0;
  insideNodes = NULL;
}

Obstacle::Obstacle(const float* _pos, float _angle,
		   float _width, float _breadth, float _height,
		   bool drive, bool shoot)
{
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
  angle = _angle;
  size[0] = _width;
  size[1] = _breadth;
  size[2] = _height;

  driveThrough = drive;
  shootThrough = shoot;
  ZFlip = false;
  source = WorldSource;

  insideNodeCount = 0;
  insideNodes = NULL;
}

Obstacle::~Obstacle()
{
  delete[] insideNodes;
  return;
}

bool			Obstacle::isValid() const
{
  for (int a = 0; a < 3; a++) {
    if ((extents.mins[a] < -maxExtent) || (extents.maxs[a] > maxExtent)) {
      return false;
    }
  }
  return true;
}

void			Obstacle::setExtents()
{
  float xspan = (fabsf(cosf(angle)) * size[0]) + (fabsf(sinf(angle)) * size[1]);
  float yspan = (fabsf(cosf(angle)) * size[1]) + (fabsf(sinf(angle)) * size[0]);
  extents.mins[0] = pos[0] - xspan;
  extents.maxs[0] = pos[0] + xspan;
  extents.mins[1] = pos[1] - yspan;
  extents.maxs[1] = pos[1] + yspan;
  extents.mins[2] = pos[2];
  extents.maxs[2] = pos[2] + size[2];
  return;
}

bool			Obstacle::isFlatTop ( void ) const
{
  return false;
}

void			Obstacle::setZFlip ( void )
{
  ZFlip = true;
}

bool			Obstacle::getZFlip ( void ) const
{
  return ZFlip;
}


bool			Obstacle::isCrossing(const float*, float,
						float, float, float, float*) const
{
  // never crossing by default
  return false;
}

float			Obstacle::getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float width, float breadth,
				const float* oPos, float oAzimuth,
				float oWidth, float oBreadth, float oHeight,
				float* normal) const
{
  static const float	square[4][2] = {
				{  1.0f,  1.0f },
				{ -1.0f,  1.0f },
				{ -1.0f, -1.0f },
				{  1.0f, -1.0f }
			};

  // construct a ray between the old and new positions of each corner
  // of the moving object.  test each ray against object and record
  // the minimum valid intersection time.  valid times are [0,1].
  float c1 = cosf(azimuth1), s1 = sinf(azimuth1);
  float c2 = cosf(azimuth2), s2 = sinf(azimuth2);

  int i, bestSide = -1;
  float minTime = 1.0f;
  for (i = 0; i < 4; i++) {
    float p[3], d[3];
    p[0] = pos1[0] + square[i][0]*c1*width - square[i][1]*s1*breadth;
    p[1] = pos1[1] + square[i][0]*s1*width + square[i][1]*c1*breadth;
    p[2] = 0;
    d[0] = pos2[0] + square[i][0]*c2*width - square[i][1]*s2*breadth - p[0];
    d[1] = pos2[1] + square[i][0]*s2*width + square[i][1]*c2*breadth - p[1];
    d[2] = 0;
    int side;
    const float t = timeAndSideRayHitsRect(Ray(p, d),
				oPos, oAzimuth, oWidth, oBreadth, side);
    if (side >= 0 && t <= minTime) {
      minTime = t;
      bestSide = side;
    }
  }

  // check time to intersect roof (on the way down;  don't care about way up)
  if (pos2[2] < pos1[2]) {
    const float t = (pos1[2] - oHeight - oPos[2]) / (pos1[2] - pos2[2]);
    if (t >= 0.0f && t <= minTime) {
      minTime = t;
      bestSide = 4;
    }
  } else if (pos2[2] == pos1[2]) {
    if (pos1[2] == (oHeight + oPos[2])) {
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
    float v[2], p[2], p2[2], d[2];
    v[0] = oPos[0] + square[i][0] * c1 * oWidth - square[i][1] * s1 * oBreadth;
    v[1] = oPos[1] + square[i][0] * s1 * oWidth + square[i][1] * c1 * oBreadth;

    p[0] = (v[0]-pos1[0]) * cosf(-azimuth1) - (v[1]-pos1[1]) * sinf(-azimuth1);
    p[1] = (v[0]-pos1[0]) * sinf(-azimuth1) + (v[1]-pos1[1]) * cosf(-azimuth1);

    p2[0] = (v[0]-pos2[0]) * cosf(-azimuth2) - (v[1]-pos2[1]) * sinf(-azimuth2);
    p2[1] = (v[0]-pos2[0]) * sinf(-azimuth2) + (v[1]-pos2[1]) * cosf(-azimuth2);

    d[0] = p2[0] - p[0];
    d[1] = p2[1] - p[1];
    int side;
    const float t = timeAndSideRayHitsOrigRect(p, d, width, breadth, side);
    if (side >= 0 && t <= minTime) {
      minTime = t;
      bestSide = side;
      isObstacle = true;
    }
  }

  // get normal
  if (bestSide == -1) return -1.0f;
  if (bestSide == 4) {
    normal[0] = 0.0f;
    normal[1] = 0.0f;
    normal[2] = 1.0f;
  }
  else if (!isObstacle) {
    const float _angle = (float)(0.5 * M_PI * (float)bestSide + oAzimuth);
    normal[0] = cosf(_angle);
    normal[1] = sinf(_angle);
    normal[2] = 0.0f;
  }
  else {
    const float _angle = (float)(0.5 * M_PI * (float)bestSide +
			minTime * (azimuth2 - azimuth1) + azimuth1);
    normal[0] = -cosf(_angle);
    normal[1] = -sinf(_angle);
    normal[2] = 0.0f;
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
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8



