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
    const char*		getType() const;
    static const char*	getClassName(); // const
    float		intersect(const Ray &) const;
    void		getNormal(const float *p, float *n) const;
    void		get3DNormal(const float* p, float* n) const;

    bool                inCylinder(const float* p, float radius, float height) const;
    bool                inBox(const float* p, float angle,
                              float halfWidth, float halfBreadth, float height) const;
    bool                inMovingBox(const float* oldP, float oldAngle,
                                    const float *newP, float newAngle,
                                    float halfWidth, float halfBreadth, float height) const;
    bool                isCrossing(const float* p, float angle,
                                   float halfWidth, float halfBreadth, float height,
                                   float* plane) const;
                                   
    bool		getHitNormal(const float *pos1, float azimuth1,
				const float *pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				float *normal) const;
    void		getCorner(int index, float *pos) const;
    int			getTeam() const;
    std::string	        userTextures[2];
 private:
    static const char*	typeName;
    int team;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

