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
#include "FlagWarpSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "common/StateDatabase.h"
#include "game/BZDBCache.h"
#include "bzflag/SceneRenderer.h" // FIXME (SceneRenderer.cpp is in src/bzflag)

// local headers
#include "geometry/ViewFrustum.h"


const float   FlagWarpSize  = 7.5f;  // meters
const float FlagWarpAlpha = 0.5f;

const fvec4 FlagWarpSceneNode::color[7] = {
  fvec4(0.25, 1.0,  0.25, 1.0f),
  fvec4(0.25, 0.25, 1.0,  1.0f),
  fvec4(1.0,  0.0,  1.0,  1.0f),
  fvec4(1.0,  0.25, 0.25, 1.0f),
  fvec4(1.0,  0.5,  0.0,  1.0f),
  fvec4(1.0,  1.0,  0.0,  1.0f),
  fvec4(1.0,  1.0,  1.0,  1.0f)
};


FlagWarpSceneNode::FlagWarpSceneNode(const fvec3& pos)
  : renderNode(this) {
  move(pos);
  setRadius(1.25f * FlagWarpSize * FlagWarpSize);
  size = 1.0f;
}

FlagWarpSceneNode::~FlagWarpSceneNode() {
  // do nothing
}


void FlagWarpSceneNode::setSizeFraction(float _size) {
  size = _size;
}


void FlagWarpSceneNode::move(const fvec3& pos) {
  setCenter(pos);
}


float FlagWarpSceneNode::getDistanceSq(const fvec3& eye) const {
  // shift position of warp down a little because a flag and it's warp
  // are at the same position but we want the warp to appear below the
  // flag.
  fvec3 shifted = getSphere().xyz();
  shifted.z -= 0.2f;
  return (eye - shifted).lengthSq();
}


void FlagWarpSceneNode::notifyStyleChange() {
  OpenGLGStateBuilder builder(gstate);
  if (BZDBCache::blend) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(FlagWarpAlpha);
  }
  gstate = builder.getState();
}


void FlagWarpSceneNode::addRenderNodes(
  SceneRenderer& renderer) {
  renderer.addRenderNode(&renderNode, &gstate);
}


//
// FlagWarpSceneNode::FlagWarpRenderNode
//

FlagWarpSceneNode::FlagWarpRenderNode::FlagWarpRenderNode(
  const FlagWarpSceneNode* _sceneNode) :
  sceneNode(_sceneNode) {
  // do nothing
}


FlagWarpSceneNode::FlagWarpRenderNode::~FlagWarpRenderNode() {
  // do nothing
}


void FlagWarpSceneNode::FlagWarpRenderNode::render() {
  // make a perturbed ring
  fvec2 geom[12];
  for (int i = 0; i < 12; i++) {
    const float r = FlagWarpSize * (0.9f + 0.2f * (float)bzfrand());
    geom[i].x = r * cosf((float)(2.0 * M_PI * double(i) / 12.0));
    geom[i].y = r * sinf((float)(2.0 * M_PI * double(i) / 12.0));
  }

  const fvec4& sphere = sceneNode->getSphere();
  glPushMatrix();
  glTranslatef(sphere.x, sphere.y, sphere.z);

  if (sphere.z > RENDERER.getViewFrustum().getEye().z) {
    for (int i = 0; i < 7; i++) {
      const float s = sceneNode->size - 0.05f * float(i);
      if (s < 0.0f) {
        break;
      }
      myColor4fv(fvec4(color[i].rgb(), FlagWarpAlpha));
      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0.0f, 0.0f);
      glVertex2fv(s * geom[0]);
      glVertex2fv(s * geom[11]);
      glVertex2fv(s * geom[10]);
      glVertex2fv(s * geom[9]);
      glVertex2fv(s * geom[8]);
      glVertex2fv(s * geom[7]);
      glVertex2fv(s * geom[6]);
      glVertex2fv(s * geom[5]);
      glVertex2fv(s * geom[4]);
      glVertex2fv(s * geom[3]);
      glVertex2fv(s * geom[2]);
      glVertex2fv(s * geom[1]);
      glVertex2fv(s * geom[0]);
      glEnd(); // 14 verts -> 12 tris
      addTriangleCount(12);
      glTranslatef(0.0f, 0.0f, -0.01f);
    }
  }
  else {
    for (int i = 0; i < 7; i++) {
      const float s = sceneNode->size - 0.05f * float(i);
      if (s < 0.0f) {
        break;
      }
      myColor4fv(fvec4(color[i].rgb(), FlagWarpAlpha));
      glBegin(GL_TRIANGLE_FAN);
      glVertex2f(0.0f, 0.0f);
      glVertex2fv(s * geom[0]);
      glVertex2fv(s * geom[1]);
      glVertex2fv(s * geom[2]);
      glVertex2fv(s * geom[3]);
      glVertex2fv(s * geom[4]);
      glVertex2fv(s * geom[5]);
      glVertex2fv(s * geom[6]);
      glVertex2fv(s * geom[7]);
      glVertex2fv(s * geom[8]);
      glVertex2fv(s * geom[9]);
      glVertex2fv(s * geom[10]);
      glVertex2fv(s * geom[11]);
      glVertex2fv(s * geom[0]);
      glEnd(); // 14 verts -> 12 tris
      addTriangleCount(12);
      glTranslatef(0.0f, 0.0f, 0.01f);
    }
  }

  glPopMatrix();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
