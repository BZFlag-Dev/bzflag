/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BaseBuilding:
 *	Encapsulates a base in the game environment
 */

#ifndef BZF_BASE_BUILDING_H
#define BZF_BASE_BUILDING_H

#include "AList.h"
#include "Obstacle.h"

class BaseBuilding : public Obstacle {
  public:
    			BaseBuilding(const float *pos, float rotation,
				const float *size, int _team);
			~BaseBuilding();
    BzfString		getType() const;
    BzfString		getClassName(); // const
    float		intersect(const Ray &) const;
    void		getNormal(const float *p, float *n) const;
    boolean		isInside(const float *p, float radius) const;
    boolean		isInside(const float *p, float angle,
				float halfWidth, float halfBreadth) const;
    boolean		isCrossing(const float *p, float angle,
				float halfWidth, float halfBreadth,
				float *plane) const;
    boolean		getHitNormal(const float *pos1, float azimuth1,
				const float *pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float *normal) const;
    ObstacleSceneNodeGenerator* newSceneNodeGenerator() const;
    void		getCorner(int index, float *pos) const;
    const int		getTeam() const;
  private:
    static BzfString	typeName;
    int team;
};

BZF_DEFINE_ALIST(BaseBuildings, BaseBuilding);

class BaseSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class BaseBuilding;
  			~BaseSceneNodeGenerator();
  public:
    WallSceneNode*	getNextNode(float, float, boolean);
  protected:
    			BaseSceneNodeGenerator(const BaseBuilding *);
  private:
    const BaseBuilding *base;
};

#endif
