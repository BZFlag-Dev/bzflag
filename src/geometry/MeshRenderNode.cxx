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

// implementation header
#include "MeshRenderNode.h"

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
                                   const glm::vec4 *_color,
                                   int _lod, int _set,
                                   const Extents* _exts, int tris) :
    color(_color)
{
    drawMgr = _drawMgr;
    xformMatrix = _xformMatrix;
    normalize = _normalize;
    lod = _lod;
    set = _set;
    exts = _exts;
    triangles = tris;
}


void OpaqueRenderNode::render()
{
    const bool switchLights = (exts != NULL);
    if (switchLights)
        RENDERER.disableLights(exts->mins, exts->maxs);

    // set the color
    myColor4fv(*color);

    // do the transformation
    glPushMatrix();
    glMultMatrixf(xformMatrix);
    if (normalize)
        glEnable(GL_NORMALIZE);

    // draw the elements
    drawMgr->executeSet(lod, set, BZDBCache::lighting, BZDBCache::texture);

    // undo the transformation
    if (normalize)
        glDisable(GL_NORMALIZE);
    glPopMatrix();

    if (switchLights)
        RENDERER.reenableLights();

    addTriangleCount(triangles);

    return;
}


void OpaqueRenderNode::renderRadar()
{
    glPushMatrix();
    glMultMatrixf(xformMatrix);
    drawMgr->executeSetGeometry(lod, set);
    glPopMatrix();

    addTriangleCount(triangles);

    return;
}


void OpaqueRenderNode::renderShadow()
{
    glPushMatrix();
    glMultMatrixf(xformMatrix);
    drawMgr->executeSetGeometry(lod, set);
    glPopMatrix();

    addTriangleCount(triangles);

    return;
}


const glm::vec3 OpaqueRenderNode::getPosition() const
{
    return glm::vec3(0.0f);
}


/******************************************************************************/

AlphaGroupRenderNode::AlphaGroupRenderNode(MeshDrawMgr* _drawMgr,
        GLfloat *_xformMatrix,
        bool _normalize,
        const glm::vec4 *_color,
        int _lod, int _set,
        const Extents* _exts,
        const GLfloat _pos[3],
        int _triangles) :
    OpaqueRenderNode(_drawMgr, _xformMatrix, _normalize,
                     _color, _lod, _set, _exts, _triangles)
{
    pos = glm::make_vec3(_pos);
    return;
}


const glm::vec3 AlphaGroupRenderNode::getPosition() const
{
    return pos;
}


void AlphaGroupRenderNode::setPosition(const GLfloat* _pos)
{
    pos = glm::make_vec3(_pos);
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
