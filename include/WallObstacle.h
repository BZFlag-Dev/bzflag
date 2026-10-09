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

/* WallObstacle:
 *  Encapsulates an infinite wall in the game environment.
 */

#ifndef BZF_WALL_OBSTACLE_H
#define BZF_WALL_OBSTACLE_H

// Inherits from
#include "Obstacle.h"

// System headers
#include <string>

class WallObstacle final : public Obstacle
{
public:
    WallObstacle();
    WallObstacle(const float* pos, float rotation,
                 float breadth, float height, bool ricochet);
    ~WallObstacle();

    const char*     getType() const override;
    static const char*  getClassName(); // const

    float       intersect(const Ray&) const override;
    void        getNormal(const float* p, float* n) const override;

    bool        inCylinder(const float* p, float radius, float height) const override;
    bool        inBox(const float* p, float angle,
                      float halfWidth, float halfBreadth, float height) const override;
    bool        inMovingBox(const float* oldP, float oldAngle,
                            const float *newP, float newAngle,
                            float halfWidth, float halfBreadth, float height) const override;

    bool        getHitNormal(
        const float* pos1, float azimuth1,
        const float* pos2, float azimuth2,
        float halfWidth, float halfBreadth,
        float height,
        float* normal) const override;

    int packSize() const override;
    void *pack(void*) const override;
    const void *unpack(const void*) override;

    void print(std::ostream& out, const std::string& indent) const override;

    std::string     userTextures[1];

private:
    void finalize();

private:
    float       plane[4];
    static const char*  typeName;
};

#endif // BZF_WALL_OBSTACLE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
