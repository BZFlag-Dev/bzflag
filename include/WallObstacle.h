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

/* WallObstacle:
 *	Encapsulates an infinite wall in the game environment.
 */

#ifndef	BZF_WALL_OBSTACLE_H
#define	BZF_WALL_OBSTACLE_H

#include "common.h"
#include "Obstacle.h"

class WallObstacle : public Obstacle {
  public:
			WallObstacle(const float* pos, float rotation,
					float breadth, float height);
			~WallObstacle();

    std::string		getType() const;
    static std::string	getClassName(); // const

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
    bool		isInside(const float* p, float radius) const;
    bool		isInside(const float* p, float angle,
				float halfWidth, float halfBreadth) const;
    bool		isInside(const float* oldP, float oldAngle,
                         const float* p, float angle,
				         float halfWidth, float halfBreadth) const;
    bool		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				float* normal) const;

    std::string	        userTextures[1];

  private:
    float		plane[4];
    static std::string	typeName;
};

#endif // BZF_WALL_OBSTACLE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

