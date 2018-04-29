/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* BoxBuilding:
 *	Encapsulates a box in the game environment.
 */

#ifndef	BZF_BOX_BUILDING_H
#define	BZF_BOX_BUILDING_H

#include "common.h"
#include <string>
#include "Obstacle.h"

class BoxBuilding : public Obstacle {
  public:
			BoxBuilding();
			BoxBuilding(const float* pos, float rotation,
					float width, float breadth, float height,
					bool drive = false, bool shoot = false,
					bool ricochet = false, bool invisible = false);
			~BoxBuilding();

    Obstacle*		copyWithTransform(const MeshTransform&) const;

    const char*		getType() const;
    static const char*	getClassName(); // const

    bool		isFlatTop() const;

    float		intersect(const Ray&) const;
    void		getNormal(const float* p, float* n) const;
    void		get3DNormal(const float* p, float* n) const;
    inline bool	 isInvisible() const;

    bool		inCylinder(const float* p, float radius, float height) const;
    bool		inBox(const float* p, float angle,
			      float halfWidth, float halfBreadth, float height) const;
    bool		inMovingBox(const float* oldP, float oldAngle,
				    const float *newP, float newAngle,
				    float halfWidth, float halfBreadth, float height) const;
    bool		isCrossing(const float* p, float angle,
				   float halfWidth, float halfBreadth, float height,
				   float* plane) const;

    bool		getHitNormal(
				const float* pos1, float azimuth1,
				const float* pos2, float azimuth2,
				float halfWidth, float halfBreadth,
				float height,
				float* normal) const;

    void		getCorner(int index, float* pos) const;

    int packSize() const;
    void *pack(void*) const;
    const void *unpack(const void*);

    void print(std::ostream& out, const std::string& indent) const;
    void printOBJ(std::ostream& out, const std::string& indent) const;

    std::string	userTextures[2];

  private:
    void finalize();

 private:
   static const char*	typeName;
   bool noNodes;
};


//
// BoxBuilding
//

bool BoxBuilding::isInvisible() const {
  return noNodes;
}


#endif // BZF_BOX_BUILDING_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
