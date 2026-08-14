/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "BoxSceneNodeGenerator.h"
#include "WallSceneNode.h"
#include "BoxBuilding.h"
#include "QuadWallSceneNode.h"

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

WallSceneNode*      BoxSceneNodeGenerator::getNextNode(
    float uRepeats, float vRepeats, bool lod)
{
    if (getNodeNumber() == 6) return NULL;

    glm::vec3 base, sCorner, tCorner;
    switch (incNodeNumber())
    {
    case 1:
        base    = box->getCorner(0);
        sCorner = box->getCorner(1);
        tCorner = box->getCorner(4);
        break;
    case 2:
        base    = box->getCorner(1);
        sCorner = box->getCorner(2);
        tCorner = box->getCorner(5);
        break;
    case 3:
        base    = box->getCorner(2);
        sCorner = box->getCorner(3);
        tCorner = box->getCorner(6);
        break;
    case 4:
        base    = box->getCorner(3);
        sCorner = box->getCorner(0);
        tCorner = box->getCorner(7);
        break;
    case 5:                         //This is the top polygon
        base    = box->getCorner(4);
        sCorner = box->getCorner(5);
        tCorner = box->getCorner(7);
        break;
    case 6:                         //This is the bottom polygon
        //Don't generate the bottom polygon if on the ground (or lower)
        if (box->getPosition()[2] > 0.0f)
        {
            base    = box->getCorner(0);
            sCorner = box->getCorner(3);
            tCorner = box->getCorner(1);
        }
        else
            return NULL;
        break;
    }

    const glm::vec3 sEdge = sCorner - base;
    const glm::vec3 tEdge = tCorner - base;
    return new QuadWallSceneNode(base, sEdge, tEdge, uRepeats, vRepeats, lod);
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
