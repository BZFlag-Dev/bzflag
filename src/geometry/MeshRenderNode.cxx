/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
                                   GLuint _xformList, bool _normalize,
                                   const GLfloat* _color,
                                   int _lod, int _set,
                                   const Extents* _exts)
{
  drawMgr = _drawMgr;
  xformList = _xformList;
  normalize = _normalize;
  lod = _lod;
  set = _set;
  color = _color;
  exts = _exts;
}


void OpaqueRenderNode::render()
{
  const bool switchLights = ((exts != NULL) && BZDBCache::lighting);
  if (switchLights) {
    RENDERER.disableLights(exts->mins, exts->maxs);
  }

  // set the color
  SceneNode::myColor4fv(color);

  // do the transformation
  if (xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(xformList);
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
  if (xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }

  if (switchLights) {
    RENDERER.reenableLights();
  }
  return;
}


void OpaqueRenderNode::renderRadar()
{
  if (xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(xformList);
  }
  drawMgr->executeSet(lod, set, false, false);
  if (xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }
  return;
}


void OpaqueRenderNode::renderShadow()
{
  if (xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(xformList);
  }
  drawMgr->executeSet(lod, set, false, false);
  if (xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }
  return;
}


/******************************************************************************/

AlphaGroupRenderNode::AlphaGroupRenderNode(MeshDrawMgr* _drawMgr,
                                           GLuint _xformList,
                                           bool _normalize,
                                           const GLfloat* _color,
                                           int _lod, int _set,
                                           const Extents* _exts,
                                           const GLfloat _pos[3]) :
    OpaqueRenderNode(_drawMgr, _xformList, _normalize,
                     _color, _lod, _set, _exts)
{
  memcpy(pos, _pos, sizeof(GLfloat[3]));
  return;
}


/******************************************************************************/

AlphaRenderNode::AlphaRenderNode(MeshDrawMgr* _drawMgr,
                                 GLuint _xformList,
                                 const GLfloat* _color,
                                 const GLfloat* _pos,
                                 GLenum _indexType, void* _indices)
{
  drawMgr = _drawMgr;
  xformList = _xformList;
  color = _color;
  memcpy (pos, _pos, sizeof(GLfloat[3]));
  indexType = _indexType;
  indices = _indices;
}                                 

AlphaRenderNode::~AlphaRenderNode()
{
  if (indexType == GL_UNSIGNED_INT) {
    delete[] (unsigned int*)indices;
  } else {
    delete[] (unsigned short*)indices;
  }
}


inline void AlphaRenderNode::drawV() const
{
//  drawMgr->loadVertices();
//  glDrawElements(GL_TRIANGLES, 3, indexType, indices);
  MeshDrawMgr::disableArrays();
}

inline void AlphaRenderNode::drawVN() const
{
//  drawMgr->loadVertices();
//  drawMgr->loadNormals();
//  glDrawElements(GL_TRIANGLES, 3, indexType, indices);
  MeshDrawMgr::disableArrays();
}

inline void AlphaRenderNode::drawVT() const
{
//  drawMgr->loadVertices();
//  drawMgr->loadTexcoords();
//  glDrawElements(GL_TRIANGLES, 3, indexType, indices);
  MeshDrawMgr::disableArrays();
}

inline void AlphaRenderNode::drawVTN() const
{
//  drawMgr->loadVertices();
//  drawMgr->loadNormals();
//  drawMgr->loadTexcoords();
//  glDrawElements(GL_TRIANGLES, 3, indexType, indices);
  MeshDrawMgr::disableArrays();
}

void AlphaRenderNode::render()
{
  // do the transformation
  if (xformList != INVALID_GL_LIST_ID) {
    glPushMatrix();
    glCallList(xformList);
  }
  
  // set the color
  SceneNode::myColor4fv(color);
//  sceneNode->setColor();

  if (BZDBCache::lighting) {
    if (BZDBCache::texture) {
      drawVTN();
    } else {
      drawVN();
    }
  } else {
    if (BZDBCache::texture) {
      drawVT();
    } else {
      drawV();
    }
  }

  // do the transformation
  if (xformList != INVALID_GL_LIST_ID) {
    glPopMatrix();
  }
}

void AlphaRenderNode::renderRadar()
{
  drawV();
}

void AlphaRenderNode::renderShadow()
{
  drawV();
}


/******************************************************************************/


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
