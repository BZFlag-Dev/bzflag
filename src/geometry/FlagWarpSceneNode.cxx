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
#include "FlagWarpSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "VBO_Handler.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

const float     FlagWarpSize =  7.5;        // meters
const GLfloat       FlagWarpAlpha = 0.5f;
const GLfloat       FlagWarpSceneNode::color[7][3] =
{
    { 0.25, 1.0, 0.25 },
    { 0.25, 0.25, 1.0 },
    { 1.0, 0.0, 1.0 },
    { 1.0, 0.25, 0.25 },
    { 1.0, 0.5, 0.0 },
    { 1.0, 1.0, 0.0 },
    { 1.0, 1.0, 1.0 }
};

FlagWarpSceneNode::FlagWarpSceneNode(const GLfloat pos[3]) :
    renderNode(this)
{
    move(pos);
    setRadius(1.25f * FlagWarpSize * FlagWarpSize);
    size = 1.0f;
}

FlagWarpSceneNode::~FlagWarpSceneNode()
{
    // do nothing
}

void            FlagWarpSceneNode::setSizeFraction(GLfloat _size)
{
    size = _size;
}

void            FlagWarpSceneNode::move(const GLfloat pos[3])
{
    setCenter(pos);
}

GLfloat         FlagWarpSceneNode::getDistance(const GLfloat* eye) const
{
    // shift position of warp down a little because a flag and it's warp
    // are at the same position but we want the warp to appear below the
    // flag.
    const GLfloat* mySphere = getSphere();
    return (eye[0] - mySphere[0]) * (eye[0] - mySphere[0]) +
           (eye[1] - mySphere[1]) * (eye[1] - mySphere[1]) +
           (eye[2] - mySphere[2] + 0.2f) * (eye[2] - mySphere[2] + 0.2f);
}

void            FlagWarpSceneNode::notifyStyleChange()
{
    OpenGLGStateBuilder builder(gstate);
    {
        builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        builder.setStipple(1.0f);
    }
    gstate = builder.getState();
}

void            FlagWarpSceneNode::addRenderNodes(
    SceneRenderer& renderer)
{
    renderer.addRenderNode(&renderNode, &gstate);
}

//
// FlagWarpSceneNode::FlagWarpRenderNode
//

FlagWarpSceneNode::FlagWarpRenderNode::FlagWarpRenderNode(
    const FlagWarpSceneNode* _sceneNode) :
    sceneNode(_sceneNode)
{
    // do nothing
}

FlagWarpSceneNode::FlagWarpRenderNode::~FlagWarpRenderNode()
{
    // do nothing
}

void            FlagWarpSceneNode::FlagWarpRenderNode::render()
{
    // make a perturbed ring
    glm::vec3 geom[12];
    for (int i = 0; i < 12; i++)
    {
        const GLfloat r = FlagWarpSize * (0.9f + 0.2f * (float)bzfrand());
        geom[i].x = r * cosf((float)(2.0 * M_PI * double(i) / 12.0));
        geom[i].y = r * sinf((float)(2.0 * M_PI * double(i) / 12.0));
        geom[i].z = 0.0f;
    }

    const GLfloat* sphere = sceneNode->getSphere();
    glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);

    glm::vec3 vertex[14];
    bool up = sphere[2] > RENDERER.getViewFrustum().getEye()[2];

    vertex[0]  = glm::vec3(0.0f, 0.0f, 0.0f);
    vertex[1]  = geom[0];
    if (up)
    {
        vertex[2]  = geom[11];
        vertex[3]  = geom[10];
        vertex[4]  = geom[9];
        vertex[5]  = geom[8];
        vertex[6]  = geom[7];
        vertex[7]  = geom[6];
        vertex[8]  = geom[5];
        vertex[9]  = geom[4];
        vertex[10] = geom[3];
        vertex[11] = geom[2];
        vertex[12] = geom[1];
    }
    else
    {
        vertex[2]  = geom[1];
        vertex[3]  = geom[2];
        vertex[4]  = geom[3];
        vertex[5]  = geom[4];
        vertex[6]  = geom[5];
        vertex[7]  = geom[6];
        vertex[8]  = geom[7];
        vertex[9]  = geom[8];
        vertex[10] = geom[9];
        vertex[11] = geom[10];
        vertex[12] = geom[11];
    }
    vertex[13] = geom[0];

    int vboIndex = vboV.vboAlloc(14);
    vboV.vertexData(vboIndex, 14, vertex);
    vboV.enableArrays();
    for (int i = 0; i < 7; i++)
    {
        GLfloat s = sceneNode->size - 0.05f * float(i);
        if (s < 0.0f) break;
        myColor4f(color[i][0], color[i][1], color[i][2], FlagWarpAlpha);
        glPushMatrix();
        glScalef(s, s, 0);
        if (up)
            glDrawArrays(GL_TRIANGLE_FAN, vboIndex, 14);
        else
            glDrawArrays(GL_TRIANGLE_FAN, vboIndex, 14);
        glPopMatrix();
        // 14 verts -> 12 tris
        addTriangleCount(12);
        if (up)
            glTranslatef(0.0f, 0.0f, -0.01f);
        else
            glTranslatef(0.0f, 0.0f, 0.01f);
    }
    vboV.vboFree(vboIndex);

    glPopMatrix();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
