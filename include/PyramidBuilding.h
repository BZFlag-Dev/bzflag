/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
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

#include "common.h"
#include "Obstacle.h"

class PyramidBuilding : public Obstacle {
  public:
			PyramidBuilding(const float* pos, float rotation,
				float width, float breadth, float height);
			~PyramidBuilding();

    std::string		getType() const;
    static std::string	getClassName(); // const

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
    bool		isInside(const float* p, float radius) const;
    bool		isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const;
    bool		isCrossing(const float* p, float angle,
				float halfWidth, float halfBreadth,
				float* plane) const;
    bool		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float* normal) const;

    ObstacleSceneNodeGenerator*	newSceneNodeGenerator() const;

    void		getCorner(int index, float* pos) const;

  protected:
    float		shrinkFactor(float z) const;

  private:
    static std::string	typeName;
};

class PyramidSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class PyramidBuilding;
  public:
			~PyramidSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, bool);

  protected:
			PyramidSceneNodeGenerator(const PyramidBuilding*);

  private:
    const PyramidBuilding*	pyramid;
};

#endif // BZF_PYRAMID_BUILDING_H
// ex: shiftwidth=2 tabstop=8
