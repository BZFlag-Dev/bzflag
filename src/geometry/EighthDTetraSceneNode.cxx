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

const int		TetraPolygons = 20; //FIXME

// FIXME - this can cause spin loops...

EighthDTetraSceneNode::EighthDTetraSceneNode(const float (*vertices)[3],
                                             const float (*planes)[4]) :
				EighthDimSceneNode(TetraPolygons),
				renderNode(this, vertices)
{
  int i, p, v, a;

  // get pseudo-center
  GLfloat center[3];
  for (a = 0; a < 3; a++) {
    center[a] = 0.0f;
    for (int v = 0; v < 4; v++) {
      center[a] = center[a] + vertices[v][a];
    }
    center[a] = 0.25f * center[a];
  }
  GLfloat radiusSquared = 0.0f;
  for (v = 0; v < 4; v++) {
    GLfloat out[3];
    out[0] = vertices[v][0] - center[0];
    out[1] = vertices[v][1] - center[1];
    out[2] = vertices[v][2] - center[2];
    float radsqu = (out[0] * out[0]) + (out[1] * out[1]) + (out[2] * out[2]);
    if (radsqu > radiusSquared) {
      radiusSquared = radsqu;
    }
  }
    
  float radius = sqrtf (radiusSquared);
  
  for (i = 0; i < TetraPolygons; i++) {
    GLfloat vertex[3][3];
    
    for (int j = 0; j < 3; j++) {
      bool done;
      do {
        done = true;
        vertex[j][0] = center[0] + radius * ((float)bzfrand() - 0.5f);
        vertex[j][1] = center[1] + radius * ((float)bzfrand() - 0.5f);
        vertex[j][2] = center[2] + radius * ((float)bzfrand() - 0.5f);
        // test the 4 planes
        for (p = 0; p < 4; p++) {
          float dist = (planes[p][0] * vertex[j][0]) +
                       (planes[p][1] * vertex[j][1]) +
                       (planes[p][2] * vertex[j][2]) + planes[p][3];
          if (dist > 0.0f) {
            done = false;
            break;
          }
        }
      } while (!done);
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

