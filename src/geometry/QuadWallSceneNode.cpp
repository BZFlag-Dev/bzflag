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
#include "QuadWallSceneNode.h"

// system headers
#include <math.h>

// common headers
#include "bzfgl.h"
#include "bzflag/SceneRenderer.h" // FIXME (SceneRenderer.cpp is in src/bzflag)
#include "game/BZDBCache.h"
#include "game/Intersect.h"

// local headers
#include "geometry/ViewFrustum.h"


//
// QuadWallSceneNode::Geometry
//

QuadWallSceneNode::Geometry::Geometry(QuadWallSceneNode* _wall,
                                      int uCount, int vCount,
                                      const fvec3& base,
                                      const fvec3& uEdge,
                                      const fvec3& vEdge,
                                      const float* _normal,
                                      float uOffset, float vOffset,
                                      float uRepeats, float vRepeats,
                                      bool fixedUVs)
  : wall(_wall)
  , style(0)
  , ds(uCount)
  , dt(vCount)
  , dsq(uCount / 4)
  , dsr(uCount % 4)
  , normal(_normal)
  , vertex((uCount + 1) * (vCount + 1))
  , uv((uCount + 1) * (vCount + 1)) {
  for (int n = 0, j = 0; j <= vCount; j++) {
    const float t = (float)j / (float)vCount;
    for (int i = 0; i <= uCount; n++, i++) {
      const float s = (float)i / (float)uCount;
      vertex[n][0] = base[0] + s * uEdge[0] + t * vEdge[0];
      vertex[n][1] = base[1] + s * uEdge[1] + t * vEdge[1];
      vertex[n][2] = base[2] + s * uEdge[2] + t * vEdge[2];
      uv[n][0] = uOffset + s * uRepeats;
      uv[n][1] = vOffset + t * vRepeats;
    }
  }

  static BZDB_bool remapTexCoords("remapTexCoords");
  if (remapTexCoords && !fixedUVs) {
    const float uScale = 10.0f / floorf(10.0f * uEdge.length() / uRepeats);
    const float vScale = 10.0f / floorf(10.0f * vEdge.length() / vRepeats);
    if (fabsf(normal[2]) > 0.999f) {
      // horizontal surface
      for (int i = 0; i < vertex.getSize(); i++) {
        uv[i][0] = uScale * vertex[i][0];
        uv[i][1] = vScale * vertex[i][1];
      }
    }
    else {
      // vertical surface
      const fvec2 nh = fvec2(normal[0], normal[1]).normalize();
      const float vs = 1.0f / sqrtf(1.0f - (normal[2] * normal[2]));
      for (int i = 0; i < vertex.getSize(); i++) {
        const fvec3& v = vertex[i];
        const float uGeoScale = (nh[0] * v[1]) - (nh[1] * v[0]);
        const float vGeoScale = v[2] * vs;
        uv[i][0] = uScale * uGeoScale;
        uv[i][1] = vScale * vGeoScale;
      }
    }
  }

  triangles = 2 * (uCount * vCount);
}


QuadWallSceneNode::Geometry::~Geometry() {
  // do nothing
}


#define RENDER(_e)              \
  for (int k = 0, t = 0; t < dt; t++) {         \
    glBegin(GL_TRIANGLE_STRIP);           \
    for (int s = 0; s < dsq; k += 4, s++) {       \
      _e(k+ds+1);             \
      _e(k);                \
      _e(k+ds+2);             \
      _e(k+1);                \
      _e(k+ds+3);             \
      _e(k+2);                \
      _e(k+ds+4);             \
      _e(k+3);                \
    }                 \
    switch (dsr) {              \
      case 3:               \
  _e(k+ds+1);             \
  _e(k);                \
  k++;                \
  /* fall through */            \
      case 2:               \
  _e(k+ds+1);             \
  _e(k);                \
  k++;                \
  /* fall through */            \
      case 1:               \
  _e(k+ds+1);             \
  _e(k);                \
  k++;                \
  /* fall through */            \
      case 0:               \
  /* don't forget right edge of last quad on row */   \
  _e(k+ds+1);             \
  _e(k);                \
  k++;                \
    }                 \
    glEnd();                \
  }
#define EMITV(_i) glVertex3fv(vertex[_i])
#define EMITVT(_i)  glTexCoord2fv(uv[_i]); glVertex3fv(vertex[_i])


void QuadWallSceneNode::Geometry::render() {
  wall->applyColor();
  glNormal3fv(normal);
  if (style >= 2) {
    drawVT();
  }
  else {
    drawV();
  }
  addTriangleCount(triangles);
  return;
}


void QuadWallSceneNode::Geometry::renderShadow() {
  int last = (ds + 1) * dt;
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3fv(vertex[last]);
  glVertex3fv(vertex[0]);
  glVertex3fv(vertex[last + ds]);
  glVertex3fv(vertex[ds]);
  glEnd();
  addTriangleCount(2);
}


void QuadWallSceneNode::Geometry::drawV() const {
  RENDER(EMITV)
}


void QuadWallSceneNode::Geometry::drawVT() const {
  RENDER(EMITVT)
}


const fvec3& QuadWallSceneNode::Geometry::getVertex(int i) const {
  return vertex[i];
}


//
// QuadWallSceneNode
//

QuadWallSceneNode::QuadWallSceneNode(const fvec3& base,
                                     const fvec3& uEdge,
                                     const fvec3& vEdge,
                                     float uOffset,
                                     float vOffset,
                                     float uRepeats,
                                     float vRepeats,
                                     bool makeLODs) {
  init(base, uEdge, vEdge, uOffset, vOffset, uRepeats, vRepeats, makeLODs, false);
}


QuadWallSceneNode::QuadWallSceneNode(const fvec3& base,
                                     const fvec3& uEdge,
                                     const fvec3& vEdge,
                                     float uRepeats,
                                     float vRepeats,
                                     bool makeLODs,
                                     bool fixedUVs) {
  init(base, uEdge, vEdge, 0.0f, 0.0f, uRepeats, vRepeats, makeLODs, fixedUVs);
}


void QuadWallSceneNode::init(const fvec3& base,
                             const fvec3& uEdge,
                             const fvec3& vEdge,
                             float uOffset,
                             float vOffset,
                             float uRepeats,
                             float vRepeats,
                             bool makeLODs,
                             bool fixedUVs) {
  // record plane and bounding sphere info
  fvec4 myPlane;
  myPlane.xyz() = fvec3::cross(uEdge, vEdge);
  myPlane.w = -fvec3::dot(base, myPlane.xyz());
  setPlane(myPlane);

  fvec4 mySphere;
  mySphere.xyz() = 0.5f * (uEdge + vEdge);
  mySphere.w = mySphere.xyz().lengthSq();
  mySphere.xyz() += base;
  setSphere(mySphere);

  // get length of sides
  const float uLength = uEdge.length();
  const float vLength = vEdge.length();
  float area = uLength * vLength;


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

  // compute how many LODs required to get smaller edge down to
  // elements no bigger than 4 units on a side.
  int uElements = int(uLength) / 2;
  int vElements = int(vLength) / 2;
  int uLevels = 1, vLevels = 1;
  while (uElements >>= 1) { uLevels++; }
  while (vElements >>= 1) { vLevels++; }
  int numLevels = (uLevels < vLevels ? uLevels : vLevels);

  // if overly rectangular then add levels to square it up
  bool needsSquaring = false;
  if (makeLODs) {
    if (uLevels >= vLevels + 2) {
      needsSquaring = true;
      numLevels += (uLevels - vLevels) / 2;
    }
    else if (vLevels >= uLevels + 2) {
      needsSquaring = true;
      numLevels += (vLevels - uLevels) / 2;
    }
  }

  // if no lod's required then don't make any except most coarse
  if (!makeLODs) {
    numLevels = 1;
  }

  // make level of detail and element area arrays
  nodes = new Geometry*[numLevels];
  float* areas = new float[numLevels];

  // make top level (single polygon)
  int level = 0;
  uElements = 1;
  vElements = 1;
  areas[level] = area;
  nodes[level++] = new Geometry(this, uElements, vElements,
                                base, uEdge, vEdge,
                                getPlaneRaw(), uOffset, vOffset,
                                uRepeats, vRepeats, fixedUVs);
  shadowNode = new Geometry(this, uElements, vElements,
                            base, uEdge, vEdge,
                            getPlaneRaw(), uOffset, vOffset,
                            uRepeats, vRepeats, fixedUVs);
  shadowNode->setStyle(0);

  // make squaring levels if necessary
  if (needsSquaring) {
    uElements = 1;
    vElements = 1;
    if (uLevels > vLevels) {
      int count = (uLevels - vLevels) / 2;
      while (count-- > 0) {
        uElements <<= 2;
        areas[level] = area / (float)uElements;
        nodes[level++] = new Geometry(this, uElements, vElements,
                                      base, uEdge, vEdge,
                                      getPlaneRaw(), uOffset, vOffset,
                                      uRepeats, vRepeats, fixedUVs);

      }
      area /= (float)uElements;
    }
    else {
      int count = (vLevels - uLevels) / 2;
      while (count-- > 0) {
        vElements <<= 2;
        areas[level] = area / (float)vElements;
        nodes[level++] = new Geometry(this, uElements, vElements,
                                      base, uEdge, vEdge,
                                      getPlaneRaw(), uOffset, vOffset,
                                      uRepeats, vRepeats, fixedUVs);

      }
      area /= (float)vElements;
    }
  }

  // make remaining levels by doubling elements in each dimension
  while (level < numLevels) {
    uElements <<= 1;
    vElements <<= 1;
    area *= 0.25f;
    areas[level] = area;
    nodes[level++] = new Geometry(this, uElements, vElements,
                                  base, uEdge, vEdge,
                                  getPlaneRaw(), uOffset, vOffset,
                                  uRepeats, vRepeats, fixedUVs);
  }

  // record extents info
  for (int i = 0; i < 4; i++) {
    const fvec3& point = getVertex(i);
    extents.expandToPoint(point);
  }

  // record LOD info
  setNumLODs(numLevels, areas);
}


QuadWallSceneNode::~QuadWallSceneNode() {
  // free LODs
  const int numLevels = getNumLODs();
  for (int i = 0; i < numLevels; i++) {
    delete nodes[i];
  }
  delete[] nodes;
  delete shadowNode;
}


int QuadWallSceneNode::split(const fvec4& _plane,
                             SceneNode*& front, SceneNode*& back) const {
  // need to reorder vertices into counterclockwise order
  fvec3Array vertex(4);
  fvec2Array uv(4);
  for (int i = 0; i < 4; i++) {
    int j = i;
    if (j == 2 || j == 3) { j = 5 - j; }
    vertex[i] = nodes[0]->vertex[j];
    uv[i] = nodes[0]->uv[j];
  }
  return WallSceneNode::splitWall(_plane, vertex, uv, front, back);
}


void QuadWallSceneNode::addRenderNodes(SceneRenderer& renderer) {
  const int lod = pickLevelOfDetail(renderer);
  nodes[lod]->setStyle(getStyle());
  renderer.addRenderNode(nodes[lod], getWallGState());
}


void QuadWallSceneNode::addShadowNodes(SceneRenderer& renderer) {
  renderer.addShadowNode(shadowNode);
}


bool QuadWallSceneNode::inAxisBox(const Extents& exts) const {
  if (!extents.touches(exts)) {
    return false;
  }

  // NOTE: inefficient
  const fvec3 vertices[4] = {
    nodes[0]->getVertex(0),
    nodes[0]->getVertex(1),
    nodes[0]->getVertex(2),
    nodes[0]->getVertex(3)
  };

  return Intersect::testPolygonInAxisBox(4, vertices, getPlaneRaw(), exts);
}


int QuadWallSceneNode::getVertexCount() const {
  return 4;
}


const fvec3& QuadWallSceneNode::getVertex(int vertex) const {
  // re-map these to a counter-clockwise order
  const int remap[4] = {0, 1, 3, 2};
  return nodes[0]->getVertex(remap[vertex]);
}


void QuadWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes) {
  RenderSet rs = { nodes[0], getWallGState() };
  rnodes.push_back(rs);
  return;
}


void QuadWallSceneNode::renderRadar() {
  if (plane[2] > 0.0f) {
    nodes[0]->renderRadar();
  }
  return;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
