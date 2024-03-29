/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __EMPTYSCENENODEGENERATOR_H__
#define __EMPTYSCENENODEGENERATOR_H__

#include "ObstacleSceneNodeGenerator.h"

class EmptySceneNodeGenerator : public ObstacleSceneNodeGenerator
{
public:
    virtual     ~EmptySceneNodeGenerator();

    virtual WallSceneNode* getNextNode(float uRepeats, float vRepeats,
                                       bool lod);
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
