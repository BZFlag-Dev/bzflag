/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "Obstacle.h"
#include "Intersect.h"
#include <math.h>

Obstacle::Obstacle(const float* _pos, float _angle,
				float _width, float _breadth, float _height) :
				angle(_angle),
				width(_width),
				breadth(_breadth),
				height(_height)
{
  pos[0] = _pos[0];
  pos[1] = _pos[1];
  pos[2] = _pos[2];
}

Obstacle::~Obstacle()
{
  // do nothing
}

boolean			Obstacle::isCrossing(const float*, float,
						float, float, float*) const
{
  // never crossing by default
  return False;
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
    float p[2], d[2];
    p[0] = pos1[0] + square[i][0]*c1*width - square[i][1]*s1*breadth;
    p[1] = pos1[1] + square[i][0]*s1*width + square[i][1]*c1*breadth;
    d[0] = pos2[0] + square[i][0]*c2*width - square[i][1]*s2*breadth - p[0];
    d[1] = pos2[1] + square[i][0]*s2*width + square[i][1]*c2*breadth - p[1];
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
  }

  // now do the same with obstacle's corners against moving object.
  // we must transform the building into moving object's space.
  boolean isObstacle = False;
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
      isObstacle = True;
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
    const float angle = 0.5f * M_PI * (float)bestSide + oAzimuth;
    normal[0] = cosf(angle);
    normal[1] = sinf(angle);
    normal[2] = 0.0f;
  }
  else {
    const float angle = 0.5f * M_PI * (float)bestSide +
			minTime * (azimuth2 - azimuth1) + azimuth1;
    normal[0] = -cosf(angle);
    normal[1] = -sinf(angle);
    normal[2] = 0.0f;
  }
  return minTime;
}

//
// ObstacleSceneNodeGenerator
//

ObstacleSceneNodeGenerator::ObstacleSceneNodeGenerator() :
				node(0)
{
  // do nothing
}

ObstacleSceneNodeGenerator::~ObstacleSceneNodeGenerator()
{
  // do nothing
}
// ex: shiftwidth=2 tabstop=8
