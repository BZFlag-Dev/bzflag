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

/* BoxBuilding:
 *	Encapsulates a box in the game environment.
 */

#ifndef	BZF_BOX_BUILDING_H
#define	BZF_BOX_BUILDING_H

#include "AList.h"
#include "Obstacle.h"

class BoxBuilding : public Obstacle {
  public:
			BoxBuilding(const float* pos, float rotation,
				float width, float breadth, float height);
			~BoxBuilding();

    BzfString		getType() const;
    static BzfString	getClassName(); // const

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
    boolean		isInside(const float* p, float radius) const;
    boolean		isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const;
    boolean		isCrossing(const float* p, float angle,
				float halfWidth, float halfBreadth,
				float* plane) const;
    boolean		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float* normal) const;

    ObstacleSceneNodeGenerator*	newSceneNodeGenerator() const;

    void		getCorner(int index, float* pos) const;

  private:
    static BzfString	typeName;
};

BZF_DEFINE_ALIST(BoxBuildings, BoxBuilding);

class BoxSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class BoxBuilding;
  public:
			~BoxSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, boolean);

  protected:
			BoxSceneNodeGenerator(const BoxBuilding*);

  private:
    const BoxBuilding*	box;
};

#endif // BZF_BOX_BUILDING_H
