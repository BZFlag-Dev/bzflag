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

#ifndef __BASESCENENODEGENERATOR_H__
#define __BASESCENENODEGENERATOR_H__

#include "ObstacleSceneNodeGenerator.h"
#include "BaseBuilding.h"

class BaseSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class SceneDatabaseBuilder;
			~BaseSceneNodeGenerator();
  public:
    WallSceneNode*	getNextNode(float, float, bool);
  protected:
			BaseSceneNodeGenerator(const BaseBuilding *);
  private:
    const BaseBuilding *base;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
