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

#ifndef __TELEPORTERSCENENODEGENERATOR_H__
#define __TELEPORTERSCENENODEGENERATOR_H__

#include "ObstacleSceneNodeGenerator.h"

class TeleporterSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class Teleporter;
  public:
			~TeleporterSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, bool);

  protected:
			TeleporterSceneNodeGenerator(const Teleporter*);

  private:
    const Teleporter*	teleporter;
};

#endif
