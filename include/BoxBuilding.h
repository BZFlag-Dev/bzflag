/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
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

// Inherits from
#include "Obstacle.h"

// System headers
#include <string>
#include <glm/fwd.hpp>

class BoxBuilding : public Obstacle
{
public:
    BoxBuilding();
    BoxBuilding(const float* pos, float rotation,
                float width, float breadth, float height,
                bool drive = false, bool shoot = false,
                bool ricochet = false, bool invisible = false);
    ~BoxBuilding();

    Obstacle*       copyWithTransform(const MeshTransform&) const override;

    const char*     getType() const override;
    static const char*  getClassName(); // const

    bool        isFlatTop() const override final;

    float       intersect(const Ray&) const override final;
    void        getNormal(const float* p, float* n) const override final;
    void        get3DNormal(const float* p, float* n) const override final;
    inline bool  isInvisible() const;

    bool        inCylinder(const float* p, float radius, float height) const override;
    bool        inBox(const float* p, float angle,
                      float halfWidth, float halfBreadth, float height) const override final;
    bool        inMovingBox(const float* oldP, float oldAngle,
                            const float *newP, float newAngle,
                            float halfWidth, float halfBreadth, float height) const override;
    bool        isCrossing(const float* p, float angle,
                           float halfWidth, float halfBreadth, float height,
                           float* plane) const override;

    bool        getHitNormal(
        const float* pos1, float azimuth1,
        const float* pos2, float azimuth2,
        float halfWidth, float halfBreadth,
        float height,
        float* normal) const override final;

    glm::vec3 getCorner(int index) const;

    int packSize() const override;
    void *pack(void*) const override;
    const void *unpack(const void*) override;

    void print(std::ostream& out, const std::string& indent) const override;
    void printOBJ(std::ostream& out, const std::string& indent) const override;

    std::string userTextures[2];

protected:
    void finalize();

private:
    static const char*   typeName;
    bool noNodes;
};


//
// BoxBuilding
//

bool BoxBuilding::isInvisible() const
{
    return noNodes;
}


#endif // BZF_BOX_BUILDING_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
