/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* WallObstacle:
 *	Encapsulates an infinite wall in the game environment.
 */

#ifndef	BZF_WALL_OBSTACLE_H
#define	BZF_WALL_OBSTACLE_H

#include "common.h"
#include <string>
#include "Obstacle.h"

class WallObstacle : public Obstacle {
  public:
			WallObstacle();
			WallObstacle(const fvec3& pos, float rotation,
				     float breadth, float height, bool ricochet);
			~WallObstacle();

    const char*		getType() const;
    static const char*	getClassName(); // const

    float		intersect(const Ray&) const;
    void		getNormal(const fvec3& p, fvec3& n) const;

    bool		inCylinder(const fvec3& p, float radius, float height) const;
    bool		inBox(const fvec3& p, float angle,
			      float halfWidth, float halfBreadth, float height) const;
    bool		inMovingBox(const fvec3& oldP, float oldAngle,
				    const fvec3& newP, float newAngle,
				    float halfWidth, float halfBreadth, float height) const;

    bool		getHitNormal(
				const fvec3& pos1, float azimuth1,
				const fvec3& pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				fvec3& normal) const;

    int packSize() const;
    void *pack(void*) const;
    void *unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;
    virtual int getTypeID() const {return wallType;}

    std::string		userTextures[1];

  private:
    void finalize();

  private:
    float		plane[4];
    static const char*	typeName;
};

#endif // BZF_WALL_OBSTACLE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
