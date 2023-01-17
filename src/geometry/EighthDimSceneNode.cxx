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

// interface header
#include "EighthDimSceneNode.h"

// system headers
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glm/gtc/random.hpp>

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

void EighthDimSceneNode::setPolygon(int index, const glm::vec3 vertex[3])
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
    poly = new glm::vec3[numPolygons][3];

    // make random colors
    for (int i = 0; i < numPolygons; i++)
        color[i] = glm::linearRand(glm::vec4(0.2f),
                                   glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
}

EighthDimSceneNode::EighthDimRenderNode::~EighthDimRenderNode()
{
    delete[] color;
    delete[] poly;
}

void            EighthDimSceneNode::EighthDimRenderNode::render()
{
    // draw polygons
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numPolygons; i++)
    {
        myColor4fv(color[i]);
        glVertex3f(poly[i][0].x, poly[i][0].y, poly[i][0].z);
        glVertex3f(poly[i][2].x, poly[i][2].y, poly[i][2].z);
        glVertex3f(poly[i][1].x, poly[i][1].y, poly[i][1].z);
    }
    glEnd();
}

void            EighthDimSceneNode::EighthDimRenderNode::setPolygon(
    int index, const glm::vec3 vertex[3])
{
    for (int i = 0; i < 3; i++)
        poly[index][i] = vertex[i];
}

const glm::vec3 EighthDimSceneNode::EighthDimRenderNode::getPosition() const
{
    return sceneNode->getCenter();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
