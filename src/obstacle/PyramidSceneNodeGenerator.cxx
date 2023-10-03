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

// Interface
#include "PyramidSceneNodeGenerator.h"

// System headers
#include <glm/gtc/type_ptr.hpp>

// Common headers
#include "PyramidBuilding.h"
#include "bzfgl.h"
#include "TriWallSceneNode.h"
#include "QuadWallSceneNode.h"

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

WallSceneNode*      PyramidSceneNodeGenerator::getNextNode(
    float uRepeats, float vRepeats, bool lod)
{

    bool isQuad = false;

    if (getNodeNumber() == 5) return NULL;

    glm::vec3 base, sCorner, tCorner;
    if (pyramid->getZFlip())
    {
        switch (incNodeNumber())
        {
        case 1:
            base    = pyramid->getCorner(4);
            sCorner = pyramid->getCorner(1);
            tCorner = pyramid->getCorner(0);
            isQuad = false;
            break;
        case 2:
            base    = pyramid->getCorner(4);
            sCorner = pyramid->getCorner(2);
            tCorner = pyramid->getCorner(1);
            isQuad = false;
            break;
        case 3:
            base    = pyramid->getCorner(4);
            sCorner = pyramid->getCorner(3);
            tCorner = pyramid->getCorner(2);
            isQuad = false;
            break;
        case 4:
            base    = pyramid->getCorner(4);
            sCorner = pyramid->getCorner(0);
            tCorner = pyramid->getCorner(3);
            isQuad = false;
            break;
        case 5:
            base    = pyramid->getCorner(0);
            sCorner = pyramid->getCorner(1);
            tCorner = pyramid->getCorner(3);
            isQuad = true;
            break;
        }
    }
    else
    {
        switch (incNodeNumber())
        {
        case 1:
            base    = pyramid->getCorner(0);
            sCorner = pyramid->getCorner(1);
            tCorner = pyramid->getCorner(4);
            isQuad = false;
            break;
        case 2:
            base    = pyramid->getCorner(1);
            sCorner = pyramid->getCorner(2);
            tCorner = pyramid->getCorner(4);
            isQuad = false;
            break;
        case 3:
            base    = pyramid->getCorner(2);
            sCorner = pyramid->getCorner(3);
            tCorner = pyramid->getCorner(4);
            isQuad = false;
            break;
        case 4:
            base    = pyramid->getCorner(3);
            sCorner = pyramid->getCorner(0);
            tCorner = pyramid->getCorner(4);
            isQuad = false;
            break;
        case 5:
            if ((pyramid->getPosition()[2] > 0.0f) || pyramid->getZFlip())
            {
                base    = pyramid->getCorner(0);
                sCorner = pyramid->getCorner(3);
                tCorner = pyramid->getCorner(1);
                isQuad = true;
            }
            else
                return NULL;
            break;
        }
    }

    const glm::vec3 sEdge = sCorner - base;
    const glm::vec3 tEdge = tCorner - base;

    if (isQuad == false)
        return new TriWallSceneNode(
                   base, sEdge, tEdge, uRepeats, vRepeats, lod);
    else
        return new QuadWallSceneNode(
                   base, sEdge, tEdge, uRepeats, vRepeats, lod);

}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
