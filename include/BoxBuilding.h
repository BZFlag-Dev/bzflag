/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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

#include "Obstacle.h"

class BoxBuilding : public Obstacle {
  public:
			BoxBuilding(const float* pos, float rotation,
				float width, float breadth, float height);
			~BoxBuilding();

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

  private:
    static std::string	typeName;
};

class BoxSceneNodeGenerator : public ObstacleSceneNodeGenerator {
  friend class BoxBuilding;
  public:
			~BoxSceneNodeGenerator();

    WallSceneNode*	getNextNode(float, float, bool);

  protected:
			BoxSceneNodeGenerator(const BoxBuilding*);

  private:
    const BoxBuilding*	box;
};

#endif // BZF_BOX_BUILDING_H
// ex: shiftwidth=2 tabstop=8
