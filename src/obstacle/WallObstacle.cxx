/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
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
#include "WallObstacle.h"
#include "Intersect.h"
#include "QuadWallSceneNode.h"

BzfString		WallObstacle::typeName("WallObstacle");

WallObstacle::WallObstacle(const float* p, float a, float b, float h) :
				Obstacle(p, a, 0.0, b, h)
{
  // compute normal
  plane[0] = cosf(a);
  plane[1] = sinf(a);
  plane[2] = 0.0;
  plane[3] = -(p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2]);
}

WallObstacle::~WallObstacle()
{
  // do nothing
}

BzfString		WallObstacle::getType() const
{
  return typeName;
}

BzfString		WallObstacle::getClassName() // const
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

boolean			WallObstacle::isInside(const float* p, float r) const
{
  return p[0] * plane[0] + p[1] * plane[1] + p[2] * plane[2] + plane[3] < r;
}

boolean			WallObstacle::isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const
{
  const float xWidth = cosf(angle);
  const float yWidth = sinf(angle);
  const float xBreadth = -yWidth;
  const float yBreadth = xWidth;
  float corner[3];
  corner[2] = p[2];

  // check to see if any corner is inside negative half-space
  corner[0] = p[0] - xWidth * halfWidth - xBreadth * halfBreadth;
  corner[1] = p[1] - yWidth * halfWidth - yBreadth * halfBreadth;
  if (isInside(corner, 0.0)) return True;
  corner[0] = p[0] + xWidth * halfWidth - xBreadth * halfBreadth;
  corner[1] = p[1] + yWidth * halfWidth - yBreadth * halfBreadth;
  if (isInside(corner, 0.0)) return True;
  corner[0] = p[0] - xWidth * halfWidth + xBreadth * halfBreadth;
  corner[1] = p[1] - yWidth * halfWidth + yBreadth * halfBreadth;
  if (isInside(corner, 0.0)) return True;
  corner[0] = p[0] + xWidth * halfWidth + xBreadth * halfBreadth;
  corner[1] = p[1] + yWidth * halfWidth + yBreadth * halfBreadth;
  if (isInside(corner, 0.0f)) return True;

  return False;
}

boolean			WallObstacle::getHitNormal(
				const float*, float,
				const float*, float,
				float, float,
				float* normal) const
{
  getNormal(NULL, normal);
  return True;
}

ObstacleSceneNodeGenerator*	WallObstacle::newSceneNodeGenerator() const
{
  return new WallSceneNodeGenerator(this);
}

//
// WallSceneNodeGenerator
//

WallSceneNodeGenerator::WallSceneNodeGenerator(const WallObstacle* _wall) :
				wall(_wall)
{
  // do nothing
}

WallSceneNodeGenerator::~WallSceneNodeGenerator()
{
  // do nothing
}

WallSceneNode*		WallSceneNodeGenerator::getNextNode(
				float uRepeats, float vRepeats, boolean lod)
{
  if (getNodeNumber() == 1) return NULL;

  GLfloat base[3];
  GLfloat sEdge[3];
  GLfloat tEdge[3];
  const float* pos = wall->getPosition();
  const float c = cosf(wall->getRotation());
  const float s = sinf(wall->getRotation());
  const float h = wall->getBreadth();
  switch (incNodeNumber()) {
    case 1:
      base[0] = pos[0] + s * h;
      base[1] = pos[1] - c * h;
      base[2] = 0.0f;
      sEdge[0] = -2.0f * s * h;
      sEdge[1] = 2.0f * c * h;
      sEdge[2] = 0.0f;
      tEdge[0] = 0.0f;
      tEdge[1] = 0.0f;
      tEdge[2] = wall->getHeight();
      break;
  }
  return new QuadWallSceneNode(base, sEdge, tEdge, uRepeats, vRepeats, lod);
}
