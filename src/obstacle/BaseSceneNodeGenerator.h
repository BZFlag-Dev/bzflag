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

#ifndef __BASESCENENODEGENERATOR_H__
#define __BASESCENENODEGENERATOR_H__

#include "ObstacleSceneNodeGenerator.h"

class BaseSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class BaseBuilding;
			~BaseSceneNodeGenerator();
  public:
    WallSceneNode*	getNextNode(float, float, bool);
  protected:
			BaseSceneNodeGenerator(const BaseBuilding *);
  private:
    const BaseBuilding *base;
};

#endif