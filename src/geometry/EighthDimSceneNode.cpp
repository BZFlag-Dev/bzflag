/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "EighthDimSceneNode.h"

// system headers
#include <stdlib.h>
#include <string.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "StateDatabase.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cpp is in src/bzflag)


EighthDimSceneNode::EighthDimSceneNode(int numPolygons)
: renderNode(this, numPolygons)
{
  // do nothing
}


EighthDimSceneNode::~EighthDimSceneNode()
{
  // do nothing
}


bool EighthDimSceneNode::cull(const ViewFrustum&) const
{
  // no culling
  return false;
}


void EighthDimSceneNode::notifyStyleChange()
{
  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_NONE);
  if (BZDB.isTrue("blend")) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    builder.setStipple(0.75f);
  }
  gstate = builder.getState();
}


void EighthDimSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}


void EighthDimSceneNode::setPolygon(int index, const fvec3 vertices[3])
{
  renderNode.setPolygon(index, vertices);
}


//
// EighthDimSceneNode::EighthDimRenderNode
//

EighthDimSceneNode::EighthDimRenderNode::EighthDimRenderNode(
				const EighthDimSceneNode* _sceneNode,
				int numPolys)
: sceneNode(_sceneNode)
, numPolygons(numPolys)
{
  color = new fvec4[numPolygons];
  polys = new Vert3[numPolygons];

  // make random colors
  for (int i = 0; i < numPolygons; i++) {
    color[i].r = 0.2f + 0.8f * (float)bzfrand();
    color[i].g = 0.2f + 0.8f * (float)bzfrand();
    color[i].b = 0.2f + 0.8f * (float)bzfrand();
    color[i].a = 0.2f + 0.6f * (float)bzfrand();
  }
}


EighthDimSceneNode::EighthDimRenderNode::~EighthDimRenderNode()
{
  delete[] color;
  delete[] polys;
}


void EighthDimSceneNode::EighthDimRenderNode::render()
{
  // draw polygons
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < numPolygons; i++) {
    myColor4fv(color[i]);
    glVertex3fv(polys[i][0]);
    glVertex3fv(polys[i][2]);
    glVertex3fv(polys[i][1]);
  }
  glEnd();
}


void EighthDimSceneNode::EighthDimRenderNode::setPolygon(int index,
                                                         const fvec3 v[3])
{
  polys[index][0] = v[0];
  polys[index][1] = v[1];
  polys[index][2] = v[2];
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
