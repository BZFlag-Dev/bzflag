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
#include "TriWallSceneNode.h"

// system headers
#include <math.h>
#include <stdlib.h>

// common headers
#include "bzfgl.h"
#include "Intersect.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cxx is in src/bzflag)


//
// TriWallSceneNode::Geometry
//

TriWallSceneNode::Geometry::Geometry(TriWallSceneNode* _wall, int eCount,
  const fvec3& base, const fvec3& uEdge, const fvec3& vEdge,
  const float* _normal, float uRepeats, float vRepeats)
: wall(_wall)
, style(0)
, de(eCount)
, normal(_normal)
, vertex((eCount+1) * (eCount+2) / 2)
, uv((eCount+1) * (eCount+2) / 2)
{
  for (int n = 0, j = 0; j <= eCount; j++) {
    const int k = eCount - j;
    const float t = (float)j / (float)eCount;
    for (int i = 0; i <= k; n++, i++) {
      const float s = (float)i / (float)eCount;
      vertex[n][0] = base[0] + s * uEdge[0] + t * vEdge[0];
      vertex[n][1] = base[1] + s * uEdge[1] + t * vEdge[1];
      vertex[n][2] = base[2] + s * uEdge[2] + t * vEdge[2];
      uv[n][0] = 0.0f + s * uRepeats;
      uv[n][1] = 0.0f + t * vRepeats;
    }
  }
  triangles = (eCount * eCount);
}


TriWallSceneNode::Geometry::~Geometry()
{
  // do nothing
}


#define	RENDER(_e)			\
  for (int k = 0, t = 0; t < de; t++) {	\
    int e = de - t;			\
    glBegin(GL_TRIANGLE_STRIP);		\
    for (int s = 0; s < e; k++, s++) {	\
      _e(k+e+1);			\
      _e(k);				\
    }					\
    _e(k);				\
    glEnd();				\
    k++;				\
  }
#define EMITV(_i)	glVertex3fv(vertex[_i])
#define EMITVT(_i)	glTexCoord2fv(uv[_i]); glVertex3fv(vertex[_i])


void TriWallSceneNode::Geometry::render()
{
  wall->setColor();
  glNormal3fv(normal);
  if (style >= 2) {
    drawVT();
  } else {
    drawV();
  }
  addTriangleCount(triangles);
  return;
}

void TriWallSceneNode::Geometry::renderShadow()
{
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3fv(vertex[(de + 1) * (de + 2) / 2 - 1]);
  glVertex3fv(vertex[0]);
  glVertex3fv(vertex[de]);
  glEnd();
  addTriangleCount(1);
}


void TriWallSceneNode::Geometry::drawV() const
{
  RENDER(EMITV)
}


void TriWallSceneNode::Geometry::drawVT() const
{
  RENDER(EMITVT)
}


const fvec3& TriWallSceneNode::Geometry::getVertex(int i) const
{
  return vertex[i];
}


//
// TriWallSceneNode
//

TriWallSceneNode::TriWallSceneNode(const fvec3& base,
                                   const fvec3& uEdge, const fvec3& vEdge,
                                   float uRepeats, float vRepeats,
                                   bool makeLODs)
{
  // record plane info
  fvec4 myPlane;
  myPlane.xyz() = fvec3::cross(uEdge, vEdge);
  myPlane.w = -fvec3::dot(base, myPlane.xyz());
  setPlane(myPlane);

  // record bounding sphere info -- ought to calculate center and
  // and radius of circumscribing sphere but it's late and i'm tired.
  // i'll just calculate something easy.  it hardly matters as it's
  // hard to tightly bound a triangle with a sphere.
  fvec4 mySphere;
  mySphere.xyz() = 0.5f * (uEdge + vEdge);
  mySphere.w = mySphere.xyz().lengthSq();
  mySphere.xyz() += base;
  setSphere(mySphere);

  // get length of sides
  const float uLength = uEdge.length();
  const float vLength = vEdge.length();
  float area = 0.5f * uLength * vLength;

  // If negative then these values aren't a number of times to repeat
  // the texture along the surface but the width, or a desired scaled
  // width, of the texture itself. Repeat the texture as many times
  // as necessary to fit the surface.
  if (uRepeats < 0.0f) {
    uRepeats = - uLength / uRepeats;
  }

  if (vRepeats < 0.0f) {
    vRepeats = - vLength / vRepeats;
  }

  // compute how many LODs required to get larger edge down to
  // elements no bigger than 4 units on a side.
  int uElements = int(uLength) / 2;
  int vElements = int(vLength) / 2;
  int uLevels = 1, vLevels = 1;
  while (uElements >>= 1) uLevels++;
  while (vElements >>= 1) vLevels++;
  int numLevels = (uLevels > vLevels ? uLevels : vLevels);

  // if no lod's required then don't make any except most coarse
  if (!makeLODs)
    numLevels = 1;

  // make level of detail and element area arrays
  nodes = new Geometry*[numLevels];
  float* areas = new float[numLevels];

  // make top level (single polygon)
  int level = 0;
  uElements = 1;
  areas[level] = area;
  nodes[level++] = new Geometry(this, uElements, base, uEdge, vEdge,
                                getPlaneRaw(), uRepeats, vRepeats);
  shadowNode = new Geometry(this, uElements, base, uEdge, vEdge,
                            getPlaneRaw(), uRepeats, vRepeats);
  shadowNode->setStyle(0);

  // make remaining levels by doubling elements in each dimension
  while (level < numLevels) {
    uElements <<= 1;
    area *= 0.25f;
    areas[level] = area;
    nodes[level++] = new Geometry(this, uElements, base, uEdge, vEdge,
                                  getPlaneRaw(), uRepeats, vRepeats);
  }

  // record extents info
  for (int i = 0; i < 3; i++) {
    extents.expandToPoint(getVertex(i));
  }

  // record LOD info
  setNumLODs(numLevels, areas);
}


TriWallSceneNode::~TriWallSceneNode()
{
  // free LODs
  const int numLevels = getNumLODs();
  for (int i = 0; i < numLevels; i++) {
    delete nodes[i];
  }
  delete[] nodes;
  delete shadowNode;
}


bool TriWallSceneNode::cull(const ViewFrustum& frustum) const
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


int TriWallSceneNode::split(const fvec4& _plane,
                            SceneNode*& front, SceneNode*& back) const
{
  return WallSceneNode::splitWall(_plane,
                                  nodes[0]->vertex, nodes[0]->uv, front, back);
}


void TriWallSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  const int lod = pickLevelOfDetail(renderer);
  nodes[lod]->setStyle(getStyle());
  renderer.addRenderNode(nodes[lod], getWallGState());
}


void TriWallSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  renderer.addShadowNode(shadowNode);
}


bool TriWallSceneNode::inAxisBox(const Extents& exts) const
{
  if (!extents.touches(exts)) {
    return false;
  }

  // NOTE: inefficient
  const fvec3 vertices[3] = {
    nodes[0]->getVertex(0),
    nodes[0]->getVertex(1),
    nodes[0]->getVertex(2)
  };

  return Intersect::testPolygonInAxisBox(3, vertices, getPlaneRaw(), exts);
}


int TriWallSceneNode::getVertexCount () const
{
  return 3;
}


const fvec3& TriWallSceneNode::getVertex(int vertex) const
{
  return nodes[0]->getVertex(vertex);
}


void TriWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
  RenderSet rs = { nodes[0], getWallGState() };
  rnodes.push_back(rs);
  return;
}


void TriWallSceneNode::renderRadar()
{
  if (plane[2] > 0.0f) {
    nodes[0]->renderRadar();
  }
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
