/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "BaseSceneNodeGenerator.h"
#include "obstacle/BaseBuilding.h"
#include "bzfgl.h"
#include "geometry/QuadWallSceneNode.h"


BaseSceneNodeGenerator::BaseSceneNodeGenerator(const BaseBuilding* _base) : base(_base) {
  // do nothing
}

BaseSceneNodeGenerator::~BaseSceneNodeGenerator() {
  // do nothing
}

WallSceneNode*  BaseSceneNodeGenerator::getNextNode(float uRepeats, float vRepeats, bool lod) {
  const float height = base->getHeight() + base->getPosition()[2];
  if (getNodeNumber() >= 1 && height == 0) { return NULL; }
  if (getNodeNumber() >= 6) { return NULL; }
  fvec3 bPoint, sCorner, tCorner;

  bool fixedUVs = false;

  if (height == 0) {
    incNodeNumber();
    base->getCorner(0, bPoint);
    base->getCorner(3, tCorner);
    base->getCorner(1, sCorner);
    fixedUVs = true;
  }
  else {
    switch (incNodeNumber()) {
      case 1:  // This is the top polygon
        base->getCorner(4, bPoint);
        base->getCorner(5, sCorner);
        base->getCorner(7, tCorner);
        fixedUVs = true;
        break;
      case 2:
        base->getCorner(0, bPoint);
        base->getCorner(1, sCorner);
        base->getCorner(4, tCorner);
        break;
      case 3:
        base->getCorner(1, bPoint);
        base->getCorner(2, sCorner);
        base->getCorner(5, tCorner);
        break;
      case 4:
        base->getCorner(2, bPoint);
        base->getCorner(3, sCorner);
        base->getCorner(6, tCorner);
        break;
      case 5:
        base->getCorner(3, bPoint);
        base->getCorner(0, sCorner);
        base->getCorner(7, tCorner);
        break;
      case 6:  // This is the bottom polygon
        fixedUVs = true;
        if (base->getPosition()[2] > 0.0f) {
          // Only generate if above ground level
          base->getCorner(0, bPoint);
          base->getCorner(3, sCorner);
          base->getCorner(1, tCorner);
        }
        else {
          return NULL;
        }
        break;

    }
  }

  const fvec3 sEdge = sCorner - bPoint;
  const fvec3 tEdge = tCorner - bPoint;

  WallSceneNode* retval = new QuadWallSceneNode(bPoint, sEdge, tEdge, uRepeats, vRepeats, lod, fixedUVs);

  return retval;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
