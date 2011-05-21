/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
 *  Encapsulates a box in the game environment.
 */

#ifndef BZF_BOX_BUILDING_H
#define BZF_BOX_BUILDING_H

#include "common.h"
#include <string>
#include "Obstacle.h"

class BoxBuilding : public Obstacle {
  public:
    BoxBuilding();
    BoxBuilding(const fvec3& pos, float rotation,
                float width, float breadth, float height,
                unsigned char drive, unsigned char shoot, bool ricochet,
                bool invisible);
    ~BoxBuilding();

    Obstacle*   copyWithTransform(const MeshTransform&) const;

    const char*   getType() const;
    ObstacleType  getTypeID() const { return boxType; }

    static const char*  getClassName(); // const

    bool    isFlatTop() const;

    float   intersect(const Ray&) const;
    void    getNormal(const fvec3& p, fvec3& n) const;
    void    get3DNormal(const fvec3& p, fvec3& n) const;
    inline bool  isInvisible() const;

    bool    inCylinder(const fvec3& p, float radius, float height) const;
    bool    inBox(const fvec3& p, float angle,
                  float halfWidth, float halfBreadth, float height) const;
    bool    inMovingBox(const fvec3& oldP, float oldAngle,
                        const fvec3& newP, float newAngle,
                        float halfWidth, float halfBreadth, float height) const;
    bool    isCrossing(const fvec3& p, float angle,
                       float halfWidth, float halfBreadth, float height,
                       fvec4* plane) const;

    bool    getHitNormal(
      const fvec3& pos1, float azimuth1,
      const fvec3& pos2, float azimuth2,
      float halfWidth, float halfBreadth,
      float height,
      fvec3& normal) const;

    void    getCorner(int index, fvec3& pos) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;
    void printOBJ(std::ostream& out, const std::string& indent) const;

  private:
    void finalize();

  private:
    static const char* typeName;
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
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
