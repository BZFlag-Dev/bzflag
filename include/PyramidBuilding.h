/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* PyramidBuilding:
 *	Encapsulates a pyramid in the game environment.
 */

#ifndef	BZF_PYRAMID_BUILDING_H
#define	BZF_PYRAMID_BUILDING_H

#include "AList.h"
#include "Obstacle.h"

class PyramidBuilding : public Obstacle {
  public:
			PyramidBuilding(const float* pos, float rotation,
				float width, float breadth, float height);
			~PyramidBuilding();

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

  protected:
    float		shrinkFactor(float z) const;

  private:
    static BzfString	typeName;
};

BZF_DEFINE_ALIST(PyramidBuildings, PyramidBuilding);

class PyramidSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class PyramidBuilding;
  public:
			~PyramidSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, boolean);

  protected:
			PyramidSceneNodeGenerator(const PyramidBuilding*);

  private:
    const PyramidBuilding*	pyramid;
};

#endif // BZF_PYRAMID_BUILDING_H
