/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "EighthDimSceneNode.h"
#include "SceneRenderer.h"

EighthDimSceneNode::EighthDimSceneNode(int numPolygons) :
				renderNode(this, numPolygons)
{
  // do nothing
}

EighthDimSceneNode::~EighthDimSceneNode()
{
  // do nothing
}

boolean			EighthDimSceneNode::cull(const ViewFrustum&) const
{
  // no culling
  return False;
}

void			EighthDimSceneNode::notifyStyleChange(
				const SceneRenderer& renderer)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_NONE);
  if (renderer.useBlending()) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else {
    builder.setStipple(0.75f);
  }
  gstate = builder.getState();
}

void			EighthDimSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

void			EighthDimSceneNode::setPolygon(int index,
						const GLfloat vertex[3][3])
{
  renderNode.setPolygon(index, vertex);
}

//
// EighthDimSceneNode::EighthDimRenderNode
//

EighthDimSceneNode::EighthDimRenderNode::EighthDimRenderNode(
				const EighthDimSceneNode* _sceneNode,
				int numPolys) :
				sceneNode(_sceneNode),
				numPolygons(numPolys)
{
  color = (GLfloat(*)[4])new GLfloat[4 * numPolygons];
  poly = (GLfloat(*)[3][3])new GLfloat[9 * numPolygons];

  // make random colors
  for (int i = 0; i < numPolygons; i++) {
    color[i][0] = 0.2f + 0.8f * (float)bzfrand();
    color[i][1] = 0.2f + 0.8f * (float)bzfrand();
    color[i][2] = 0.2f + 0.8f * (float)bzfrand();
    color[i][3] = 0.2f + 0.6f * (float)bzfrand();
  }
}

EighthDimSceneNode::EighthDimRenderNode::~EighthDimRenderNode()
{
  delete[] color;
  delete[] poly;
}

void			EighthDimSceneNode::EighthDimRenderNode::render()
{
  // draw polygons
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < numPolygons; i++) {
    myColor4fv(color[i]);
    glVertex3fv(poly[i][0]);
    glVertex3fv(poly[i][2]);
    glVertex3fv(poly[i][1]);
  }
  glEnd();
}

void			EighthDimSceneNode::EighthDimRenderNode::setPolygon(
				int index, const GLfloat vertex[3][3])
{
  ::memcpy(poly[index], vertex, sizeof(GLfloat[3][3]));
}
// ex: shiftwidth=2 tabstop=8
