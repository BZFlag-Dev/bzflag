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

/* BaseBuilding:
 *	Encapsulates a base in the game environment
 */

#ifndef BZF_BASE_BUILDING_H
#define BZF_BASE_BUILDING_H

#include "common.h"
#include "Obstacle.h"

class BaseBuilding : public Obstacle {
  public:
			BaseBuilding(const float *pos, float rotation,
				const float *size, int _team);
			~BaseBuilding();
    std::string		getType() const;
    std::string		getClassName(); // const
    float		intersect(const Ray &) const;
    void		getNormal(const float *p, float *n) const;
    void		get3DNormal(const float* p, float* n) const;
    bool		isInside(const float *p, float radius) const;
    bool		isInside(const float *p, float angle,
				float halfWidth, float halfBreadth) const;
    bool		isInside(const float* oldP, float oldAngle,
				 const float* p, float angle,
				 float halfWidth, float halfBreadth) const;
    bool		isCrossing(const float *p, float angle,
				float halfWidth, float halfBreadth,
				float *plane) const;
    bool		getHitNormal(const float *pos1, float azimuth1,
				const float *pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				float *normal) const;
    ObstacleSceneNodeGenerator* newSceneNodeGenerator() const;
    void		getCorner(int index, float *pos) const;
    int			getTeam() const;
    std::string	        userTextures[2];
 private:
    static std::string	typeName;
    int team;
};

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

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

