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

// interface header
#include "QuadWallSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "Intersect.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

//
// QuadWallSceneNode::Geometry
//

QuadWallSceneNode::Geometry::Geometry(QuadWallSceneNode* _wall,
				int uCount, int vCount,
				const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				const GLfloat* _normal,
				float uOffset, float vOffset,
				float uRepeats, float vRepeats) :
				wall(_wall),
				style(0),
				ds(uCount),
				dt(vCount),
				dsq(uCount / 4),
				dsr(uCount % 4),
				normal(_normal),
				vertex((uCount+1) * (vCount+1)),
				uv((uCount+1) * (vCount+1))
{
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
  triangles = 2 * (uCount * vCount);
}

QuadWallSceneNode::Geometry::~Geometry()
{
  // do nothing
}

#define	RENDER(_e)							\
  for (int k = 0, t = 0; t < dt; t++) {					\
    glBegin(GL_TRIANGLE_STRIP);						\
    for (int s = 0; s < dsq; k += 4, s++) {				\
      _e(k+ds+1);							\
      _e(k);								\
      _e(k+ds+2);							\
      _e(k+1);								\
      _e(k+ds+3);							\
      _e(k+2);								\
      _e(k+ds+4);							\
      _e(k+3);								\
    }									\
    switch (dsr) {							\
      case 3:								\
	_e(k+ds+1);							\
	_e(k);								\
	k++;								\
	/* fall through */						\
      case 2:								\
	_e(k+ds+1);							\
	_e(k);								\
	k++;								\
	/* fall through */						\
      case 1:								\
	_e(k+ds+1);							\
	_e(k);								\
	k++;								\
	/* fall through */						\
      case 0:								\
	/* don't forget right edge of last quad on row */		\
	_e(k+ds+1);							\
	_e(k);								\
	k++;								\
    }									\
    glEnd();								\
  }
#define EMITV(_i)	glVertex3fv(vertex[_i])
#define EMITVT(_i)	glTexCoord2fv(uv[_i]); glVertex3fv(vertex[_i])

void			QuadWallSceneNode::Geometry::render()
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

void			QuadWallSceneNode::Geometry::renderShadow()
{
  int last = (ds + 1) * dt;
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3fv(vertex[last]);
  glVertex3fv(vertex[0]);
  glVertex3fv(vertex[last + ds]);
  glVertex3fv(vertex[ds]);
  glEnd();
  addTriangleCount(2);
}

void			QuadWallSceneNode::Geometry::drawV() const
{
  RENDER(EMITV)
}

void			QuadWallSceneNode::Geometry::drawVT() const
{
  RENDER(EMITVT)
}

const GLfloat*		QuadWallSceneNode::Geometry::getVertex(int i) const
{
  return vertex[i];
}

//
// QuadWallSceneNode
//

QuadWallSceneNode::QuadWallSceneNode(const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				float uOffset,
				float vOffset,
				float uRepeats,
				float vRepeats,
				bool makeLODs)
{
  init(base, uEdge, vEdge, uOffset, vOffset, uRepeats, vRepeats, makeLODs);
}

QuadWallSceneNode::QuadWallSceneNode(const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				float uRepeats,
				float vRepeats,
				bool makeLODs)
{
  init(base, uEdge, vEdge, 0.0f, 0.0f, uRepeats, vRepeats, makeLODs);
}

void			QuadWallSceneNode::init(const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				float uOffset,
				float vOffset,
				float uRepeats,
				float vRepeats,
				bool makeLODs)
{
  // record plane and bounding sphere info
  GLfloat myPlane[4], mySphere[4];
  myPlane[0] = uEdge[1] * vEdge[2] - uEdge[2] * vEdge[1];
  myPlane[1] = uEdge[2] * vEdge[0] - uEdge[0] * vEdge[2];
  myPlane[2] = uEdge[0] * vEdge[1] - uEdge[1] * vEdge[0];
  myPlane[3] = -(myPlane[0] * base[0] + myPlane[1] * base[1]
		 + myPlane[2] * base[2]);
  setPlane(myPlane);
  mySphere[0] = 0.5f * (uEdge[0] + vEdge[0]);
  mySphere[1] = 0.5f * (uEdge[1] + vEdge[1]);
  mySphere[2] = 0.5f * (uEdge[2] + vEdge[2]);
  mySphere[3] = mySphere[0]*mySphere[0] + mySphere[1]*mySphere[1]
    + mySphere[2]*mySphere[2];
  mySphere[0] += base[0];
  mySphere[1] += base[1];
  mySphere[2] += base[2];
  setSphere(mySphere);

  // get length of sides
  const float uLength = sqrtf(float(uEdge[0] * uEdge[0] +
				uEdge[1] * uEdge[1] + uEdge[2] * uEdge[2]));
  const float vLength = sqrtf(float(vEdge[0] * vEdge[0] +
				vEdge[1] * vEdge[1] + vEdge[2] * vEdge[2]));
  float area = uLength * vLength;

  // If negative then these values aren't a number of times to repeat
  // the texture along the surface but the width, or a desired scaled
  // width, of the texture itself. Repeat the texture as many times
  // as necessary to fit the surface.
  if (uRepeats < 0.0f)
  {
      uRepeats = - uLength / uRepeats;
  }

  if (vRepeats < 0.0f)
  {
      vRepeats = - vLength / vRepeats;
  }

  // compute how many LODs required to get smaller edge down to
  // elements no bigger than 4 units on a side.
  int uElements = int(uLength) / 2;
  int vElements = int(vLength) / 2;
  int uLevels = 1, vLevels = 1;
  while (uElements >>= 1) uLevels++;
  while (vElements >>= 1) vLevels++;
  int numLevels = (uLevels < vLevels ? uLevels : vLevels);

  // if overly rectangular then add levels to square it up
  bool needsSquaring = false;
  if (makeLODs) {
    if (uLevels >= vLevels+2) {
      needsSquaring = true;
      numLevels += (uLevels - vLevels) / 2;
    }
    else if (vLevels >= uLevels+2) {
      needsSquaring = true;
      numLevels += (vLevels - uLevels) / 2;
    }
  }

  // if no lod's required then don't make any except most coarse
  if (!makeLODs)
    numLevels = 1;

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
				getPlane(), uOffset, vOffset,
				uRepeats, vRepeats);
  shadowNode = new Geometry(this, uElements, vElements,
				base, uEdge, vEdge,
				getPlane(), uOffset, vOffset,
				uRepeats, vRepeats);
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
				getPlane(), uOffset, vOffset,
				uRepeats, vRepeats);

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
				getPlane(), uOffset, vOffset,
				uRepeats, vRepeats);

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
				getPlane(), uOffset, vOffset,
				uRepeats, vRepeats);
  }

  // record extents info
  for (int i = 0; i < 4; i++) {
    const float* point = getVertex(i);
    extents.expandToPoint(point);
  }

  // record LOD info
  setNumLODs(numLevels, areas);
}

QuadWallSceneNode::~QuadWallSceneNode()
{
  // free LODs
  const int numLevels = getNumLODs();
  for (int i = 0; i < numLevels; i++)
    delete nodes[i];
  delete[] nodes;
  delete shadowNode;
}

int			QuadWallSceneNode::split(const float *_plane,
				SceneNode*& front, SceneNode*& back) const
{
  // need to reorder vertices into counterclockwise order
  GLfloat3Array vertex(4);
  GLfloat2Array uv(4);
  for (int i = 0; i < 4; i++) {
    int j = i;
    if (j == 2 || j == 3) j = 5 - j;
    vertex[i][0] = nodes[0]->vertex[j][0];
    vertex[i][1] = nodes[0]->vertex[j][1];
    vertex[i][2] = nodes[0]->vertex[j][2];
    uv[i][0] = nodes[0]->uv[j][0];
    uv[i][1] = nodes[0]->uv[j][1];
  }
  return WallSceneNode::splitWall(_plane, vertex, uv, front, back);
}

void			QuadWallSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  const int lod = pickLevelOfDetail(renderer);
  nodes[lod]->setStyle(getStyle());
  renderer.addRenderNode(nodes[lod], getWallGState());
}

void			QuadWallSceneNode::addShadowNodes(
				SceneRenderer& renderer)
{
  renderer.addShadowNode(shadowNode);
}

bool		    QuadWallSceneNode::inAxisBox(const Extents& exts) const
{
  if (!extents.touches(exts)) {
    return false;
  }

  // NOTE: inefficient
  float vertices[4][3];
  memcpy (vertices[0], nodes[0]->getVertex(0), sizeof(float[3]));
  memcpy (vertices[1], nodes[0]->getVertex(1), sizeof(float[3]));
  memcpy (vertices[2], nodes[0]->getVertex(2), sizeof(float[3]));
  memcpy (vertices[3], nodes[0]->getVertex(3), sizeof(float[3]));

  return testPolygonInAxisBox (4, vertices, getPlane(), exts);
}

int		     QuadWallSceneNode::getVertexCount () const
{
  return 4;
}

const GLfloat*	  QuadWallSceneNode::getVertex (int vertex) const
{
  // re-map these to a counter-clockwise order
  const int order[4] = {0, 1, 3, 2};
  return nodes[0]->getVertex(order[vertex]);
}


void QuadWallSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
  RenderSet rs = { nodes[0], getWallGState() };
  rnodes.push_back(rs);
  return;
}


void QuadWallSceneNode::renderRadar()
{
  if (plane[2] > 0.0f) {
    nodes[0]->renderRadar();
  }
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

