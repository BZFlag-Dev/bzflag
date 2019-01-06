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
#include "SphereSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "SceneRenderer.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "OpenGLMaterial.h"
#include "TextureManager.h"
#include "VBO_Handler.h"
#include "OpenGLCommon.h"

// local implementation headers
#include "ViewFrustum.h"

class SphereVBOs : public VBOclient
{
public:
    SphereVBOs();
    virtual ~SphereVBOs();

    void initVBO();

    int getLOD(float pixelsSqr);
    int draw(int lod);
    void drawFullScreenRect();
private:
    int calcTriCount(int slices, int stacks);

    int lodIndex[sphereLods];
    int lodDim[sphereLods];
    int rectIndex;
    float lodPixelsSqr[sphereLods];
    int listTriangleCount[sphereLods];
    void buildSphereList(int lod);
};


/******************************************************************************/

//
// SphereSceneNode
//

SphereSceneNode::SphereSceneNode(const GLfloat pos[3], GLfloat _radius)
{
    transparent = false;

    OpenGLGStateBuilder builder(gstate);
    builder.disableCulling();
    gstate = builder.getState();

    setColor(1.0f, 1.0f, 1.0f, 1.0f);

    // position sphere
    move(pos, _radius);

    return;
}


SphereSceneNode::~SphereSceneNode()
{
    // do nothing
}


void SphereSceneNode::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    transparent = (color[3] != 1.0f);
}


void SphereSceneNode::setColor(const GLfloat* rgba)
{
    color[0] = rgba[0];
    color[1] = rgba[1];
    color[2] = rgba[2];
    color[3] = rgba[3];
    transparent = (color[3] != 1.0f);
}


void SphereSceneNode::move(const GLfloat pos[3], GLfloat _radius)
{
    radius = _radius;
    setCenter(pos);
    setRadius(radius * radius);
}


void SphereSceneNode::notifyStyleChange()
{
    OpenGLGStateBuilder builder(gstate);
    if (transparent)
    {
        {
            builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            builder.setStipple(1.0f);
            builder.setNeedsSorting(true);
        }
    }
    else
    {
        builder.resetBlending();
        builder.setStipple(1.0f);
        builder.setNeedsSorting(false);
    }
    gstate = builder.getState();
}


SphereVBOs *sphereVBO;

/******************************************************************************/

//
// SphereLodSceneNode
//


bool SphereLodSceneNode::initialized = false;

void SphereLodSceneNode::init()
{
    initialized = false; // no lists yet
    sphereVBO = new SphereVBOs;
    return;
}


void SphereLodSceneNode::kill()
{
    if (sphereVBO)
    {
        delete sphereVBO;
        sphereVBO = NULL;
    }
    return;
}


SphereLodSceneNode::SphereLodSceneNode(const GLfloat pos[3], GLfloat _radius) :
    SphereSceneNode(pos, _radius),
    renderNode(this)
{
    if (!initialized)
        initialized = true;

    inside = false;
    shockWave = false;

    renderNode.setLod(0);

    // adjust the gstate for this type of sphere
    OpenGLGStateBuilder builder(gstate);
    builder.setCulling(GL_BACK);
    builder.setShading(GL_SMOOTH);
    const float spec[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    const float emis[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    OpenGLMaterial glmat(spec, emis, 64.0f);
    builder.setMaterial(glmat);
    gstate = builder.getState();
    return;
}


SphereLodSceneNode::~SphereLodSceneNode()
{
    return;
}


void SphereLodSceneNode::setShockWave(bool value)
{
    shockWave = value;
    return;
}


void SphereLodSceneNode::addRenderNodes(SceneRenderer& renderer)
{
    const ViewFrustum& view = renderer.getViewFrustum();
    const float* s = getSphere();
    const float* e = view.getEye();
    const float dx = e[0] - s[0];
    const float dy = e[1] - s[1];
    const float dz = e[2] - s[2];

    float distSqr = (dx*dx) + (dy*dy) + (dz*dz);
    if (distSqr <= 0.0f)
        distSqr = 1.0e-6f;

    const float lpp = renderer.getLengthPerPixel();
    float ppl;
    if (lpp <= 0.0f)
        ppl = +MAXFLOAT;
    else
        ppl = 1.0f / lpp;
    const float pixelsSqr = (s[3] * (ppl * ppl)) / distSqr;

    int lod = sphereVBO->getLOD(pixelsSqr);
    renderNode.setLod(lod);

    inside = (distSqr < s[3]);

    renderer.addRenderNode(&renderNode, &gstate);

    return;
}


void SphereLodSceneNode::addShadowNodes(SceneRenderer&)
{
    return;
}


//
// SphereLodSceneNode::SphereLodRenderNode
//

SphereLodSceneNode::SphereLodRenderNode::SphereLodRenderNode(
    const SphereLodSceneNode* _sceneNode) :
    sceneNode(_sceneNode)
{
    return;
}


SphereLodSceneNode::SphereLodRenderNode::~SphereLodRenderNode()
{
    return;
}


void SphereLodSceneNode::SphereLodRenderNode::setLod(int _lod)
{
    lod = _lod;
    return;
}


void SphereLodSceneNode::SphereLodRenderNode::render()
{
    const GLfloat radius = sceneNode->radius;
    const GLfloat* sphere = sceneNode->getSphere();

    glEnable(GL_CLIP_PLANE0);

    glEnable(GL_RESCALE_NORMAL);

    const bool transparent = sceneNode->transparent;

    glPushMatrix();
    {
        glTranslatef(sphere[0], sphere[1], sphere[2]);
        glScalef(radius, radius, radius);

        // invert the color within contained volume
        if (sceneNode->shockWave)
        {
            if (transparent)
                glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);

            glLogicOp(GL_INVERT);
            glEnable(GL_COLOR_LOGIC_OP);
            {
                glCullFace(GL_FRONT);
#ifdef DEBUG_RENDERING
                int count = sphereVBO->draw(lod);
                addTriangleCount(count);
#else
                sphereVBO->draw(lod);
#endif
                glCullFace(GL_BACK);
                if (!sceneNode->inside)
                {
#ifdef DEBUG_RENDERING
                    count = sphereVBO->draw(lod);
                    addTriangleCount(count);
#else
                    sphereVBO->draw(lod);
#endif
                }
                else
                {
                    sphereVBO->drawFullScreenRect();
                    addTriangleCount(2);
                }
            }
            glDisable(GL_COLOR_LOGIC_OP);

            if (transparent)
                glEnable(GL_BLEND);
            glEnable(GL_LIGHTING);
        }

        // draw the surface
        myColor4fv(sceneNode->color);
        {
            glCullFace(GL_FRONT);
#ifdef DEBUG_RENDERING
            int count = sphereVBO->draw(lod);
            addTriangleCount(count);
#else
            sphereVBO->draw(lod);
#endif
        }
        glCullFace(GL_BACK);
        if (!sceneNode->inside)
        {
#ifdef DEBUG_RENDERING
            int count = sphereVBO->draw(lod);
            addTriangleCount(count);
#else
            sphereVBO->draw(lod);
#endif
        }
        else
        {
            glDisable(GL_LIGHTING);
            sphereVBO->drawFullScreenRect();
            glEnable(GL_LIGHTING);
            addTriangleCount(2);
        }
    }
    glPopMatrix();

    glDisable(GL_RESCALE_NORMAL);

    glDisable(GL_CLIP_PLANE0);

    return;
}


/******************************************************************************/


SphereVBOs::SphereVBOs()
{
    for (int i = 0; i < sphereLods; i++)
    {
        lodPixelsSqr[i] = 0.0f;
        lodIndex[i] = -1;
    }
    rectIndex = -1;
    vboManager.registerClient(this);
}


SphereVBOs::~SphereVBOs()
{
    for (int i = 0; i < sphereLods; i++)
        vboVTN.vboFree(lodIndex[i]);
    vboV.vboFree(rectIndex);
    vboManager.unregisterClient(this);
}


void SphereVBOs::buildSphereList(int lod)
{
    GLint slices;

    switch (lod)
    {
    case 0:
        slices = 32;
        break;
    case 1:
        slices = 16;
        break;
    case 2:
        slices = 8;
        break;
    case 3:
        slices = 6;
        break;
    case 4:
        slices = 4;
        break;
    default:
        slices = 4;
        break;
    }

    lodDim[lod]   = slices * (slices + 1) * 2;
    listTriangleCount[lod] = calcTriCount(slices, slices);
    lodIndex[lod] = vboVTN.vboAlloc(lodDim[lod]);

    int vboIndex = lodIndex[lod];
    int vboDim   = lodDim[lod];

    GLint i,j;
    GLfloat zHigh;
    GLfloat zLow = -1;
    GLfloat sintemp2;
    GLfloat sintemp1 = 0;
    GLfloat tHigh;
    GLfloat tLow = 0;

    std::vector<glm::vec3> vertex;
    std::vector<glm::vec2> textur;

    vertex.reserve(vboDim);
    textur.reserve(vboDim);
    for (j = slices - 1; j >= 0; j--)
    {
        zHigh    = zLow;
        sintemp2 = sintemp1;
        tHigh    = tLow;
        if (j)
        {
            GLfloat percent = (float) j / slices;
            GLfloat angleZ  = (float)(M_PI * percent);
            zLow     = cos(angleZ);
            sintemp1 = sin(angleZ);
            tLow     = 1 - percent;
        }
        else
        {
            zLow     = 1;
            sintemp1 = 0;
            tLow     = 1;
        }

        textur.push_back(glm::vec2(1, tHigh));
        vertex.push_back(glm::vec3(0, sintemp2, zHigh));

        textur.push_back(glm::vec2(1, tLow));
        vertex.push_back(glm::vec3(0, sintemp1, zLow));

        for (i = 1; i < slices; i++)
        {
            GLfloat angleT = (float)(2 * M_PI * i / slices);
            GLfloat sinCache = sin(angleT);
            GLfloat cosCache = cos(angleT);
            GLfloat x = sinCache * sintemp2;
            GLfloat y = cosCache * sintemp2;
            GLfloat s = 1 - (float) i / slices;

            textur.push_back(glm::vec2(s, tHigh));
            vertex.push_back(glm::vec3(x, y, zHigh));

            x = sinCache * sintemp1;
            y = cosCache * sintemp1;
            textur.push_back(glm::vec2(s, tLow));
            vertex.push_back(glm::vec3(x, y, zLow));
        }

        textur.push_back(glm::vec2(0, tHigh));
        vertex.push_back(glm::vec3(0, sintemp2, zHigh));

        textur.push_back(glm::vec2(0, tLow));
        vertex.push_back(glm::vec3(0, sintemp1, zLow));
    }

    vboVTN.vertexData(vboIndex, vertex);
    vboVTN.normalData(vboIndex, vertex);
    vboVTN.textureData(vboIndex, textur);
}


int SphereVBOs::calcTriCount(int slices, int stacks)
{
    const int trifans = 2 * slices;
    const int quads = 2 * (slices * (stacks - 2));
    return (trifans + quads);
}


void SphereVBOs::initVBO()
{
    buildSphereList(0);
    lodPixelsSqr[0] = 80.0f * 80.0f;

    buildSphereList(1);
    lodPixelsSqr[1] = 40.0f * 40.0f;

    buildSphereList(2);
    lodPixelsSqr[2] = 20.0f * 20.0f;

    buildSphereList(3);
    lodPixelsSqr[3] = 10.0f * 10.0f;

    buildSphereList(4);
    lodPixelsSqr[4] = 5.0f * 5.0f;

    glm::vec3 vertex[4];

    vertex[0] = glm::vec3(+1, -1, 0);
    vertex[1] = glm::vec3(+1, +1, 0);
    vertex[2] = glm::vec3(-1, -1, 0);
    vertex[3] = glm::vec3(-1, +1, 0);

    rectIndex = vboV.vboAlloc(4);
    vboV.vertexData(rectIndex, 4, vertex);
}


int SphereVBOs::getLOD(float pixelsSqr)
{
    int lod;
    for (lod = 0; lod < (sphereLods - 1); lod++)
        if (lodPixelsSqr[lod] < pixelsSqr)
            break;
    return lod;
}


int SphereVBOs::draw(int lod)
{
    const int vboIndex = lodIndex[lod];
    const int vboDim   = lodDim[lod];
    vboVTN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, vboDim);
    return listTriangleCount[lod];
}

void SphereVBOs::drawFullScreenRect()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    vboV.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, rectIndex, 4);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
