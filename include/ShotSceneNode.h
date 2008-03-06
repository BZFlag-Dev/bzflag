/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* ShotSceneNode:
 *	Interface for shot scene nodes.
 */

#ifndef	BZF_SHOT_SCENE_NODE_H
#define	BZF_SHOT_SCENE_NODE_H

#include "common.h"
#include "SceneNode.h"

class ShotSceneNode : public SceneNode {
  public:
			ShotSceneNode() { }
			~ShotSceneNode() { }

    virtual void	move(const GLfloat pos[3], const GLfloat forward[3])=0;
};

#endif // BZF_SHOT_SCENE_NODE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

