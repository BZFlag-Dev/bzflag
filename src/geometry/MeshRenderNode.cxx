/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

// implementation header
#include "MeshRenderNode.h"

// system headers
#include <string.h>

// common headers
#include "RenderNode.h"
#include "MeshDrawMgr.h"
#include "OpenGLGState.h"
#include "SceneNode.h"
#include "SceneRenderer.h"

#include "Extents.h"
#include "StateDatabase.h"
#include "BZDBCache.h"


fvec3 OpaqueRenderNode::junk(0.0f, 0.0f, 0.0f);


//============================================================================//


OpaqueRenderNode::OpaqueRenderNode(MeshDrawMgr* _drawMgr,
				   GLuint* _xformList, bool _normalize,
				   const fvec4* _color,
				   int _lod, int _set,
				   const Extents* _exts, int tris)
{
  drawMgr = _drawMgr;
  xformList = _xformList;
  normalize = _normalize;
  lod = _lod;
  set = _set;
  color = _color;
  exts = _exts;
  triangles = tris;
}


void OpaqueRenderNode::render()
{
  const bool switchLights = (exts != NULL);
  if (switchLights) {
    RENDERER.disableLights(*exts);
  }

  // set the color
  myColor4fv(*color);

  // do the transformation
  if (*xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(*xformList);
  }
  if (normalize) {
    glEnable(GL_NORMALIZE);
  }

  // draw the elements
  drawMgr->executeSet(lod, set, BZDBCache::lighting, BZDBCache::texture);

  // undo the transformation
  if (normalize) {
    glDisable(GL_NORMALIZE);
  }
  if (*xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }

  if (switchLights) {
    RENDERER.reenableLights();
  }

  addTriangleCount(triangles);

  return;
}


void OpaqueRenderNode::renderRadar()
{
  if (*xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(*xformList);
  }
  drawMgr->executeSetGeometry(lod, set);
  if (*xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }

  addTriangleCount(triangles);

  return;
}


void OpaqueRenderNode::renderShadow()
{
  if (*xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(*xformList);
  }
  drawMgr->executeSetGeometry(lod, set);
  if (*xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }

  addTriangleCount(triangles);

  return;
}


//============================================================================//

AlphaGroupRenderNode::AlphaGroupRenderNode(MeshDrawMgr* _drawMgr,
					   GLuint* _xformList,
					   bool _normalize,
					   const fvec4* _color,
					   int _lod, int _set,
					   const Extents* _exts,
					   const fvec3& _pos,
					   int _triangles) :
    OpaqueRenderNode(_drawMgr, _xformList, _normalize,
		     _color, _lod, _set, _exts, _triangles)
{
  pos = _pos;
  return;
}


void AlphaGroupRenderNode::setPosition(const fvec3& _pos)
{
  pos = _pos;
  return;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
