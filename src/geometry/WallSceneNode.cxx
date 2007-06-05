/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

WallSceneNode::WallSceneNode() : numLODs(0),
				elementAreas(NULL),
				style(0)
{
  noPlane      = false;
  dynamicColor = NULL;
  color[3] = 1.0f;
  modulateColor[3] = 1.0f;
  lightedColor[3] = 1.0f;
  lightedModulateColor[3] = 1.0f;
  color[3] = 1.0f;
  color[3] = 1.0f;
  color[3] = 1.0f;
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

void			WallSceneNode::setNumLODs(int num, float* areas)
{
  numLODs = num;
  elementAreas = areas;
}

void			WallSceneNode::setPlane(const GLfloat _plane[4])
{
  // get normalization factor
  const float n = 1.0f / sqrtf((_plane[0] * _plane[0]) +
					       (_plane[1] * _plane[1]) +
					       (_plane[2] * _plane[2]));

  // store normalized plane equation
  plane[0] = n * _plane[0];
  plane[1] = n * _plane[1];
  plane[2] = n * _plane[2];
  plane[3] = n * _plane[3];
}

bool			WallSceneNode::cull(const ViewFrustum& frustum) const
{
  // cull if eye is behind (or on) plane
  const GLfloat* eye = frustum.getEye();
  const float eyedot = (eye[0] * plane[0]) +
		       (eye[1] * plane[1]) +
		       (eye[2] * plane[2]) + plane[3];
  if (eyedot <= 0.0f) {
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
  const GLfloat* mySphere = getSphere();
  bool inside = true;
  for (i = 0; i < planeCount; i++) {
    const GLfloat* norm = frustum.getSide(i);
    d[i] = (mySphere[0] * norm[0]) +
	   (mySphere[1] * norm[1]) +
	   (mySphere[2] * norm[2]) + norm[3];
    if (d[i] < 0.0f) {
      d2[i] = d[i] * d[i];
      if (d2[i] > mySphere[3]) {
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
    const GLfloat* norm = frustum.getSide(i);
    const GLfloat c = norm[0]*plane[0] + norm[1]*plane[1] + norm[2]*plane[2];
    if (d2[i] > mySphere[3] * (1.0f - c*c)) {
      return true;
    }
  }

  // probably visible
  return false;
}

int			WallSceneNode::pickLevelOfDetail(
					const SceneRenderer& renderer) const
{
  if (!BZDBCache::tesselation) {
    return 0;
  }

  int bestLOD = 0;

  const GLfloat* mySphere = getSphere();
  const int numLights = renderer.getNumLights();
  for (int i = 0; i < numLights; i++) {
    const GLfloat* pos = renderer.getLight(i).getPosition();

    // get signed distance from plane
    GLfloat pd = pos[0] * plane[0] + pos[1] * plane[1] +
		pos[2] * plane[2] + plane[3];

    // ignore if behind wall
    if (pd < 0.0f) continue;

    // get squared distance from center of wall
    GLfloat ld = (pos[0] - mySphere[0]) * (pos[0] - mySphere[0]) +
		(pos[1] - mySphere[1]) * (pos[1] - mySphere[1]) +
		(pos[2] - mySphere[2]) * (pos[2] - mySphere[2]);

    // pick representative distance
    GLfloat d = (ld > 1.5f * mySphere[3]) ? ld : pd * pd;

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

GLfloat			WallSceneNode::getDistance(const GLfloat* eye) const
{
  const GLfloat d = plane[0] * eye[0] + plane[1] * eye[1] +
					plane[2] * eye[2] + plane[3];
  return d * d;
}

void			WallSceneNode::setColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
}

void			WallSceneNode::setDynamicColor(const GLfloat* rgba)
{
  dynamicColor = rgba;
  return;
}

void			WallSceneNode::setBlending(bool blend)
{
  wantBlending = blend;
  return;
}

void			WallSceneNode::setSphereMap(bool sphereMapping)
{
  wantSphereMap = sphereMapping;
  return;
}

void			WallSceneNode::setColor(const GLfloat* rgba)
{
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
}

void			WallSceneNode::setModulateColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  modulateColor[0] = r;
  modulateColor[1] = g;
  modulateColor[2] = b;
  modulateColor[3] = a;
}

void			WallSceneNode::setModulateColor(const GLfloat* rgba)
{
  modulateColor[0] = rgba[0];
  modulateColor[1] = rgba[1];
  modulateColor[2] = rgba[2];
  modulateColor[3] = rgba[3];
}

void			WallSceneNode::setLightedColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  lightedColor[0] = r;
  lightedColor[1] = g;
  lightedColor[2] = b;
  lightedColor[3] = a;
}

void			WallSceneNode::setLightedColor(const GLfloat* rgba)
{
  lightedColor[0] = rgba[0];
  lightedColor[1] = rgba[1];
  lightedColor[2] = rgba[2];
  lightedColor[3] = rgba[3];
}

void			WallSceneNode::setLightedModulateColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  lightedModulateColor[0] = r;
  lightedModulateColor[1] = g;
  lightedModulateColor[2] = b;
  lightedModulateColor[3] = a;
}

void			WallSceneNode::setLightedModulateColor(
				const GLfloat* rgba)
{
  lightedModulateColor[0] = rgba[0];
  lightedModulateColor[1] = rgba[1];
  lightedModulateColor[2] = rgba[2];
  lightedModulateColor[3] = rgba[3];
}

void			WallSceneNode::setAlphaThreshold(float thresh)
{
  alphaThreshold = thresh;
}

void			WallSceneNode::setNoCulling(bool value)
{
  noCulling = value;
}

void			WallSceneNode::setNoSorting(bool value)
{
  noSorting = value;
}

void			WallSceneNode::setMaterial(const OpenGLMaterial& mat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(mat, RENDERER.useQuality() > _LOW_QUALITY);
  gstate = builder.getState();
}

void			WallSceneNode::setTexture(const int tex)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(tex);
  gstate = builder.getState();
}

void			WallSceneNode::setTextureMatrix(const GLfloat* texmat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTextureMatrix(texmat);
  gstate = builder.getState();
}

void			WallSceneNode::notifyStyleChange()
{
  float alpha;
  bool lighted = (BZDBCache::lighting && gstate.isLighted());
  OpenGLGStateBuilder builder(gstate);
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
    alpha = lighted ? lightedModulateColor[3] : modulateColor[3];
  }
  else {
    builder.enableTexture(false);
    builder.enableTextureMatrix(false);
    alpha = lighted ? lightedColor[3] : color[3];
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

void			WallSceneNode::copyStyle(WallSceneNode* node)
{
  gstate = node->gstate;
  useColorTexture = node->useColorTexture;
  dynamicColor = node->dynamicColor;
  setColor(node->color);
  setModulateColor(node->modulateColor);
  setLightedColor(node->lightedColor);
  setLightedModulateColor(node->lightedModulateColor);
  isBlended = node->isBlended;
  wantBlending = node->wantBlending;
  wantSphereMap = node->wantSphereMap;
}

void			WallSceneNode::setColor()
{
  if (BZDBCache::texture && useColorTexture) {
    myColor4f(1,1,1,1);
  }
  else if (dynamicColor != NULL) {
    myColor4fv(dynamicColor);
  }
  else {
    switch (style) {
      case 0: myColor4fv(color); break;
      case 1: myColor4fv(lightedColor); break;
      case 2: myColor4fv(modulateColor); break;
      case 3: myColor4fv(lightedModulateColor); break;
    }
  }
}

int WallSceneNode::splitWall(const GLfloat* splitPlane,
			     const GLfloat3Array& vertices,
			     const GLfloat2Array& texcoords,
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
    const GLfloat d = (vertices[i][0] * splitPlane[0]) +
		      (vertices[i][1] * splitPlane[1]) +
		      (vertices[i][2] * splitPlane[2]) + splitPlane[3];
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
  int firstFront = -1, firstBack = -1;

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
  int lastFront = (firstFront + frontCount - 1) % count;
  int lastBack = (firstBack + backCount - 1) % count;

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
  GLfloat3Array vertexFront(frontCount);
  GLfloat2Array uvFront(frontCount);
  GLfloat3Array vertexBack(backCount);
  GLfloat2Array uvBack(backCount);

  // fill in the splitting vertices
  int frontIndex = 0;
  int backIndex = 0;
  if (firstFront != lastBack) {
    GLfloat splitVertex[3], splitUV[2];
    splitEdge(dists[firstFront], dists[lastBack],
	      vertices[firstFront], vertices[lastBack],
	      texcoords[firstFront], texcoords[lastBack],
	      splitVertex, splitUV);
    memcpy(vertexFront[0], splitVertex, sizeof(GLfloat[3]));
    memcpy(uvFront[0], splitUV, sizeof(GLfloat[2]));
    frontIndex++; // bump up the head
    const int last = backCount - 1;
    memcpy(vertexBack[last], splitVertex, sizeof(GLfloat[3]));
    memcpy(uvBack[last], splitUV, sizeof(GLfloat[2]));
  }
  if (firstBack != lastFront) {
    GLfloat splitVertex[3], splitUV[2];
    splitEdge(dists[firstBack], dists[lastFront],
	      vertices[firstBack], vertices[lastFront],
	      texcoords[firstBack], texcoords[lastFront],
	      splitVertex, splitUV);
    memcpy(vertexBack[0], splitVertex, sizeof(GLfloat[3]));
    memcpy(uvBack[0], splitUV, sizeof(GLfloat[2]));
    backIndex++; // bump up the head
    const int last = frontCount - 1;
    memcpy(vertexFront[last], splitVertex, sizeof(GLfloat[3]));
    memcpy(uvFront[last], splitUV, sizeof(GLfloat[2]));
  }

  // fill in the old front side vertices
  const int endFront = (lastFront + 1) % count;
  for (i = firstFront; i != endFront; i = (i + 1) % count) {
    memcpy(vertexFront[frontIndex], vertices[i], sizeof(GLfloat[3]));
    memcpy(uvFront[frontIndex], texcoords[i], sizeof(GLfloat[2]));
    frontIndex++;
  }

  // fill in the old back side vertices
  const int endBack = (lastBack + 1) % count;
  for (i = firstBack; i != endBack; i = (i + 1) % count) {
    memcpy(vertexBack[backIndex], vertices[i], sizeof(GLfloat[3]));
    memcpy(uvBack[backIndex], texcoords[i], sizeof(GLfloat[2]));
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
			      const GLfloat* p1, const GLfloat* p2,
			      const GLfloat* uv1, const GLfloat* uv2,
			      GLfloat* p, GLfloat* uv) // const
{
  // compute fraction along edge where split occurs
  float t1 = (d2 - d1);
  if (t1 != 0.0f) { // shouldn't happen
    t1 = -(d1 / t1);
  }

  // compute vertex
  p[0] = p1[0] + (t1 * (p2[0] - p1[0]));
  p[1] = p1[1] + (t1 * (p2[1] - p1[1]));
  p[2] = p1[2] + (t1 * (p2[2] - p1[2]));

  // compute texture coordinate
  uv[0] = uv1[0] + (t1 * (uv2[0] - uv1[0]));
  uv[1] = uv1[1] + (t1 * (uv2[1] - uv1[1]));
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
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
