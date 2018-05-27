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
#include "SceneNode.h"

// system implementation headers
#include <string.h>
#include <math.h>

// common implementation headers
#include "Extents.h"
#include "RenderNode.h"
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

void            (*SceneNode::stipple)(GLfloat);

SceneNode::SceneNode()
{
    static bool init = false;

    if (!init)
    {
        init = true;
        setColorOverride(false);
    }
    memset(sphere, 0, sizeof(GLfloat) * 4);

    setCenter(0.0f, 0.0f, 0.0f);
    setRadius(0.0f);

    noPlane = true;
    occluder = false;
    octreeState = OctreeCulled;

    return;
}

SceneNode::~SceneNode()
{
    // do nothing
}


bool            SceneNode::colorOverride = true;
void            SceneNode::noStipple(GLfloat) {}

void            SceneNode::setColorOverride(bool on)
{
    colorOverride = on;
    if (on)
        stipple  = &noStipple;
    else
        stipple  = &OpenGLGState::setStipple;
}

void            SceneNode::setRadius(GLfloat radiusSquared)
{
    sphere[3] = radiusSquared;
}

void            SceneNode::setCenter(const GLfloat center[3])
{
    sphere[0] = center[0];
    sphere[1] = center[1];
    sphere[2] = center[2];
}

void            SceneNode::setCenter(GLfloat x, GLfloat y, GLfloat z)
{
    sphere[0] = x;
    sphere[1] = y;
    sphere[2] = z;
}

void            SceneNode::setSphere(const GLfloat _sphere[4])
{
    sphere[0] = _sphere[0];
    sphere[1] = _sphere[1];
    sphere[2] = _sphere[2];
    sphere[3] = _sphere[3];
}

void            SceneNode::notifyStyleChange()
{
    // do nothing
}

void            SceneNode::addRenderNodes(SceneRenderer&)
{
    // do nothing
}

void            SceneNode::addShadowNodes(SceneRenderer&)
{
    // do nothing
}

void            SceneNode::addLight(SceneRenderer&)
{
    // do nothing
}

GLfloat         SceneNode::getDistance(const GLfloat* eye) const
{
    return (eye[0] - sphere[0]) * (eye[0] - sphere[0]) +
           (eye[1] - sphere[1]) * (eye[1] - sphere[1]) +
           (eye[2] - sphere[2]) * (eye[2] - sphere[2]);
}

int         SceneNode::split(const float*,
                             SceneNode*&, SceneNode*&) const
{
    // can't split me
    return 1;
}

bool            SceneNode::cull(const ViewFrustum& view) const
{
    // if center of object is outside view frustum and distance is
    // greater than radius of object then cull.
    const int planeCount = view.getPlaneCount();
    for (int i = 0; i < planeCount; i++)
    {
        const GLfloat* norm = view.getSide(i);
        const GLfloat d = (sphere[0] * norm[0]) +
                          (sphere[1] * norm[1]) +
                          (sphere[2] * norm[2]) + norm[3];
        if ((d < 0.0f) && ((d * d) > sphere[3])) return true;
    }
    return false;
}


bool SceneNode::cullShadow(int, const float (*)[4]) const
{
    // currently only used for dynamic nodes by ZSceneDatabase
    // we let the octree deal with the static nodes
    return true;
}


bool SceneNode::inAxisBox (const Extents& exts) const
{
    if (!extents.touches(exts))
        return false;
    return true;
}

int SceneNode::getVertexCount () const
{
    return 0;
}

const GLfloat* SceneNode::getVertex (int) const
{
    return NULL;
}


//
// GLfloat2Array
//

GLfloat2Array::GLfloat2Array(const GLfloat2Array& a) :
    size(a.size)
{
    data = new GLfloat2[size];
    ::memcpy(data, a.data, size * sizeof(GLfloat2));
}

GLfloat2Array&      GLfloat2Array::operator=(const GLfloat2Array& a)
{
    if (this != &a)
    {
        delete[] data;
        size = a.size;
        data = new GLfloat2[size];
        ::memcpy(data, a.data, size * sizeof(GLfloat2));
    }
    return *this;
}


//
// GLfloat3Array
//

GLfloat3Array::GLfloat3Array(const GLfloat3Array& a) :
    size(a.size)
{
    data = new GLfloat3[size];
    ::memcpy(data, a.data, size * sizeof(GLfloat3));
}

GLfloat3Array&      GLfloat3Array::operator=(const GLfloat3Array& a)
{
    if (this != &a)
    {
        delete[] data;
        size = a.size;
        data = new GLfloat3[size];
        ::memcpy(data, a.data, size * sizeof(GLfloat3));
    }
    return *this;
}


void SceneNode::getRenderNodes(std::vector<RenderSet>&)
{
    return; // do nothing
}


void SceneNode::renderRadar()
{
    printf ("SceneNode::renderRadar() called, implement in subclass\n");
    return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
