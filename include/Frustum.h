/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Frustum
 * Encapsulates a camera.
 */

#ifndef BZF_FRUSTUM_H
#define BZF_FRUSTUM_H

// Before everything
#include "common.h"

// System headers
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// FIXME -- will need a means for off center projections for
// looking through teleporters

class Frustum
{
public:
    Frustum();
    ~Frustum();

    const glm::vec3 &getEye() const;
    const glm::vec3 getDirection() const;
    const glm::vec4 &getViewPlane() const;
    const glm::vec3 &getUp() const;
    const glm::vec4 &getSide(int index) const;
    int         getPlaneCount() const;
    const glm::mat4 &getViewMatrix() const;
    float       getFOVx() const;
    float       getFOVy() const;
    const glm::mat4 &getProjectionMatrix() const;
    float       getAreaFactor() const;

    void        setView(const glm::vec3 &eye, const glm::vec3 &target);
    void        setProjection(float fov,
                              float m_near, float m_far, float m_deep_far,
                              int width, int height, int viewHeight);
    void        setOffset(float eyeOffset, float focalPlane);
    void        setFarPlaneCull(bool useCulling);
    void        flipVertical();

    // used for radar culling
    void        setOrthoPlanes(const Frustum& view,
                               float width, float breadth);

protected:
    glm::vec3   eye;
    glm::mat4   viewMatrix;
    glm::mat4   billboardMatrix;
    glm::mat4   projectionMatrix;
    glm::mat4   deepProjectionMatrix;
    float       m_near, m_far;
    float       fovx, fovy;
    float       areaFactor;

private:
    void        makePlane(const glm::vec3 &v1, const glm::vec3 &v2, int);

    glm::vec3   target;
    glm::vec3   up;
    glm::vec4   plane[6];        // pointing in
    int         planeCount;
};

//
// Frustum
//

inline const glm::vec3 &Frustum::getEye() const
{
    return eye;
}

inline const glm::vec3 Frustum::getDirection() const
{
    return glm::vec3(plane[0]);
}

inline const glm::vec4 &Frustum::getViewPlane() const
{
    return plane[0];
}

inline const glm::vec4 &Frustum::getSide(int index) const
{
    return plane[index];
}

inline int      Frustum::getPlaneCount() const
{
    return planeCount;
}

inline const glm::vec3 &Frustum::getUp() const
{
    return up;
}

inline float        Frustum::getFOVx() const
{
    return fovx;
}

inline float        Frustum::getFOVy() const
{
    return fovy;
}

inline const glm::mat4 &Frustum::getViewMatrix() const
{
    return viewMatrix;
}

inline const glm::mat4 &Frustum::getProjectionMatrix() const
{
    return projectionMatrix;
}

inline float        Frustum::getAreaFactor() const
{
    return areaFactor;
}

#endif // BZF_FRUSTUM_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
