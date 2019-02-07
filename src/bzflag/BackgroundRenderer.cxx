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
#include "BackgroundRenderer.h"

// system headers
#include <string.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// common headers
#include "OpenGLMaterial.h"
#include "TextureManager.h"
#include "BZDBCache.h"
#include "BzMaterial.h"
#include "TextureMatrix.h"
#include "ParseColor.h"
#include "BZDBCache.h"
#include "VBO_Handler.h"
#include "OpenGLCommon.h"

// local headers
#include "daylight.h"
#include "stars.h"
#include "MainWindow.h"
#include "SceneNode.h"
#include "effectsRenderer.h"

static const glm::vec2 squareShape[4] =
{
    {  1.0f,  1.0f }, { -1.0f,  1.0f },
    { -1.0f, -1.0f }, {  1.0f, -1.0f }
};


glm::vec3           BackgroundRenderer::skyPyramid[5];
const GLfloat       BackgroundRenderer::cloudRepeats = 3.0f;
static const int    NumMountainFaces = 16;

GLfloat         BackgroundRenderer::groundColor[4];
GLfloat         BackgroundRenderer::groundColorInv[4];

const GLfloat       BackgroundRenderer::defaultGroundColor[4] =
{
    1.0f, 1.00f, 1.0f, 1.0f
};
const GLfloat       BackgroundRenderer::defaultGroundColorInv[4] =
{
    1.00f, 1.00f, 1.00f, 1.0f
};
const GLfloat       BackgroundRenderer::receiverColor[3] =
{ 0.3f, 0.55f, 0.3f };
const GLfloat       BackgroundRenderer::receiverColorInv[3] =
{ 0.55f, 0.3f, 0.55f };

BackgroundRenderer::BackgroundRenderer(const SceneRenderer&) :
    blank(false),
    invert(false),
    style(0),
    gridSpacing(60.0f), // meters
    gridCount(4.0f),
    mountainsAvailable(false),
    numMountainTextures(0),
    mountainsGState(NULL),
    cloudDriftU(0.0f),
    cloudDriftV(0.0f),
    cloudVBOIndex(-1), sunVBOIndex(-1), moonVBOIndex(-1), starVBOIndex(-1),
    moonSegements(BZDB.evalInt("moonSegments"))
{
    static bool init = false;
    OpenGLGStateBuilder gstate;
    static const GLfloat  black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    static const GLfloat  white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    OpenGLMaterial defaultMaterial(black, black, 0.0f);
    OpenGLMaterial rainMaterial(white, white, 0.0f);

    // initialize global to class stuff
    if (!init)
    {
        init = true;
        resizeSky();
    }

    // initialize the celestial vectors
    static const float up[3] = { 0.0f, 0.0f, 1.0f };
    memcpy(sunDirection, up, sizeof(float[3]));
    memcpy(moonDirection, up, sizeof(float[3]));

    // make ground materials
    setupSkybox();
    setupGroundMaterials();

    TextureManager &tm = TextureManager::instance();

    // make grid stuff
    gstate.reset();
    gstate.setBlending();
    gstate.setSmoothing();
    gridGState = gstate.getState();

    // make receiver stuff
    gstate.reset();
    gstate.setShading();
    gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE);
    receiverGState = gstate.getState();

    // sun shadow stuff
    gstate.reset();
    gstate.setStipple(0.5f);
    gstate.disableCulling();
    sunShadowsGState = gstate.getState();

    // sky stuff
    gstate.reset();
    gstate.setShading();
    skyGState = gstate.getState();
    gstate.reset();
    sunGState = gstate.getState();
    gstate.reset();
    gstate.setBlending((GLenum)GL_ONE, (GLenum)GL_ONE);
// if (useMoonTexture)
//   gstate.setTexture(*moonTexture);
    moonGState[0] = gstate.getState();
    gstate.reset();
// if (useMoonTexture)
//   gstate.setTexture(*moonTexture);
    moonGState[1] = gstate.getState();
    gstate.reset();
    starGState[0] = gstate.getState();
    gstate.reset();
    gstate.setBlending();
    gstate.setSmoothing();
    starGState[1] = gstate.getState();

    // make cloud stuff
    cloudsAvailable = false;
    int cloudsTexture = tm.getTextureID( "clouds" );
    if (cloudsTexture >= 0)
    {
        cloudsAvailable = true;
        gstate.reset();
        gstate.setShading();
        gstate.setBlending((GLenum)GL_SRC_ALPHA, (GLenum)GL_ONE_MINUS_SRC_ALPHA);
        gstate.setMaterial(defaultMaterial);
        gstate.setTexture(cloudsTexture);
        gstate.setAlphaFunc();
        cloudsGState = gstate.getState();
    }

    // rain stuff
    weather.init();
    // effects
    EFFECTS.init();

    // make mountain stuff
    mountainsAvailable = false;
    {
        int mountainTexture;
        int height = 0;
        int i;

        numMountainTextures = 0;
        while (1)
        {
            char text[256];
            sprintf (text, "mountain%d", numMountainTextures + 1);
            mountainTexture = tm.getTextureID (text, false);
            if (mountainTexture < 0)
                break;
            const ImageInfo & info = tm.getInfo (mountainTexture);
            height = info.y;
            numMountainTextures++;
        }

        if (numMountainTextures > 0)
        {
            mountainsAvailable = true;

            // prepare common gstate
            gstate.reset ();
            gstate.setShading ();
            gstate.setBlending ();
            gstate.setMaterial (defaultMaterial);
            gstate.setAlphaFunc ();

            // find power of two at least as large as height
            int scaledHeight = 1;
            while (scaledHeight < height)
                scaledHeight <<= 1;

            // choose minimum width
            int minWidth = scaledHeight;
            if (minWidth > scaledHeight)
                minWidth = scaledHeight;
            mountainsMinWidth = minWidth;

            // prepare each texture
            mountainsGState = new OpenGLGState[numMountainTextures];
            mountainsVBOIndex = new int[numMountainTextures];
            for (i = 0; i < numMountainTextures; i++)
            {
                char text[256];
                sprintf (text, "mountain%d", i + 1);
                gstate.setTexture (tm.getTextureID (text));
                mountainsGState[i] = gstate.getState ();
            }
        }
    }
    for (int j = 0; j < numMountainTextures; j++)
        mountainsVBOIndex[j] = -1;

    vboManager.registerClient(this);

    // create display lists
    doInitDisplayLists();

    // reset the sky color when it changes
    BZDB.addCallback("_skyColor", bzdbCallback, this);

    // recreate display lists when context is recreated
    OpenGLGState::registerContextInitializer(freeContext, initContext,
            (void*)this);

    notifyStyleChange();
}

BackgroundRenderer::~BackgroundRenderer()
{
    vboV.vboFree(sunVBOIndex);
    vboV.vboFree(moonVBOIndex);
    vboVC.vboFree(starVBOIndex);
    vboVTC.vboFree(cloudVBOIndex);
    for (int j = 0; j < numMountainTextures; j++)
        vboVTN.vboFree(mountainsVBOIndex[j]);
    vboManager.unregisterClient(this);
    BZDB.removeCallback("_skyColor", bzdbCallback, this);
    OpenGLGState::unregisterContextInitializer(freeContext, initContext,
            (void*)this);
    delete[] mountainsGState;
    delete[] mountainsVBOIndex;
}


#define GROUND_DIVS (4) //FIXME -- seems to be enough

void BackgroundRenderer::initVBO()
{

    // sun first.  sun is a disk that should be about a half a degree wide
    // with a normal (60 degree) perspective.
    const float sunRadius = atanf((float)M_PI / 3.0f) / 60.0f;

    glm::vec3 sunVertex[21];
    sunVertex[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 20; i++)
    {
        const float angle = (float)(2.0 * M_PI * double(i) / 19.0);
        sunVertex[i + 1] = glm::vec3(
                               1.0f,
                               sunRadius * sinf(angle),
                               sunRadius * cosf(angle));
    }
    sunVBOIndex = vboV.vboAlloc(21);
    vboV.vertexData(sunVBOIndex, 21, sunVertex);

    moonVBOIndex = vboV.vboAlloc(2 * moonSegements);

    // make stars list
    std::vector<glm::vec4> starColor;
    std::vector<glm::vec3> starVertex;

    for (int i = 0; i < (int)NumStars; i++)
    {
        starColor.push_back(glm::vec4(stars[i][0], stars[i][1], stars[i][2], 1.0f));
        starVertex.push_back(glm::vec3(stars[i][3], stars[i][4], stars[i][5]));
    }
    starVBOIndex = vboVC.vboAlloc(NumStars);
    vboVC.colorData(starVBOIndex, starColor);
    vboVC.vertexData(starVBOIndex, starVertex);

    cloudVBOIndex = vboVTC.vboAlloc(16);
    const int numFacesPerTexture = (NumMountainFaces +
                                    numMountainTextures - 1) / numMountainTextures;
    for (int j = 0; j < numMountainTextures; j++)
        mountainsVBOIndex[j] = vboVTN.vboAlloc(numFacesPerTexture * 4 + 6);
    setSkyVBO();
    if (cloudsAvailable)
        setCloudVBO();
    if (numMountainTextures > 0)
        setMountainsVBO();
}


void BackgroundRenderer::bzdbCallback(const std::string& name, void* data)
{
    BackgroundRenderer* br = (BackgroundRenderer*) data;
    if (name == "_skyColor")
        br->setSkyColors();
    return;
}


void BackgroundRenderer::setupGroundMaterials()
{
    TextureManager &tm = TextureManager::instance();

    // see if we have a map specified material
    const BzMaterial* bzmat = MATERIALMGR.findMaterial("GroundMaterial");

    groundTextureID = -1;
    groundTextureMatrix = NULL;

    if (bzmat == NULL)
    {
        // default ground material
        memcpy (groundColor, defaultGroundColor, sizeof(GLfloat[4]));
        groundTextureID = tm.getTextureID(BZDB.get("stdGroundTexture").c_str(), true);
    }
    else
    {
        // map specified material
        bzmat->setReference();
        memcpy (groundColor, bzmat->getDiffuse(), sizeof(GLfloat[4]));
        if (bzmat->getTextureCount() > 0)
        {
            groundTextureID = tm.getTextureID(bzmat->getTextureLocal(0).c_str(), false);
            if (groundTextureID < 0)
            {
                // use the default as a backup (default color too)
                memcpy (groundColor, defaultGroundColor, sizeof(GLfloat[4]));
                groundTextureID = tm.getTextureID(BZDB.get("stdGroundTexture").c_str(), true);
            }
            else
            {
                // only apply the texture matrix if the texture is valid
                const int texMatId = bzmat->getTextureMatrix(0);
                const TextureMatrix* texmat = TEXMATRIXMGR.getMatrix(texMatId);
                if (texmat != NULL)
                    groundTextureMatrix = texmat->getMatrix();
            }
        }
    }

    static const GLfloat  black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    OpenGLMaterial defaultMaterial(black, black, 0.0f);

    OpenGLGStateBuilder gb;

    // ground gstates
    gb.reset();
    gb.setMaterial(defaultMaterial);
    gb.setTexture(groundTextureID);
    gb.setTextureMatrix(groundTextureMatrix);
    groundGState = gb.getState();


    // default inverted ground material
    int groundInvTextureID = -1;
    memcpy (groundColorInv, defaultGroundColorInv, sizeof(GLfloat[4]));
    if (groundInvTextureID < 0)
        groundInvTextureID = tm.getTextureID(BZDB.get("zoneGroundTexture").c_str(), false);

    // inverted ground gstates
    gb.reset();
    gb.setMaterial(defaultMaterial);
    gb.setTexture(groundInvTextureID);
    invGroundGState = gb.getState();

    return;
}


void BackgroundRenderer::notifyStyleChange()
{
    // some stuff is drawn only for certain states
    cloudsVisible = cloudsAvailable;
    mountainsVisible = mountainsAvailable;
    shadowsVisible = BZDB.isTrue("shadows");
    starGStateIndex = BZDB.isTrue("smooth");

    // fixup gstates
    OpenGLGStateBuilder gstate;
    gstate.reset();
    if (BZDB.isTrue("smooth"))
    {
        gstate.setBlending();
        gstate.setSmoothing();
    }
    gridGState = gstate.getState();
}


void BackgroundRenderer::resize()
{
    resizeSky();
    doFreeDisplayLists();
    doInitDisplayLists();
    setSkyVBO();
    if (numMountainTextures > 0)
        setMountainsVBO();
}


void BackgroundRenderer::setCelestial(const float sunDir[3],
                                      const float moonDir[3])
{
    // set sun and moon positions
    sunDirection[0] = sunDir[0];
    sunDirection[1] = sunDir[1];
    sunDirection[2] = sunDir[2];
    moonDirection[0] = moonDir[0];
    moonDirection[1] = moonDir[1];
    moonDirection[2] = moonDir[2];

    setSkyVBO();

    return;
}


void BackgroundRenderer::setSkyColors()
{
    // change sky colors according to the sun position
    GLfloat colors[4][3];
    getSkyColor(sunDirection, colors);

    skyZenithColor[0] = colors[0][0];
    skyZenithColor[1] = colors[0][1];
    skyZenithColor[2] = colors[0][2];
    skySunDirColor[0] = colors[1][0];
    skySunDirColor[1] = colors[1][1];
    skySunDirColor[2] = colors[1][2];
    skyAntiSunDirColor[0] = colors[2][0];
    skyAntiSunDirColor[1] = colors[2][1];
    skyAntiSunDirColor[2] = colors[2][2];
    skyCrossSunDirColor[0] = colors[3][0];
    skyCrossSunDirColor[1] = colors[3][1];
    skyCrossSunDirColor[2] = colors[3][2];

    return;
}


void BackgroundRenderer::setSkyVBO()
{

    //
    // update objects in sky.  the appearance of these objects will
    // be wrong until setCelestial is called with the appropriate
    // arguments.
    //
    makeCelestialLists();


    glm::mat4 mat = glm::mat4(1.0f);

    // compute display list for moon
    float coverage = (moonDirection[0] * sunDirection[0]) +
                     (moonDirection[1] * sunDirection[1]) +
                     (moonDirection[2] * sunDirection[2]);
    // hack coverage to lean towards full
    coverage = (coverage < 0.0f) ? -sqrtf(-coverage) : coverage * coverage;
    float worldSize = BZDBCache::worldSize;
    const float moonRadius = 2.0f * worldSize *
                             atanf((float)((60.0 * M_PI / 180.0) / 60.0));
    // limbAngle is dependent on moon position but sun is so much farther
    // away that the moon's position is negligible.  rotate sun and moon
    // so that moon is on the horizon in the +x direction, then compute
    // the angle to the sun position in the yz plane.
    float sun2[3];
    const float moonAzimuth = atan2f(moonDirection[1], moonDirection[0]);
    const float moonAltitude = asinf(moonDirection[2]);
    sun2[0] = sunDirection[0] * cosf(moonAzimuth) + sunDirection[1] * sinf(moonAzimuth);
    sun2[1] = sunDirection[1] * cosf(moonAzimuth) - sunDirection[0] * sinf(moonAzimuth);
    sun2[2] = sunDirection[2] * cosf(moonAltitude) - sun2[0] * sinf(moonAltitude);
    const float limbAngle = atan2f(sun2[2], sun2[1]);

    mat = glm::mat4(1.0f);
    mat = glm::rotate(
              mat,
              (GLfloat)atan2f(moonDirection[1], moonDirection[0]),
              glm::vec3(0.0f, 0.0f, 1.0f));
    mat = glm::rotate(
              mat,
              (GLfloat)asinf(moonDirection[2]),
              glm::vec3(0.0f, -1.0f, 0.0f));
    mat = glm::rotate(
              mat,
              (GLfloat)limbAngle,
              glm::vec3(1.0f, 0.0f, 0.0f));
    std::vector<glm::vec3> moonVertex;
    moonVertex.push_back(mat * glm::vec4(2.0f * worldSize, 0.0f, -moonRadius, 0.0f));
    for (int i = 0; i < moonSegements - 1; i++)
    {
        const float angle = (float)(M_PI * double(i - moonSegements / 2 - 1) /
                                    moonSegements);
        float sinAngle = sinf(angle);
        float cosAngle = cosf(angle);
        moonVertex.push_back(mat * glm::vec4(
                                 2.0f * worldSize,
                                 coverage * moonRadius * cosAngle,
                                 moonRadius * sinAngle,
                                 0.0f));
        moonVertex.push_back(mat * glm::vec4(
                                 2.0f * worldSize,
                                 moonRadius * cosAngle,
                                 moonRadius * sinAngle,
                                 0.0f));
    }
    moonVertex.push_back(mat * glm::vec4(
                             2.0f * worldSize,
                             0.0f,
                             moonRadius,
                             0.0f));
    vboV.vertexData(moonVBOIndex, moonVertex);
}


void BackgroundRenderer::setCloudVBO()
{
    int i;

    const float worldSize = BZDBCache::worldSize;

    const GLfloat groundSize = 10.0f * worldSize;
    glm::vec2 groundPlane[4];
    for (i = 0; i < 4; i++)
        groundPlane[i] = groundSize * squareShape[i];

    // make vertices for cloud polygons
    glm::vec3 cloudsOuter[4], cloudsInner[4];
    const GLfloat uvScale = 0.25f;
    for (i = 0; i < 4; i++)
    {
        GLfloat height = 120.0f * BZDBCache::tankHeight;
        cloudsOuter[i] = glm::vec3(groundPlane[i], height);
        cloudsInner[i] = glm::vec3(uvScale * groundPlane[i], height);
    }

    std::vector<glm::vec4> cloudColor;
    std::vector<glm::vec3> cloudVertex;
    std::vector<glm::vec2> cloudTexture;

    // inner clouds -- full opacity
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[3]);
    cloudVertex.push_back(cloudsInner[3]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[2]);
    cloudVertex.push_back(cloudsInner[2]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[0]);
    cloudVertex.push_back(cloudsInner[0]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsInner[1]);

    // Insert degenerated triangles
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsInner[1]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsOuter[1]);

    // outer clouds -- fade to zero opacity at outer edge
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    cloudTexture.push_back(cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsOuter[1]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsInner[1]);

    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    cloudTexture.push_back(cloudRepeats * squareShape[2]);
    cloudVertex.push_back(cloudsOuter[2]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[2]);
    cloudVertex.push_back(cloudsInner[2]);

    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    cloudTexture.push_back(cloudRepeats * squareShape[3]);
    cloudVertex.push_back(cloudsOuter[3]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[3]);
    cloudVertex.push_back(cloudsInner[3]);

    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    cloudTexture.push_back(cloudRepeats * squareShape[0]);
    cloudVertex.push_back(cloudsOuter[0]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[0]);
    cloudVertex.push_back(cloudsInner[0]);

    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    cloudTexture.push_back(cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsOuter[1]);
    cloudColor.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    cloudTexture.push_back(uvScale * cloudRepeats * squareShape[1]);
    cloudVertex.push_back(cloudsInner[1]);

    vboVTC.vertexData(cloudVBOIndex, cloudVertex);
    vboVTC.textureData(cloudVBOIndex, cloudTexture);
    vboVTC.colorData(cloudVBOIndex, cloudColor);
}


//
// mountains
//
void BackgroundRenderer::setMountainsVBO()
{
    int i, j;

    const float worldSize = BZDBCache::worldSize;

    // prepare display lists.  need at least NumMountainFaces, but
    // we also need a multiple of the number of subtextures.  put
    // all the faces using a given texture into the same list.
    const int numFacesPerTexture = (NumMountainFaces +
                                    numMountainTextures - 1) / numMountainTextures;
    const float angleScale = (float)(M_PI / (numMountainTextures * numFacesPerTexture));
    int n = numFacesPerTexture / 2;
    float hightScale = mountainsMinWidth / 256.0f;
    for (j = 0; j < numMountainTextures; n += numFacesPerTexture, j++)
    {
        float angle = 0.0f;
        float frac  = 0.0f;

        std::vector<glm::vec3> mountainsVertex;
        std::vector<glm::vec3> mountainsNormal;
        std::vector<glm::vec2> mountainsTexture;

        glm::vec3 normal;

        for (i = 0; i <= numFacesPerTexture; i++)
        {
            angle = angleScale * (float)(i + n);
            frac = (float)i / (float)numFacesPerTexture;
            if (numMountainTextures != 1)
                frac *= ((float)(mountainsMinWidth - 2) + 1.0f) /
                        (float)mountainsMinWidth;
            normal = glm::vec3(-M_SQRT1_2 * cosf(angle),
                               -M_SQRT1_2 * sinf(angle),
                               M_SQRT1_2);
            mountainsNormal.push_back(normal);
            mountainsTexture.push_back(glm::vec2(frac, 0.02f));
            mountainsVertex.push_back(glm::vec3(
                                          2.25f * worldSize * cosf(angle),
                                          2.25f * worldSize * sinf(angle),
                                          0.0f));
            mountainsNormal.push_back(normal);
            mountainsTexture.push_back(glm::vec2(frac, 0.99f));
            mountainsVertex.push_back(glm::vec3(
                                          2.25f * worldSize * cosf(angle),
                                          2.25f * worldSize * sinf(angle),
                                          0.45f * worldSize * hightScale));
        }
        mountainsNormal.push_back(normal);
        mountainsTexture.push_back(glm::vec2(frac, 0.99f));
        mountainsVertex.push_back(glm::vec3(
                                      2.25f * worldSize * cosf(angle),
                                      2.25f * worldSize * sinf(angle),
                                      0.45f * worldSize * hightScale));
        angle = (float)(M_PI + angleScale * n);
        normal = glm::vec3(-M_SQRT1_2 * cosf(angle),
                           -M_SQRT1_2 * sinf(angle),
                           M_SQRT1_2);
        mountainsNormal.push_back(normal);
        mountainsTexture.push_back(glm::vec2(0.0f, 0.02f));
        mountainsVertex.push_back(glm::vec3(
                                      2.25f * worldSize * cosf(angle),
                                      2.25f * worldSize * sinf(angle),
                                      0.0f));
        for (i = 0; i <= numFacesPerTexture; i++)
        {
            angle = (float)(M_PI + angleScale * (double)(i + n));
            frac = (float)i / (float)numFacesPerTexture;
            if (numMountainTextures != 1)
                frac *= ((float)(mountainsMinWidth - 2) + 1.0f) /
                        (float)mountainsMinWidth;
            normal = glm::vec3(-M_SQRT1_2 * cosf(angle),
                               -M_SQRT1_2 * sinf(angle),
                               M_SQRT1_2);
            mountainsNormal.push_back(normal);
            mountainsTexture.push_back(glm::vec2(frac, 0.02f));
            mountainsVertex.push_back(glm::vec3(
                                          2.25f * worldSize * cosf(angle),
                                          2.25f * worldSize * sinf(angle),
                                          0.0f));
            mountainsNormal.push_back(normal);
            mountainsTexture.push_back(glm::vec2(frac, 0.99f));
            mountainsVertex.push_back(glm::vec3(
                                          2.25f * worldSize * cosf(angle),
                                          2.25f * worldSize * sinf(angle),
                                          0.45f * worldSize*hightScale));
        }
        vboVTN.textureData(mountainsVBOIndex[j], mountainsTexture);
        vboVTN.normalData(mountainsVBOIndex[j], mountainsNormal);
        vboVTN.vertexData(mountainsVBOIndex[j],mountainsVertex);
    }
}


void BackgroundRenderer::makeCelestialLists()
{
    setSkyColors();

    // get a few other things concerning the sky
    doShadows = areShadowsCast(sunDirection);
    doStars = areStarsVisible(sunDirection);
    doSunset = getSunsetTop(sunDirection, sunsetTop);

    return;
}


void BackgroundRenderer::addCloudDrift(GLfloat uDrift, GLfloat vDrift)
{
    cloudDriftU += 0.01f * uDrift / cloudRepeats;
    cloudDriftV += 0.01f * vDrift / cloudRepeats;
    if (cloudDriftU > 1.0f) cloudDriftU -= 1.0f;
    else if (cloudDriftU < 0.0f) cloudDriftU += 1.0f;
    if (cloudDriftV > 1.0f) cloudDriftV -= 1.0f;
    else if (cloudDriftV < 0.0f) cloudDriftV += 1.0f;
}


void BackgroundRenderer::renderSky(SceneRenderer& renderer, bool mirror)
{
    if (!BZDBCache::drawSky)
        return;
    drawSky(renderer, mirror);
}


void BackgroundRenderer::renderGround()
{
    drawGround();
}


void BackgroundRenderer::renderGroundEffects(SceneRenderer& renderer,
        bool drawingMirror)
{
    // zbuffer should be disabled.  either everything is coplanar with
    // the ground or is drawn back to front and is occluded by everything
    // drawn after it.  also use projection with very far clipping plane.

    if (!blank)
    {
        if (doShadows && shadowsVisible && !drawingMirror)
            drawGroundShadows(renderer);

        // draw light receivers on ground (little meshes under light sources so
        // the ground gets illuminated).  this is necessary because lighting is
        // performed only at a vertex, and the ground's vertices are a few
        // kilometers away.
        if (!drawingMirror && BZDBCache::drawGroundLights)
        {
            if (BZDBCache::tesselation && (renderer.useQuality() >= 1))
            {
//    (BZDB.get(StateDatabase::BZDB_FOGMODE) == "none")) {
                // not really tesselation, but it is tied to the "Best" lighting,
                // avoid on foggy maps, because the blending function accumulates
                // too much brightness.
                drawAdvancedGroundReceivers(renderer);
            }
            else
                drawGroundReceivers(renderer);
        }

        {
            // light the mountains (so that they get dark when the sun goes down).
            // don't do zbuffer test since they occlude all drawn before them and
            // are occluded by all drawn after.
            if (mountainsVisible && BZDBCache::drawMountains)
                drawMountains();

            // draw clouds
            if (cloudsVisible && BZDBCache::drawClouds)
            {
                cloudsGState.setState();
                glMatrixMode(GL_TEXTURE);
                glPushMatrix();
                glTranslatef(cloudDriftU, cloudDriftV, 0.0f);
                glNormal3f(0.0f, 0.0f, 1.0f);
                vboVTC.enableArrays();
                glDrawArrays(
                    GL_TRIANGLE_STRIP,
                    cloudVBOIndex,
                    16);
                glLoadIdentity();   // maybe works around bug in some systems
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
            }
        }
    }
}


void BackgroundRenderer::renderEnvironment(SceneRenderer& renderer, bool update)
{
    if (blank)
        return;

    if (update)
        weather.update();
    weather.draw(renderer);

    if (update)
        EFFECTS.update();
    EFFECTS.draw(renderer);
}


void BackgroundRenderer::resizeSky()
{
    // sky pyramid must fit inside far clipping plane
    // (adjusted for the deepProjection matrix)
    const GLfloat skySize = 3.0f * BZDBCache::worldSize;
    for (int i = 0; i < 4; i++)
    {
        skyPyramid[i][0] = skySize * squareShape[i][0];
        skyPyramid[i][1] = skySize * squareShape[i][1];
        skyPyramid[i][2] = 0.0f;
    }
    skyPyramid[4][0] = 0.0f;
    skyPyramid[4][1] = 0.0f;
    skyPyramid[4][2] = skySize;
}


void BackgroundRenderer::setupSkybox()
{
    haveSkybox = false;

    int i;
    const char *skyboxNames[6] =
    {
        "LeftSkyboxMaterial",
        "FrontSkyboxMaterial",
        "RightSkyboxMaterial",
        "BackSkyboxMaterial",
        "TopSkyboxMaterial",
        "BottomSkyboxMaterial"
    };
    TextureManager& tm = TextureManager::instance();
    const BzMaterial* bzmats[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

    // try to load the textures
    for (i = 0; i < 6; i++)
    {
        bzmats[i] = MATERIALMGR.findMaterial(skyboxNames[i]);
        if ((bzmats[i] == NULL) || (bzmats[i]->getTextureCount() <= 0))
            break;
        skyboxTexID[i] = tm.getTextureID(bzmats[i]->getTextureLocal(0).c_str());
        if (skyboxTexID[i] < 0)
            break;
    }

    // unload textures if they were not all successful
    if (i != 6)
    {
        while (i >= 0)
        {
            if ((bzmats[i] != NULL) && (bzmats[i]->getTextureCount() > 0))
            {
                // NOTE: this could delete textures the might be used elsewhere
                tm.removeTexture(bzmats[i]->getTextureLocal(0).c_str());
            }
            i--;
        }
        return;
    }

    // reference map specified materials
    for (i = 0; i < 6; i++)
        bzmats[i]->setReference();

    // setup the corner colors
    const int cornerFaces[8][3] =
    {
        {5, 0, 1}, {5, 1, 2}, {5, 2, 3}, {5, 3, 0},
        {4, 0, 1}, {4, 1, 2}, {4, 2, 3}, {4, 3, 0}
    };
    for (i = 0; i < 8; i++)
    {
        for (int c = 0; c < 4; c++)
        {
            skyboxColor[i][c] = 0.0f;
            for (int f = 0; f < 3; f++)
                skyboxColor[i][c] += bzmats[cornerFaces[i][f]]->getDiffuse()[c];
            skyboxColor[i][c] /= 3.0f;
        }
    }

    haveSkybox = true;

    return;
}


void BackgroundRenderer::drawSkybox()
{
    // sky box must fit inside far clipping plane
    // (adjusted for the deepProjection matrix)
    const float d = 3.0f * BZDBCache::worldSize;
    const glm::vec3 verts[8] =
    {
        {-d, -d, -d}, {+d, -d, -d}, {+d, +d, -d}, {-d, +d, -d},
        {-d, -d, +d}, {+d, -d, +d}, {+d, +d, +d}, {-d, +d, +d}
    };
    const glm::vec2 txcds[4] =
    {
        {1.0f, 0.0f}, {0.0f, 0.0f},
        {0.0f, 1.0f}, {1.0f, 1.0f}
    };

    TextureManager& tm = TextureManager::instance();

    OpenGLGState::resetState();

    const GLfloat (*color)[4] = skyboxColor;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);

    glm::vec2 textures[4];
    glm::vec4 colors[4];
    glm::vec3 vertices[4];

    textures[0] = txcds[0];
    textures[1] = txcds[1];
    textures[2] = txcds[3];
    textures[3] = txcds[2];

    int vboIndex = vboVTC.vboAlloc(4);

    vboVTC.textureData(vboIndex, 4, textures);

    if (!BZDBCache::drawGround)
    {
        tm.bind(skyboxTexID[5]); // bottom
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        colors[0]   = glm::vec4(glm::make_vec3(color[2]), 1);
        colors[1]   = glm::vec4(glm::make_vec3(color[3]), 1);
        colors[2]   = glm::vec4(glm::make_vec3(color[1]), 1);
        colors[3]   = glm::vec4(glm::make_vec3(color[0]), 1);
        vertices[0] = verts[2];
        vertices[1] = verts[3];
        vertices[2] = verts[1];
        vertices[3] = verts[0];

        vboVTC.colorData(vboIndex, 4, colors);
        vboVTC.vertexData(vboIndex, 4, vertices);
        vboVTC.enableArrays();
        glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
    }

    tm.bind(skyboxTexID[4]); // top
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    colors[0]   = glm::vec4(glm::make_vec3(color[5]), 1);
    colors[1]   = glm::vec4(glm::make_vec3(color[4]), 1);
    colors[2]   = glm::vec4(glm::make_vec3(color[6]), 1);
    colors[3]   = glm::vec4(glm::make_vec3(color[7]), 1);
    vertices[0] = verts[5];
    vertices[1] = verts[4];
    vertices[2] = verts[6];
    vertices[3] = verts[7];

    vboVTC.colorData(vboIndex, 4, colors);
    vboVTC.vertexData(vboIndex, 4, vertices);
    vboVTC.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);

    tm.bind(skyboxTexID[0]); // left
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    colors[0]   = glm::vec4(glm::make_vec3(color[0]), 1);
    colors[1]   = glm::vec4(glm::make_vec3(color[3]), 1);
    colors[2]   = glm::vec4(glm::make_vec3(color[6]), 1);
    colors[3]   = glm::vec4(glm::make_vec3(color[7]), 1);
    vertices[0] = verts[0];
    vertices[1] = verts[3];
    vertices[2] = verts[6];
    vertices[3] = verts[7];

    vboVTC.colorData(vboIndex, 4, colors);
    vboVTC.vertexData(vboIndex, 4, vertices);
    vboVTC.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);

    tm.bind(skyboxTexID[1]); // front
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    colors[0]   = glm::vec4(glm::make_vec3(color[1]), 1);
    colors[1]   = glm::vec4(glm::make_vec3(color[0]), 1);
    colors[2]   = glm::vec4(glm::make_vec3(color[5]), 1);
    colors[3]   = glm::vec4(glm::make_vec3(color[4]), 1);
    vertices[0] = verts[1];
    vertices[1] = verts[0];
    vertices[2] = verts[5];
    vertices[3] = verts[4];

    vboVTC.colorData(vboIndex, 4, colors);
    vboVTC.vertexData(vboIndex, 4, vertices);
    vboVTC.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);

    tm.bind(skyboxTexID[2]); // right
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    colors[0]   = glm::vec4(glm::make_vec3(color[2]), 1);
    colors[1]   = glm::vec4(glm::make_vec3(color[1]), 1);
    colors[2]   = glm::vec4(glm::make_vec3(color[6]), 1);
    colors[3]   = glm::vec4(glm::make_vec3(color[5]), 1);
    vertices[0] = verts[2];
    vertices[1] = verts[1];
    vertices[2] = verts[6];
    vertices[3] = verts[5];

    vboVTC.colorData(vboIndex, 4, colors);
    vboVTC.vertexData(vboIndex, 4, vertices);
    vboVTC.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);

    tm.bind(skyboxTexID[3]); // back
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    colors[0]   = glm::vec4(glm::make_vec3(color[3]), 1);
    colors[1]   = glm::vec4(glm::make_vec3(color[2]), 1);
    colors[2]   = glm::vec4(glm::make_vec3(color[7]), 1);
    colors[3]   = glm::vec4(glm::make_vec3(color[6]), 1);
    vertices[0] = verts[3];
    vertices[1] = verts[2];
    vertices[2] = verts[7];
    vertices[3] = verts[6];

    vboVTC.colorData(vboIndex, 4, colors);
    vboVTC.vertexData(vboIndex, 4, vertices);
    vboVTC.enableArrays();
    glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, 4);
    vboVTC.vboFree(vboIndex);

    glShadeModel(GL_FLAT);
    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}


void BackgroundRenderer::drawSky(SceneRenderer& renderer, bool mirror)
{
    glPushMatrix();

    const bool doSkybox = haveSkybox;

    if (!doSkybox)
    {
        // rotate sky so that horizon-point-toward-sun-color is actually
        // toward the sun
        glRotatef((GLfloat)((atan2f(sunDirection[1], sunDirection[0]) * 180.0 + 135.0) / M_PI),
                  0.0f, 0.0f, 1.0f);

        // draw sky
        skyGState.setState();
        if (!doSunset)
        {
            glm::vec4 colors[6];
            glm::vec3 vertices[6];

            colors[0] = glm::vec4(skyZenithColor, 1);
            colors[1] = glm::vec4(skyCrossSunDirColor, 1);
            colors[2] = glm::vec4(skySunDirColor, 1);
            colors[3] = glm::vec4(skyCrossSunDirColor, 1);
            colors[4] = glm::vec4(skyAntiSunDirColor, 1);
            colors[5] = glm::vec4(skyCrossSunDirColor, 1);

            vertices[0] = skyPyramid[4];
            vertices[1] = skyPyramid[0];
            vertices[2] = skyPyramid[3];
            vertices[3] = skyPyramid[2];
            vertices[4] = skyPyramid[1];
            vertices[5] = skyPyramid[0];

            int vboIndex = vboVC.vboAlloc(6);
            vboVC.colorData(vboIndex, 6, colors);
            vboVC.vertexData(vboIndex, 6, vertices);
            vboVC.enableArrays();

            // just a pyramid
            glDrawArrays(GL_TRIANGLE_FAN, vboIndex, 6);
            vboVC.vboFree(vboIndex);
        }
        else
        {
            glm::vec3 sunsetTopPoint;
            sunsetTopPoint[0] = skyPyramid[3][0] * (1.0f - sunsetTop);
            sunsetTopPoint[1] = skyPyramid[3][1] * (1.0f - sunsetTop);
            sunsetTopPoint[2] = skyPyramid[4][2] * sunsetTop;

            // overall shape is a pyramid, but the solar sides are two
            // triangles each.  the top triangle is all zenith color,
            // the bottom goes from zenith to sun-dir color.
            glm::vec4 colors[16];
            glm::vec3 vertices[16];

            colors[0] = glm::vec4(skyZenithColor, 1);
            colors[1] = glm::vec4(skyCrossSunDirColor, 1);
            colors[2] = glm::vec4(skyAntiSunDirColor, 1);
            colors[3] = glm::vec4(skyCrossSunDirColor, 1);
            colors[4] = glm::vec4(skyZenithColor, 1);
            colors[5] = glm::vec4(skyCrossSunDirColor, 1);
            colors[6] = glm::vec4(skyZenithColor, 1);
            colors[7] = glm::vec4(skyZenithColor, 1);
            colors[8] = glm::vec4(skyZenithColor, 1);
            colors[9] = glm::vec4(skyCrossSunDirColor, 1);
            colors[10] = glm::vec4(skyZenithColor, 1);
            colors[11] = glm::vec4(skyCrossSunDirColor, 1);
            colors[12] = glm::vec4(skySunDirColor, 1);
            colors[13] = glm::vec4(skyCrossSunDirColor, 1);
            colors[14] = glm::vec4(skyZenithColor, 1);
            colors[15] = glm::vec4(skySunDirColor, 1);

            vertices[0] = skyPyramid[4];
            vertices[1] = skyPyramid[2];
            vertices[2] = skyPyramid[1];
            vertices[3] = skyPyramid[0];
            vertices[4] = skyPyramid[4];
            vertices[5] = skyPyramid[0];
            vertices[6] = sunsetTopPoint;
            vertices[7] = skyPyramid[4];
            vertices[8] = sunsetTopPoint;
            vertices[9] = skyPyramid[2];
            vertices[10] = sunsetTopPoint;
            vertices[11] = skyPyramid[0];
            vertices[12] = skyPyramid[3];
            vertices[13] = skyPyramid[2];
            vertices[14] = sunsetTopPoint;
            vertices[15] = skyPyramid[3];

            int vboIndex = vboVC.vboAlloc(16);
            vboVC.colorData(vboIndex, 16, colors);
            vboVC.vertexData(vboIndex, 16, vertices);
            vboVC.enableArrays();

            glDrawArrays(GL_TRIANGLE_FAN, vboIndex, 4);
            glDrawArrays(GL_TRIANGLES, vboIndex + 4, 12);
            vboVC.vboFree(vboIndex);
        }
    }

    glLoadIdentity();
    renderer.getViewFrustum().executeOrientation();

    const bool useClipPlane = (mirror && (doSkybox || BZDBCache::drawCelestial));

    if (useClipPlane)
        glEnable(GL_CLIP_PLANE0);

    if (doSkybox)
        drawSkybox();

    if (BZDBCache::drawCelestial)
    {
        float worldSize = BZDBCache::worldSize;
        if (sunDirection[2] > -0.009f)
        {
            sunGState.setState();
            const GLfloat *col = renderer.getSunScaledColor();
            glColor4f(col[0], col[1], col[2], 1.0f);
            vboV.enableArrays();
            glPushMatrix();
            glRotatef((GLfloat)(atan2f(sunDirection[1], (sunDirection[0])) * 180.0 / M_PI),
                      0.0f, 0.0f, 1.0f);
            glRotatef((GLfloat)(asinf(sunDirection[2]) * 180.0 / M_PI), 0.0f, -1.0f, 0.0f);
            glScalef(2.0f * worldSize, 2.0f * worldSize, 2.0f * worldSize);
            glDrawArrays(GL_TRIANGLE_FAN, sunVBOIndex, 21);
            glPopMatrix();
        }

        if (doStars)
        {
            starGState[starGStateIndex].setState();
            vboVC.enableArrays();
            glPushMatrix();
            glMultMatrixf(renderer.getCelestialTransform());
            glScalef(worldSize, worldSize, worldSize);
            glDrawArrays(GL_POINTS, starVBOIndex, NumStars);
            glPopMatrix();
        }

        if (moonDirection[2] > -0.009f)
        {
            moonGState[doStars ? 1 : 0].setState();
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            vboV.enableArrays();
            glDrawArrays(GL_TRIANGLE_STRIP, moonVBOIndex, 2 * moonSegements);
        }
    }

    if (useClipPlane)
        glDisable(GL_CLIP_PLANE0);

    glPopMatrix();
}


void BackgroundRenderer::drawGround()
{
    if (!BZDBCache::drawGround)
        return;

    {
        // draw ground
        glNormal3f(0.0f, 0.0f, 1.0f);
        if (invert)
        {
            glColor4f(groundColorInv[0],
                      groundColorInv[1],
                      groundColorInv[2],
                      groundColorInv[3]);
            invGroundGState.setState();
        }
        else
        {
            float color[4];
            if (BZDB.isSet("GroundOverideColor") &&
                    parseColorString(BZDB.get("GroundOverideColor"), color))
                glColor4f(color[0], color[1], color[2], color[3]);
            else
                glColor4f(groundColor[0], groundColor[1], groundColor[2], groundColor[3]);
            groundGState.setState();
        }

        drawGroundCentered();
    }
}

void BackgroundRenderer::drawGroundCentered()
{
    const float groundSize = 10.0f * BZDBCache::worldSize;
    const float centerSize = 128.0f;

    const ViewFrustum& frustum = RENDERER.getViewFrustum();
    float center[2] = { frustum.getEye()[0], frustum.getEye()[1] };
    const float minDist = -groundSize + centerSize;
    const float maxDist = +groundSize - centerSize;
    if (center[0] < minDist)
        center[0] = minDist;
    if (center[0] > maxDist)
        center[0] = maxDist;
    if (center[1] < minDist)
        center[1] = minDist;
    if (center[1] > maxDist)
        center[1] = maxDist;

    const glm::vec2 vertices[8] =
    {
        { -groundSize, -groundSize },
        { +groundSize, -groundSize },
        { +groundSize, +groundSize },
        { -groundSize, +groundSize },
        { center[0] - centerSize, center[1] - centerSize },
        { center[0] + centerSize, center[1] - centerSize },
        { center[0] + centerSize, center[1] + centerSize },
        { center[0] - centerSize, center[1] + centerSize }
    };

    const float repeat = BZDB.eval("groundHighResTexRepeat");
    const int indices[5][4] =
    {
        { 4, 5, 7, 6 },
        { 0, 1, 4, 5 },
        { 1, 2, 5, 6 },
        { 2, 3, 6, 7 },
        { 3, 0, 7, 4 },
    };

    glNormal3f(0.0f, 0.0f, 1.0f);
    {
        glm::vec2 textures[20];
        glm::vec3 vertex[20];

        int vboIndex = vboVT.vboAlloc(20);

        int k = 0;
        for (int q = 0; q < 5; q++)
        {
            for (int c = 0; c < 4; c++)
            {
                const int index = indices[q][c];
                textures[k] = vertices[index] * repeat;
                vertex[k]   = glm::vec3(vertices[index], 0);
                k++;
            }
        }
        vboVT.textureData(vboIndex, 20, textures);
        vboVT.vertexData(vboIndex, 20, vertex);
        k = 0;
        vboVT.enableArrays();
        for (int q = 0; q < 5; q++)
        {
            glDrawArrays(GL_TRIANGLE_STRIP, vboIndex + k, 4);
            k+= 4;
        }
        vboVT.vboFree(vboIndex);
    }

    return;
}


void BackgroundRenderer::drawGroundGrid(
    SceneRenderer& renderer)
{
    const GLfloat* pos = renderer.getViewFrustum().getEye();
    const GLfloat xhalf = gridSpacing * (gridCount + floorf(pos[2] / 4.0f));
    const GLfloat yhalf = gridSpacing * (gridCount + floorf(pos[2] / 4.0f));
    const GLfloat x0 = floorf(pos[0] / gridSpacing) * gridSpacing;
    const GLfloat y0 = floorf(pos[1] / gridSpacing) * gridSpacing;
    GLfloat i;

    gridGState.setState();

    std::vector<glm::vec3> vertices;

    // x lines
    for (i = -xhalf; i <= xhalf; i += gridSpacing)
    {
        vertices.push_back(glm::vec3(x0 + i, y0 - yhalf, 0));
        vertices.push_back(glm::vec3(x0 + i, y0 + yhalf, 0));
    }
    int xSize = vertices.size();
    /* z lines */
    for (i = -yhalf; i <= yhalf; i += gridSpacing)
    {
        vertices.push_back(glm::vec3(x0 - xhalf, y0 + i, 0));
        vertices.push_back(glm::vec3(x0 + xhalf, y0 + i, 0));
    }

    int vboIndex = vboV.vboAlloc(vertices.size());
    vboV.vertexData(vboIndex, vertices);
    vboV.enableArrays();

    if (doShadows) glColor4f(0.0f, 0.75f, 0.5f, 1.0f);
    else glColor4f(0.0f, 0.4f, 0.3f, 1.0f);

    glDrawArrays(GL_LINES, vboIndex, xSize);

    if (doShadows) glColor4f(0.5f, 0.75f, 0.0f, 1.0f);
    else glColor4f(0.3f, 0.4f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, vboIndex + xSize, vertices.size() - xSize);
    vboV.vboFree(vboIndex);
}

void BackgroundRenderer::drawGroundShadows(
    SceneRenderer& renderer)
{
    // draw sun shadows -- always stippled so overlapping shadows don't
    // accumulate darkness.  make and multiply by shadow projection matrix.
    GLfloat shadowProjection[16];
    shadowProjection[0] = shadowProjection[5] = shadowProjection[15] = 1.0f;
    shadowProjection[8] = -sunDirection[0] / sunDirection[2];
    shadowProjection[9] = -sunDirection[1] / sunDirection[2];
    shadowProjection[1] = shadowProjection[2] =
                              shadowProjection[3] = shadowProjection[4] =
                                          shadowProjection[6] = shadowProjection[7] =
                                                  shadowProjection[10] = shadowProjection[11] =
                                                          shadowProjection[12] = shadowProjection[13] =
                                                                  shadowProjection[14] = 0.0f;
    glPushMatrix();
    glMultMatrixf(shadowProjection);

    // disable color updates
    SceneNode::setColorOverride(true);

    // disable the unused arrays
    VBO_Handler::globalArraysEnabling(false);

    if (BZDBCache::stencilShadows)
    {
        OpenGLGState::resetState();
        const float shadowAlpha = BZDB.eval("shadowAlpha");
        glColor4f(0.0f, 0.0f, 0.0f, shadowAlpha);
        if (shadowAlpha < 1.0f)
        {
            // use the stencil to avoid overlapping shadows
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilFunc(GL_NOTEQUAL, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glEnable(GL_STENCIL_TEST);

            // turn on blending, and kill culling
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
        }
    }
    else
    {
        // use stippling to avoid overlapping shadows
        sunShadowsGState.setState();
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // render those nodes
    renderer.getShadowList().render();

    // revert to OpenGLGState defaults
    if (BZDBCache::stencilShadows)
    {
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);
        glBlendFunc(GL_ONE, GL_ZERO);
    }

    // enable color updates
    SceneNode::setColorOverride(false);

    OpenGLGState::resetState();

    // re-enable the arrays
    VBO_Handler::globalArraysEnabling(true);

    glPopMatrix();
}


static void setupBlackFog(float fogColor[4])
{
    static const float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glGetFloatv(GL_FOG_COLOR, fogColor);
    glFogfv(GL_FOG_COLOR, black);
}


void BackgroundRenderer::drawGroundReceivers(SceneRenderer& renderer)
{
    static const int receiverRings = 4;
    static const int receiverSlices = 8;
    static const float receiverRingSize = 1.2f;   // meters
    static glm::vec2 angle[receiverSlices + 1];

    static bool init = false;
    if (!init)
    {
        init = true;
        const float receiverSliceAngle = (float)(2.0 * M_PI / double(receiverSlices));
        for (int i = 0; i <= receiverSlices; i++)
        {
            angle[i][0] = cosf((float)i * receiverSliceAngle);
            angle[i][1] = sinf((float)i * receiverSliceAngle);
        }
    }

    const int count = renderer.getNumAllLights();
    if (count == 0)
        return;

    // bright sun dims intensity of ground receivers
    const float B = 1.0f - (0.6f * renderer.getSunBrightness());

    receiverGState.setState();

    // setup black fog
    float fogColor[4];
    setupBlackFog(fogColor);

    glPushMatrix();
    int i, j;
    for (int k = 0; k < count; k++)
    {
        const OpenGLLight& light = renderer.getLight(k);
        if (light.getOnlyReal())
            continue;

        const GLfloat* pos = light.getPosition();
        const GLfloat* lightColor = light.getColor();
        const GLfloat* atten = light.getAttenuation();

        // point under light
        float d = pos[2];
        float I = B / (atten[0] + d * (atten[1] + d * atten[2]));

        // maximum value
        const float maxVal = (lightColor[0] > lightColor[1]) ?
                             ((lightColor[0] > lightColor[2]) ?
                              lightColor[0] : lightColor[2]) :
                             ((lightColor[1] > lightColor[2]) ?
                              lightColor[1] : lightColor[2]);

        // if I is too attenuated, don't bother drawing anything
        if ((I * maxVal) < 0.02f)
            continue;

        // move to the light's position
        glTranslatef(pos[0], pos[1], 0.0f);

        // set the main lighting color
        glm::vec4 color;
        color[0] = lightColor[0];
        color[1] = lightColor[1];
        color[2] = lightColor[2];
        color[3] = I;

        // draw ground receiver, computing lighting at each vertex ourselves
        {
            std::vector<glm::vec4> colors;
            std::vector<glm::vec3> vertices;

            colors.push_back(color);
            vertices.push_back(glm::vec3(0, 0, 0));

            // inner ring
            d = hypotf(receiverRingSize, pos[2]);
            I = B / (atten[0] + d * (atten[1] + d * atten[2]));
            I *= pos[2] / d;
            color[3] = I;
            for (j = 0; j <= receiverSlices; j++)
            {
                colors.push_back(color);
                vertices.push_back(glm::vec3(receiverRingSize * angle[j], 0));
            }
            int vboIndex = vboVC.vboAlloc(vertices.size());
            vboVC.colorData(vboIndex, colors);
            vboVC.vertexData(vboIndex, vertices);
            vboVC.enableArrays();
            glDrawArrays(GL_TRIANGLE_FAN, vboIndex, vertices.size());
            vboVC.vboFree(vboIndex);
        }
        triangleCount += receiverSlices;

        for (i = 1; i < receiverRings; i++)
        {
            const GLfloat innerSize = receiverRingSize * GLfloat(i * i);
            const GLfloat outerSize = receiverRingSize * GLfloat((i + 1) * (i + 1));

            // compute inner and outer lit colors
            d = hypotf(innerSize, pos[2]);
            I = B / (atten[0] + d * (atten[1] + d * atten[2]));
            I *= pos[2] / d;
            float innerAlpha = I;

            if (i + 1 == receiverRings)
                I = 0.0f;
            else
            {
                d = hypotf(outerSize, pos[2]);
                I = B / (atten[0] + d * (atten[1] + d * atten[2]));
                I *= pos[2] / d;
            }
            float outerAlpha = I;

            std::vector<glm::vec4> colors;
            std::vector<glm::vec3> vertices;

            for (j = 0; j <= receiverSlices; j++)
            {
                color[3] = innerAlpha;
                colors.push_back(color);
                vertices.push_back(glm::vec3(angle[j] * innerSize, 0));
                color[3] = outerAlpha;
                colors.push_back(color);
                vertices.push_back(glm::vec3(angle[j] * outerSize, 0));
            }
            int vboIndex = vboVC.vboAlloc(vertices.size());
            vboVC.colorData(vboIndex, colors);
            vboVC.vertexData(vboIndex, vertices);
            vboVC.enableArrays();
            glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, vertices.size());
            vboVC.vboFree(vboIndex);
        }
        triangleCount += (receiverSlices * receiverRings * 2);

        glTranslatef(-pos[0], -pos[1], 0.0f);
    }
    glPopMatrix();

    glFogfv(GL_FOG_COLOR, fogColor);
}


void BackgroundRenderer::drawAdvancedGroundReceivers(SceneRenderer& renderer)
{
    const float minLuminance = 0.02f;
    static const int receiverSlices = 32;
    static const float receiverRingSize = 0.5f;   // meters
    static glm::vec2 angle[receiverSlices + 1];

    static bool init = false;
    if (!init)
    {
        init = true;
        const float receiverSliceAngle = (float)(2.0 * M_PI / double(receiverSlices));
        for (int i = 0; i <= receiverSlices; i++)
        {
            angle[i][0] = cosf((float)i * receiverSliceAngle);
            angle[i][1] = sinf((float)i * receiverSliceAngle);
        }
    }

    const int count = renderer.getNumAllLights();
    if (count == 0)
        return;

    // setup the ground tint
    const GLfloat* gndColor = groundColor;
    GLfloat overrideColor[4];
    if (BZDB.isSet("GroundOverideColor") &&
            parseColorString(BZDB.get("GroundOverideColor"), overrideColor))
        gndColor = overrideColor;

    const bool useTexture = groundTextureID >= 0;
    OpenGLGState advGState;
    OpenGLGStateBuilder builder;
    builder.setShading(GL_SMOOTH);
    builder.setBlending((GLenum)GL_ONE, (GLenum)GL_ONE);
    if (useTexture)
    {
        builder.setTexture(groundTextureID);
        builder.setTextureMatrix(groundTextureMatrix);
    }
    advGState = builder.getState();
    advGState.setState();

    // setup black fog
    float fogColor[4];
    setupBlackFog(fogColor);

    glm::vec4 sParam;
    glm::vec4 tParam;
    // lazy way to get texcoords
    if (useTexture)
    {
        const float repeat = BZDB.eval("groundHighResTexRepeat");
        const glm::vec4 sPlane = { repeat, 0.0f, 0.0f, 0.0f };
        const glm::vec4 tPlane = { 0.0f, repeat, 0.0f, 0.0f };

        ViewFrustum &viewFrustum = RENDERER.getViewFrustum();
        sParam = viewFrustum.eyeLinear(sPlane);
        tParam = viewFrustum.eyeLinear(tPlane);
    }

    glPushMatrix();
    int i, j;
    for (int k = 0; k < count; k++)
    {
        const OpenGLLight& light = renderer.getLight(k);
        if (light.getOnlyReal())
            continue;

        // get the light parameters
        const GLfloat* pos = light.getPosition();
        const GLfloat* lightColor = light.getColor();
        const GLfloat* atten = light.getAttenuation();

        // point under light
        float d = pos[2];
        float I = 1.0f / (atten[0] + d * (atten[1] + d * atten[2]));

        // set the main lighting color
        float baseColor[3];
        baseColor[0] = gndColor[0] * lightColor[0];
        baseColor[1] = gndColor[1] * lightColor[1];
        baseColor[2] = gndColor[2] * lightColor[2];
        if (invert)   // beats me, should just color logic op the static nodes
        {
            baseColor[0] = 1.0f - baseColor[0];
            baseColor[1] = 1.0f - baseColor[1];
            baseColor[2] = 1.0f - baseColor[2];
        }

        // maximum value
        const float maxVal = (baseColor[0] > baseColor[1]) ?
                             ((baseColor[0] > baseColor[2]) ?
                              baseColor[0] : baseColor[2]) :
                             ((baseColor[1] > baseColor[2]) ?
                              baseColor[1] : baseColor[2]);

        // if I is too attenuated, don't bother drawing anything
        if ((I * maxVal) < minLuminance)
            continue;

        // move to the light's position
        glTranslatef(pos[0], pos[1], 0.0f);

        float innerSize;
        glm::vec3 innerColor;
        float outerSize;
        glm::vec3 outerColor;

        glm::vec4 vertex;
        GLfloat   s;
        GLfloat   t;

        // draw ground receiver, computing lighting at each vertex ourselves
        {
            std::vector<glm::vec4> colors;
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec2> textures;

            // center point
            innerColor[0] = I * baseColor[0];
            innerColor[1] = I * baseColor[1];
            innerColor[2] = I * baseColor[2];
            colors.push_back(glm::vec4(innerColor, 0));

            vertex = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            if (useTexture)
            {
                s = glm::dot(vertex, sParam);
                t = glm::dot(vertex, tParam);
                textures.push_back(glm::vec2(s, t));
            }
            vertices.push_back(glm::vec3(vertex));

            // inner ring
            d = hypotf(receiverRingSize, pos[2]);
            I = 1.0f / (atten[0] + d * (atten[1] + d * atten[2]));
            I *= pos[2] / d; // diffuse angle factor
            outerColor[0] = I * baseColor[0];
            outerColor[1] = I * baseColor[1];
            outerColor[2] = I * baseColor[2];
            outerSize = receiverRingSize;
            for (j = 0; j <= receiverSlices; j++)
            {
                colors.push_back(glm::vec4(outerColor, 0));
                vertex = glm::vec4(outerSize * angle[j], 0.0f, 1.0f);
                if (useTexture)
                {
                    s = glm::dot(vertex, sParam);
                    t = glm::dot(vertex, tParam);
                    textures.push_back(glm::vec2(s, t));
                }
                vertices.push_back(glm::vec3(vertex));
            }
            int vboIndex = vboVC.vboAlloc(vertices.size());
            vboVC.colorData(vboIndex, colors);
            vboVC.vertexData(vboIndex, vertices);
            vboVC.enableArrays();
            glDrawArrays(GL_TRIANGLE_FAN, vboIndex, vertices.size());
            vboVC.vboFree(vboIndex);
        }
        triangleCount += receiverSlices;

        bool moreRings = true;
        for (i = 2; moreRings; i++)
        {
            // inner ring
            innerSize = outerSize;
            innerColor = outerColor;

            // outer ring
            outerSize = receiverRingSize * GLfloat(i * i);
            d = hypotf(outerSize, pos[2]);
            I = 1.0f / (atten[0] + d * (atten[1] + d * atten[2]));
            I *= pos[2] / d; // diffuse angle factor
            if ((I * maxVal) < minLuminance)
            {
                I = 0.0f;
                moreRings = false; // bail after this ring
            }
            outerColor[0] = I * baseColor[0];
            outerColor[1] = I * baseColor[1];
            outerColor[2] = I * baseColor[2];

            {
                std::vector<glm::vec4> colors;
                std::vector<glm::vec3> vertices;
                std::vector<glm::vec2> textures;

                for (j = 0; j <= receiverSlices; j++)
                {
                    colors.push_back(glm::vec4(innerColor, 0));
                    vertex = glm::vec4(innerSize * angle[j], 0.0f, 1.0f);
                    if (useTexture)
                    {
                        s = glm::dot(vertex, sParam);
                        t = glm::dot(vertex, tParam);
                        textures.push_back(glm::vec2(s, t));
                    }
                    vertices.push_back(glm::vec3(vertex));
                    colors.push_back(glm::vec4(outerColor, 0));
                    vertex = glm::vec4(outerSize * angle[j], 0.0f, 1.0f);
                    if (useTexture)
                    {
                        s = glm::dot(vertex, sParam);
                        t = glm::dot(vertex, tParam);
                        textures.push_back(glm::vec2(s, t));
                    }
                    vertices.push_back(glm::vec3(vertex));
                }
                int vboIndex = vboVC.vboAlloc(vertices.size());
                vboVC.colorData(vboIndex, colors);
                vboVC.vertexData(vboIndex, vertices);
                vboVC.enableArrays();
                glDrawArrays(GL_TRIANGLE_STRIP, vboIndex, vertices.size());
                vboVC.vboFree(vboIndex);
            }
        }
        triangleCount += (receiverSlices * 2 * (i - 2));

        glTranslatef(-pos[0], -pos[1], 0.0f);
    }
    glPopMatrix();

    glFogfv(GL_FOG_COLOR, fogColor);
}


void BackgroundRenderer::drawMountains(void)
{
    const int numFacesPerTexture = (NumMountainFaces +
                                    numMountainTextures - 1) / numMountainTextures;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < numMountainTextures; i++)
    {
        mountainsGState[i].setState();
        vboVTN.enableArrays();
        glDrawArrays(
            GL_TRIANGLE_STRIP,
            mountainsVBOIndex[i],
            numFacesPerTexture * 4 + 6);
    }
}


void BackgroundRenderer::doFreeDisplayLists()
{
    // don't forget the tag-along
    weather.freeContext();
    EFFECTS.freeContext();

    return;
}


void BackgroundRenderer::doInitDisplayLists()
{
    // don't forget the tag-along
    weather.rebuildContext();
    EFFECTS.rebuildContext();

}


void BackgroundRenderer::freeContext(void* self)
{
    ((BackgroundRenderer*)self)->doFreeDisplayLists();
}


void BackgroundRenderer::initContext(void* self)
{
    ((BackgroundRenderer*)self)->doInitDisplayLists();
}


const GLfloat*  BackgroundRenderer::getSunDirection() const
{
    if (areShadowsCast(sunDirection))
        return sunDirection;
    else
        return NULL;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
