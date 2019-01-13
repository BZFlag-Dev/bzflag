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
#include "LaserSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "VBO_Handler.h"
#include "VBO_Drawing.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

const GLfloat       LaserRadius = 0.1f;

LaserSceneNode::LaserSceneNode(const GLfloat pos[3], const GLfloat forward[3]) :
    renderNode(this)
{
    // prepare rendering info
    azimuth = (float)(180.0 / M_PI*atan2f(forward[1], forward[0]));
    elevation = (float)(-180.0 / M_PI*atan2f(forward[2], hypotf(forward[0],forward[1])));
    length = hypotf(forward[0], hypotf(forward[1], forward[2]));

    // setup sphere
    setCenter(pos);
    setRadius(length * length);

    OpenGLGStateBuilder builder(gstate);
    builder.disableCulling();
    gstate = builder.getState();

    first = false;
    setColor(1,1,1);
    setCenterColor(1,1,1);
}

void LaserSceneNode::setColor(float r, float g, float b)
{
    color = glm::vec4(r, g, b, 1.0f);
}


void LaserSceneNode::setCenterColor(float r, float g, float b)
{
    centerColor = glm::vec4(r, g, b, 1.0f);
}

LaserSceneNode::~LaserSceneNode()
{
    // do nothing
}

void            LaserSceneNode::setTexture(const int texture)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setTexture(texture);
    builder.enableTexture(texture>=0);
    gstate = builder.getState();
}

bool            LaserSceneNode::cull(const ViewFrustum&) const
{
    // no culling
    return false;
}

void            LaserSceneNode::notifyStyleChange()
{
    OpenGLGStateBuilder builder(gstate);
    builder.enableTexture(true);
    {
        // add in contribution from laser
        builder.setBlending(GL_SRC_ALPHA, GL_ONE);
        builder.setSmoothing(BZDB.isTrue("smooth"));
    }
    gstate = builder.getState();
}

void            LaserSceneNode::addRenderNodes(
    SceneRenderer& renderer)
{
    renderer.addRenderNode(&renderNode, &gstate);
}

//
// LaserSceneNode::LaserRenderNode
//

GLfloat         LaserSceneNode::LaserRenderNode::geom[6][2];

LaserSceneNode::LaserRenderNode::LaserRenderNode(
    const LaserSceneNode* _sceneNode) :
    sceneNode(_sceneNode)
{
    // initialize geometry if first instance
    static bool init = false;
    if (!init)
    {
        init = true;
        for (int i = 0; i < 6; i++)
        {
            geom[i][0] = -LaserRadius * cosf((float)(2.0 * M_PI * double(i) / 6.0));
            geom[i][1] =  LaserRadius * sinf((float)(2.0 * M_PI * double(i) / 6.0));
        }
    }
}

LaserSceneNode::LaserRenderNode::~LaserRenderNode()
{
    // do nothing
}

void LaserSceneNode::LaserRenderNode::render()
{
    const bool blackFog = RENDERER.isFogActive();
    if (blackFog)
        glFogfv(GL_FOG_COLOR, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));

    if (RENDERER.useQuality() >= 1)
        renderGeoLaser();
    else
        renderFlatLaser();

    if (blackFog)
        glFogfv(GL_FOG_COLOR, glm::value_ptr(RENDERER.getFogColor()));
}

void LaserSceneNode::LaserRenderNode::renderGeoLaser()
{
    const float len = sceneNode->length;
    const GLfloat* sphere = sceneNode->getSphere();
    glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);

    glm::vec4 coreColor = sceneNode->centerColor;
    coreColor.a     = 0.85f;
    glm::vec4 mainColor = sceneNode->color;
    mainColor.a     = 0.125f;

    myColor4f(coreColor.r, coreColor.g, coreColor.b, coreColor.a);
    glPushMatrix();
    glScalef(0.0625f, 0.0625f, len);
    DRAWER.cylinder10();
    glPopMatrix();
    addTriangleCount(20);

    myColor4f(mainColor.r, mainColor.g, mainColor.b, mainColor.a);
    glPushMatrix();
    glScalef(0.1f, 0.1f, len);
    DRAWER.cylinder16();
    glPopMatrix();
    addTriangleCount(32);

    glPushMatrix();
    glScalef(0.2f, 0.2f, len);
    DRAWER.cylinder24();
    glPopMatrix();
    addTriangleCount(48);

    glPushMatrix();
    glScalef(0.4f, 0.4f, len);
    DRAWER.cylinder32();
    glPopMatrix();
    addTriangleCount(64);

    glPushMatrix();
    glScalef(0.5f, 0.5f, 0.5f);
    if (sceneNode->first)
    {
        DRAWER.sphere32();
        addTriangleCount(32 * 32 * 2);
    }
    else
    {
        DRAWER.sphere12();
        addTriangleCount(12 * 12 * 2);
    }
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}


void LaserSceneNode::LaserRenderNode::renderFlatLaser()
{
    const float len = sceneNode->length;
    const GLfloat *sphere = sceneNode->getSphere();
    glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);

    {
        int vboIndex = vboVT.vboAlloc(14);
        glm::vec2 textur[14];
        glm::vec3 vertex[14];

        myColor3f(1.0f, 1.0f, 1.0f);
        textur[0] = glm::vec2(0.5f,  0.5f);
        vertex[0] = glm::vec3(0.0f,  0.0f,  0.0f);
        textur[1] = glm::vec2(0.0f,  0.0f);
        vertex[1] = glm::vec3(0.0f,  0.0f,  1.0f);
        textur[2] = glm::vec2(0.0f,  0.0f);
        vertex[2] = glm::vec3(0.0f,  1.0f,  0.0f);
        textur[3] = glm::vec2(0.0f,  0.0f);
        vertex[3] = glm::vec3(0.0f,  0.0f, -1.0f);
        textur[4] = glm::vec2(0.0f,  0.0f);
        vertex[4] = glm::vec3(0.0f, -1.0f,  0.0f);
        textur[5] = glm::vec2(0.0f,  0.0f);
        vertex[5] = glm::vec3(0.0f,  0.0f,  1.0f);

        textur[6] = glm::vec2(0.0f,  0.0f);
        vertex[6] = glm::vec3(0.0f,  0.0f,  1.0f);
        textur[7] = glm::vec2(0.0f,  1.0f);
        vertex[7] = glm::vec3(len,  0.0f,  1.0f);
        textur[8] = glm::vec2(1.0f,  0.0f);
        vertex[8] = glm::vec3(0.0f,  0.0f, -1.0f);
        textur[9] = glm::vec2(1.0f,  1.0f);
        vertex[9] = glm::vec3(len,  0.0f, -1.0f);

        textur[10] = glm::vec2(0.0f,  0.0f);
        vertex[10] = glm::vec3(  0.0f,  1.0f,  0.0f);
        textur[11] = glm::vec2(0.0f,  1.0f);
        vertex[11] = glm::vec3(   len,  1.0f,  0.0f);
        textur[12] = glm::vec2(1.0f,  0.0f);
        vertex[12] = glm::vec3(  0.0f, -1.0f,  0.0f);
        textur[13] = glm::vec2(1.0f,  1.0f);
        vertex[13] = glm::vec3(   len, -1.0f,  0.0f);

        vboVT.textureData(vboIndex, 14, textur);
        vboVT.vertexData(vboIndex,  14, vertex);
        vboVT.enableArrays();
        glDrawArrays(GL_TRIANGLE_FAN,   vboIndex,      6);
        glDrawArrays(GL_TRIANGLE_STRIP, vboIndex +  6, 4);
        glDrawArrays(GL_TRIANGLE_STRIP, vboIndex + 10, 4);
        vboVT.vboFree(vboIndex);

        addTriangleCount(8);
    }

    glPopMatrix();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
