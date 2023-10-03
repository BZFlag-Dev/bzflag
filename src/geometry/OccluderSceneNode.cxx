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


OccluderSceneNode::OccluderSceneNode(const MeshFace* face) :
    plane(face->getPlane())
{
    int i;

    setOccluder(true);

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
    setCenter(pos);

    float maxRadiusSq = 0.0f;
    for (const auto& v : vertices)
    {
        float rSq = glm::distance2(pos, v);
        if (rSq > maxRadiusSq)
            maxRadiusSq = rSq;
    }
    setRadius(maxRadiusSq);

    return;
}


const glm::vec4 *OccluderSceneNode::getPlane() const
{
    return &plane;
}

bool OccluderSceneNode::cull(const ViewFrustum& frustum) const
{
    // cull if eye is behind (or on) plane
    const auto eye = glm::vec4(frustum.getEye(), 1.0f);
    if (glm::dot(eye, plane) <= 0.0f)
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
            vertices.data(),
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

const glm::vec3 &OccluderSceneNode::getVertex (int vertex) const
{
    return vertices[vertex];
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
