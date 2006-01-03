/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <math.h>
#include "WallSceneNodeGenerator.h"
#include "WallObstacle.h"
#include "bzfgl.h"
#include "QuadWallSceneNode.h"


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
				float uRepeats, float vRepeats, bool lod)
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
