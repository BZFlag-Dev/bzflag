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

/* ArcObstacle:
 *  Encapsulates an arc in the game environment.
 */

#ifndef BZF_ARC_OBSTACLE_H
#define BZF_ARC_OBSTACLE_H

#include "common.h"
#include <string>
#include "obstacle/Obstacle.h"
#include "obstacle/MeshObstacle.h"
#include "game/MeshTransform.h"
#include "game/BzMaterial.h"


class ArcObstacle : public Obstacle {
  public:

    enum {
      Top,
      Bottom,
      Inside,
      Outside,
      StartFace,
      EndFace,
      MaterialCount
    };

    ArcObstacle();
    ArcObstacle(const MeshTransform& transform,
                const fvec3& _pos, const fvec3& _size,
                float _rotation, float _angle, float _ratio,
                const float _texsize[4], bool _useNormals,
                int _divisions, const BzMaterial* mats[MaterialCount],
                int physics, bool bounce,
                unsigned char drive, unsigned char shoot, bool ricochet);
    ~ArcObstacle();

    Obstacle* copyWithTransform(const MeshTransform&) const;

    MeshObstacle* makeMesh();

    const char*  getType() const;
    ObstacleType getTypeID() const { return arcType; }

    static const char* getClassName(); // const

    bool isValid() const;
    bool isFlatTop() const;

    float intersect(const Ray&) const;
    void getNormal(const fvec3& p, fvec3& n) const;
    void get3DNormal(const fvec3& p, fvec3& n) const;
    bool getHitNormal(const fvec3& pos1, float azimuth1,
                      const fvec3& pos2, float azimuth2,
                      float halfWidth, float halfBreadth,
                      float height, fvec3& normal) const;
    bool inCylinder(const fvec3& p, float radius, float height) const;
    bool inBox(const fvec3& p, float angle,
               float halfWidth, float halfBreadth, float height) const;
    bool inMovingBox(const fvec3& oldP, float oldAngle,
                     const fvec3& newP, float newAngle,
                     float halfWidth, float halfBreadth, float height) const;
    bool isCrossing(const fvec3& p, float angle,
                    float halfWidth, float halfBreadth, float height,
                    fvec4* plane) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    void finalize();
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
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
