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

/* PyramidBuilding:
 *  Encapsulates a pyramid in the game environment.
 */

#ifndef BZF_PYRAMID_BUILDING_H
#define BZF_PYRAMID_BUILDING_H

// Inherits from
#include "Obstacle.h"

// System headers
#include <string>
#include <glm/fwd.hpp>

class PyramidBuilding final : public Obstacle
{
public:
    PyramidBuilding();
    PyramidBuilding(const float* pos, float rotation,
                    float width, float breadth, float height, bool drive = false, bool shoot = false, bool ricochet = false);
    ~PyramidBuilding();

    virtual Obstacle*   copyWithTransform(const MeshTransform&) const override;

    const char*     getType() const override;
    static const char*  getClassName(); // const

    bool        isFlatTop() const override;

    float       intersect(const Ray&) const override;
    void        getNormal(const float* p, float* n) const override;
    void        get3DNormal(const float* p, float* n) const override;

    bool        inCylinder(const float* p, float radius, float height) const override;
    bool        inBox(const float* p, float angle,
                      float halfWidth, float halfBreadth, float height) const override;
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
        float* normal) const override;

    glm::vec3 getCorner(int index) const;

    int packSize() const override;
    void *pack(void*) const override;
    const void *unpack(const void*) override;

    void print(std::ostream& out, const std::string& indent) const override;
    void printOBJ(std::ostream& out, const std::string& indent) const override;

    std::string     userTextures[1];

private:
    void finalize();

    // compute minimum shrinking for height between z and z + height
    float       shrinkFactor(float z, float height = 0.0) const;

private:
    static const char*  typeName;
};

#endif // BZF_PYRAMID_BUILDING_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
