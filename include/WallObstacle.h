/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WallObstacle:
 *	Encapsulates an infinite wall in the game environment.
 */

#ifndef	BZF_WALL_OBSTACLE_H
#define	BZF_WALL_OBSTACLE_H

#include "AList.h"
#include "Obstacle.h"

class WallObstacle : public Obstacle {
  public:
			WallObstacle(const float* pos, float rotation,
					float breadth, float height);
			~WallObstacle();

    BzfString		getType() const;
    static BzfString	getClassName(); // const

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
    boolean		isInside(const float* p, float radius) const;
    boolean		isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const;
    boolean		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float* normal) const;

    ObstacleSceneNodeGenerator*	newSceneNodeGenerator() const;

  private:
    float		plane[4];
    static BzfString	typeName;
};

BZF_DEFINE_ALIST(WallObstacles, WallObstacle);

class WallSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class WallObstacle;
  public:
			~WallSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, boolean);

  protected:
			WallSceneNodeGenerator(const WallObstacle*);

  private:
    const WallObstacle*	wall;
};

#endif // BZF_WALL_OBSTACLE_H
// ex: shiftwidth=2 tabstop=8
