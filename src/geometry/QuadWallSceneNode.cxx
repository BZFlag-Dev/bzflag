/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <math.h>
#include "QuadWallSceneNode.h"
#include "ViewFrustum.h"
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
  if (style >= 2) drawVT();
  else drawV();
}

void			QuadWallSceneNode::Geometry::drawV() const
{
  RENDER(EMITV)
}

void			QuadWallSceneNode::Geometry::drawVT() const
{
  RENDER(EMITVT)
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
				boolean makeLODs)
{
  init(base, uEdge, vEdge, uOffset, vOffset, uRepeats, vRepeats, makeLODs);
}

QuadWallSceneNode::QuadWallSceneNode(const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				float uRepeats,
				float vRepeats,
				boolean makeLODs)
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
				boolean makeLODs)
{
  // record plane and bounding sphere info
  GLfloat plane[4], sphere[4];
  plane[0] = uEdge[1] * vEdge[2] - uEdge[2] * vEdge[1];
  plane[1] = uEdge[2] * vEdge[0] - uEdge[0] * vEdge[2];
  plane[2] = uEdge[0] * vEdge[1] - uEdge[1] * vEdge[0];
  plane[3] = -(plane[0] * base[0] + plane[1] * base[1] + plane[2] * base[2]);
  setPlane(plane);
  sphere[0] = 0.5f * (uEdge[0] + vEdge[0]);
  sphere[1] = 0.5f * (uEdge[1] + vEdge[1]);
  sphere[2] = 0.5f * (uEdge[2] + vEdge[2]);
  sphere[3] = sphere[0]*sphere[0] + sphere[1]*sphere[1] + sphere[2]*sphere[2];
  sphere[0] += base[0];
  sphere[1] += base[1];
  sphere[2] += base[2];
  setSphere(sphere);

  // get length of sides
  const float uLength = sqrtf(float(uEdge[0] * uEdge[0] +
				uEdge[1] * uEdge[1] + uEdge[2] * uEdge[2]));
  const float vLength = sqrtf(float(vEdge[0] * vEdge[0] +
				vEdge[1] * vEdge[1] + vEdge[2] * vEdge[2]));
  float area = uLength * vLength;

  // compute how many LODs required to get smaller edge down to
  // elements no bigger than 4 units on a side.
  int uElements = int(uLength) / 2;
  int vElements = int(vLength) / 2;
  int uLevels = 1, vLevels = 1;
  while (uElements >>= 1) uLevels++;
  while (vElements >>= 1) vLevels++;
  int numLevels = (uLevels < vLevels ? uLevels : vLevels);

  // if overly rectangular then add levels to square it up
  boolean needsSquaring = False;
  if (makeLODs) {
    if (uLevels >= vLevels+2) {
      needsSquaring = True;
      numLevels += (uLevels - vLevels) / 2;
    }
    else if (vLevels >= uLevels+2) {
      needsSquaring = True;
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

int			QuadWallSceneNode::split(const float* plane,
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
  return WallSceneNode::splitWall(plane, vertex, uv, front, back);
}

void			QuadWallSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  const int lod = pickLevelOfDetail(renderer);
  nodes[lod]->setStyle(getStyle());
  renderer.addRenderNode(nodes[lod], &getGState());
}

void			QuadWallSceneNode::addShadowNodes(
				SceneRenderer& renderer)
{
  renderer.addShadowNode(shadowNode);
}
// ex: shiftwidth=2 tabstop=8
