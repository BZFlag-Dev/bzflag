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
#include "PolyWallSceneNode.h"

// system headers
#include <assert.h>
#include <math.h>

// common headers
#include "bzfgl.h"
#include "SceneRenderer.h" // FIXME (SceneRenderer.cxx is in src/bzflag)

//
// PolyWallSceneNode::Geometry
//

PolyWallSceneNode::Geometry::Geometry(PolyWallSceneNode* _wall,
				const fvec3Array& _vertex,
				const fvec2Array& _uv,
				const float* _normal) :
				wall(_wall),
				normal(_normal),
				vertex(_vertex),
				uv(_uv)
{
  // do nothing
}

PolyWallSceneNode::Geometry::~Geometry()
{
  // do nothing
}

void			PolyWallSceneNode::Geometry::render()
{
  wall->setColor();
  glNormal3fv(normal);
  if (style >= 2) {
    drawVT();
  } else {
    drawV();
  }
  addTriangleCount(vertex.getSize() - 2);
  return;
}

void			PolyWallSceneNode::Geometry::drawV() const
{
  const int count = vertex.getSize();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < count; i++)
    glVertex3fv(vertex[i]);
  glEnd();
}

void			PolyWallSceneNode::Geometry::drawVT() const
{
  const int count = vertex.getSize();
  glBegin(GL_TRIANGLE_FAN);
  for (int i = 0; i < count; i++) {
    glTexCoord2fv(uv[i]);
    glVertex3fv(vertex[i]);
  }
  glEnd();
}

//
// PolyWallSceneNode
//

PolyWallSceneNode::PolyWallSceneNode(const fvec3Array& vertex,
                                     const fvec2Array& uv)
{
  const int count = vertex.getSize();
  assert(uv.getSize() == count);

  // figure out plane (find non-colinear edges and get cross product)
  fvec3 uEdge, vEdge;
  fvec4 myPlane;
  float uLen, vLen, nLen;
  uEdge = vertex[0] - vertex[count - 1];
  uLen = uEdge.lengthSq();
  int i;
  for (i = 1; i < count; i++) {
    vEdge = vertex[i] - vertex[i - 1];
    vLen = vEdge.lengthSq();
    myPlane.xyz() = fvec3::cross(uEdge, vEdge);
    nLen = myPlane.lengthSq();
    if (nLen > (1.0e-5f * uLen * vLen)) {
      break;
    }
    uEdge = vEdge;
    uLen = vLen;
  }
  myPlane.w = -fvec3::dot(myPlane.xyz(), vertex[0]);
  setPlane(myPlane);

  // choose axis to ignore (the one with the largest normal component)
  const fvec4& norm = getPlaneRaw();
  const int ignoreAxis =
    (fabsf(norm.x) > fabsf(norm.y)) ? ((fabsf(norm.x) > fabsf(norm.z)) ? 0 : 2)
                                    : ((fabsf(norm.y) > fabsf(norm.z)) ? 1 : 2);

  // project vertices onto plane
  fvec2Array flat(vertex.getSize());
  switch (ignoreAxis) {
    case 0:
      for (i = 0; i < count; i++) {
	flat[i][0] = vertex[i][1];
	flat[i][1] = vertex[i][2];
      }
      break;
    case 1:
      for (i = 0; i < count; i++) {
	flat[i][0] = vertex[i][2];
	flat[i][1] = vertex[i][0];
      }
      break;
    case 2:
      for (i = 0; i < count; i++) {
	flat[i][0] = vertex[i][0];
	flat[i][1] = vertex[i][1];
      }
      break;
  }

  // compute area of polygon
  float* area = new float[1];
  area[0] = 0.0f;
  int j;
  for (j = count - 1, i = 0; i < count; j = i, i++) {
    area[0] += (flat[j][0] * flat[i][1]) - (flat[j][1] * flat[i][0]);
  }
  area[0] = 0.5f * fabsf(area[0]) / norm[ignoreAxis];
  node = new Geometry(this, vertex, uv, norm);
  shadowNode = new Geometry(this, vertex, uv, norm);
  shadowNode->setStyle(0);

  // set lod info
  setNumLODs(1, area);

  // compute bounding sphere, put center at average of vertices
  fvec4 mySphere(0.0f, 0.0f, 0.0f, 0.0f);
  for (i = 0; i < count; i++) {
    mySphere.xyz() += vertex[i];
  }
  mySphere.xyz() /= (float)count;

  for (i = 0; i < count; i++) {
    const float radSq = (mySphere.xyz() - vertex[i]).lengthSq();
    if (mySphere.w < radSq) {
      mySphere.w = radSq;
    }
  }

  setSphere(mySphere);
}

PolyWallSceneNode::~PolyWallSceneNode()
{
  delete node;
  delete shadowNode;
}

int			PolyWallSceneNode::split(const fvec4& _plane,
				SceneNode*& front, SceneNode*& back) const
{
  return WallSceneNode::splitWall(_plane, node->vertex, node->uv, front, back);
}

void			PolyWallSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  node->setStyle(getStyle());
  renderer.addRenderNode(node, getWallGState());
}

void			PolyWallSceneNode::addShadowNodes(
				SceneRenderer& renderer)
{
  renderer.addShadowNode(shadowNode);
}


void PolyWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
  RenderSet rs = { node, getWallGState() };
  rnodes.push_back(rs);
  return;
}


void PolyWallSceneNode::renderRadar()
{
  if (plane[2] > 0.0f) {
    node->renderRadar();
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
