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

#include <math.h>
#include <stdlib.h>
#include "common.h"
#include "TriWallSceneNode.h"
#include "SceneRenderer.h"
#include "Intersect.h"

//
// TriWallSceneNode::Geometry
//

TriWallSceneNode::Geometry::Geometry(TriWallSceneNode* _wall, int eCount,
  const GLfloat base[3], const GLfloat uEdge[3], const GLfloat vEdge[3],
  const GLfloat* _normal, float uRepeats, float vRepeats) :
    wall(_wall), style(0), de(eCount), normal(_normal),
    vertex((eCount+1) * (eCount+2) / 2),
    uv((eCount+1) * (eCount+2) / 2)
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
}


TriWallSceneNode::Geometry::~Geometry()
{
  // do nothing
}


#define	RENDER(_e)							\
  for (int k = 0, t = 0; t < de; t++) {					\
    int e = de - t;							\
    glBegin(GL_TRIANGLE_STRIP);						\
    for (int s = 0; s < e; k++, s++) {					\
      _e(k+e+1);							\
      _e(k);								\
    }									\
    _e(k);								\
    glEnd();								\
    k++;								\
  }
#define EMITV(_i)	glVertex3fv(vertex[_i])
#define EMITVT(_i)	glTexCoord2fv(uv[_i]); glVertex3fv(vertex[_i])

void			TriWallSceneNode::Geometry::render()
{
  wall->setColor();
  glNormal3fv(normal);
  if (style >= 2) drawVT();
  else drawV();
}


void			TriWallSceneNode::Geometry::drawV() const
{
  RENDER(EMITV)
}


void			TriWallSceneNode::Geometry::drawVT() const
{
  RENDER(EMITVT)
}


const GLfloat*		TriWallSceneNode::Geometry::getVertex(int i) const
{
  return vertex[i];
}


//
// TriWallSceneNode
//

TriWallSceneNode::TriWallSceneNode(const GLfloat base[3],
				const GLfloat uEdge[3],
				const GLfloat vEdge[3],
				float uRepeats,
				float vRepeats,
				bool makeLODs)
{
  // record plane info
  GLfloat plane[4], sphere[4];
  plane[0] = uEdge[1] * vEdge[2] - uEdge[2] * vEdge[1];
  plane[1] = uEdge[2] * vEdge[0] - uEdge[0] * vEdge[2];
  plane[2] = uEdge[0] * vEdge[1] - uEdge[1] * vEdge[0];
  plane[3] = -(plane[0] * base[0] + plane[1] * base[1] + plane[2] * base[2]);
  setPlane(plane);

  // record bounding sphere info -- ought to calculate center and
  // and radius of circumscribing sphere but it's late and i'm tired.
  // i'll just calculate something easy.  it hardly matters as it's
  // hard to tightly bound a triangle with a sphere.
  sphere[0] = 0.5f * (uEdge[0] + vEdge[0]);
  sphere[1] = 0.5f * (uEdge[1] + vEdge[1]);
  sphere[2] = 0.5f * (uEdge[2] + vEdge[2]);
  sphere[3] = sphere[0]*sphere[0] + sphere[1]*sphere[1] + sphere[2]*sphere[2];
  sphere[0] += base[0];
  sphere[1] += base[1];
  sphere[2] += base[2];
  setSphere(sphere);

  // get length of sides
  const float uLength = sqrtf(uEdge[0] * uEdge[0] +
				uEdge[1] * uEdge[1] + uEdge[2] * uEdge[2]);
  const float vLength = sqrtf(vEdge[0] * vEdge[0] +
				vEdge[1] * vEdge[1] + vEdge[2] * vEdge[2]);
  float area = 0.5f * uLength * vLength;

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
				getPlane(), uRepeats, vRepeats);
  shadowNode = new Geometry(this, uElements, base, uEdge, vEdge,
				getPlane(), uRepeats, vRepeats);
  shadowNode->setStyle(0);

  // make remaining levels by doubling elements in each dimension
  while (level < numLevels) {
    uElements <<= 1;
    area *= 0.25f;
    areas[level] = area;
    nodes[level++] = new Geometry(this, uElements, base, uEdge, vEdge,
				getPlane(), uRepeats, vRepeats);
  }

  // record extents info
  int i, j;
  for (i = 0; i < 3; i++) {
    mins[i] = +MAXFLOAT;
    maxs[i] = -MAXFLOAT;
  }
  for (i = 0; i < 3; i++) {
    const float* point = getVertex(i);
    for (j = 0; j < 3; j++) {
      if (point[j] < mins[j]) {
        mins[j] = point[j];
      }
      if (point[j] > maxs[j]) {
        maxs[j] = point[j];
      }
    }
  }

  // record LOD info
  setNumLODs(numLevels, areas);
}


TriWallSceneNode::~TriWallSceneNode()
{
  // free LODs
  const int numLevels = getNumLODs();
  for (int i = 0; i < numLevels; i++)
    delete nodes[i];
  delete[] nodes;
  delete shadowNode;
}


bool			TriWallSceneNode::cull(const ViewFrustum& frustum) const
{
  // cull if eye is behind (or on) plane
  const GLfloat* eye = frustum.getEye();
  const GLfloat* plane = getPlane();
  if (eye[0]*plane[0] + eye[1]*plane[1] + eye[2]*plane[2] + plane[3] <= 0.0f) {
    return true;
  }

  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  const Frustum* f = (const Frustum *) &frustum;
  if (testAxisBoxInFrustum(mins, maxs, f) == Outside) {
    return true;
  }

  // probably visible
  return false;
}


int			TriWallSceneNode::split(const float* plane,
				SceneNode*& front, SceneNode*& back) const
{
  return WallSceneNode::splitWall(plane,
				nodes[0]->vertex, nodes[0]->uv, front, back);
}


void			TriWallSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  const int lod = pickLevelOfDetail(renderer);
  nodes[lod]->setStyle(getStyle());
  renderer.addRenderNode(nodes[lod], &getGState());
}


void			TriWallSceneNode::addShadowNodes(
				SceneRenderer& renderer)
{
  renderer.addShadowNode(shadowNode);
}


void                    TriWallSceneNode::getExtents(float* _mins, float* _maxs) const
{
  memcpy (_mins, mins, 3 * sizeof(float));
  memcpy (_maxs, maxs, 3 * sizeof(float));
  return;
}


bool                    TriWallSceneNode::inAxisBox(const float* boxMins,
                                                    const float* boxMaxs) const
{
  if ((mins[0] > boxMaxs[0]) || (maxs[0] < boxMins[0]) ||
      (mins[1] > boxMaxs[1]) || (maxs[1] < boxMins[1]) ||
      (mins[2] > boxMaxs[2]) || (maxs[2] < boxMins[2])) {
    return false;
  }
  
  // NOTE: inefficient
  float vertices[3][3];
  memcpy (vertices[0], nodes[0]->getVertex(0), sizeof(float[3]));
  memcpy (vertices[1], nodes[0]->getVertex(1), sizeof(float[3]));
  memcpy (vertices[2], nodes[0]->getVertex(2), sizeof(float[3]));
  
  return testPolygonInAxisBox (3, vertices, getPlane(), boxMins, boxMaxs);
}


int                     TriWallSceneNode::getVertexCount () const
{
  return 3;
}


const GLfloat*          TriWallSceneNode::getVertex (int vertex) const
{
  return nodes[0]->getVertex(vertex);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

