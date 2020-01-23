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

// interface header
#include "MeshPolySceneNode.h"

// system headers
#include <assert.h>
#include <math.h>

// common implementation headers
#include "Intersect.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

// FIXME - no tesselation is done on for shot lighting


//
// MeshPolySceneNode::Geometry
//

MeshPolySceneNode::Geometry::Geometry(MeshPolySceneNode* _node,
                                      const GLfloat3Array& _vertices, const GLfloat3Array& _normals,
                                      const GLfloat2Array& _texcoords, const glm::vec3 *_normal) :
    vertices(_vertices), normals(_normals), texcoords(_texcoords)
{
    sceneNode = _node;
    normal = _normal;
    style = 0;
    return;
}


MeshPolySceneNode::Geometry::~Geometry()
{
    // do nothing
    return;
}

inline void MeshPolySceneNode::Geometry::drawV() const
{
    const int count = vertices.getSize();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < count; i++)
        glVertex3fv(vertices[i]);
    glEnd();
    return;
}


inline void MeshPolySceneNode::Geometry::drawVT() const
{
    const int count = vertices.getSize();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < count; i++)
    {
        glTexCoord2fv(texcoords[i]);
        glVertex3fv(vertices[i]);
    }
    glEnd();
    return;
}


inline void MeshPolySceneNode::Geometry::drawVN() const
{
    const int count = vertices.getSize();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < count; i++)
    {
        glNormal3fv(normals[i]);
        glVertex3fv(vertices[i]);
    }
    glEnd();
    return;
}


inline void MeshPolySceneNode::Geometry::drawVTN() const
{
    const int count = vertices.getSize();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < count; i++)
    {
        glTexCoord2fv(texcoords[i]);
        glNormal3fv(normals[i]);
        glVertex3fv(vertices[i]);
    }
    glEnd();
    return;
}


void MeshPolySceneNode::Geometry::render()
{
    sceneNode->setColor();

    if (normals.getSize() != 0)
    {
        if (style >= 2)
            drawVTN();
        else
            drawVN();
    }
    else
    {
        glNormal3f(normal->x, normal->y, normal->z);
        if (style >= 2)
            drawVT();
        else
            drawV();
    }

    addTriangleCount(vertices.getSize() - 2);
    return;
}


void MeshPolySceneNode::Geometry::renderRadar()
{
    drawV();
    addTriangleCount(vertices.getSize() - 2);
    return;
}


void MeshPolySceneNode::Geometry::renderShadow()
{
    drawV();
    addTriangleCount(vertices.getSize() - 2);
    return;
}


const glm::vec3 MeshPolySceneNode::Geometry::getPosition() const
{
    return sceneNode->getCenter();
}


//
// MeshPolySceneNode
//

MeshPolySceneNode::MeshPolySceneNode(const float _plane[4],
                                     bool _noRadar, bool _noShadow,
                                     const GLfloat3Array& vertices,
                                     const GLfloat3Array& normals,
                                     const GLfloat2Array& texcoords) :
    node(this, vertices, normals, texcoords, &normal)
{
    int i, j;
    const int count = vertices.getSize();
    assert(texcoords.getSize() == count);
    assert((normals.getSize() == 0) || (normals.getSize() == count));

    setPlane(glm::make_vec4(_plane));

    noRadar = _noRadar || (plane[2] <= 0.0f); // pre-cull if we can
    noShadow = _noShadow;

    // choose axis to ignore (the one with the largest normal component)
    int ignoreAxis;
    if (fabsf(normal[0]) > fabsf(normal[1]))
    {
        if (fabsf(normal[0]) > fabsf(normal[2]))
            ignoreAxis = 0;
        else
            ignoreAxis = 2;
    }
    else
    {
        if (fabsf(normal[1]) > fabsf(normal[2]))
            ignoreAxis = 1;
        else
            ignoreAxis = 2;
    }

    // project vertices onto plane
    GLfloat2Array flat(count);
    switch (ignoreAxis)
    {
    case 0:
        for (i = 0; i < count; i++)
        {
            flat[i][0] = vertices[i][1];
            flat[i][1] = vertices[i][2];
        }
        break;
    case 1:
        for (i = 0; i < count; i++)
        {
            flat[i][0] = vertices[i][2];
            flat[i][1] = vertices[i][0];
        }
        break;
    case 2:
        for (i = 0; i < count; i++)
        {
            flat[i][0] = vertices[i][0];
            flat[i][1] = vertices[i][1];
        }
        break;
    }

    // compute area of polygon
    float* area = new float[1];
    area[0] = 0.0f;
    for (j = count - 1, i = 0; i < count; j = i, i++)
        area[0] += flat[j][0] * flat[i][1] - flat[j][1] * flat[i][0];
    area[0] = 0.5f * fabsf(area[0]) / normal[ignoreAxis];

    // set lod info
    setNumLODs(1, area);

    // compute bounding sphere, put center at average of vertices
    auto mySphere = glm::vec4(0.0f);
    for (i = 0; i < count; i++)
    {
        mySphere[0] += vertices[i][0];
        mySphere[1] += vertices[i][1];
        mySphere[2] += vertices[i][2];
    }
    mySphere[0] /= (float)count;
    mySphere[1] /= (float)count;
    mySphere[2] /= (float)count;
    for (i = 0; i < count; i++)
    {
        const float dx = mySphere[0] - vertices[i][0];
        const float dy = mySphere[1] - vertices[i][1];
        const float dz = mySphere[2] - vertices[i][2];
        GLfloat r = ((dx * dx) + (dy * dy) + (dz * dz));
        if (r > mySphere[3])
            mySphere[3] = r;
    }
    setSphere(mySphere);

    // record extents info
    for (i = 0; i < count; i++)
        extents.expandToPoint(glm::make_vec3(vertices[i]));

    return;
}


MeshPolySceneNode::~MeshPolySceneNode()
{
    return;
}


bool MeshPolySceneNode::cull(const ViewFrustum& frustum) const
{
    // cull if eye is behind (or on) plane
    const auto eye = frustum.getEye();
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


bool MeshPolySceneNode::inAxisBox (const Extents& exts) const
{
    if (!extents.touches(exts))
        return false;

    return testPolygonInAxisBox (getVertexCount(), getVertices(), plane, exts);
}


const glm::vec3 MeshPolySceneNode::getVertex(int i) const
{
    return glm::make_vec3(node.getVertex(i));
}


void MeshPolySceneNode::addRenderNodes(SceneRenderer& renderer)
{
    node.setStyle(getStyle());
    const glm::vec4 *dyncol = getDynamicColor();
    if ((dyncol == NULL) || (dyncol->a != 0.0f))
        renderer.addRenderNode(&node, getWallGState());
    return;
}


void MeshPolySceneNode::addShadowNodes(SceneRenderer& renderer)
{
    if (!noShadow)
    {
        const glm::vec4 *dyncol = getDynamicColor();
        if ((dyncol == NULL) || (dyncol->a != 0.0f))
            renderer.addShadowNode(&node);
    }
    return;
}


void MeshPolySceneNode::renderRadar()
{
    if (!noRadar)
        node.renderRadar();
    return;
}


void MeshPolySceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
    RenderSet rs = { &node, getWallGState() };
    rnodes.push_back(rs);
    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
