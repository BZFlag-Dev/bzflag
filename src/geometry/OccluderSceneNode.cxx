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


// interface header
#include "OccluderSceneNode.h"

// system headers
#include <math.h>
#include <string.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

// common implementation headers
#include "MeshFace.h"
#include "Intersect.h"
#include "ViewFrustum.h"


OccluderSceneNode::OccluderSceneNode(const MeshFace* face)
{
    int i;

    setOccluder(true);

    // record plane info
    memcpy(plane, face->getPlane(), sizeof(float[4]));

    // record extents info
    extents = face->getExtents();

    // record vertex info
    const int vertexCount = face->getVertexCount();
    vertices.resize(vertexCount);
    for (i = 0; i < vertexCount; i++)
        vertices[i] = face->getVertex(i);

    // record sphere info
    glm::vec3 pos(0.0f);
    for (const auto&v : vertices)
        pos += v;
    if (vertexCount > 0)
        pos /= static_cast<float>(vertexCount);

    float maxRadiusSq = 0.0f;
    for (const auto& v : vertices)
    {
        float rSq = glm::distance2(pos, v);
        if (rSq > maxRadiusSq)
            maxRadiusSq = rSq;
    }
    GLfloat mySphere[4] = { pos.x, pos.y, pos.z, maxRadiusSq };
    setSphere(mySphere);

    return;
}


const GLfloat* OccluderSceneNode::getPlane() const
{
    return plane;
}

bool OccluderSceneNode::cull(const ViewFrustum& frustum) const
{
    // cull if eye is behind (or on) plane
    const GLfloat* eye = frustum.getEye();
    if (((eye[0] * plane[0]) + (eye[1] * plane[1]) + (eye[2] * plane[2]) +
            plane[3]) <= 0.0f)
        return true;

    // if the Visibility culler tells us that we're
    // fully visible, then skip the rest of these tests
    if (octreeState == OctreeVisible)
        return false;

    const Frustum* f = (const Frustum *) &frustum;
    if (testAxisBoxInFrustum(extents, f) == Outside)
        return true;

    // probably visible
    return false;
}


bool OccluderSceneNode::inAxisBox (const Extents& exts) const
{
    if (!extents.touches(exts))
        return false;

    return testPolygonInAxisBox(
            static_cast<int>(vertices.size()),
            reinterpret_cast<const float (*)[3]>(vertices.data()),
            plane,
            exts);
}

void OccluderSceneNode::addShadowNodes(SceneRenderer&)
{
    return;
}

void OccluderSceneNode::addRenderNodes(SceneRenderer&)
{
    return;
}

void OccluderSceneNode::renderRadar()
{
    return;
}

int OccluderSceneNode::getVertexCount () const
{
    return vertices.size();
}

const GLfloat* OccluderSceneNode::getVertex (int vertex) const
{
    return glm::value_ptr(vertices[vertex]);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
