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

/* ArcObstacle:
 *  Encapsulates an arc in the game environment.
 */

#ifndef BZF_ARC_OBSTACLE_H
#define BZF_ARC_OBSTACLE_H

// Inherits from
#include "Obstacle.h"

// System headers
#include <string>

// Common headers
#include "MeshObstacle.h"
#include "MeshTransform.h"
#include "BzMaterial.h"


class ArcObstacle final : public Obstacle
{
public:

    enum
    {
        Top,
        Bottom,
        Inside,
        Outside_,
        StartFace,
        EndFace,
        MaterialCount
    };

    ArcObstacle() = default;
    ArcObstacle(const MeshTransform& transform,
                const float* _pos, const float* _size,
                float _rotation, float _angle, float _ratio,
                const float _texsize[4], bool _useNormals,
                int _divisions, const BzMaterial* mats[MaterialCount],
                int physics, bool bounce, bool drive, bool shoot, bool ricochet);
    ~ArcObstacle() = default;

    Obstacle* copyWithTransform(const MeshTransform&) const override;

    MeshObstacle* makeMesh();

    const char* getType() const override;
    static const char* getClassName(); // const
    bool isValid() const override;
    bool isFlatTop() const override;

    int packSize() const override;
    void *pack(void*) const override;
    const void *unpack(const void*) override;

    void print(std::ostream& out, const std::string& indent) const override;

private:
    MeshObstacle* makePie(bool isCircle, float a, float r, float h,
                          float radius, float squish, float texsz[4]);
    MeshObstacle* makeRing(bool isCircle, float a, float r, float h,
                           float inrad, float outrad, float squish,
                           float texsz[4]);
private:
    static const char* typeName;

    MeshTransform transform;
    int divisions;
    float sweepAngle;
    float ratio;
    int phydrv;
    bool smoothBounce;
    bool useNormals;
    float texsize[4];
    const BzMaterial* materials[MaterialCount];
};


#endif // BZF_ARC_OBSTACLE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
