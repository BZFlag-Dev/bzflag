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

// Interface
#include "Frustum.h"

// System headers
#include <math.h>
#include <string.h>
#include <glm/gtc/matrix_access.hpp>


Frustum::Frustum():
    viewMatrix(1.0f), billboardMatrix(1.0f)
{
    auto defaultEye    = glm::vec3(0);
    auto defaultTarget = glm::vec3(0, 1, 0);
    static float identity[16] = { 1.0, 0.0, 0.0, 0.0,
                                  0.0, 1.0, 0.0, 0.0,
                                  0.0, 0.0, 1.0, 0.0,
                                  0.0, 0.0, 0.0, 1.0
                                };

    // initialize view and projection matrices to identity
    ::memcpy(projectionMatrix, identity, sizeof(projectionMatrix));
    ::memcpy(deepProjectionMatrix, identity, sizeof(deepProjectionMatrix));

    setProjection((float)(M_PI/4.0), 1.0f, 100.0f, 1000.0f, 1, 1, 1);
    setView(defaultEye, defaultTarget);
}


Frustum::~Frustum()
{
    // do nothing
}


void Frustum::setView(const glm::vec3 &_eye, const glm::vec3 &_target)
{
    // set eye and target points
    eye    = _eye;
    target = _target;

    viewMatrix = glm::lookAt(eye, target, glm::vec3(0, 0, 1.0f));


    // build billboard matrix.  billboard matrix performs rotation
    // so that polygons drawn in the xy plane face the camera.
    billboardMatrix = glm::transpose(viewMatrix);
    billboardMatrix[0][3] = 0;
    billboardMatrix[1][3] = 0;
    billboardMatrix[2][3] = 0;

    auto right =  glm::vec3(glm::row(viewMatrix, 0));
    up         =  glm::vec3(glm::row(viewMatrix, 1));
    auto z     = -glm::vec3(glm::row(viewMatrix, 2));

    // compute vectors of frustum edges
    const float xs = fabsf(1.0f / projectionMatrix[0]);
    const float ys = fabsf(1.0f / projectionMatrix[5]);
    glm::vec3 edge[4];
    edge[0] = z - xs * right - ys * up;
    edge[1] = z + xs * right - ys * up;
    edge[2] = z + xs * right + ys * up;
    edge[3] = z - xs * right + ys * up;

    // make frustum planes
    plane[0] = glm::vec4(z, -glm::dot(eye, z) - m_near);
    makePlane(edge[0], edge[3], 1);
    makePlane(edge[2], edge[1], 2);
    makePlane(edge[1], edge[0], 3);
    makePlane(edge[3], edge[2], 4);
    plane[5]    = -plane[0];
    plane[5].w += m_far;
}


glm::vec4 Frustum::eyeSpace(glm::vec4 eyePlane)
{
    return eyePlane * viewMatrixInv;
}


void Frustum::setFarPlaneCull(bool useCulling)
{
    // far clip plane
    if (useCulling)
        planeCount = 6;
    else
        planeCount = 5;
}


void Frustum::setProjection(float fov,
                            float _m_near, float _m_far, float m_deep_far,
                            int width, int height, int viewHeight)
{
    // do easy stuff
    m_near = _m_near;
    m_far = _m_far;
    fovx = fov;

    // clear the far plane culling here
    planeCount = 5;

    // compute projectionMatrix
    const float s = 1.0f / tanf(fov / 2.0f);
    const float fracHeight = 1.0f - float(viewHeight) / float(height);
    projectionMatrix[0] = s;
    projectionMatrix[5] = (1.0f - fracHeight) * s * float(width) / float(viewHeight);
    projectionMatrix[8] = 0.0f;
    projectionMatrix[9] = -fracHeight;
    projectionMatrix[10] = -(m_far + m_near) / (m_far - m_near);
    projectionMatrix[11] = -1.0f;
    projectionMatrix[12] = 0.0f;
    projectionMatrix[14] = -2.0f * m_far * m_near / (m_far - m_near);
    projectionMatrix[15] = 0.0f;

    deepProjectionMatrix[0] = projectionMatrix[0];
    deepProjectionMatrix[5] = projectionMatrix[5];
    deepProjectionMatrix[8] = projectionMatrix[8];
    deepProjectionMatrix[9] = projectionMatrix[9];
    deepProjectionMatrix[11] = projectionMatrix[11];
    deepProjectionMatrix[12] = projectionMatrix[12];
    deepProjectionMatrix[15] = projectionMatrix[15];
    deepProjectionMatrix[10] = -(m_deep_far + m_near) / (m_deep_far - m_near);
    deepProjectionMatrix[14] = -2.0f * m_deep_far * m_near / (m_deep_far - m_near);

    // get field of view in y direction
    fovy = 2.0f * atanf(1.0f / projectionMatrix[5]);

    // compute areaFactor
    areaFactor = 0.25f * s * float(height);
    areaFactor = (float)(M_PI * areaFactor * areaFactor);
}


void Frustum::setOffset(float eyeOffset, float focalPlane)
{
    projectionMatrix[12] = 0.5f * eyeOffset * projectionMatrix[0];
    projectionMatrix[8] = projectionMatrix[12] / focalPlane;
    deepProjectionMatrix[8] = projectionMatrix[8];
    deepProjectionMatrix[12] = projectionMatrix[12];
}


void Frustum::makePlane(const glm::vec3 &v1, const glm::vec3 &v2, int index)
{
    // get normal by crossing v1 and v2 and normalizing
    auto n = glm::normalize(glm::cross(v1, v2));
    plane[index] = glm::vec4(n, -glm::dot(eye, n));
}


// these next two functions should be more generic
// flipX, flipY, flipZ, all with and offset along the axis
void Frustum::flipVertical()
{
    eye[2] = -eye[2];
    target[2] = -target[2];
    setView(eye, target);
    projectionMatrix[5] = -projectionMatrix[5];
    deepProjectionMatrix[5] = -deepProjectionMatrix[5];

    return;
}


void Frustum::flipHorizontal()
{
    eye[0] = -eye[0];
    target[0] = -target[0];
    setView(eye, target);
    projectionMatrix[0] = -projectionMatrix[0];
    deepProjectionMatrix[0] = -deepProjectionMatrix[0];
    return;
}


// used for radar culling, not really a frustum
void Frustum::setOrthoPlanes(const Frustum& view, float width, float breadth)
{
    // setup the eye, and the clipping planes
    eye = glm::make_vec3(view.getEye());

    float front[2], left[2];
    const float* dir = view.getDirection();
    float len = (dir[0] * dir[0]) + (dir[1] * dir[1]);
    if (len != 0)
    {
        len = 1.0f / sqrtf(len);
        front[0] = dir[0] * len;
        front[1] = dir[1] * len;
    }
    else
    {
        front[0] = 1.0f;
        front[1] = 0.0f;
    }

    left[0] = -front[1];
    left[1] = +front[0];

    plane[1][0] = +left[0];
    plane[1][1] = +left[1];
    plane[1][3] = -((eye[0] * plane[1][0]) + (eye[1] * plane[1][1])) + width;

    plane[2][0] = -left[0];
    plane[2][1] = -left[1];
    plane[2][3] = -((eye[0] * plane[2][0]) + (eye[1] * plane[2][1])) + width;

    plane[3][0] = +front[0];
    plane[3][1] = +front[1];
    plane[3][3] = -((eye[0] * plane[3][0]) + (eye[1] * plane[3][1])) + breadth;

    plane[4][0] = -front[0];
    plane[4][1] = -front[1];
    plane[4][3] = -((eye[0] * plane[4][0]) + (eye[1] * plane[4][1])) + breadth;

    plane[1][2] = 0.0f;
    plane[2][2] = 0.0f;
    plane[3][2] = 0.0f;
    plane[4][2] = 0.0f;

    // disable the near and far planes
    plane[0][0] = plane[0][1] = 0.0f;
    plane[0][2] = 1.0f;
    plane[0][3] = -1.0e6;
    plane[5][0] = plane[0][1] = 0.0f;
    plane[5][2] = 1.0f;
    plane[5][3] = -1.0e6;

    planeCount = 5;

    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
