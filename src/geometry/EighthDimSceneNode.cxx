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
#include "EighthDimSceneNode.h"

// system headers
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

// common implementation header
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

EighthDimSceneNode::EighthDimSceneNode(int numPolygons) :
    renderNode(this, numPolygons)
{
    // do nothing
}

EighthDimSceneNode::~EighthDimSceneNode()
{
    // do nothing
}

bool            EighthDimSceneNode::cull(const ViewFrustum&) const
{
    // no culling
    return false;
}

void            EighthDimSceneNode::notifyStyleChange()
{
    OpenGLGStateBuilder builder(gstate);
    builder.disableCulling();
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gstate = builder.getState();
}

void            EighthDimSceneNode::addRenderNodes(
    SceneRenderer& renderer)
{
    renderer.addRenderNode(&renderNode, &gstate);
}

void            EighthDimSceneNode::setPolygon(int index,
        const GLfloat vertex[3][3])
{
    renderNode.setPolygon(index, vertex);
}

//
// EighthDimSceneNode::EighthDimRenderNode
//

EighthDimSceneNode::EighthDimRenderNode::EighthDimRenderNode(
    const EighthDimSceneNode* _sceneNode,
    int numPolys) :
    sceneNode(_sceneNode),
    numPolygons(numPolys)
{
    color = new glm::vec4[numPolygons];
    poly = (glm::vec3(*)[3])new glm::vec3[3 * numPolygons];

    // make random colors
    for (int i = 0; i < numPolygons; i++)
    {
        color[i][0] = 0.2f + 0.8f * (float)bzfrand();
        color[i][1] = 0.2f + 0.8f * (float)bzfrand();
        color[i][2] = 0.2f + 0.8f * (float)bzfrand();
        color[i][3] = 0.2f + 0.6f * (float)bzfrand();
    }
    vboIndex = -1;
    vboManager.registerClient(this);
}

EighthDimSceneNode::EighthDimRenderNode::~EighthDimRenderNode()
{
    vboVC.vboFree(vboIndex);
    vboManager.unregisterClient(this);
    delete[] color;
    delete[] poly;
}

void EighthDimSceneNode::EighthDimRenderNode::initVBO()
{
    vboIndex = vboVC.vboAlloc(numPolygons * 3);
    fillVBO();
}

void EighthDimSceneNode::EighthDimRenderNode::fillVBO()
{
    std::vector<glm::vec4> col;
    std::vector<glm::vec3> vertex;

    col.reserve(numPolygons * 3);
    vertex.reserve(numPolygons * 3);

    for (int i = 0; i < numPolygons; i++)
    {
        for (int j = 3; j > 0; j--)
            col.push_back(color[i]);
        vertex.push_back(poly[i][0]);
        vertex.push_back(poly[i][2]);
        vertex.push_back(poly[i][1]);
    }

    vboVC.colorData(vboIndex, col);
    vboVC.vertexData(vboIndex, vertex);
}

void            EighthDimSceneNode::EighthDimRenderNode::render()
{
    // draw polygons
    if (!colorOverride)
        vboVC.enableArrays();
    else
        vboVC.enableArrays(false, false, false);
    glDrawArrays(GL_TRIANGLES, vboIndex, numPolygons * 3);
}

void            EighthDimSceneNode::EighthDimRenderNode::setPolygon(
    int index, const GLfloat vertex[3][3])
{
    ::memcpy(poly[index], vertex, sizeof(GLfloat[3][3]));
    if (index + 1 >= numPolygons)
        fillVBO();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
