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

/* PyramidBuilding:
 *	Encapsulates a pyramid in the game environment.
 */

#ifndef	BZF_PYRAMID_BUILDING_H
#define	BZF_PYRAMID_BUILDING_H

#include "common.h"
#include <string>
#include "Obstacle.h"

class PyramidBuilding : public Obstacle {
  public:
			PyramidBuilding(const float* pos, float rotation,
				float width, float breadth, float height, bool drive = false, bool shoot = false);
			~PyramidBuilding();

    const char*		getType() const;
    static const char*	getClassName(); // const

    bool		isFlatTop() const;

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
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

    bool		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				float* normal) const;

    void		getCorner(int index, float* pos) const;

    std::string	        userTextures[1];

  protected:
    // compute minimum shrinking for height between z and z + height
    float		shrinkFactor(float z, float height = 0.0) const;

  private:
    static const char*	typeName;
};

#endif // BZF_PYRAMID_BUILDING_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

