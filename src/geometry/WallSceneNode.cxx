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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "WallSceneNode.h"
#include "ViewFrustum.h"
#include "SceneRenderer.h"
#include "PolyWallSceneNode.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

WallSceneNode::WallSceneNode() : numLODs(0),
				elementAreas(NULL),
				style(0)
{
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
  ZFlip = false;
  useColorTexture = false;
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
  const float n = 1.0f / sqrtf(_plane[0] * _plane[0] +
				_plane[1] * _plane[1] + _plane[2] * _plane[2]);

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
  const GLfloat* plane = getPlane();
  if (eye[0]*plane[0] + eye[1]*plane[1] + eye[2]*plane[2] + plane[3] <= 0.0f)
    return true;
    
  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible)
    return false;
 
  // get signed distance of wall center to each frustum side.
  // if more than radius outside then cull
  int i;
  float d[5], d2[5];
  const GLfloat* sphere = getSphere();
  for (i = 0; i < 5; i++) {
    const GLfloat* norm = frustum.getSide(i);
    d[i] = sphere[0] * norm[0] + sphere[1] * norm[1] +
		sphere[2] * norm[2] + norm[3];
    if (d[i] < 0.0f && (d2[i] = d[i]*d[i]) > sphere[3])
      return true;
  }

  // see if center of wall is inside each frustum side
  if (d[0] >= 0.0f && d[1] >= 0.0f && d[2] >= 0.0f &&
      d[3] >= 0.0f && d[4] >= 0.0f)
    return false;

  // most complicated test:  for sides sphere is behind, see if
  // center is beyond radius times the sine of the angle between
  // the normals, or equivalently:
  //	distance^2 > radius^2 * (1 - cos^2)
  // if so the wall is outside the view frustum
  for (i = 0; i < 5; i++) {
    if (d[i] >= 0.0f) continue;
    const GLfloat* norm = frustum.getSide(i);
    const GLfloat c = norm[0]*plane[0] + norm[1]*plane[1] + norm[2]*plane[2];
    if (d2[i] > sphere[3] * (1.0f - c*c)) return true;
  }

  // probably visible
  return false;
}

int			WallSceneNode::pickLevelOfDetail(
					const SceneRenderer& renderer) const
{
  int bestLOD = 0;

  const GLfloat* sphere = getSphere();
  const int numLights = renderer.getNumLights();
  for (int i = 0; i < numLights; i++) {
    const GLfloat* pos = renderer.getLight(i).getPosition();

    // get signed distance from plane
    GLfloat pd = pos[0] * plane[0] + pos[1] * plane[1] +
		pos[2] * plane[2] + plane[3];

    // ignore if behind wall
    if (pd < 0.0f) continue;

    // get squared distance from center of wall
    GLfloat ld = (pos[0] - sphere[0]) * (pos[0] - sphere[0]) +
		(pos[1] - sphere[1]) * (pos[1] - sphere[1]) +
		(pos[2] - sphere[2]) * (pos[2] - sphere[2]);

    // pick representative distance
    GLfloat d = (ld > 1.5f * sphere[3]) ? ld : pd * pd;

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
  const bool oldTransparent = (color[3] != 1.0f);
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  if (oldTransparent != (color[3] != 1.0f)) forceNotifyStyleChange();
}

void			WallSceneNode::setColor(const GLfloat* rgba)
{
  const bool oldTransparent = (color[3] != 1.0f);
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
  if (oldTransparent != (color[3] != 1.0f)) forceNotifyStyleChange();
}

void			WallSceneNode::setModulateColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  const bool oldTransparent = (modulateColor[3] != 1.0f);
  modulateColor[0] = r;
  modulateColor[1] = g;
  modulateColor[2] = b;
  modulateColor[3] = a;
  if (oldTransparent != (modulateColor[3] != 1.0f)) forceNotifyStyleChange();
}

void			WallSceneNode::setModulateColor(const GLfloat* rgba)
{
  const bool oldTransparent = (modulateColor[3] != 1.0f);
  modulateColor[0] = rgba[0];
  modulateColor[1] = rgba[1];
  modulateColor[2] = rgba[2];
  modulateColor[3] = rgba[3];
  if (oldTransparent != (modulateColor[3] != 1.0f)) forceNotifyStyleChange();
}

void			WallSceneNode::setLightedColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  const bool oldTransparent = (lightedColor[3] != 1.0f);
  lightedColor[0] = r;
  lightedColor[1] = g;
  lightedColor[2] = b;
  lightedColor[3] = a;
  if (oldTransparent != (lightedColor[3] != 1.0f)) forceNotifyStyleChange();
}

void			WallSceneNode::setLightedColor(const GLfloat* rgba)
{
  const bool oldTransparent = (lightedColor[3] != 1.0f);
  lightedColor[0] = rgba[0];
  lightedColor[1] = rgba[1];
  lightedColor[2] = rgba[2];
  lightedColor[3] = rgba[3];
  if (oldTransparent != (lightedColor[3] != 1.0f)) forceNotifyStyleChange();
}

void			WallSceneNode::setLightedModulateColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  const bool oldTransparent = (lightedModulateColor[3] != 1.0f);
  lightedModulateColor[0] = r;
  lightedModulateColor[1] = g;
  lightedModulateColor[2] = b;
  lightedModulateColor[3] = a;
  if (oldTransparent != (lightedModulateColor[3] != 1.0f))
    forceNotifyStyleChange();
}

void			WallSceneNode::setLightedModulateColor(
				const GLfloat* rgba)
{
  const bool oldTransparent = (lightedModulateColor[3] != 1.0f);
  lightedModulateColor[0] = rgba[0];
  lightedModulateColor[1] = rgba[1];
  lightedModulateColor[2] = rgba[2];
  lightedModulateColor[3] = rgba[3];
  if (oldTransparent != (lightedModulateColor[3] != 1.0f))
    forceNotifyStyleChange();
}

bool			WallSceneNode::isTransparent() const
{
  if (color[3] < 1.0f) {
    return true;
  } else {
    return false;
  }
}

void			WallSceneNode::setMaterial(const OpenGLMaterial& mat)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(mat);
  gstate = builder.getState();
  forceNotifyStyleChange();
}

void			WallSceneNode::setTexture(const int tex)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(tex);
  gstate = builder.getState();
  forceNotifyStyleChange();
}

void			WallSceneNode::notifyStyleChange(
				const SceneRenderer&)
{
  float alpha;
  bool lighted = (BZDB.isTrue("lighting") && gstate.isLighted());
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
    alpha = lighted ? lightedModulateColor[3] : modulateColor[3];
  }
  else {
    builder.enableTexture(false);
    alpha = lighted ? lightedColor[3] : color[3];
  }
  builder.enableTextureReplace(BZDB.isTrue("_texturereplace"));
  builder.enableMaterial(lighted);
  if (BZDBCache::blend && alpha != 1.0f) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(alpha);
  }
  gstate = builder.getState();
}

void			WallSceneNode::copyStyle(WallSceneNode* node)
{
  gstate = node->gstate;
  setColor(node->color);
  setModulateColor(node->modulateColor);
  setLightedColor(node->lightedColor);
  setLightedModulateColor(node->lightedModulateColor);
  forceNotifyStyleChange();
}

void			WallSceneNode::setColor()
{
  if (BZDBCache::texture && useColorTexture)
      glColor4f(1,1,1,1);
  else {
    switch (style) {
      case 0: myColor4fv(color); break;
      case 1: myColor4fv(lightedColor); break;
      case 2: myColor4fv(modulateColor); break;
      case 3: myColor4fv(lightedModulateColor); break;
    }
  }
}

int			WallSceneNode::splitWall(const GLfloat* plane,
				const GLfloat3Array& vertex,
				const GLfloat2Array& uv,
				SceneNode*& front, SceneNode*& back) // const
{
  // see if polygon formed by vertex list is split by plane.  if
  // so, split polygon and make two new PolyWallSceneNode's.

  // first figure out which side of plane last vertex is on
  const int count = vertex.getSize();
  int i, j = count, initInFront;
  do {
    j--;
    GLfloat d = vertex[j][0] * plane[0] + vertex[j][1] * plane[1] +
					vertex[j][2] * plane[2] + plane[3];
    if (fabsf(d) < 1.0e-1f) initInFront = -1;		// ambiguous
    else initInFront = (d >= 0.0f);			// well-defined
  } while (j > 0 && initInFront == -1);
  if (initInFront == -1) {
    // coplanar so can't be split, pick a side at random
    return (bzfrand() >= 0.5f) ? 1 : -1;
  }

  // no count how many vertices are on either side of the plane
  // (including vertices created when splitting)
  int frontCount = 0, backCount = 0;
  bool prevInFront = initInFront != 0, inFront;
  for (j = count - 1, i = 0; i < count; j = i, i++) {
    GLfloat d = vertex[i][0] * plane[0] + vertex[i][1] * plane[1] +
					vertex[i][2] * plane[2] + plane[3];
    if (fabsf(d) < 1.0e-1f) {				// ambiguous
      // if last point then ignore since it was accounted for when the
      // first edge was split, otherwise use previous side
      if (i == count - 1) break;
      inFront = prevInFront;
    }
    else inFront = (d >= 0.0f);				// well-defined
    if (inFront != prevInFront) {
      // split edge -- add a vertex to each side
      prevInFront = inFront;
      frontCount++;
      backCount++;
    }
    if (inFront) frontCount++;
    else backCount++;
  }

  // if either count is zero (or degenerate) then it's not split
  if (frontCount <= 2) return -1;
  else if (backCount <= 2) return 1;

  // make space for new polygons
  GLfloat3Array vertexFront(frontCount);
  GLfloat2Array uvFront(frontCount);
  GLfloat3Array vertexBack(backCount);
  GLfloat2Array uvBack(backCount);

  // fill in new polygons
  frontCount = 0;
  backCount = 0;
  prevInFront = initInFront != 0;
  for (j = count - 1, i = 0; i < count; j = i, i++) {
    GLfloat d = vertex[i][0] * plane[0] + vertex[i][1] * plane[1] +
					vertex[i][2] * plane[2] + plane[3];
    if (fabsf(d) < 1.0e-1f) {				// ambiguous
      // if last point then ignore since it was accounted for when the
      // first edge was split, otherwise use previous side
      if (i == count - 1) break;
      inFront = prevInFront;
    }
    else inFront = (d >= 0.0f);				// well-defined
    if (inFront != prevInFront) {
      // split edge -- add a vertex to each side
      GLfloat splitVertex[3], splitUV[2];
      splitEdge(vertex[j], vertex[i], uv[j], uv[i], plane, splitVertex, splitUV);
      vertexFront[frontCount][0] = splitVertex[0];
      vertexFront[frontCount][1] = splitVertex[1];
      vertexFront[frontCount][2] = splitVertex[2];
      uvFront[frontCount][0] = splitUV[0];
      uvFront[frontCount][1] = splitUV[1];
      vertexBack[backCount][0] = splitVertex[0];
      vertexBack[backCount][1] = splitVertex[1];
      vertexBack[backCount][2] = splitVertex[2];
      uvBack[backCount][0] = splitUV[0];
      uvBack[backCount][1] = splitUV[1];

      prevInFront = inFront;
      frontCount++;
      backCount++;
    }
    if (inFront) {
      vertexFront[frontCount][0] = vertex[i][0];
      vertexFront[frontCount][1] = vertex[i][1];
      vertexFront[frontCount][2] = vertex[i][2];
      uvFront[frontCount][0] = uv[i][0];
      uvFront[frontCount][1] = uv[i][1];
      frontCount++;
    }
    else {
      vertexBack[backCount][0] = vertex[i][0];
      vertexBack[backCount][1] = vertex[i][1];
      vertexBack[backCount][2] = vertex[i][2];
      uvBack[backCount][0] = uv[i][0];
      uvBack[backCount][1] = uv[i][1];
      backCount++;
    }
  }

  // make new nodes
  front = new PolyWallSceneNode(vertexFront, uvFront);
  back = new PolyWallSceneNode(vertexBack, uvBack);

  return 0;
}

void			WallSceneNode::splitEdge(const GLfloat* p1,
				const GLfloat* p2, const GLfloat* uv1,
				const GLfloat* uv2, const GLfloat* plane,
				GLfloat* p, GLfloat* uv) // const
{
  // compute fraction along edge where split occurs
  float e[3];
  e[0] = p2[0] - p1[0];
  e[1] = p2[1] - p1[1];
  e[2] = p2[2] - p1[2];
  GLfloat t = plane[0] * e[0] + plane[1] * e[1] + plane[2] * e[2];
  // should never be zero (edge coplanar with plane) but play it safe
  if (t != 0.0f)
    t = -(plane[0] * p1[0] + plane[1] * p1[1] + plane[2] * p1[2] + plane[3])/t;

  // compute vertex and uv at split
  p[0] = p1[0] + t * e[0];
  p[1] = p1[1] + t * e[1];
  p[2] = p1[2] + t * e[2];
  uv[0] = uv1[0] + t * (uv2[0] - uv1[0]);
  uv[1] = uv1[1] + t * (uv2[1] - uv1[1]);
}


void			WallSceneNode::getExtents (float* mins, float* maxs) const
{
  for (int i = 0; i < 3; i++) {
    mins[i] = -1234.0f;
    maxs[i] = +1234.0f;
  }
  return;
}


bool			WallSceneNode::inAxisBox (const float*, const float*) const
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

