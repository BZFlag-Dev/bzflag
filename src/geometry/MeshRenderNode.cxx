/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// implementation header
#include "MeshRenderNode.h"

// System headers
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

// common implementation headers
#include "RenderNode.h"
#include "MeshDrawMgr.h"
#include "OpenGLGState.h"
#include "SceneNode.h"
#include "SceneRenderer.h"

#include "Extents.h"
#include "StateDatabase.h"
#include "BZDBCache.h"


/******************************************************************************/


OpaqueRenderNode::OpaqueRenderNode(MeshDrawMgr* _drawMgr,
                                   GLfloat *_xformMatrix, bool _normalize,
                                   const GLfloat* _color,
                                   int _lod, int _set,
                                   const Extents* _exts, int tris)
{
    drawMgr = _drawMgr;
    xformMatrix = _xformMatrix;
    normalize = _normalize;
    lod = _lod;
    set = _set;
    color = _color;
    exts = _exts;
    triangles = tris;
}


const GLfloat* OpaqueRenderNode::getPosition() const
{
    // Better to not crash if called on sort
    static GLfloat nullVec3[3];
    return nullVec3;
}

void OpaqueRenderNode::render()
{
    const bool switchLights = (exts != NULL);
    if (switchLights)
        RENDERER.disableLights(exts->mins, exts->maxs);

    // set the color
    myColor4fv(color);

    // do the transformation
    if (xformMatrix)
    {
        glPushMatrix();
        glMultMatrixf(xformMatrix);
    }
    if (normalize)
        glEnable(GL_NORMALIZE);

    // draw the elements
    drawMgr->executeSet(lod, set, BZDBCache::lighting, BZDBCache::texture);

    // undo the transformation
    if (normalize)
        glDisable(GL_NORMALIZE);
    if (xformMatrix)
        glPopMatrix();

    if (switchLights)
        RENDERER.reenableLights();

    addTriangleCount(triangles);

    return;
}


void OpaqueRenderNode::renderShadow()
{
    if (xformMatrix)
    {
        glPushMatrix();
        glMultMatrixf(xformMatrix);
    }
    drawMgr->executeSetGeometry(lod, set);
    if (xformMatrix)
        glPopMatrix();

    addTriangleCount(triangles);

    return;
}


/******************************************************************************/

AlphaGroupRenderNode::AlphaGroupRenderNode(MeshDrawMgr* _drawMgr,
        GLfloat *_xformMatrix,
        bool _normalize,
        const GLfloat* _color,
        int _lod, int _set,
        const Extents* _exts,
        glm::vec3 _pos,
        int _triangles) :
    OpaqueRenderNode(_drawMgr, _xformMatrix, _normalize,
                     _color, _lod, _set, _exts, _triangles)
{
    pos = _pos;
    return;
}

const GLfloat* AlphaGroupRenderNode::getPosition() const
{
    return glm::value_ptr(pos);
}

void AlphaGroupRenderNode::setPosition(glm::vec3 _pos)
{
    pos = _pos;
    return;
}


/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
