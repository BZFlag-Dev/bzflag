/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "EighthDPyrSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common implementation header
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

const int		PyrPolygons = 20;

EighthDPyrSceneNode::EighthDPyrSceneNode(const float pos[3],
					const float size[3], float rotation) :
				EighthDimSceneNode(PyrPolygons),
				renderNode(this, pos, size, rotation)
{
  // get rotation stuff
  const float c = cosf(rotation);
  const float s = sinf(rotation);

  // compute polygons
  const GLfloat polySize = size[0] / powf(float(PyrPolygons), 0.3333f);
  const GLfloat slope = size[2] / size[0];
  for (int i = 0; i < PyrPolygons; i++) {
    GLfloat base[3], vertex[3][3];
    base[0] = (size[0] - 0.5f * polySize) * (2.0f * (float)bzfrand() - 1.0f);
    base[1] = (size[1] - 0.5f * polySize) * (2.0f * (float)bzfrand() - 1.0f);
    base[2] = (size[2] - slope * hypotf(base[0], base[1])) * (float)bzfrand();
    for (int j = 0; j < 3; j++) {
      // pick point around origin
      GLfloat p[3];
      p[0] = base[0] + polySize * ((float)bzfrand() - 0.5f);
      p[1] = base[1] + polySize * ((float)bzfrand() - 0.5f);
      p[2] = base[2] + polySize * ((float)bzfrand() - 0.5f);

      // make sure it's inside the box
      if (p[0] < -size[0]) p[0] = -size[0];
      else if (p[0] > size[0]) p[0] = size[0];
      if (p[1] < -size[1]) p[1] = -size[1];
      else if (p[1] > size[1]) p[1] = size[1];
      GLfloat height = size[2] - slope * hypotf(p[0], p[1]);
      if (p[2] < 0.0f) p[2] = 0.0f;
      else if (p[2] > height) p[2] = height;

      // rotate it
      vertex[j][0] = pos[0] + c * p[0] - s * p[1];
      vertex[j][1] = pos[1] + s * p[0] + c * p[1];
      vertex[j][2] = pos[2] + p[2];
    }

    setPolygon(i, vertex);
  }

  // set sphere
  setCenter(pos);
  setRadius(0.25f * (size[0]*size[0] + size[1]*size[1] + size[2]*size[2]));
}

EighthDPyrSceneNode::~EighthDPyrSceneNode()
{
  // do nothing
}

void			EighthDPyrSceneNode::notifyStyleChange()
{
  EighthDimSceneNode::notifyStyleChange();

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

void			EighthDPyrSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  EighthDimSceneNode::addRenderNodes(renderer);
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// EighthDPyrSceneNode::EighthDPyrRenderNode
//

EighthDPyrSceneNode::EighthDPyrRenderNode::EighthDPyrRenderNode(
				const EighthDPyrSceneNode* _sceneNode,
				const float pos[3],
				const float size[3], float rotation) :
				sceneNode(_sceneNode)
{
  // get rotation stuff
  const float c = cosf(rotation);
  const float s = sinf(rotation);

  // compute corners
  corner[0][0] = pos[0] + c * size[0] - s * size[1];
  corner[0][1] = pos[1] + s * size[0] + c * size[1];
  corner[1][0] = pos[0] - c * size[0] - s * size[1];
  corner[1][1] = pos[1] - s * size[0] + c * size[1];
  corner[2][0] = pos[0] - c * size[0] + s * size[1];
  corner[2][1] = pos[1] - s * size[0] - c * size[1];
  corner[3][0] = pos[0] + c * size[0] + s * size[1];
  corner[3][1] = pos[1] + s * size[0] - c * size[1];
  corner[0][2] = corner[1][2] = corner[2][2] = corner[3][2] = pos[2];
  corner[4][0] = pos[0];
  corner[4][1] = pos[1];
  corner[4][2] = pos[2] + size[2];
}

EighthDPyrSceneNode::EighthDPyrRenderNode::~EighthDPyrRenderNode()
{
  // do nothing
}

void			EighthDPyrSceneNode::EighthDPyrRenderNode::render()
{
  myColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINE_LOOP);
  glVertex3fv(corner[0]);
  glVertex3fv(corner[1]);
  glVertex3fv(corner[2]);
  glVertex3fv(corner[3]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3fv(corner[0]);  glVertex3fv(corner[4]);
  glVertex3fv(corner[1]);  glVertex3fv(corner[4]);
  glVertex3fv(corner[2]);  glVertex3fv(corner[4]);
  glVertex3fv(corner[3]);  glVertex3fv(corner[4]);
  glEnd();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
