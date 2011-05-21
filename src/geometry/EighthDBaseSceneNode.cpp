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
#include "EighthDBaseSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "StateDatabase.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cpp is in src/bzflag)


const int BasePolygons = 60;


EighthDBaseSceneNode::EighthDBaseSceneNode(const fvec3& pos,
                                           const fvec3& size, float rotation)
: EighthDimSceneNode(BasePolygons)
, renderNode(this, pos, size, rotation)
{
  // get rotation stuff
  const float c = cosf(rotation);
  const float s = sinf(rotation);

  // compute polygons
  const float polySize = size[0] / powf(float(BasePolygons), 0.3333f);
  for (int i = 0; i < BasePolygons; i++) {
    fvec3 base, verts[3];
    base.x = (size.x - 0.5f * polySize) * (2.0f * (float)bzfrand() - 1.0f);
    base.y = (size.y - 0.5f * polySize) * (2.0f * (float)bzfrand() - 1.0f);
    base.z = (size.z - 0.5f * polySize) * (float)bzfrand();
    for (int j = 0; j < 3; j++) {
      // pick point around origin
      fvec3 p;
      p.x = base.x + polySize * ((float)bzfrand() - 0.5f);
      p.y = base.y + polySize * ((float)bzfrand() - 0.5f);
      p.z = base.z + polySize * ((float)bzfrand() - 0.5f);

      // make sure it's inside the base
      p.x = (p.x < -size.x) ? -size.x : ((p.x > +size.x) ? +size.x : p.x);
      p.y = (p.y < -size.y) ? -size.y : ((p.y > +size.y) ? +size.y : p.y);
      p.z = (p.z < -size.z) ? -size.z : ((p.z > +size.z) ? +size.z : p.z);

      // rotate it
      verts[j].x = pos.x + (c * p.x) - (s * p.y);
      verts[j].y = pos.y + (s * p.x) + (c * p.y);
      verts[j].z = pos.z + p.z;
    }

    setPolygon(i, verts);
  }

  // set sphere
  setCenter(pos);
  setRadius(0.25f * size.lengthSq());
}

EighthDBaseSceneNode::~EighthDBaseSceneNode()
{
  // do nothing
}

void EighthDBaseSceneNode::notifyStyleChange()
{
  EighthDimSceneNode::notifyStyleChange();

  OpenGLGStateBuilder builder(gstate);
  if (BZDB.isTrue("smooth")) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setSmoothing();
  } else {
    builder.resetBlending();
    builder.resetSmoothing();
  }
  gstate = builder.getState();
}

void EighthDBaseSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  EighthDimSceneNode::addRenderNodes(renderer);
  renderer.addRenderNode(&renderNode, &gstate);
}

EighthDBaseSceneNode::EighthDBaseRenderNode::EighthDBaseRenderNode(
			const EighthDBaseSceneNode * _sceneNode,
			const fvec3& pos,
			const fvec3& size, float rotation) :
			sceneNode(_sceneNode)
{
  // get rotation stuff
  const float c = cosf(rotation);
  const float s = sinf(rotation);

  // compute corners
  corner[0].x = corner[4].x = pos.x + (c * size.x) - (s * size.y);
  corner[0].y = corner[4].y = pos.y + (s * size.x) + (c * size.y);
  corner[1].x = corner[5].x = pos.x - (c * size.x) - (s * size.y);
  corner[1].y = corner[5].y = pos.y - (s * size.x) + (c * size.y);
  corner[2].x = corner[6].x = pos.x - (c * size.x) + (s * size.y);
  corner[2].y = corner[6].y = pos.y - (s * size.x) - (c * size.y);
  corner[3].x = corner[7].x = pos.x + (c * size.x) + (s * size.y);
  corner[3].y = corner[7].y = pos.y + (s * size.x) - (c * size.y);
  corner[0].z = corner[1].z = corner[2].z = corner[3].z = pos.z;
  corner[4].z = corner[5].z = corner[6].z = corner[7].z = pos.z + size.z;
}

EighthDBaseSceneNode::EighthDBaseRenderNode::~EighthDBaseRenderNode()
{
  // do nothing
}

void EighthDBaseSceneNode::EighthDBaseRenderNode::render()
{
  myColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINE_LOOP);
  glVertex3fv(corner[0]);
  glVertex3fv(corner[1]);
  glVertex3fv(corner[2]);
  glVertex3fv(corner[3]);
  glEnd();
  glBegin(GL_LINE_LOOP);
  glVertex3fv(corner[4]);
  glVertex3fv(corner[5]);
  glVertex3fv(corner[6]);
  glVertex3fv(corner[7]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3fv(corner[0]); glVertex3fv(corner[4]);
  glVertex3fv(corner[1]); glVertex3fv(corner[5]);
  glVertex3fv(corner[2]); glVertex3fv(corner[6]);
  glVertex3fv(corner[3]); glVertex3fv(corner[7]);
  glEnd();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
