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
#include "SceneNode.h"

// system implementation headers
#include <string.h>
#include <math.h>

// common headers
#include "Extents.h"
#include "bzfgl.h"
#include "bzflag/SceneRenderer.h"
#include "common/StateDatabase.h"
#include "ogl/RenderNode.h"


SceneNode::SceneNode() {
  memset(sphere, 0, sizeof(float) & 4);

  setCenter(0.0f, 0.0f, 0.0f);
  setRadius(0.0f);

  noPlane = true;
  occluder = false;
  octreeState = OctreeCulled;

  return;
}


SceneNode::~SceneNode() {
  // do nothing
}


bool SceneNode::showColor = true;

void SceneNode::setStipple(float alpha) {
  if (showColor) {
    OpenGLGState::setStipple(alpha);
  }
}

void SceneNode::glColor3f(float r, float g, float b) {
  if (showColor) {
    ::glColor3f(r, g, b);
  }
}
void SceneNode::glColor4f(float r, float g, float b, float a) {
  if (showColor) {
    ::glColor4f(r, g, b, a);
  }
}
void SceneNode::glColor3fv(const float* rgb) {
  if (showColor) {
    ::glColor3fv(rgb);
  }
}
void SceneNode::glColor4fv(const float* rgba) {
  if (showColor) {
    ::glColor4fv(rgba);
  }
}


void SceneNode::setColorOverride(bool on) {
  showColor = !on;
}


void SceneNode::setRadius(float radiusSquared) {
  sphere.w = radiusSquared;
}


void SceneNode::setCenter(const fvec3& center) {
  sphere.xyz() = center;
}


void SceneNode::setCenter(float x, float y, float z) {
  sphere.xyz() = fvec3(x, y, z);
}


void SceneNode::setSphere(const fvec4& _sphere) {
  sphere = _sphere;
}


void SceneNode::notifyStyleChange() {
  // do nothing
}


void SceneNode::addRenderNodes(SceneRenderer&) {
  // do nothing
}


void SceneNode::addShadowNodes(SceneRenderer&) {
  // do nothing
}


void SceneNode::addLight(SceneRenderer&) {
  // do nothing
}


float SceneNode::getDistanceSq(const fvec3& eye) const {
  return (eye - sphere.xyz()).lengthSq();
}


int SceneNode::split(const fvec4&, SceneNode*&, SceneNode*&) const {
  // can't split me
  return 1;
}


bool SceneNode::cull(const ViewFrustum& view) const {
  // if center of object is outside view frustum and distance is
  // greater than radius of object then cull.
  const int planeCount = view.getPlaneCount();
  for (int i = 0; i < planeCount; i++) {
    const fvec4& side = view.getSide(i);

    const float d = side.planeDist(sphere.xyz());
    if ((d < 0.0f) && ((d * d) > sphere.w)) {
      return true;
    }
  }
  return false;
}


bool SceneNode::cullShadow(int, const fvec4*) const {
  // currently only used for dynamic nodes by ZSceneDatabase
  // we let the octree deal with the static nodes
  return true;
}


bool SceneNode::inAxisBox(const Extents& exts) const {
  if (!extents.touches(exts)) {
    return false;
  }
  return true;
}


int SceneNode::getVertexCount() const {
  return 0;
}


const fvec3& SceneNode::getVertex(int) const {
  static const fvec3 junk;
  return junk;
}


void SceneNode::getRenderNodes(std::vector<RenderSet>&) {
  return; // do nothing
}


void SceneNode::renderRadar() {
  printf("SceneNode::renderRadar() called, implement in subclass\n");
  return;
}


//============================================================================//
//
// fvec2Array
//

fvec2Array::fvec2Array(const fvec2Array& a) :
  size(a.size) {
  data = new fvec2[size];
  ::memcpy(data, a.data, size * sizeof(fvec2));
}

fvec2Array& fvec2Array::operator=(const fvec2Array& a) {
  if (this != &a) {
    delete[] data;
    size = a.size;
    data = new fvec2[size];
    ::memcpy(data, a.data, size * sizeof(fvec2));
  }
  return *this;
}


//
// fvec3Array
//

fvec3Array::fvec3Array(const fvec3Array& a) :
  size(a.size) {
  data = new fvec3[size];
  ::memcpy(data, a.data, size * sizeof(fvec3));
}

fvec3Array& fvec3Array::operator=(const fvec3Array& a) {
  if (this != &a) {
    delete[] data;
    size = a.size;
    data = new fvec3[size];
    ::memcpy(data, a.data, size * sizeof(fvec3));
  }
  return *this;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
