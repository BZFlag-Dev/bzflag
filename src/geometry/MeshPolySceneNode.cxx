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

// interface header
#include "MeshPolySceneNode.h"

// system headers
#include <assert.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "Intersect.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cxx is in src/bzflag)


// FIXME - no tesselation is done on for shot lighting


//
// MeshPolySceneNode::Geometry
//

MeshPolySceneNode::Geometry::Geometry(MeshPolySceneNode* node,
                                      const fvec3Array& _vertices,
                                      const fvec3Array& _normals,
                                      const fvec2Array& _texcoords,
                                      const fvec3& _normal)
: normal(_normal)
, vertices(_vertices)
, normals(_normals)
, texcoords(_texcoords)
{
  sceneNode = node;
  style = 0;
  return;
}


MeshPolySceneNode::Geometry::~Geometry()
{
  // do nothing
  return;
}

inline void MeshPolySceneNode::Geometry::drawV() const
{
  const int count = vertices.getSize();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < count; i++) {
    glVertex3fv(vertices[i]);
  }
  glEnd();
  return;
}


inline void MeshPolySceneNode::Geometry::drawVT() const
{
  const int count = vertices.getSize();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < count; i++) {
    glTexCoord2fv(texcoords[i]);
    glVertex3fv(vertices[i]);
  }
  glEnd();
  return;
}


inline void MeshPolySceneNode::Geometry::drawVN() const
{
  const int count = vertices.getSize();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < count; i++) {
    glNormal3fv(normals[i]);
    glVertex3fv(vertices[i]);
  }
  glEnd();
  return;
}


inline void MeshPolySceneNode::Geometry::drawVTN() const
{
  const int count = vertices.getSize();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < count; i++) {
    glTexCoord2fv(texcoords[i]);
    glNormal3fv(normals[i]);
    glVertex3fv(vertices[i]);
  }
  glEnd();
  return;
}


void MeshPolySceneNode::Geometry::render()
{
  sceneNode->setColor();

  if (normals.getSize() != 0) {
    if (style >= 2) {
      drawVTN();
    } else {
      drawVN();
    }
  } else {
    glNormal3fv(normal);
    if (style >= 2) {
      drawVT();
    } else {
      drawV();
    }
  }

  addTriangleCount(vertices.getSize() - 2);
  return;
}


void MeshPolySceneNode::Geometry::renderRadar()
{
  drawV();
  addTriangleCount(vertices.getSize() - 2);
  return;
}


void MeshPolySceneNode::Geometry::renderShadow()
{
  drawV();
  addTriangleCount(vertices.getSize() - 2);
  return;
}


//
// MeshPolySceneNode
//

MeshPolySceneNode::MeshPolySceneNode(const fvec4& _plane,
				     bool _noRadar, bool _noShadow,
				     const fvec3Array& vertices,
				     const fvec3Array& normals,
				     const fvec2Array& texcoords)
: node(this, vertices, normals, texcoords, plane.xyz())
{
  int i, j;
  const int count = vertices.getSize();
  assert(texcoords.getSize() == count);
  assert((normals.getSize() == 0) || (normals.getSize() == count));

  setPlane(_plane);

  noRadar = _noRadar || (plane[2] <= 0.0f); // pre-cull if we can
  noShadow = _noShadow;

  // choose axis to ignore (the one with the largest normal component)
  int ignoreAxis;
  const fvec4& normal = getPlaneRaw();
  if (fabsf(normal[0]) > fabsf(normal[1])) {
    if (fabsf(normal[0]) > fabsf(normal[2])) {
      ignoreAxis = 0;
    } else {
      ignoreAxis = 2;
    }
  } else {
    if (fabsf(normal[1]) > fabsf(normal[2])) {
      ignoreAxis = 1;
    } else {
      ignoreAxis = 2;
    }
  }

  // project vertices onto plane
  fvec2Array flat(count);
  switch (ignoreAxis) {
    case 0:
      for (i = 0; i < count; i++) {
	flat[i].x = vertices[i].y;
	flat[i].y = vertices[i].z;
      }
      break;
    case 1:
      for (i = 0; i < count; i++) {
	flat[i].x = vertices[i].z;
	flat[i].y = vertices[i].x;
      }
      break;
    case 2:
      for (i = 0; i < count; i++) {
	flat[i].x = vertices[i].x;
	flat[i].y = vertices[i].y;
      }
      break;
  }

  // compute area of polygon
  float* area = new float[1];
  area[0] = 0.0f;
  for (j = count - 1, i = 0; i < count; j = i, i++) {
    area[0] += (flat[j][0] * flat[i][1]) - (flat[j][1] * flat[i][0]);
  }
  area[0] = 0.5f * fabsf(area[0]) / normal[ignoreAxis];

  // set lod info
  setNumLODs(1, area);

  // compute the sphere center at the average of vertices
  fvec4 mySphere(0.0f, 0.0f, 0.0f, 0.0f);
  for (i = 0; i < count; i++) {
    mySphere.xyz() += vertices[i];
  }
  mySphere /= (float)count;
  // compute the sphere radius squared, use the maximum value
  for (i = 0; i < count; i++) {
    const float lenSq = (mySphere.xyz() - vertices[i]).lengthSq();
    if (mySphere.w < lenSq) {
      mySphere.w = lenSq;
    }
  }
  setSphere(mySphere);

  // record extents info
  for (i = 0; i < count; i++) {
    extents.expandToPoint(vertices[i]);
  }

  return;
}


MeshPolySceneNode::~MeshPolySceneNode()
{
  return;
}


bool MeshPolySceneNode::cull(const ViewFrustum& frustum) const
{
  // cull if eye is behind (or on) plane
  const fvec3& eye = frustum.getEye();
  if (plane.planeDist(eye) <= 0.0f) {
    return true;
  }

  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  const Frustum* f = (const Frustum *) &frustum;
  if (Intersect::testAxisBoxInFrustum(extents, f) == Intersect::Outside) {
    return true;
  }

  // probably visible
  return false;
}


bool MeshPolySceneNode::inAxisBox (const Extents& exts) const
{
  if (!extents.touches(exts)) {
    return false;
  }

  return Intersect::testPolygonInAxisBox(getVertexCount(),
                                         (const fvec3*)getVertices(),
                                         getPlaneRaw(), exts);
}


int MeshPolySceneNode::split(const fvec4& splitPlane,
			     SceneNode*& front, SceneNode*& back) const
{
  if (node.normals.getSize() > 0) {
    return splitWallVTN(splitPlane, node.vertices, node.normals, node.texcoords,
			front, back);
  }
  else {
    return splitWallVT(splitPlane, node.vertices, node.texcoords, front, back);
  }
}


void MeshPolySceneNode::addRenderNodes(SceneRenderer& renderer)
{
  node.setStyle(getStyle());
  const fvec4* dyncol = getDynamicColor();
  if ((dyncol == NULL) || (dyncol->a != 0.0f)) {
    renderer.addRenderNode(&node, getWallGState());
  }
  return;
}


void MeshPolySceneNode::addShadowNodes(SceneRenderer& renderer)
{
  if (!noShadow) {
    const fvec4* dyncol = getDynamicColor();
    if ((dyncol == NULL) || (dyncol->a != 0.0f)) {
      renderer.addShadowNode(&node);
    }
  }
  return;
}


void MeshPolySceneNode::renderRadar()
{
  if (!noRadar) {
    node.renderRadar();
  }
  return;
}


int MeshPolySceneNode::splitWallVTN(const fvec4& splitPlane,
				    const fvec3Array& vertices,
				    const fvec3Array& normals,
				    const fvec2Array& texcoords,
				    SceneNode*& front, SceneNode*& back) const
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
  fvec3Array vertexFront(frontCount);
  fvec3Array normalFront(frontCount);
  fvec2Array uvFront(frontCount);
  fvec3Array vertexBack(backCount);
  fvec3Array normalBack(backCount);
  fvec2Array uvBack(backCount);

  // fill in the splitting vertices
  int frontIndex = 0;
  int backIndex = 0;
  if (firstFront != lastBack) {
    fvec3 splitVertex, splitNormal;
    fvec2 splitUV;
    splitEdgeVTN(dists[firstFront],     dists[lastBack],
		 vertices[firstFront],  vertices[lastBack],
		 normals[firstFront],   normals[lastBack],
		 texcoords[firstFront], texcoords[lastBack],
		 splitVertex, splitNormal, splitUV);
    vertexFront[0] = splitVertex;
    normalFront[0] = splitNormal;
    uvFront[0] = splitUV;
    frontIndex++; // bump up the head
    const int last = backCount - 1;
    vertexBack[last] = splitVertex;
    normalBack[last] = splitNormal;
    uvBack[last] = splitUV;
  }
  if (firstBack != lastFront) {
    fvec3 splitVertex, splitNormal;
    fvec2 splitUV;
    splitEdgeVTN(dists[firstBack],     dists[lastFront],
		 vertices[firstBack],  vertices[lastFront],
		 normals[firstBack],   normals[lastFront],
		 texcoords[firstBack], texcoords[lastFront],
		 splitVertex, splitNormal, splitUV);
    vertexBack[0] = splitVertex;
    normalBack[0] = splitNormal;
    uvBack[0] = splitUV;
    backIndex++; // bump up the head
    const int last = frontCount - 1;
    vertexFront[last] = splitVertex;
    normalFront[last] = splitNormal;
    uvFront[last] = splitUV;
  }

  // fill in the old front side vertices
  const int endFront = (lastFront + 1) % count;
  for (i = firstFront; i != endFront; i = (i + 1) % count) {
    vertexFront[frontIndex] = vertices[i];
    normalFront[frontIndex] = normals[i];
    uvFront[frontIndex] = texcoords[i];
    frontIndex++;
  }

  // fill in the old back side vertices
  const int endBack = (lastBack + 1) % count;
  for (i = firstBack; i != endBack; i = (i + 1) % count) {
    vertexBack[backIndex] = vertices[i];
    normalBack[backIndex] = normals[i];
    uvBack[backIndex] = texcoords[i];
    backIndex++;
  }

  // make new nodes
  front = new MeshPolySceneNode(getPlaneRaw(), noRadar, noShadow,
				vertexFront, normalFront, uvFront);
  back = new MeshPolySceneNode(getPlaneRaw(), noRadar, noShadow,
			       vertexBack, normalBack, uvBack);

  // free the arrays, if required
  if (count > staticSize) {
    delete[] array;
    delete[] dists;
  }

  return 0; // generated new front and back nodes
}


void MeshPolySceneNode::splitEdgeVTN(float d1, float d2,
				     const fvec3& p1,  const fvec3& p2,
				     const fvec3& n1,  const fvec3& n2,
				     const fvec2& uv1, const fvec2& uv2,
				     fvec3& p, fvec3& n, fvec2& uv) const
{
  // compute fraction along edge where split occurs
  float t1 = (d2 - d1);
  if (t1 != 0.0f) { // shouldn't happen
    t1 = -(d1 / t1);
  }

  // compute vertex
  p = p1 + (t1 * (p2 - p1));

  // compute normal
  const float t2 = (1.0f - t1);
  n = (n1 * t2) + (n2 * t1);
  fvec3::normalize(n);

  // compute texture coordinate
  uv = uv1 + (t1 * (uv2 - uv1));

  return;
}


int MeshPolySceneNode::splitWallVT(const fvec4& splitPlane,
				   const fvec3Array& vertices,
				   const fvec2Array& texcoords,
				   SceneNode*& front, SceneNode*& back) const
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
  fvec3Array vertexFront(frontCount);
  fvec3Array normalFront(0);
  fvec2Array uvFront(frontCount);
  fvec3Array vertexBack(backCount);
  fvec3Array normalBack(0);
  fvec2Array uvBack(backCount);

  // fill in the splitting vertices
  int frontIndex = 0;
  int backIndex = 0;
  if (firstFront != lastBack) {
    fvec3 splitVertex;
    fvec2 splitUV;
    splitEdgeVT(dists[firstFront],     dists[lastBack],
		vertices[firstFront],  vertices[lastBack],
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
    splitEdgeVT(dists[firstBack],     dists[lastFront],
		vertices[firstBack],  vertices[lastFront],
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
  front = new MeshPolySceneNode(getPlaneRaw(), noRadar, noShadow,
				vertexFront, normalFront, uvFront);
  back = new MeshPolySceneNode(getPlaneRaw(), noRadar, noShadow,
			       vertexBack, normalBack, uvBack);

  // free the arrays, if required
  if (count > staticSize) {
    delete[] array;
    delete[] dists;
  }

  return 0; // generated new front and back nodes
}


void MeshPolySceneNode::splitEdgeVT(float d1, float d2,
				    const fvec3& p1, const fvec3& p2,
				    const fvec2& uv1, const fvec2& uv2,
				    fvec3& p, fvec2& uv) const
{
  // compute fraction along edge where split occurs
  float t1 = (d2 - d1);
  if (t1 != 0.0f) { // shouldn't happen
    t1 = -(d1 / t1);
  }

  // compute vertex
  p = p1 + (t1 * (p2 - p1));

  // compute texture coordinate
  uv = uv1 + (t1 * (uv2 - uv1));

  return;
}


void MeshPolySceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
  RenderSet rs = { &node, getWallGState() };
  rnodes.push_back(rs);
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
