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
 
#include "TetraSceneNodeGenerator.h"
#include "TetraBuilding.h"
#include "bzfgl.h"
#include "TriWallSceneNode.h"

//
// TetraSceneNodeGenerator
//

TetraSceneNodeGenerator::TetraSceneNodeGenerator(
				const TetraBuilding* _tetra) :
				tetra(_tetra)
{
  // do nothing
}

TetraSceneNodeGenerator::~TetraSceneNodeGenerator()
{
  // do nothing
}

WallSceneNode*		TetraSceneNodeGenerator::getNextNode(
				float uRepeats, float vRepeats, bool lod)
{
  while ((getNodeNumber() < 4) && !tetra->isVisiblePlane(getNodeNumber())) {
    incNodeNumber();
  }

  if (getNodeNumber() >= 4) {
    return NULL;
  }

  GLfloat base[3], sCorner[3], tCorner[3];

  switch (incNodeNumber()) {
    case 1:
      tetra->getCorner(1, base);
      tetra->getCorner(2, sCorner);
      tetra->getCorner(3, tCorner);
      break;
    case 2:
      tetra->getCorner(0, base);
      tetra->getCorner(3, sCorner);
      tetra->getCorner(2, tCorner);
      break;
    case 3:
      tetra->getCorner(0, base);
      tetra->getCorner(1, sCorner);
      tetra->getCorner(3, tCorner);
      break;
    case 4:
      tetra->getCorner(0, base);
      tetra->getCorner(2, sCorner);
      tetra->getCorner(1, tCorner);
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

  return new TriWallSceneNode(base, sEdge, tEdge, uRepeats, vRepeats, lod);

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
