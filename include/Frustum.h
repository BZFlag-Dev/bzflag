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

/* Frustum
 * Encapsulates a camera.
 */

#ifndef BZF_FRUSTUM_H
#define BZF_FRUSTUM_H

// Before Everything
#include "common.h"

// System headers
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

// FIXME -- will need a means for off center projections for
// looking through teleporters

class Frustum
{
public:
    Frustum();
    ~Frustum();

    const float*    getEye() const;
    const float*    getDirection() const;
    const float*    getUp() const;
    const float*    getSide(int index) const;
    int         getPlaneCount() const;
    float       getNear() const;
    float       getFar() const;
    const float*    getViewMatrix() const;
    float       getFOVx() const;
    float       getFOVy() const;
    const float*    getProjectionMatrix() const;
    float       getAreaFactor() const;

    void        setView(const glm::vec3 &eye, const glm::vec3 &target);
    void        setProjection(float fov,
                              float m_near, float m_far, float m_deep_far,
                              int width, int height, int viewHeight);
    void        setOffset(float eyeOffset, float focalPlane);
    void        setFarPlaneCull(bool useCulling);
    void        flipVertical();
    void        flipHorizontal();

    // used for radar culling
    void        setOrthoPlanes(const Frustum& view,
                               float width, float breadth);

protected:
    void        makePlane(const glm::vec3 &v1, const glm::vec3 &v2, int);

protected:
    glm::vec3   eye;
    glm::vec3   target;
    glm::vec3   up;
    glm::vec4   plane[6];        // pointing in
    int         planeCount;
    float       tilt;
    float       rotation;
    glm::mat4   viewMatrix;
    glm::mat4   billboardMatrix;
    float       m_near, m_far;
    float       fovx, fovy;
    float       areaFactor;
    float       projectionMatrix[16];
    float       deepProjectionMatrix[16];
};

//
// Frustum
//

inline const float* Frustum::getEye() const
{
    return glm::value_ptr(eye);
}

inline const float* Frustum::getDirection() const
{
    return glm::value_ptr(plane[0]);
}

inline const float* Frustum::getSide(int index) const
{
    return glm::value_ptr(plane[index]);
}

inline int      Frustum::getPlaneCount() const
{
    return planeCount;
}

inline const float* Frustum::getUp() const
{
    return glm::value_ptr(up);
}

inline float        Frustum::getNear() const
{
    return m_near;
}

inline float        Frustum::getFar() const
{
    return m_far;
}

inline float        Frustum::getFOVx() const
{
    return fovx;
}

inline float        Frustum::getFOVy() const
{
    return fovy;
}

inline const float* Frustum::getViewMatrix() const
{
    return glm::value_ptr(viewMatrix);
}

inline const float* Frustum::getProjectionMatrix() const
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
