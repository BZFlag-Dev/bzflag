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

#ifndef __WALLSCENENODEGENERATOR_H__
#define __WALLSCENENODEGENERATOR_H__

#include "ObstacleSceneNodeGenerator.h"
#include "WallObstacle.h"

class WallSceneNodeGenerator : public ObstacleSceneNodeGenerator
{
    friend class SceneDatabaseBuilder;

public:
    ~WallSceneNodeGenerator();

    WallSceneNode*  getNextNode(float, float, bool);

protected:
    WallSceneNodeGenerator(const WallObstacle*);

private:
    const WallObstacle* wall;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
