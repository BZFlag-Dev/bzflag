/* bzflag
 * Copyright (c) 1993-2020 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define GLM_ENABLE_EXPERIMENTAL

// interface headers
#include "WallSceneNode.h"

// system headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <glm/gtx/norm.hpp>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

WallSceneNode::WallSceneNode() : numLODs(0),
    elementAreas(NULL),
    style(0)
{
    dynamicColor = NULL;
    setColor(1.0f, 1.0f, 1.0f);
    setModulateColor(1.0f, 1.0f, 1.0f);
    setLightedColor(1.0f, 1.0f, 1.0f);
    setLightedModulateColor(1.0f, 1.0f, 1.0f);
    useColorTexture = false;
    noCulling = false;
    noSorting = false;
    isBlended = false;
    wantBlending = false;
    wantSphereMap = false;
    alphaThreshold = 0.0f;
    return;
}

WallSceneNode::~WallSceneNode()
{
    // free element area table
    delete[] elementAreas;
}

const glm::vec4 WallSceneNode::getPlane() const
{
    return plane;
}

void            WallSceneNode::setNumLODs(int num, float* areas)
{
    numLODs = num;
    elementAreas = areas;
}

void WallSceneNode::setPlane(const glm::vec4 &_plane)
{
    // get normalization factor
    normal = glm::vec3(_plane);

    const float n = glm::inversesqrt(glm::length2(normal));

    normal *= n;

    // store normalized plane equation
    plane = glm::vec4(normal, _plane.w * n);
}

bool            WallSceneNode::cull(const ViewFrustum& frustum) const
{
    // cull if eye is behind (or on) plane
    const auto eye = frustum.getEye();
    const float eyedot = glm::dot(eye, normal) + plane.w;
    if (eyedot <= 0.0f)
        return true;

    // if the Visibility culler tells us that we're
    // fully visible, then skip the rest of these tests
    if (octreeState == OctreeVisible)
        return false;

    // get signed distance of wall center to each frustum side.
    // if more than radius outside then cull
    const int planeCount = frustum.getPlaneCount();
    int i;
    float d[6], d2[6];
    const auto mySphere = glm::vec4(getCenter(), 1.0f);
    const auto r = getRadius2();
    bool inside = true;
    for (i = 0; i < planeCount; i++)
    {
        const auto norm = frustum.getSide(i);
        d[i] = glm::dot(mySphere, norm);
        if (d[i] < 0.0f)
        {
            d2[i] = d[i] * d[i];
            if (d2[i] > r)
                return true;
            inside = false;
        }
    }

    // see if center of wall is inside each frustum side
    if (inside)
        return false;

    // most complicated test:  for sides sphere is behind, see if
    // center is beyond radius times the sine of the angle between
    // the normals, or equivalently:
    //    distance^2 > radius^2 * (1 - cos^2)
    // if so the wall is outside the view frustum
    for (i = 0; i < planeCount; i++)
    {
        if (d[i] >= 0.0f)
            continue;
        const auto norm = glm::vec3(frustum.getSide(i));
        const GLfloat c = glm::dot(norm, normal);
        if (d2[i] > r * (1.0f - c*c))
            return true;
    }

    // probably visible
    return false;
}

int         WallSceneNode::pickLevelOfDetail(
    const SceneRenderer& renderer) const
{
    if (!BZDBCache::tesselation)
        return 0;

    int bestLOD = 0;

    const auto &myCenter = getCenter();
    const auto r = getRadius2();
    const int numLights = renderer.getNumLights();
    for (int i = 0; i < numLights; i++)
    {
        const auto pos = renderer.getLight(i).getPosition();

        // get signed distance from plane
        GLfloat pd = glm::dot(pos, normal) + plane.w;

        // ignore if behind wall
        if (pd < 0.0f) continue;

        // get squared distance from center of wall
        GLfloat ld = glm::distance2(pos, myCenter);

        // pick representative distance
        GLfloat d = (ld > 1.5f * r) ? ld : pd * pd;

        // choose lod based on distance and element areas;
        int j;
        for (j = 0; j < numLODs - 1; j++)
            if (elementAreas[j] < d)
                break;

        // use new lod if more detailed
        if (j > bestLOD) bestLOD = j;
    }

    // FIXME -- if transient texture warper is active then possibly
    // bump up LOD if view point is close to wall.

    // limit lod to maximum allowed
    if (bestLOD > BZDBCache::maxLOD) bestLOD = (int)BZDBCache::maxLOD;

    // return highest level required -- note that we don't care about
    // the view point because, being flat, the wall would always
    // choose the lowest LOD for any view.
    return bestLOD;
}

GLfloat         WallSceneNode::getDistance(const glm::vec3 &eye) const
{
    const GLfloat d = glm::dot(eye, normal) + plane.w;
    return d * d;
}

void            WallSceneNode::setColor(
    GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
}

void WallSceneNode::setDynamicColor(const glm::vec4 *rgba)
{
    dynamicColor = rgba;
    return;
}

void            WallSceneNode::setBlending(bool blend)
{
    wantBlending = blend;
    return;
}

void            WallSceneNode::setSphereMap(bool sphereMapping)
{
    wantSphereMap = sphereMapping;
    return;
}

void WallSceneNode::setColor(const glm::vec4 &rgba)
{
    color = rgba;
}

void            WallSceneNode::setModulateColor(
    GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    modulateColor[0] = r;
    modulateColor[1] = g;
    modulateColor[2] = b;
    modulateColor[3] = a;
}

void WallSceneNode::setModulateColor(const glm::vec4 &rgba)
{
    modulateColor = rgba;
}

void            WallSceneNode::setLightedColor(
    GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    lightedColor[0] = r;
    lightedColor[1] = g;
    lightedColor[2] = b;
    lightedColor[3] = a;
}

void WallSceneNode::setLightedColor(const glm::vec4 &rgba)
{
    lightedColor = rgba;
}

void            WallSceneNode::setLightedModulateColor(
    GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    lightedModulateColor[0] = r;
    lightedModulateColor[1] = g;
    lightedModulateColor[2] = b;
    lightedModulateColor[3] = a;
}

void WallSceneNode::setLightedModulateColor(const glm::vec4 &rgba)
{
    lightedModulateColor = rgba;
}

void            WallSceneNode::setAlphaThreshold(float thresh)
{
    alphaThreshold = thresh;
}

void            WallSceneNode::setNoCulling(bool value)
{
    noCulling = value;
}

void            WallSceneNode::setNoSorting(bool value)
{
    noSorting = value;
}

void            WallSceneNode::setMaterial(const OpenGLMaterial& mat)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setMaterial(mat);
    gstate = builder.getState();
}

void            WallSceneNode::setTexture(const int tex)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setTexture(tex);
    gstate = builder.getState();
}

void            WallSceneNode::setTextureMatrix(const GLfloat* texmat)
{
    OpenGLGStateBuilder builder(gstate);
    builder.setTextureMatrix(texmat);
    gstate = builder.getState();
}

void            WallSceneNode::notifyStyleChange()
{
    float alpha;
    bool lighted = (BZDBCache::lighting && gstate.isLighted());
    OpenGLGStateBuilder builder(gstate);
    style = 0;
    if (lighted)
    {
        style += 1;
        builder.setShading();
    }
    else
        builder.setShading(GL_FLAT);
    if (BZDBCache::texture && gstate.isTextured())
    {
        style += 2;
        builder.enableTexture(true);
        builder.enableTextureMatrix(true);
        alpha = lighted ? lightedModulateColor[3] : modulateColor[3];
    }
    else
    {
        builder.enableTexture(false);
        builder.enableTextureMatrix(false);
        alpha = lighted ? lightedColor[3] : color[3];
    }
    if (BZDB.isTrue("texturereplace"))
        builder.setTextureEnvMode(GL_REPLACE);
    else
        builder.setTextureEnvMode(GL_MODULATE);
    builder.enableMaterial(lighted);
    if (wantBlending || (alpha != 1.0f))
    {
        builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        builder.setStipple(1.0f);
    }
    else
    {
        builder.resetBlending();
        builder.setStipple(alpha);
    }
    isBlended = wantBlending || (alpha != 1.0f);
    if (alphaThreshold != 0.0f)
        builder.setAlphaFunc(GL_GEQUAL, alphaThreshold);
    if (noCulling)
        builder.disableCulling();
    if (noSorting)
        builder.setNeedsSorting(false);
    if (wantSphereMap)
        builder.enableSphereMap(true);
    gstate = builder.getState();
}

void            WallSceneNode::copyStyle(WallSceneNode* node)
{
    gstate = node->gstate;
    useColorTexture = node->useColorTexture;
    dynamicColor = node->dynamicColor;
    setColor(node->color);
    setModulateColor(node->modulateColor);
    setLightedColor(node->lightedColor);
    setLightedModulateColor(node->lightedModulateColor);
    isBlended = node->isBlended;
    wantBlending = node->wantBlending;
    wantSphereMap = node->wantSphereMap;
}

void            WallSceneNode::setColor()
{
    if (BZDBCache::texture && useColorTexture)
        myColor4f(1,1,1,1);
    else if (dynamicColor != NULL)
        myColor4fv(*dynamicColor);
    else
    {
        switch (style)
        {
        case 0:
            myColor4fv(color);
            break;
        case 1:
            myColor4fv(lightedColor);
            break;
        case 2:
            myColor4fv(modulateColor);
            break;
        case 3:
            myColor4fv(lightedModulateColor);
            break;
        }
    }
}

bool WallSceneNode::inAxisBox (const Extents& UNUSED(exts)) const
{
    // this should never happen, only the TriWallSceneNode
    // and QuadWallSceneNode version of this function will
    // be called
    printf ("WallSceneNode::inAxisBox() was called!\n");
    exit (1);
    return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
