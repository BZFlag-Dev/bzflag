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
#include "MeshFragSceneNode.h"

// system headers
#include <assert.h>
#include <math.h>
#include <string.h>

// common implementation headers
#include "Intersect.h"
#include "MeshFace.h"
#include "MeshSceneNodeGenerator.h"
#include "BzMaterial.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "SceneRenderer.h"


// FIXME - no tesselation is done on for shot lighting


//
// MeshFragSceneNode::Geometry
//

// NOTE: this should be based on visual pixel area
static int minLightDisabling = 100;

MeshFragSceneNode::Geometry::Geometry(MeshFragSceneNode* node)
{
  style = 0;
  sceneNode = node;
  list = INVALID_GL_LIST_ID;
  OpenGLGState::registerContextInitializer (freeContext, initContext, this);
  return;
}


MeshFragSceneNode::Geometry::~Geometry()
{
  OpenGLGState::unregisterContextInitializer (freeContext, initContext, this);
  return;
}


void MeshFragSceneNode::Geometry::init()
{
  initDisplayList();
  return;
}


void MeshFragSceneNode::Geometry::initDisplayList()
{
  if (list != INVALID_GL_LIST_ID) {
    glDeleteLists(list, 1);
  }
  list = INVALID_GL_LIST_ID;
  if (BZDB.isTrue("meshLists")) {
    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    drawVTN();
    glEndList();
  }
  return;
}


void MeshFragSceneNode::Geometry::freeDisplayList()
{
  if (list != INVALID_GL_LIST_ID) {
    glDeleteLists(list, 1);
  }
  list = INVALID_GL_LIST_ID;
  return;
}


void MeshFragSceneNode::Geometry::freeContext(void *data)
{
  ((MeshFragSceneNode::Geometry*)data)->freeDisplayList();
  return;
}


void MeshFragSceneNode::Geometry::initContext(void *data)
{
  ((MeshFragSceneNode::Geometry*)data)->initDisplayList();
  return;
}


inline void MeshFragSceneNode::Geometry::drawV() const
{
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, sceneNode->vertices);
  glDrawArrays(GL_TRIANGLES, 0, sceneNode->arrayCount * 3);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  return;
}


inline void MeshFragSceneNode::Geometry::drawVT() const
{
  glDisableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, sceneNode->vertices);
  glTexCoordPointer(2, GL_FLOAT, 0, sceneNode->texcoords);
  glDrawArrays(GL_TRIANGLES, 0, sceneNode->arrayCount * 3);

  glEnableClientState(GL_NORMAL_ARRAY);

  return;
}


inline void MeshFragSceneNode::Geometry::drawVN() const
{
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, sceneNode->vertices);
  glNormalPointer(GL_FLOAT, 0, sceneNode->normals);
  glDrawArrays(GL_TRIANGLES, 0, sceneNode->arrayCount * 3);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  return;
}


inline void MeshFragSceneNode::Geometry::drawVTN() const
{
  glVertexPointer(3, GL_FLOAT, 0, sceneNode->vertices);
  glNormalPointer(GL_FLOAT, 0, sceneNode->normals);
  glTexCoordPointer(2, GL_FLOAT, 0, sceneNode->texcoords);
  glDrawArrays(GL_TRIANGLES, 0, sceneNode->arrayCount * 3);

  return;
}


void MeshFragSceneNode::Geometry::render()
{
  const int triangles = sceneNode->arrayCount;
  const bool switchLights = (triangles >= minLightDisabling)
			    && BZDBCache::lighting;
  if (switchLights) {
    RENDERER.disableLights(sceneNode->extents.mins, sceneNode->extents.maxs);
  }

  // set the color
  sceneNode->setColor();

  if (list != INVALID_GL_LIST_ID) {
    glCallList(list);
  }
  else {
    if (BZDBCache::lighting) {
      if (BZDBCache::texture) {
	drawVTN();
      } else {
	drawVN();
      }
    } else {
      if (BZDBCache::texture) {
	drawVT();
      } else {
	drawV();
      }
    }
  }

  if (switchLights) {
    RENDERER.reenableLights();
  }

  addTriangleCount(triangles);

  return;
}


void MeshFragSceneNode::Geometry::renderRadar()
{
  const int triangles = sceneNode->arrayCount;
  if (list != INVALID_GL_LIST_ID) {
    glCallList(list);
  } else {
    glVertexPointer(3, GL_FLOAT, 0, sceneNode->vertices);
    glDrawArrays(GL_TRIANGLES, 0, triangles * 3);
  }
  addTriangleCount(triangles);
  return;
}


void MeshFragSceneNode::Geometry::renderShadow()
{
  const int triangles = sceneNode->arrayCount;
  if (list != INVALID_GL_LIST_ID) {
    glCallList(list);
  } else {
    glVertexPointer(3, GL_FLOAT, 0, sceneNode->vertices);
    glDrawArrays(GL_TRIANGLES, 0, triangles * 3);
  }
  addTriangleCount(triangles);
  return;
}


//
// MeshFragSceneNode
//

MeshFragSceneNode::MeshFragSceneNode(int _faceCount, const MeshFace** _faces)
				    : renderNode(this)
{
  int i, j, k;

  assert ((_faceCount > 0) && (_faces != NULL));

  // set the count
  faces = _faces;
  faceCount = _faceCount;

  // disable the plane
  noPlane = true;
  static const float fakePlane[4] = {0.0f, 0.0f, 1.0f, 0.0f};
  setPlane(fakePlane);

  const BzMaterial* bzmat = faces[0]->getMaterial();

  // disable radar and shadows if required
  noRadar = bzmat->getNoRadar();
  noShadow = bzmat->getNoShadow();

  // set lod info
  setNumLODs(0, NULL /* unused because LOD = 0 */);

  // record extents info
  for (i = 0; i < faceCount; i++) {
    const Extents& fExts = faces[i]->getExtents();
    extents.expandToBox(fExts);
  }

  // setup sphere
  float diffs[3];
  diffs[0] = extents.maxs[0] - extents.mins[0];
  diffs[1] = extents.maxs[1] - extents.mins[1];
  diffs[2] = extents.maxs[2] - extents.mins[2];
  float mySphere[4];
  mySphere[0] = 0.5f * (extents.maxs[0] + extents.mins[0]);
  mySphere[1] = 0.5f * (extents.maxs[1] + extents.mins[1]);
  mySphere[2] = 0.5f * (extents.maxs[2] + extents.mins[2]);
  mySphere[3] = 0.25f *
    ((diffs[0] * diffs[0]) + (diffs[1] * diffs[1]) + (diffs[2] * diffs[2]));
  setSphere(mySphere);

  // count the number of actual vertices
  arrayCount = 0;
  for (i = 0; i < faceCount; i++) {
    const MeshFace* face = faces[i];
    arrayCount = arrayCount + (face->getVertexCount() - 2);
  }

  // make the lists
  const int vertexCount = (arrayCount * 3);
  normals = new GLfloat[vertexCount * 3];
  vertices = new GLfloat[vertexCount * 3];
  texcoords = new GLfloat[vertexCount * 2];

  // fill in the lists
  int arrayIndex = 0;
  for (i = 0; i < faceCount; i++) {
    const MeshFace* face = faces[i];

    // pre-generate the texcoords if required
    GLfloat2Array t(face->getVertexCount());
    if (!face->useTexcoords()) {
      GLfloat3Array v(face->getVertexCount());
      for (j = 0; j < face->getVertexCount(); j++) {
	memcpy(v[j], face->getVertex(j), sizeof(float[3]));
      }
      MeshSceneNodeGenerator::makeTexcoords(face->getPlane(), v, t);
    }

    // number of triangles
    const int tcount = (face->getVertexCount() - 2);

    for (j = 0; j < tcount; j++) {
      for (k = 0; k < 3 ; k++) {
	const int aIndex = (arrayIndex + (j * 3) + k);
	int vIndex; // basically GL_TRIANGLE_FAN done the hard way
	if (k == 0) {
	  vIndex = 0;
	} else {
	  vIndex = (j + k) % face->getVertexCount();
	}

	// get the vertices
	memcpy(&vertices[aIndex * 3], face->getVertex(vIndex), sizeof(float[3]));

	// get the normals
	if (face->useNormals()) {
	  memcpy(&normals[aIndex * 3], face->getNormal(vIndex), sizeof(float[3]));
	} else {
	  memcpy(&normals[aIndex * 3], face->getPlane(), sizeof(float[3]));
	}

	// get the texcoords
	if (face->useTexcoords()) {
	  memcpy(&texcoords[aIndex * 2], face->getTexcoord(vIndex), sizeof(float[2]));
	} else {
	  memcpy(&texcoords[aIndex * 2], t[vIndex], sizeof(float[2]));
	}
      }
    }

    arrayIndex = arrayIndex + (3 * tcount);
  }

  assert(arrayIndex == (arrayCount * 3));

  renderNode.init(); // setup the display list

  return;
}


MeshFragSceneNode::~MeshFragSceneNode()
{
  delete[] faces;
  delete[] vertices;
  delete[] normals;
  delete[] texcoords;
  return;
}


bool MeshFragSceneNode::cull(const ViewFrustum& frustum) const
{
  // if the Visibility culler tells us that we're
  // fully visible, then skip the rest of these tests
  if (octreeState == OctreeVisible) {
    return false;
  }

  const Frustum* f = (const Frustum *) &frustum;
  if (testAxisBoxInFrustum(extents, f) == Outside) {
    return true;
  }

  // probably visible
  return false;
}


bool MeshFragSceneNode::inAxisBox (const Extents& exts) const
{
  // NOTE: it should be OK to use the faces while building

  float pos[3];
  pos[0] = 0.5f * (exts.maxs[0] + exts.mins[0]);
  pos[1] = 0.5f * (exts.maxs[1] + exts.mins[1]);
  pos[2] = exts.mins[2];
  float size[3];
  size[0] = 0.5f * (exts.maxs[0] - exts.mins[0]);
  size[1] = 0.5f * (exts.maxs[1] - exts.mins[1]);
  size[2] = (exts.maxs[2] - exts.mins[2]);

  for (int i = 0; i < faceCount; i++) {
    if (faces[i]->inBox(pos, 0.0f, size[0], size[1], size[2])) {
      return true;
    }
  }

  return false;
}


void MeshFragSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderNode.setStyle(getStyle());
  const GLfloat* dyncol = getDynamicColor();
  if ((dyncol == NULL) || (dyncol[3] != 0.0f)) {
    renderer.addRenderNode(&renderNode, getWallGState());
  }
  return;
}


void MeshFragSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  if (!noShadow) {
    const GLfloat* dyncol = getDynamicColor();
    if ((dyncol == NULL) || (dyncol[3] != 0.0f)) {
      renderer.addShadowNode(&renderNode);
    }
  }
  return;
}


void MeshFragSceneNode::renderRadar()
{
  if (!noRadar) {
    renderNode.renderRadar();
  }
  return;
}


void MeshFragSceneNode::getRenderNodes(std::vector<RenderSet>& rnodes)
{
  RenderSet rs = { &renderNode, getWallGState() };
  rnodes.push_back(rs);
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
