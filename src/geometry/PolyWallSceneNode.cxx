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
#include "PolyWallSceneNode.h"

// system headers
#include <assert.h>
#include <math.h>

#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

// Common headers
#include "OpenGLAPI.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

//
// PolyWallSceneNode::Geometry
//

PolyWallSceneNode::Geometry::Geometry(PolyWallSceneNode* _wall,
                                      const std::vector<glm::vec3>& _vertex,
                                      const std::vector<glm::vec2>& _uv,
                                      const GLfloat* _normal) :
    wall(_wall),
    normal(_normal),
    vertex(_vertex),
    uv(_uv)
{
    // do nothing
}

PolyWallSceneNode::Geometry::~Geometry()
{
    // do nothing
}

const GLfloat* PolyWallSceneNode::Geometry::getPosition() const
{
    return wall->getSphere();
}

void            PolyWallSceneNode::Geometry::render()
{
    wall->setColor();
    glNormal3fv(normal);
    if (style >= 2)
        drawVT();
    else
        drawV();
    addTriangleCount(static_cast<int>(vertex.size()) - 2);
    return;
}

void            PolyWallSceneNode::Geometry::drawV() const
{
    glBegin(GL_TRIANGLE_FAN);
    for (const auto &v : vertex)
        glVertex(v);
    glEnd();
}

void            PolyWallSceneNode::Geometry::drawVT() const
{
    const int count = vertex.size();
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < count; i++)
    {
        glTexCoord(uv[i]);
        glVertex(vertex[i]);
    }
    glEnd();
}

//
// PolyWallSceneNode
//

PolyWallSceneNode::PolyWallSceneNode(const std::vector<glm::vec3>& vertex,
                                     const std::vector<glm::vec2>& uv)
{
    const int count = vertex.size();
    assert(uv.size() == count);

    // figure out plane (find non-colinear edges and get cross product)
    int i;
    glm::vec3 uEdge = vertex[0] - vertex[count - 1];
    float uLen = glm::length2(uEdge);
    glm::vec3 norm(0.0f);
    for (i = 1; i < count; i++)
    {
        glm::vec3 vEdge = vertex[i] - vertex[i - 1];
        float vLen = glm::length2(vEdge);
        norm = glm::cross(uEdge, vEdge);
        float nLen = glm::length2(norm);
        if (nLen > 1.0e-5f * uLen * vLen) break;
        uEdge = vEdge;
        uLen = vLen;
    }
    GLfloat myPlane[4];
    myPlane[0] = norm.x;
    myPlane[1] = norm.y;
    myPlane[2] = norm.z;
    myPlane[3] = -glm::dot(norm, vertex[0]);
    setPlane(myPlane);

    // choose axis to ignore (the one with the largest normal component)
    int ignoreAxis;
    const auto normal = plane;
    if (fabsf(normal[0]) > fabsf(normal[1]))
        if (fabsf(normal[0]) > fabsf(normal[2]))
            ignoreAxis = 0;
        else
            ignoreAxis = 2;
    else if (fabsf(normal[1]) > fabsf(normal[2]))
        ignoreAxis = 1;
    else
        ignoreAxis = 2;

    // project vertices onto plane
    std::vector<glm::vec2> flat(count);
    switch (ignoreAxis)
    {
    case 0:
        for (i = 0; i < count; i++)
        {
            flat[i][0] = vertex[i][1];
            flat[i][1] = vertex[i][2];
        }
        break;
    case 1:
        for (i = 0; i < count; i++)
        {
            flat[i][0] = vertex[i][2];
            flat[i][1] = vertex[i][0];
        }
        break;
    case 2:
        for (i = 0; i < count; i++)
        {
            flat[i][0] = vertex[i][0];
            flat[i][1] = vertex[i][1];
        }
        break;
    }

    // compute area of polygon
    float* area = new float[1];
    area[0] = 0.0f;
    int j;
    for (j = count - 1, i = 0; i < count; j = i, i++)
        area[0] += flat[j][0] * flat[i][1] - flat[j][1] * flat[i][0];
    area[0] = 0.5f * fabsf(area[0]) / normal[ignoreAxis];
    node = new Geometry(this, vertex, uv, normal);
    shadowNode = new Geometry(this, vertex, uv, normal);
    shadowNode->setStyle(0);

    // set lod info
    setNumLODs(1, area);

    // compute bounding sphere, put center at average of vertices
    glm::vec3 center(0.0f);
    for (i = 0; i < count; i++)
        center += vertex[i];
    center /= static_cast<float>(count);

    float radiusSq = 0.0f;
    for (i = 0; i < count; i++)
    {
        float r = glm::distance2(center, vertex[i]);
        if (r > radiusSq) radiusSq = r;
    }

    GLfloat mySphere[4] = { center.x, center.y, center.z, radiusSq };
    setSphere(mySphere);
}

PolyWallSceneNode::~PolyWallSceneNode()
{
    delete node;
    delete shadowNode;
}

int         PolyWallSceneNode::split(const float* _plane,
                                     SceneNode*& front, SceneNode*& back) const
{
    return WallSceneNode::splitWall(_plane, node->vertex, node->uv, front, back);
}

void            PolyWallSceneNode::addRenderNodes(
    SceneRenderer& renderer)
{
    node->setStyle(getStyle());
    renderer.addRenderNode(node, getWallGState());
}

void            PolyWallSceneNode::addShadowNodes(
    SceneRenderer& renderer)
{
    renderer.addShadowNode(shadowNode);
}


void PolyWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
    RenderSet rs = { node, getWallGState() };
    rnodes.push_back(rs);
    return;
}


void PolyWallSceneNode::renderRadar()
{
    if (plane[2] > 0.0f)
        node->renderRadar();
    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
