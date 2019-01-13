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
#include "BoltSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TextureManager.h"
#include "VBO_Drawing.h"
#include "Singleton.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

#include "TimeKeeper.h"

const float maxRad = 0.16f;
const float boosterLen = 0.2f;
const float finRadius = 0.16f;
const float finCapSize = 0.15f;
const float finForeDelta = 0.02f;
const float noseRad = 0.086f;
const float noseLen = 0.1f;
const float bevelLen = 0.02f;
const float waistRad = 0.125f;
const float engineLen = 0.08f;

#define BOLTDRAWER (BoltDrawer::instance())

class BoltDrawer: public Singleton<BoltDrawer>, VBOclient
{
public:
    void initVBO();

    void finalStage();
    void nosecone();
    void body();
    void booster1();
    void booster2();
    void engine();
    void hemisphere1(int slices);
    void hemisphere2(int slices);
    void shaft(int slices);
protected:
    friend class Singleton<BoltDrawer>;
private:
    BoltDrawer();
    virtual ~BoltDrawer();

    int segment2Pos(int segment);
    int drawCylinder(float baseRadius, float topRadius, float height, int slices, int &vboIndex);
    const int segments[4] = {16, 25, 32, 48};

    int         vboFinIndex;
    int         noseconeIndex;
    int         noseconeCount;
    int         bodyIndex;
    int         bodyCount;
    int         booster1Index;
    int         booster1Count;
    int         booster2Index;
    int         booster2Count;
    int         engineIndex;
    int         engineCount;
    int         hemy1Index1[4];
    int         hemy1Count1[4];
    int         hemy1Index2[4];
    int         hemy1Count2[4];
    int         hemy1Index3[4];
    int         hemy1Count3[4];
    int         hemy1Index4[4];
    int         hemy1Count4[4];
    int         hemy2Index1[4];
    int         hemy2Count1[4];
    int         hemy2Index2[4];
    int         hemy2Count2[4];
    int         hemy2Index3[4];
    int         hemy2Count3[4];
    int         hemy2Index4[4];
    int         hemy2Count4[4];
    int         shaftCount[4];
    int         shaftIndex[4];

    // parametrics
    const float engineRad = 0.1f;
};

/* initialize the singleton */
template <>
BoltDrawer *Singleton<BoltDrawer>::_instance = (BoltDrawer*)0;

BoltDrawer::BoltDrawer()
{
    vboFinIndex   = -1;
    noseconeIndex = -1;
    bodyIndex     = -1;
    booster1Index = -1;
    booster2Index = -1;
    engineIndex   = -1;
    for (int i = 0; i < 4; i++)
    {
        hemy1Index1[i] = -1;
        hemy1Index2[i] = -1;
        hemy1Index3[i] = -1;
        hemy1Index4[i] = -1;
        hemy2Index1[i] = -1;
        hemy2Index2[i] = -1;
        hemy2Index3[i] = -1;
        hemy2Index4[i] = -1;
        shaftIndex[i]  = -1;
    }
    vboManager.registerClient(this);
}

BoltDrawer::~BoltDrawer()
{
    vboV.vboFree(vboFinIndex);
    vboVN.vboFree(noseconeIndex);
    vboVN.vboFree(bodyIndex);
    vboVN.vboFree(booster1Index);
    vboVN.vboFree(booster2Index);
    vboVN.vboFree(engineIndex);
    for (int i = 0; i < 4; i++)
    {
        vboVN.vboFree(hemy1Index1[i]);
        vboVN.vboFree(hemy1Index2[i]);
        vboVN.vboFree(hemy1Index3[i]);
        vboVN.vboFree(hemy1Index4[i]);
        vboVN.vboFree(hemy2Index1[i]);
        vboVN.vboFree(hemy2Index2[i]);
        vboVN.vboFree(hemy2Index3[i]);
        vboVN.vboFree(hemy2Index4[i]);
        vboVN.vboFree(shaftIndex[i]);
    }
    vboManager.unregisterClient(this);
}

void BoltDrawer::initVBO()
{
    glm::vec3 vertex[8];

    vertex[0] = glm::vec3(0,maxRad,0);
    vertex[1] = glm::vec3(0,maxRad,boosterLen);
    vertex[2] = glm::vec3(0,maxRad+finRadius,boosterLen-finForeDelta-finCapSize);
    vertex[3] = glm::vec3(0,maxRad+finRadius,boosterLen-finForeDelta);

    vertex[4] = glm::vec3(0,maxRad+finRadius,boosterLen-finForeDelta-finCapSize);
    vertex[5] = glm::vec3(0,maxRad+finRadius,boosterLen-finForeDelta);
    vertex[6] = glm::vec3(0,maxRad,0);
    vertex[7] = glm::vec3(0,maxRad,boosterLen);

    vboFinIndex = vboV.vboAlloc(8);
    vboV.vertexData(vboFinIndex, 8, vertex);

    int slices = 8;
    noseconeCount = drawCylinder(maxRad, noseRad, noseLen, slices, noseconeIndex);
    bodyCount     = drawCylinder(waistRad, maxRad, bevelLen, slices, bodyIndex);
    booster1Count = drawCylinder(maxRad, waistRad, bevelLen, slices, booster1Index);
    booster2Count = drawCylinder(waistRad, maxRad, bevelLen, slices, booster2Index);
    engineCount   = drawCylinder(engineRad, waistRad, engineLen, slices, engineIndex);
    for (int i = 0; i < 4; i++)
    {
        hemy1Count1[i] = drawCylinder(0.0f,     0.43589f, 0.1f,  segments[i], hemy1Index1[i]);
        hemy1Count2[i] = drawCylinder(0.43589f, 0.66144f, 0.15f, segments[i], hemy1Index2[i]);
        hemy1Count3[i] = drawCylinder(0.66144f, 0.86603f, 0.25f, segments[i], hemy1Index3[i]);
        hemy1Count4[i] = drawCylinder(0.86603f, 1.0f,     0.5f,  segments[i], hemy1Index4[i]);
        hemy2Count1[i] = drawCylinder(1.0f,     0.86603f, 0.5f,  segments[i], hemy2Index1[i]);
        hemy2Count2[i] = drawCylinder(0.86603f, 0.66144f, 0.25f, segments[i], hemy2Index2[i]);
        hemy2Count3[i] = drawCylinder(0.66144f, 0.43589f, 0.15f, segments[i], hemy2Index3[i]);
        hemy2Count4[i] = drawCylinder(0.43589f, 0.0f,     0.1f,  segments[i], hemy2Index4[i]);
        shaftCount[i]  = drawCylinder(1.0f,     1.0f,     1.0f,  segments[i], shaftIndex[i]);
    }
}

int BoltDrawer::drawCylinder(float baseRadius, float topRadius, float height, int slices, int &vboIndex)
{
    const int maxSlices = 48;
    const int maxDim    = 2 * (maxSlices + 1);
    const int dim       = 2 * (slices + 1);

    glm::vec3 vertex[maxDim];
    glm::vec3 normal[maxDim];
    glm::vec3 *vertexP = vertex;;
    glm::vec3 *normalP = normal;

    /* Compute length (needed for normal calculations) */
    float deltaRadius = baseRadius - topRadius;
    float length = sqrt(deltaRadius * deltaRadius + height * height);

    float zNormal  = deltaRadius / length;
    float xyNormalRatio = height / length;

    *normalP++ = glm::vec3(0.0f, xyNormalRatio, zNormal);
    *normalP++ = glm::vec3(0.0f, xyNormalRatio, zNormal);
    *vertexP++ = glm::vec3(0.0f, baseRadius,    0.0f);
    *vertexP++ = glm::vec3(0.0f, topRadius,     height);
    for (int i = 1; i < slices; i++)
    {
        float angle = (float)(2 * M_PI * i / slices);
        float sinCache = sin(angle);
        float cosCache = cos(angle);

        *normalP++ = glm::vec3(xyNormalRatio * sinCache, xyNormalRatio * cosCache, zNormal);
        *normalP++ = glm::vec3(xyNormalRatio * sinCache, xyNormalRatio * cosCache, zNormal);
        *vertexP++ = glm::vec3(baseRadius    * sinCache, baseRadius    * cosCache, 0.0f);
        *vertexP++ = glm::vec3(topRadius     * sinCache, topRadius     * cosCache, height);
    }
    *normalP++ = glm::vec3(0.0f, xyNormalRatio, zNormal);
    *normalP++ = glm::vec3(0.0f, xyNormalRatio, zNormal);
    *vertexP++ = glm::vec3(0.0f, baseRadius,    0.0f);
    *vertexP++ = glm::vec3(0.0f, topRadius,     height);
    vboIndex = vboVN.vboAlloc(dim);
    vboVN.normalData(vboIndex, dim, normal);
    vboVN.vertexData(vboIndex, dim, vertex);
    return dim;
}

void BoltDrawer::finalStage()
{
    vboV.enableArrays();
    glNormal3f(1,0,0);
    glDrawArrays(GL_TRIANGLE_STRIP, vboFinIndex,     4);
    glNormal3f(-1,0,0);
    glDrawArrays(GL_TRIANGLE_STRIP, vboFinIndex + 4, 4);
}

void BoltDrawer::nosecone()
{
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, noseconeIndex, noseconeCount);
}

void BoltDrawer::body()
{
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, bodyIndex, bodyCount);
}

void BoltDrawer::booster1()
{
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, booster1Index, booster1Count);
}

void BoltDrawer::booster2()
{
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, booster2Index, booster2Count);
}

void BoltDrawer::engine()
{
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, engineIndex, engineCount);
}

int BoltDrawer::segment2Pos(int slices)
{
    int i;
    if (slices == 16)
        i = 0;
    else if (slices == 25)
        i = 1;
    else if (slices == 32)
        i = 2;
    else if (slices == 48)
        i = 3;
    else
        abort();
    return i;
}

void BoltDrawer::hemisphere1(int slices)
{
    int i = segment2Pos(slices);;

    // 4 parts of the first hemisphere
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, hemy1Index1[i], hemy1Count1[i]);
    glTranslatef(0.0f, 0.0f, 0.1f);
    glDrawArrays(GL_TRIANGLE_STRIP, hemy1Index2[i], hemy1Count2[i]);
    glTranslatef(0.0f, 0.0f, 0.15f);
    glDrawArrays(GL_TRIANGLE_STRIP, hemy1Index3[i], hemy1Count3[i]);
    glTranslatef(0.0f, 0.0f, 0.25f);
    glDrawArrays(GL_TRIANGLE_STRIP, hemy1Index4[i], hemy1Count4[i]);
    glTranslatef(0.0f, 0.0f, 0.5f);
}

void BoltDrawer::hemisphere2(int slices)
{
    int i = segment2Pos(slices);;

    // 4 parts of the last hemisphere
    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, hemy2Index1[i], hemy2Count1[i]);
    glTranslatef(0.0f, 0.0f, 0.5f);
    glDrawArrays(GL_TRIANGLE_STRIP, hemy2Index2[i], hemy2Count2[i]);
    glTranslatef(0.0f, 0.0f, 0.25f);
    glDrawArrays(GL_TRIANGLE_STRIP, hemy2Index3[i], hemy2Count3[i]);
    glTranslatef(0.0f, 0.0f, 0.15f);
    glDrawArrays(GL_TRIANGLE_STRIP, hemy2Index4[i], hemy2Count4[i]);
    glTranslatef(0.0f, 0.0f, 0.1f);
}

void BoltDrawer::shaft(int slices)
{
    int i = segment2Pos(slices);;

    vboVN.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, shaftIndex[i], shaftCount[i]);
}

BoltSceneNode::BoltSceneNode(const GLfloat pos[3],const GLfloat vel[3], bool super) :
    isSuper(super),
    invisible(false),
    drawFlares(false),
    colorblind(false),
    size(1.0f),
    renderNode(this),
    azimuth(0),
    elevation(0),
    length(1.0f)
{

    OpenGLGStateBuilder builder(gstate);
    builder.setBlending();
    builder.setAlphaFunc();
    gstate = builder.getState();

    // prepare light
    light.setAttenuation(0, 0.05f);
    light.setAttenuation(1, 0.0f);
    light.setAttenuation(2, 0.03f);

    // prepare geometry
    move(pos, vel);
    setSize(size);
    setColor(1.0f, 1.0f, 1.0f);
    teamColor = glm::vec4(1,1,1,1);
}

BoltSceneNode::~BoltSceneNode()
{
    // do nothing
}

void            BoltSceneNode::setFlares(bool on)
{
    drawFlares = on;
}

void            BoltSceneNode::setSize(float radius)
{
    size = radius;
    setRadius(size * size);
}
void            BoltSceneNode::setTextureColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    light.setColor(1.5f * r, 1.5f * g, 1.5f * b);
    renderNode.setTextureColor(color);
}

void            BoltSceneNode::setColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    light.setColor(1.5f * r, 1.5f * g, 1.5f * b);
    renderNode.setColor(color);
}

void            BoltSceneNode::setTeamColor(const GLfloat *c)
{
    teamColor.r = c[0];
    teamColor.g = c[1];
    teamColor.b = c[2];
    teamColor.w = 1.0f;
}

void            BoltSceneNode::setColor(const GLfloat* rgb)
{
    setColor(rgb[0], rgb[1], rgb[2]);
}

bool            BoltSceneNode::getColorblind() const
{
    return colorblind;
}

void            BoltSceneNode::setColorblind(bool _colorblind)
{
    colorblind = _colorblind;
}

void            BoltSceneNode::setTexture(const int texture)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setTexture(texture);
    builder.enableTexture(texture>=0);
    gstate = builder.getState();
}

void            BoltSceneNode::setTextureAnimation(int cu, int cv)
{
    renderNode.setAnimation(cu, cv);
}

void            BoltSceneNode::move(const GLfloat pos[3],
                                    const GLfloat vel[3])
{
    setCenter(pos);
    light.setPosition(pos);
    velocity[0] = vel[0];
    velocity[1] = vel[1];
    velocity[2] = vel[2];
    length = sqrtf((vel[0] * vel[0]) +
                   (vel[1] * vel[1]) +
                   (vel[2] * vel[2]));

    azimuth   = (float)(+RAD2DEG * atan2f(vel[1], vel[0]));
    elevation = (float)(-RAD2DEG * atan2f(vel[2], sqrtf(vel[0]* vel[0] + vel[1] *vel[1])));
}

void            BoltSceneNode::addLight(
    SceneRenderer& renderer)
{
    renderer.addLight(light);
}

void            BoltSceneNode::notifyStyleChange()
{
    OpenGLGStateBuilder builder(gstate);
    builder.enableTexture(true);
    {
        const int shotLength = (int)(BZDBCache::shotLength * 3.0f);
        if (shotLength > 0 && !drawFlares)
            builder.setBlending(GL_SRC_ALPHA, GL_ONE);
        else
            builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        builder.setStipple(1.0f);
        builder.setAlphaFunc();
        if ((RENDERER.useQuality() >= 1) && drawFlares)
        {
            builder.setShading(GL_SMOOTH);
            builder.enableMaterial(false);
        }
        else
            builder.setShading(GL_FLAT);
    }
    gstate = builder.getState();
}

void            BoltSceneNode::addRenderNodes(
    SceneRenderer& renderer)
{
    renderer.addRenderNode(&renderNode, &gstate);
}

//
// BoltSceneNode::BoltRenderNode
//

const GLfloat       BoltSceneNode::BoltRenderNode::CoreFraction = 0.4f;
const GLfloat       BoltSceneNode::BoltRenderNode::FlareSize = 1.0f;
const GLfloat       BoltSceneNode::BoltRenderNode::FlareSpread = 0.08f;

BoltSceneNode::BoltRenderNode::BoltRenderNode(
    const BoltSceneNode* _sceneNode) :
    sceneNode(_sceneNode),
    numFlares(0)
{
    textureColor[0] = 1.0f;
    textureColor[1] = 1.0f;
    textureColor[2] = 1.0f;
    textureColor[3] = 1.0f;

    setAnimation(1, 1);
}

BoltSceneNode::BoltRenderNode::~BoltRenderNode()
{
    // do nothing
}

void            BoltSceneNode::BoltRenderNode::setAnimation(
    int _cu, int _cv)
{
    cu = _cu;
    cv = _cv;
    du = 1.0f / (float)cu;
    dv = 1.0f / (float)cv;

    // pick a random start frame
    const int index = (int)((float)cu * (float)cv * bzfrand());
    u = index % cu;
    v = index / cu;
    if (v >= cv) v = 0;
}
void            BoltSceneNode::BoltRenderNode::setTextureColor(const GLfloat* rgba)
{
    textureColor[0] = rgba[0];
    textureColor[1] = rgba[1];
    textureColor[2] = rgba[2];
    textureColor[3] = rgba[3];
}


void            BoltSceneNode::BoltRenderNode::setColor(
    const GLfloat* rgba)
{
    mainColor[0] = rgba[0];
    mainColor[1] = rgba[1];
    mainColor[2] = rgba[2];
    mainColor[3] = rgba[3];

    innerColor[0] = mainColor[0] + 0.5f * (1.0f - mainColor[0]);
    innerColor[1] = mainColor[1] + 0.5f * (1.0f - mainColor[1]);
    innerColor[2] = mainColor[2] + 0.5f * (1.0f - mainColor[2]);
    innerColor[3] = rgba[3];

    outerColor[0] = mainColor[0];
    outerColor[1] = mainColor[1];
    outerColor[2] = mainColor[2];
    outerColor[3] = (rgba[3] == 1.0f )? 0.1f: rgba[3];

    flareColor[0] = mainColor[0];
    flareColor[1] = mainColor[1];
    flareColor[2] = mainColor[2];
    flareColor[3] = (rgba[3] == 1.0f )? 0.667f : rgba[3];
}

void BoltSceneNode::BoltRenderNode::renderGeoGMBolt()
{
    // bzdb these 2? they control the shot size
    float gmMissleSize = BZDBCache::gmSize;

    // parametrics
    float bodyLen = 0.44f;
    float waistLen = 0.16f;

    int slices = 8;

    float rotSpeed = 90.0f;

    glDepthMask(GL_TRUE);
    glPushMatrix();
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);
    //glEnable(GL_LIGHTING);

    glm::vec4 noseColor = sceneNode->teamColor;
    glm::vec4 finColor(noseColor.r*0.5f,noseColor.g*0.5f,noseColor.b*0.5f,1);
    glm::vec4 coneColor(0.125f,0.125f,0.125f,1);
    glm::vec4 bodyColor(1,1,1,1);

    glScalef(gmMissleSize, gmMissleSize, gmMissleSize);

    glColor4f(noseColor.r,noseColor.g,noseColor.b,1.0f);
    glTranslatef(0, 0, 1);
    glRotatef((float)TimeKeeper::getCurrent().getSeconds() * rotSpeed,0,0,1);

    // nosecone
    glPushMatrix();
    glScalef(noseRad, noseRad, 0.0f);
    DRAWER.disk8();
    glPopMatrix();
    glTranslatef(0.0f, 0.0f, -noseLen);
    BOLTDRAWER.nosecone();
    addTriangleCount(slices * 2);

    // body
    myColor4f(bodyColor.r, bodyColor.g, bodyColor.b, bodyColor.a);
    glTranslatef(0, 0, -bodyLen);
    glPushMatrix();
    glScalef(maxRad, maxRad, bodyLen);
    DRAWER.cylinder8();
    glPopMatrix();
    addTriangleCount(slices);

    glTranslatef(0, 0, -bevelLen);
    BOLTDRAWER.body();
    addTriangleCount(slices);

    // waist
    myColor4f(coneColor.r, coneColor.g, coneColor.b, coneColor.a);
    glTranslatef(0, 0, -waistLen);
    glPushMatrix();
    glScalef(waistRad, waistRad, waistLen);
    DRAWER.cylinder8();
    glPopMatrix();
    addTriangleCount(slices);

    // booster
    myColor4f(bodyColor.r, bodyColor.g, bodyColor.b, 1.0f);
    glTranslatef(0, 0, -bevelLen);
    BOLTDRAWER.booster1();
    addTriangleCount(slices);

    glTranslatef(0, 0, -boosterLen);
    glPushMatrix();
    glScalef(maxRad, maxRad, boosterLen);
    DRAWER.cylinder8();
    glPopMatrix();
    addTriangleCount(slices);

    glTranslatef(0, 0, -bevelLen);
    BOLTDRAWER.booster2();
    addTriangleCount(slices);

    // engine
    myColor4f(coneColor.r, coneColor.g, coneColor.b, 1.0f);
    glTranslatef(0, 0, -engineLen);
    BOLTDRAWER.engine();
    addTriangleCount(slices);

    // fins
    myColor4f(finColor.r, finColor.g, finColor.b, 1.0f);
    glTranslatef(0, 0, engineLen + bevelLen);

    for ( int i = 0; i < 4; i++)
    {
        glRotatef(i*90.0f,0,0,1);
        BOLTDRAWER.finalStage();
    }

    glEnable(GL_TEXTURE_2D);
    // glDisable(GL_LIGHTING);

    glPopMatrix();

    glDepthMask(GL_FALSE);
}


void BoltSceneNode::BoltRenderNode::renderGeoBolt()
{
    // bzdb these 2? they control the shot size
    float lenMod = 0.0675f + (BZDBCache::shotLength * 0.0125f);
    float baseRadius = 0.225f;

    float len = sceneNode->length * lenMod;
    glPushMatrix();
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);

    float alphaMod = 1.0f;
// if (sceneNode->phasingShot)
//   alphaMod = 0.85f;

    glDisable(GL_TEXTURE_2D);

    float coreBleed = 4.5f;
    float minimumChannelVal = 0.45f;

    glm::vec3 coreColor;
    coreColor.r =  sceneNode->color[0] * coreBleed;
    coreColor.g =  sceneNode->color[1] * coreBleed;
    coreColor.b =  sceneNode->color[2] * coreBleed;
    if (coreColor.r < minimumChannelVal)
        coreColor.r = minimumChannelVal;
    if (coreColor.g < minimumChannelVal)
        coreColor.g = minimumChannelVal;
    if (coreColor.b < minimumChannelVal)
        coreColor.b = minimumChannelVal;

    myColor4f(coreColor.r, coreColor.g, coreColor.b, 0.85f * alphaMod);
    renderGeoPill(baseRadius,len,16);

    float radInc = 1.5f * baseRadius - baseRadius;
    glPushMatrix();
    glTranslatef(0, 0, -radInc * 0.5f);
    glm::vec4 c;
    c.x = sceneNode->color[0];
    c.y = sceneNode->color[1];
    c.z = sceneNode->color[2];
    c.w = 0.5f;

    myColor4f(c.r, c.g, c.b, c.a);
    renderGeoPill(1.5f * baseRadius, len + radInc, 25);
    glPopMatrix();

    radInc = 2.7f * baseRadius - baseRadius;
    glPushMatrix();
    glTranslatef(0, 0, -radInc*0.5f);
    c.w = 0.25f;
    myColor4f(c.r, c.g, c.b, c.a);
    renderGeoPill(2.7f * baseRadius, len + radInc, 32);
    glPopMatrix();

    radInc = 3.8f * baseRadius - baseRadius;
    glPushMatrix();
    glTranslatef(0, 0,-radInc*0.5f);
    c.w = 0.125f;
    myColor4f(c.r, c.g, c.b, c.a);
    renderGeoPill(3.8f * baseRadius, len + radInc, 48);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
}


void BoltSceneNode::BoltRenderNode::renderGeoPill(float radius, float len, int segments)
{
    glPushMatrix();
    glScalef(radius, radius, radius);

    // 4 parts of the first hemisphere
    BOLTDRAWER.hemisphere1(segments);
    addTriangleCount(4 * segments);

    // the "shaft"
    if (len > 2.0f * radius)
    {
        float lenMinusRads = len / radius - 2.0f;

        glPushMatrix();
        glScalef(1.0f, 1.0f, lenMinusRads);
        BOLTDRAWER.shaft(segments);
        glPopMatrix();
        addTriangleCount(segments);
        glTranslatef(0.0f, 0.0f, lenMinusRads);
    }

    // 4 parts of the last hemisphere
    BOLTDRAWER.hemisphere2(segments);
    addTriangleCount(4 * segments);

    glPopMatrix();
}

void            BoltSceneNode::BoltRenderNode::render()
{
    if (sceneNode->invisible)
        return;
    const float radius = sceneNode->size;
    const int   shotLength = (int)(BZDBCache::shotLength * 3.0f);
    const bool  experimental = (RENDERER.useQuality() >= 1);

    const bool blackFog = RENDERER.isFogActive() &&
                          ((shotLength > 0) || experimental);
    if (blackFog)
        glFogfv(GL_FOG_COLOR, glm::value_ptr(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)));

    const float* sphere = sceneNode->getSphere();
    glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);

    bool drawBillboardShot = false;
    if (experimental)
    {
        if (sceneNode->isSuper)
            renderGeoBolt();
        else
        {
            if (sceneNode->drawFlares)
            {
                if (BZDBCache::shotLength > 0)
                    renderGeoGMBolt();
                drawBillboardShot = true;
            }
            else
                drawBillboardShot = true;
        }
    }
    else
        drawBillboardShot = true;

    if (drawBillboardShot)
    {
        RENDERER.getViewFrustum().executeBillboard();
        glScalef(radius, radius, radius);
        // draw some flares
        if (sceneNode->drawFlares)
        {
            if (!RENDERER.isSameFrame())
            {
                numFlares = 3 + int(3.0f * (float)bzfrand());
                for (int i = 0; i < numFlares; i++)
                {
                    theta[i] = (float)(2.0 * M_PI * bzfrand());
                    phi[i] = (float)bzfrand() - 0.5f;
                    phi[i] *= (float)(2.0 * M_PI * fabsf(phi[i]));
                }
            }

            glDisable(GL_TEXTURE_2D);
            myColor4fv(flareColor);
            int vboIndex = vboV.vboAlloc(4);
            glm::vec3 vertex[4];
            vboV.enableArrays();
            for (int i = 0; i < numFlares; i++)
            {
                // pick random direction in 3-space.  picking a random theta with
                // a uniform distribution is fine, but doing so with phi biases
                // the directions toward the poles.  my correction doesn't remove
                // the bias completely, but moves it towards the equator, which is
                // really where i want it anyway cos the flares are more noticeable
                // there.
                const float c = FlareSize * cosf(phi[i]);
                const float s = FlareSize * sinf(phi[i]);
                const float ti = theta[i];
                const float fs = FlareSpread;
                vertex[0] = glm::vec3(0.0f, 0.0f, CoreFraction);
                vertex[1] = glm::vec3(c * cosf(ti - fs),   c * sinf(ti - fs),   s);
                vertex[2] = glm::vec3(c * cosf(ti + fs),   c * sinf(ti + fs),   s);
                vertex[3] = glm::vec3(c * cosf(ti) * 2.0f, c * sinf(ti) * 2.0f, s * 2.0f);
                vboV.vertexData(vboIndex, 4, vertex);
                glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
            }
            vboV.vboFree(vboIndex);
            glEnable(GL_TEXTURE_2D);

            addTriangleCount(numFlares * 2);
        }

        {
            // draw billboard square
            const float u0 = (float)u * du;
            const float v0 = (float)v * dv;
            const float u1 = u0 + du;
            const float v1 = v0 + dv;
            myColor4fv(textureColor); // 1.0f all
            int vboIndex = vboVT.vboAlloc(4);
            glm::vec2 texture[4];
            glm::vec3 vertex[4];
            texture[0] = glm::vec2(u0, v0);
            texture[1] = glm::vec2(u1, v0);
            texture[2] = glm::vec2(u0, v1);
            texture[3] = glm::vec2(u1, v1);
            vboVT.textureData(vboIndex, 4, texture);
            vertex[0]  = glm::vec3(-1.0f, -1.0f, 0.0f);
            vertex[1]  = glm::vec3(+1.0f, -1.0f, 0.0f);
            vertex[2]  = glm::vec3(-1.0f, +1.0f, 0.0f);
            vertex[3]  = glm::vec3(+1.0f, +1.0f, 0.0f);
            vboVT.vertexData(vboIndex, 4, vertex);
            vboVT.enableArrays();
            glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
            addTriangleCount(2);

            // draw shot trail  (more billboarded quads)
            if ((shotLength > 0) && (sceneNode->length > 1.0e-6f))
            {
                const float startSize  = 0.6f;
                const float startAlpha = 0.8f;

                glPushAttrib(GL_TEXTURE_BIT);
                TextureManager &tm = TextureManager::instance();
                const int texID = tm.getTextureID("shot_tail");
                const ImageInfo& texInfo = tm.getInfo(texID);
                if (texInfo.id >= 0)
                    texInfo.texture->execute();

                glm::vec3 vel(glm::make_vec3(sceneNode->velocity));
                const glm::vec3 dir = vel * (-1.0f / sceneNode->length);

                const float invLenPlusOne = 1.0f / (float)(shotLength + 1);
                const float shiftScale = 90.0f / (150.0f + (float)shotLength);
                float Size = sceneNode->size * startSize;
                float alpha = startAlpha;
                const float sizeStep  = Size  * invLenPlusOne;
                const float alphaStep = alpha * invLenPlusOne;

                glm::vec3 pos;
                pos.x = sphere[0];
                pos.y = sphere[1];
                pos.z = sphere[2];

                int uvCell = rand() % 16;

                for (int i = 0; i < shotLength; i++)
                {
                    Size  -= sizeStep;
                    const float s = Size * (0.65f + (1.0f * (float)bzfrand()));
                    const float shift = s * shiftScale;

                    pos += (shift * dir);
                    if (pos.z < 0.0f)
                        continue;

                    uvCell = (uvCell + 1) % 16;
                    const float U0 = (uvCell % 4 ) * 0.25f;
                    const float V0 = (uvCell / 4 ) * 0.25f;
                    const float U1 = U0 + 0.25f;
                    const float V1 = V0 + 0.25f;

                    alpha -= alphaStep;
                    glColor4f(mainColor[0],mainColor[1],mainColor[2], alpha);
                    glPopMatrix();
                    glPushMatrix();

                    glTranslatef(pos.x, pos.y, pos.z);
                    RENDERER.getViewFrustum().executeBillboard();
                    glScalef(s, s, s);

                    texture[0] = glm::vec2(U0, V0);
                    texture[1] = glm::vec2(U1, V0);
                    texture[2] = glm::vec2(U0, V1);
                    texture[3] = glm::vec2(U1, V1);
                    vboVT.textureData(vboIndex, 4, (const GLfloat (*)[2])texture);
                    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
                }

                addTriangleCount(shotLength * 2);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                glPopAttrib(); // revert the texture
            }
            vboVT.vboFree(vboIndex);
        }
    }

    glPopMatrix();

    if (blackFog)
        glFogfv(GL_FOG_COLOR, glm::value_ptr(RENDERER.getFogColor()));

    if (RENDERER.isLastFrame())
    {
        if (++u == cu)
        {
            u = 0;
            if (++v == cv)
                v = 0;
        }
    }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
