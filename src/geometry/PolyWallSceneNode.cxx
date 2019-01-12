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

// bzflag common header
#include "common.h"

// interface header
#include "PolyWallSceneNode.h"

// system headers
#include <assert.h>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

//
// PolyWallSceneNode::Geometry
//

PolyWallSceneNode::Geometry::Geometry(PolyWallSceneNode* _wall,
                                      const GLfloat3Array& _vertex,
                                      const GLfloat2Array& _uv,
                                      const glm::vec3 _normal) :
    wall(_wall),
    normal(_normal),
    vertex(_vertex),
    uv(_uv)
{
    vboIndex = -1;
    vboManager.registerClient(this);
}

PolyWallSceneNode::Geometry::~Geometry()
{
    vboVT.vboFree(vboIndex);
    vboManager.unregisterClient(this);
}

void            PolyWallSceneNode::Geometry::render()
{
    wall->setColor();
    glNormal3f(normal.x, normal.y, normal.z);
    if (style >= 2)
        vboVT.enableArrays();
    else
        vboVT.enableArrays(false, false, false);
    glDrawArrays(GL_TRIANGLE_FAN, vboIndex, vertex.getSize());
    addTriangleCount(vertex.getSize() - 2);
    return;
}

void PolyWallSceneNode::Geometry::initVBO()
{
    std::vector<glm::vec2> textur;
    std::vector<glm::vec3> myVertex;

    const int count = vertex.getSize();
    textur.reserve(count);
    myVertex.reserve(count);

    for (int i = 0; i < count; i++)
    {
        textur.push_back(glm::make_vec2(uv[i]));
        myVertex.push_back(glm::make_vec3(vertex[i]));
    }

    vboIndex = vboVT.vboAlloc(count);
    vboVT.textureData(vboIndex, textur);
    vboVT.vertexData(vboIndex, myVertex);
}

//
// PolyWallSceneNode
//

PolyWallSceneNode::PolyWallSceneNode(const GLfloat3Array& vertex,
                                     const GLfloat2Array& uv)
{
    const int count = vertex.getSize();
    assert(uv.getSize() == count);

    // figure out plane (find non-colinear edges and get cross product)
    GLfloat uEdge[3], vEdge[3], myPlane[4];
    GLfloat uLen, vLen, nLen;
    uEdge[0] = vertex[0][0] - vertex[count - 1][0];
    uEdge[1] = vertex[0][1] - vertex[count - 1][1];
    uEdge[2] = vertex[0][2] - vertex[count - 1][2];
    uLen = uEdge[0] * uEdge[0] + uEdge[1] * uEdge[1] + uEdge[2] * uEdge[2];
    int i;
    for (i = 1; i < count; i++)
    {
        vEdge[0] = vertex[i][0] - vertex[i - 1][0];
        vEdge[1] = vertex[i][1] - vertex[i - 1][1];
        vEdge[2] = vertex[i][2] - vertex[i - 1][2];
        vLen = vEdge[0] * vEdge[0] + vEdge[1] * vEdge[1] + vEdge[2] * vEdge[2];
        myPlane[0] = uEdge[1] * vEdge[2] - uEdge[2] * vEdge[1];
        myPlane[1] = uEdge[2] * vEdge[0] - uEdge[0] * vEdge[2];
        myPlane[2] = uEdge[0] * vEdge[1] - uEdge[1] * vEdge[0];
        nLen = myPlane[0] * myPlane[0] + myPlane[1] * myPlane[1]
               + myPlane[2] * myPlane[2];
        if (nLen > 1.0e-5f * uLen * vLen) break;
        uEdge[0] = vEdge[0];
        uEdge[1] = vEdge[1];
        uEdge[2] = vEdge[2];
        uLen = vLen;
    }
    myPlane[3] = -(myPlane[0] * vertex[0][0] + myPlane[1] * vertex[0][1] +
                   myPlane[2] * vertex[0][2]);
    setPlane(myPlane);

    // choose axis to ignore (the one with the largest normal component)
    int ignoreAxis;
    const glm::vec3 normal = glm::make_vec3(getPlane());
    if (fabsf(normal.x) > fabsf(normal.y))
        if (fabsf(normal.x) > fabsf(normal.z))
            ignoreAxis = 0;
        else
            ignoreAxis = 2;
    else if (fabsf(normal.y) > fabsf(normal.z))
        ignoreAxis = 1;
    else
        ignoreAxis = 2;

    // project vertices onto plane
    GLfloat2Array flat(vertex.getSize());
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
    GLfloat mySphere[4];
    mySphere[0] = mySphere[1] = mySphere[2] = mySphere[3] = 0.0f;
    for (i = 0; i < count; i++)
    {
        mySphere[0] += vertex[i][0];
        mySphere[1] += vertex[i][1];
        mySphere[2] += vertex[i][2];
    }
    mySphere[0] /= (float)count;
    mySphere[1] /= (float)count;
    mySphere[2] /= (float)count;
    for (i = 0; i < count; i++)
    {
        GLfloat r = (mySphere[0] - vertex[i][0]) * (mySphere[0] - vertex[i][0]) +
                    (mySphere[1] - vertex[i][1]) * (mySphere[1] - vertex[i][1]) +
                    (mySphere[2] - vertex[i][2]) * (mySphere[2] - vertex[i][2]);
        if (r > mySphere[3]) mySphere[3] = r;
    }
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


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
