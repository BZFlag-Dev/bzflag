/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "EighthDTetraSceneNode.h"
#include "SceneRenderer.h"
#include "StateDatabase.h"

const int		TetraPolygons = 20;


static void getRandomPoint (const float* center,
                            const float (*out)[3],
                            float* point)
{
  // pick the first direciton
  int dir = (int) (0.5f + (4.0f * bzfrand()));
  memcpy (point, center, sizeof(float[3]));
  float factor = 1.0f;
  for (int i = 0; i < 4; i++) {
    float f = (float)(factor * bzfrand());
    const float* dirvec = out[dir];
    factor = factor - f;
    point[0] = point[0] + (dirvec[0] * f);
    point[1] = point[1] + (dirvec[1] * f);
    point[2] = point[2] + (dirvec[2] * f);
    dir = (dir + 1) % 4;
    factor = factor * f;
  }
  return;
}


EighthDTetraSceneNode::EighthDTetraSceneNode(const float (*vertices)[3],
                                             const float (* /*planes*/)[4]) :
                                             EighthDimSceneNode(TetraPolygons),
                                             renderNode(this, vertices)
{
  int i, v, a;

  // get pseudo-center
  GLfloat center[3];
  for (a = 0; a < 3; a++) {
    center[a] = 0.0f;
    for (int v = 0; v < 4; v++) {
      center[a] = center[a] + vertices[v][a];
    }
    center[a] = 0.25f * center[a];
  }

  // get the outwards pointing vectors from the center
  GLfloat out[4][3];
  for (v = 0; v < 4; v++) {
    for (a = 0; a < 3; a++) {
      out[v][0] = vertices[v][0] - center[0];
      out[v][1] = vertices[v][1] - center[1];
      out[v][2] = vertices[v][2] - center[2];
    }
  }

  // get the maximum radius
  float radiusSquared = 0.0f;
  for (v = 0; v < 4; v++) {
    const float rs = (vertices[v][0] * vertices[v][0]) +
                     (vertices[v][1] * vertices[v][1]) +
                     (vertices[v][2] * vertices[v][2]);
    if (rs > radiusSquared) {
      radiusSquared = rs;
    }
  }

  // make the random nodes
  for (i = 0; i < TetraPolygons; i++) {
    GLfloat vertex[3][3];
    for (int j = 0; j < 3; j++) {
      getRandomPoint(center, out, vertex[0]);
      getRandomPoint(center, out, vertex[1]);
      getRandomPoint(center, out, vertex[2]);
    }
    setPolygon(i, vertex);
  }

  // set sphere
  setCenter(center);
  setRadius(0.25f * radiusSquared);
}

EighthDTetraSceneNode::~EighthDTetraSceneNode()
{
  // do nothing
}

void			EighthDTetraSceneNode::notifyStyleChange(
				const SceneRenderer& renderer)
{
  EighthDimSceneNode::notifyStyleChange(renderer);

  OpenGLGStateBuilder builder(gstate);
  if (BZDB.isTrue("smooth")) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setSmoothing();
  }
  else {
    builder.resetBlending();
    builder.resetSmoothing();
  }
  gstate = builder.getState();
}

void			EighthDTetraSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  EighthDimSceneNode::addRenderNodes(renderer);
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// EighthDTetraSceneNode::EighthDTetraRenderNode
//

EighthDTetraSceneNode::EighthDTetraRenderNode::EighthDTetraRenderNode(
				const EighthDTetraSceneNode* _sceneNode,
				const float (*vertices)[3]) : sceneNode(_sceneNode)
{
  memcpy (corner, vertices, 4 * sizeof(float[3]));
}

EighthDTetraSceneNode::EighthDTetraRenderNode::~EighthDTetraRenderNode()
{
  // do nothing
}

void			EighthDTetraSceneNode::EighthDTetraRenderNode::render()
{
  myColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINE_LOOP);
  glVertex3fv(corner[0]);
  glVertex3fv(corner[1]);
  glVertex3fv(corner[2]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3fv(corner[0]);  glVertex3fv(corner[3]);
  glVertex3fv(corner[1]);  glVertex3fv(corner[3]);
  glVertex3fv(corner[2]);  glVertex3fv(corner[3]);
  glEnd();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

