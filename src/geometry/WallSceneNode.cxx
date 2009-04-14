/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

// interface headers
#include "WallSceneNode.h"
#include "PolyWallSceneNode.h"

// system headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cxx is in src/bzflag)

// local headers
#include "ViewFrustum.h"


WallSceneNode::WallSceneNode()
: numLODs(0)
, elementAreas(NULL)
, style(0)
{
  noPlane = false;
  order = 0;
  dynamicColor = NULL;

  color.a = 1.0f;
  modulateColor.a = 1.0f;
  lightedColor.a = 1.0f;
  lightedModulateColor.a = 1.0f;

  setColor(1.0f, 1.0f, 1.0f);
  setModulateColor(1.0f, 1.0f, 1.0f);
  setLightedColor(1.0f, 1.0f, 1.0f);
  setLightedModulateColor(1.0f, 1.0f, 1.0f);

  useColorTexture = false;
  noCulling = false;
  noSorting = false;
  isBlended = false;
  wantBlending = false;
  wantSphereMap = false;
  alphaThreshold = 0.0f;

  return;
}


WallSceneNode::~WallSceneNode()
{
  // free element area table
  delete[] elementAreas;
}


void WallSceneNode::setNumLODs(int num, float* areas)
{
  numLODs = num;
  elementAreas = areas;
}


void WallSceneNode::setPlane(const fvec4& _plane)
{
  // store normalized plane equation
  plane = _plane;
  plane *= (1.0f / plane.xyz().length());
}


bool WallSceneNode::cull(const ViewFrustum& frustum) const
{
  // cull if eye is behind (or on) plane
  const fvec3& eye = frustum.getEye();
  const float eyeDist = plane.planeDist(eye);
  if (eyeDist <= 0.0f) {
    return true;
  }

  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  // get signed distance of wall center to each frustum side.
  // if more than radius outside then cull
  const int planeCount = frustum.getPlaneCount();
  int i;
  float d[6], d2[6];
  const fvec4& mySphere = getSphere();
  bool inside = true;
  for (i = 0; i < planeCount; i++) {
    const fvec4& side = frustum.getSide(i);
    d[i] = side.planeDist(mySphere.xyz());
    if (d[i] < 0.0f) {
      d2[i] = d[i] * d[i];
      if (d2[i] > mySphere.w) {
	return true;
      }
      inside = false;
    }
  }

  // see if center of wall is inside each frustum side
  if (inside) {
    return false;
  }

  // most complicated test:  for sides sphere is behind, see if
  // center is beyond radius times the sine of the angle between
  // the normals, or equivalently:
  //	distance^2 > radius^2 * (1 - cos^2)
  // if so the wall is outside the view frustum
  for (i = 0; i < planeCount; i++) {
    if (d[i] >= 0.0f) {
      continue;
    }
    const fvec4& norm = frustum.getSide(i);
    const float c = fvec3::dot(norm.xyz(), plane.xyz());
    if (d2[i] > mySphere.w * (1.0f - (c * c))) {
      return true;
    }
  }

  // probably visible
  return false;
}


int WallSceneNode::pickLevelOfDetail(const SceneRenderer& renderer) const
{
  if (!BZDBCache::tesselation) {
    return 0;
  }

  int bestLOD = 0;

  const fvec4& mySphere = getSphere();
  const int numLights = renderer.getNumLights();
  for (int i = 0; i < numLights; i++) {
    const fvec4& pos = renderer.getLight(i).getPosition();

    // get signed distance from plane
    const float pd = plane.planeDist(pos.xyz());

    // ignore if behind wall
    if (pd < 0.0f) {
      continue;
    }

    // get squared distance from center of wall
    float ld = (pos.xyz() - mySphere.xyz()).lengthSq();

    // pick representative distance
    float d = (ld > (1.5f * mySphere.w)) ? ld : (pd * pd);

    // choose lod based on distance and element areas;
    int j;
    for (j = 0; j < numLODs - 1; j++)
      if (elementAreas[j] < d)
	break;

    // use new lod if more detailed
    if (j > bestLOD) bestLOD = j;
  }

  // FIXME -- if transient texture warper is active then possibly
  // bump up LOD if view point is close to wall.

  // limit lod to maximum allowed
  if (bestLOD > BZDBCache::maxLOD) bestLOD = (int)BZDBCache::maxLOD;

  // return highest level required -- note that we don't care about
  // the view point because, being flat, the wall would always
  // choose the lowest LOD for any view.
  return bestLOD;
}


float WallSceneNode::getDistanceSq(const fvec3& eye) const
{
  const float d = plane.planeDist(eye);
  return (d * d);
}


void WallSceneNode::setColor(float r, float g, float b, float a)
{
  color = fvec4(r, g, b, a);
}


void WallSceneNode::setOrder(int value)
{
  order = value;
}


void WallSceneNode::setDynamicColor(const fvec4* rgba)
{
  dynamicColor = rgba;
}


void WallSceneNode::setBlending(bool blend)
{
  wantBlending = blend;
}


void WallSceneNode::setSphereMap(bool sphereMapping)
{
  wantSphereMap = sphereMapping;
}


void WallSceneNode::setColor(const fvec4& rgba)
{
  color = rgba;
}


void WallSceneNode::setModulateColor(
				float r, float g, float b, float a)
{
  modulateColor = fvec4(r, g, b, a);
}


void WallSceneNode::setModulateColor(const fvec4& rgba)
{
  modulateColor = rgba;
}


void WallSceneNode::setLightedColor(float r, float g, float b, float a)
{
  lightedColor = fvec4(r, g, b, a);
}


void WallSceneNode::setLightedColor(const fvec4& rgba)
{
  lightedColor = rgba;
}


void WallSceneNode::setLightedModulateColor(float r, float g, float b, float a)
{
  lightedModulateColor = fvec4(r, g, b, a);
}


void WallSceneNode::setLightedModulateColor(const fvec4& rgba)
{
  lightedModulateColor = rgba;
}


void WallSceneNode::setAlphaThreshold(float thresh)
{
  alphaThreshold = thresh;
}


void WallSceneNode::setNoCulling(bool value)
{
  noCulling = value;
}


void WallSceneNode::setNoSorting(bool value)
{
  noSorting = value;
}


void WallSceneNode::setMaterial(const OpenGLMaterial& mat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(mat, RENDERER.useQuality() > _LOW_QUALITY);
  gstate = builder.getState();
}


void WallSceneNode::setTexture(const int tex)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(tex);
  gstate = builder.getState();
}


void WallSceneNode::setTextureMatrix(const float* texmat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTextureMatrix(texmat);
  gstate = builder.getState();
}


void WallSceneNode::notifyStyleChange()
{
  float alpha;
  bool lighted = (BZDBCache::lighting && gstate.isLighted());
  OpenGLGStateBuilder builder(gstate);
  builder.setOrder(order);
  style = 0;
  if (lighted) {
    style += 1;
    builder.setShading();
  }
  else {
    builder.setShading(GL_FLAT);
  }
  if (BZDBCache::texture && gstate.isTextured()) {
    style += 2;
    builder.enableTexture(true);
    builder.enableTextureMatrix(true);
    alpha = lighted ? lightedModulateColor.a : modulateColor.a;
  }
  else {
    builder.enableTexture(false);
    builder.enableTextureMatrix(false);
    alpha = lighted ? lightedColor.a : color.a;
  }
  if (BZDB.isTrue("texturereplace")) {
    builder.setTextureEnvMode(GL_REPLACE);
  } else {
    builder.setTextureEnvMode(GL_MODULATE);
  }
  builder.enableMaterial(lighted);
  if (BZDBCache::blend && (wantBlending || (alpha != 1.0f))) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(alpha);
  }
  isBlended = wantBlending || (alpha != 1.0f);
  if (alphaThreshold != 0.0f) {
    builder.setAlphaFunc(GL_GEQUAL, alphaThreshold);
  }
  if (noCulling) {
    builder.setCulling(GL_NONE);
  }
  if (noSorting) {
    builder.setNeedsSorting(false);
  }
  if (wantSphereMap) {
    builder.enableSphereMap(true);
  }
  gstate = builder.getState();
}


void WallSceneNode::copyStyle(WallSceneNode* node)
{
  gstate = node->gstate;
  useColorTexture = node->useColorTexture;
  order = node->order;
  dynamicColor = node->dynamicColor;
  setColor(node->color);
  setModulateColor(node->modulateColor);
  setLightedColor(node->lightedColor);
  setLightedModulateColor(node->lightedModulateColor);
  isBlended = node->isBlended;
  wantBlending = node->wantBlending;
  wantSphereMap = node->wantSphereMap;
}


void WallSceneNode::setColor()
{
  if (BZDBCache::texture && useColorTexture) {
    myColor4f(1,1,1,1);
  }
  else if (dynamicColor != NULL) {
    myColor4fv(*dynamicColor);
  }
  else {
    switch (style) {
      case 0: { myColor4fv(color);                break; }
      case 1: { myColor4fv(lightedColor);         break; }
      case 2: { myColor4fv(modulateColor);        break; }
      case 3: { myColor4fv(lightedModulateColor); break; }
    }
  }
}

int WallSceneNode::splitWall(const fvec4& splitPlane,
			     const fvec3Array& vertices,
			     const fvec2Array& texcoords,
			     SceneNode*& front, SceneNode*& back) // const
{
  int i;
  const int count = vertices.getSize();
  const float fudgeFactor = 0.001f;
  const unsigned char BACK_SIDE = (1 << 0);
  const unsigned char FRONT_SIDE = (1 << 1);

  // arrays for tracking each vertex's side
  // and distance from the splitting plane
  // (assuming stack allocation with be faster then heap, might be wrong)
  // wonder how that compares to static vs. stack access speeds
  const int staticSize = 64;
  float* dists;
  unsigned char* array;
  float staticDists[staticSize];
  unsigned char staticArray[staticSize];
  if (count > staticSize) {
    array = new unsigned char[count];
    dists = new float[count];
  } else {
    array = staticArray;
    dists = staticDists;
  }

  // determine on which side of the plane each point lies
  int bothCount = 0;
  int backCount = 0;
  int frontCount = 0;
  for (i = 0; i < count; i++) {
    const float d = splitPlane.planeDist(vertices[i]);
    if (d < -fudgeFactor) {
      array[i] = BACK_SIDE;
      backCount++;
    }
    else if (d > fudgeFactor) {
      array[i] = FRONT_SIDE;
      frontCount++;
    }
    else {
      array[i] = (BACK_SIDE | FRONT_SIDE);
      bothCount++;
      backCount++;
      frontCount++;
    }
    dists[i] = d; // save for later
  }

  // see if we need to split
  if ((frontCount == 0) || (frontCount == bothCount)) {
    if (count > staticSize) {
      delete[] array;
      delete[] dists;
    }
    return -1; // node is on the back side
  }

  if ((backCount == 0) || (backCount == bothCount)) {
    if (count > staticSize) {
      delete[] array;
      delete[] dists;
    }
    return +1; // node is on the front side
  }

  // get the first old front and back points
  int firstFront = -1;
  int firstBack  = -1;

  for (i = 0; i < count; i++) {

    const int next = (i + 1) % count; // the next index

    if (array[next] & FRONT_SIDE) {
      if (!(array[i] & FRONT_SIDE)) {
	firstFront = next;
      }
    }
    if (array[next] & BACK_SIDE) {
      if (!(array[i] & BACK_SIDE)) {
	firstBack = next;
      }
    }
  }

  // get the last old front and back points
  const int lastFront = (firstFront + frontCount - 1) % count;
  const int lastBack  = (firstBack  + backCount - 1)  % count;

  // add in extra counts for the splitting vertices
  if (firstFront != lastBack) {
    frontCount++;
    backCount++;
  }
  if (firstBack != lastFront) {
    frontCount++;
    backCount++;
  }

  // make space for new polygons
  fvec3Array vertexFront(frontCount);
  fvec2Array uvFront(frontCount);
  fvec3Array vertexBack(backCount);
  fvec2Array uvBack(backCount);

  // fill in the splitting vertices
  int frontIndex = 0;
  int backIndex = 0;
  if (firstFront != lastBack) {
    fvec3 splitVertex;
    fvec2 splitUV;
    splitEdge(dists[firstFront], dists[lastBack],
	      vertices[firstFront], vertices[lastBack],
	      texcoords[firstFront], texcoords[lastBack],
	      splitVertex, splitUV);
    vertexFront[0] = splitVertex;
    uvFront[0] = splitUV;
    frontIndex++; // bump up the head
    const int last = backCount - 1;
    vertexBack[last] = splitVertex;
    uvBack[last] = splitUV;
  }
  if (firstBack != lastFront) {
    fvec3 splitVertex;
    fvec2 splitUV;
    splitEdge(dists[firstBack], dists[lastFront],
	      vertices[firstBack], vertices[lastFront],
	      texcoords[firstBack], texcoords[lastFront],
	      splitVertex, splitUV);
    vertexBack[0] = splitVertex;
    uvBack[0] = splitUV;
    backIndex++; // bump up the head
    const int last = frontCount - 1;
    vertexFront[last] = splitVertex;
    uvFront[last] = splitUV;
  }

  // fill in the old front side vertices
  const int endFront = (lastFront + 1) % count;
  for (i = firstFront; i != endFront; i = (i + 1) % count) {
    vertexFront[frontIndex] = vertices[i];
    uvFront[frontIndex] = texcoords[i];
    frontIndex++;
  }

  // fill in the old back side vertices
  const int endBack = (lastBack + 1) % count;
  for (i = firstBack; i != endBack; i = (i + 1) % count) {
    vertexBack[backIndex] = vertices[i];
    uvBack[backIndex] = texcoords[i];
    backIndex++;
  }

  // make new nodes
  front = new PolyWallSceneNode(vertexFront, uvFront);
  back = new PolyWallSceneNode(vertexBack, uvBack);

  // free the arrays, if required
  if (count > staticSize) {
    delete[] array;
    delete[] dists;
  }

  return 0; // generated new front and back nodes
}


void WallSceneNode::splitEdge(float d1, float d2,
			      const fvec3& p1,  const fvec3& p2,
			      const fvec2& uv1, const fvec2& uv2,
			      fvec3& p, fvec2& uv) // const
{
  // compute fraction along edge where split occurs
  float t1 = (d2 - d1);
  if (t1 != 0.0f) { // shouldn't happen
    t1 = -(d1 / t1);
  }

  // compute vertex
  p = p1 + (t1 * (p2  - p1));

  // compute texture coordinate
  uv = uv1 + (t1 * (uv2 - uv1));
}


bool WallSceneNode::inAxisBox (const Extents& /*exts*/) const
{
  // this should never happen, only the TriWallSceneNode
  // and QuadWallSceneNode version of this function will
  // be called
  printf ("WallSceneNode::inAxisBox() was called!\n");
  exit (1);
  return false;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
