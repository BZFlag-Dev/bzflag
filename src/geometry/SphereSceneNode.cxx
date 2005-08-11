/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// bzflag common header
#include "common.h"

// interface header
#include "SphereSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

//
// SphereSceneNode
//

const int		NumSlices = 2 * SphereRes;
const int		NumParts = SphereLowRes * SphereLowRes;

SphereSceneNode::SphereSceneNode(const GLfloat pos[3], GLfloat _radius) :
				transparent(false),
				lighting(false),
				renderNode(this),
				parts(NULL)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setCulling(GL_NONE);
  gstate = builder.getState();
  setColor(1.0f, 1.0f, 1.0f, 1.0f);

  // position sphere
  move(pos, _radius);
}

SphereSceneNode::~SphereSceneNode()
{
  if (parts) {
    for (int i = 0; i < NumParts; i++)
      delete parts[i];
    delete[] parts;
  }
}

void			SphereSceneNode::setColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  transparent = (color[3] != 1.0f);
}

void			SphereSceneNode::setColor(const GLfloat* rgba)
{
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
  transparent = (color[3] != 1.0f);
}

void			SphereSceneNode::move(const GLfloat pos[3],
							GLfloat _radius)
{
  radius = _radius;
  setCenter(pos);
  setRadius(radius * radius);
  if (parts) for (int i = 0; i < NumParts; i++) parts[i]->move();
}

SceneNode**		SphereSceneNode::getParts(int& numParts)
{
  if (!parts) {
    // make parts -- always use low detail sphere (if your zbuffer is
    // slow, then you probably don't want to render lots o' polygons)
    parts = new SphereFragmentSceneNode*[NumParts];
    for (int i = 0; i < SphereLowRes; i++)
      for (int j = 0; j < SphereLowRes; j++)
	parts[SphereLowRes * i + j] = new SphereFragmentSceneNode(j, i, this);
  }

  // choose number of parts to cut off bottom at around ground level
  int i;
  const GLfloat* mySphere = getSphere();
  for (i = 0; i < SphereLowRes; i++)
    if (radius * SphereRenderNode::lgeom[SphereLowRes*i][2]
	+ mySphere[2] < 0.01f)
      break;
  numParts = SphereLowRes * i;

  return (SceneNode**)parts;
}

void			SphereSceneNode::notifyStyleChange()
{
  lighting = BZDBCache::lighting;
  OpenGLGStateBuilder builder(gstate);
  if (BZDBCache::blend && transparent) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else if (transparent) {
    builder.resetBlending();
    builder.setStipple(0.5f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(1.0f);
  }
  gstate = builder.getState();
}

void			SphereSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  const GLfloat* mySphere = getSphere();
  const ViewFrustum& view = renderer.getViewFrustum();
  const float size = mySphere[3] * view.getAreaFactor() /
					getDistance(view.getEye());
  const int lod = (size < 100.0f) ? 0 : 1;

  renderNode.setHighResolution(lod != 0);

  if (BZDBCache::blend) {
    const GLfloat* eye = view.getEye();
    const float azimuth = atan2f(mySphere[1] - eye[1], eye[0] - mySphere[0]);
    const int numSlices = (lod == 1) ? NumSlices : SphereLowRes;
    renderNode.setBaseIndex(int(float(numSlices) *
				(1.0f + 0.5f * azimuth / M_PI)) % numSlices);
  }

  renderer.addRenderNode(&renderNode, &gstate);
}

void			SphereSceneNode::addShadowNodes(
                                SceneRenderer& /*renderer*/)
{
  return;
/*
  renderNode.setHighResolution(false);
  renderNode.setBaseIndex(0);
  renderer.addShadowNode(&renderNode);
*/
}

//
// SphereSceneNode::SphereRenderNode
//

GLfloat			SphereSceneNode::SphereRenderNode::
				geom[NumSlices * (SphereRes + 1)][3];
GLfloat			SphereSceneNode::SphereRenderNode::
				lgeom[SphereLowRes * (SphereLowRes + 1)][3];

SphereSceneNode::SphereRenderNode::SphereRenderNode(
				const SphereSceneNode* _sceneNode) :
				sceneNode(_sceneNode),
				highResolution(false),
				baseIndex(0)
{
  // initialize geometry if first instance
  static bool init = false;
  if (!init) {
    init = true;

    // high resolution sphere
    int i, j;
    for (i = 0; i <= SphereRes; i++) {
      const float phi = (const float)(M_PI * (0.5f - double(i) / SphereRes));
      for (j = 0; j < NumSlices; j++) {
	const float theta = (const float)(2.0 * M_PI * double(j) / NumSlices);
	geom[NumSlices * i + j][0] = cosf(theta) * cosf(phi);
	geom[NumSlices * i + j][1] = sinf(theta) * cosf(phi);
	geom[NumSlices * i + j][2] = sinf(phi);
      }
    }

    // low resolution sphere
    for (i = 0; i <= SphereLowRes; i++) {
      const float phi = (const float)(M_PI * (0.5 - double(i) / SphereLowRes));
      for (j = 0; j < SphereLowRes; j++) {
	const float theta = (const float)(2.0 * M_PI * double(j) / SphereLowRes);
	lgeom[SphereLowRes * i + j][0] = cosf(theta) * cosf(phi);
	lgeom[SphereLowRes * i + j][1] = sinf(theta) * cosf(phi);
	lgeom[SphereLowRes * i + j][2] = sinf(phi);
      }
    }
  }
}

SphereSceneNode::SphereRenderNode::~SphereRenderNode()
{
  // do nothing
}

void			SphereSceneNode::SphereRenderNode::
				setHighResolution(bool _highResolution)
{
  highResolution = _highResolution;
}

void			SphereSceneNode::SphereRenderNode::
				setBaseIndex(int _baseIndex)
{
  baseIndex = _baseIndex;
}

void			SphereSceneNode::SphereRenderNode::render()
{
  static const GLdouble groundPlane[] = { 0.0, 0.0, 1.0, 0.0 };

  int i, j;
  const GLfloat radius = sceneNode->radius;
  const GLfloat* sphere = sceneNode->getSphere();

  glClipPlane(GL_CLIP_PLANE0, groundPlane);
  glEnable(GL_CLIP_PLANE0);

  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glScalef(radius, radius, radius);

    myColor4fv(sceneNode->color);
    if (!BZDBCache::blend && sceneNode->transparent)
      myStipple(sceneNode->color[3]);
    if (sceneNode->lighting) {
      // draw with normals (normal is same as vertex!
      // one of the handy properties of a sphere.)
      if (highResolution) {
	for (i = 0; i < SphereRes; i++) {
	  glBegin(GL_QUAD_STRIP);
	  for (j = baseIndex; j < NumSlices; j++) {
	    glNormal3fv(geom[NumSlices * i + j]);
	    glVertex3fv(geom[NumSlices * i + j]);
	    glNormal3fv(geom[NumSlices * i + j + NumSlices]);
	    glVertex3fv(geom[NumSlices * i + j + NumSlices]);
	  }
	  for (j = 0; j <= baseIndex; j++) {
	    glNormal3fv(geom[NumSlices * i + j]);
	    glVertex3fv(geom[NumSlices * i + j]);
	    glNormal3fv(geom[NumSlices * i + j + NumSlices]);
	    glVertex3fv(geom[NumSlices * i + j + NumSlices]);
	  }
	  glEnd();
	}
      }
      else {
	for (i = 0; i < SphereLowRes; i++) {
	  glBegin(GL_QUAD_STRIP);
	  for (j = baseIndex; j < SphereLowRes; j++) {
	    glNormal3fv(lgeom[SphereLowRes * i + j]);
	    glVertex3fv(lgeom[SphereLowRes * i + j]);
	    glNormal3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
	    glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
	  }
	  for (j = 0; j <= baseIndex; j++) {
	    glNormal3fv(lgeom[SphereLowRes * i + j]);
	    glVertex3fv(lgeom[SphereLowRes * i + j]);
	    glNormal3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
	    glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
	  }
	  glEnd();
	}
      }
    }
    else {
      // draw without normals
      if (highResolution) {
	for (i = 0; i < SphereRes; i++) {
	  glBegin(GL_QUAD_STRIP);
	  for (j = baseIndex; j < NumSlices; j++) {
	    glVertex3fv(geom[NumSlices * i + j]);
	    glVertex3fv(geom[NumSlices * i + j + NumSlices]);
	  }
	  for (j = 0; j <= baseIndex; j++) {
	    glVertex3fv(geom[NumSlices * i + j]);
	    glVertex3fv(geom[NumSlices * i + j + NumSlices]);
	  }
	  glEnd();
	}
      }
      else {
	for (i = 0; i < SphereLowRes; i++) {
	  glBegin(GL_QUAD_STRIP);
	  for (j = baseIndex; j < SphereLowRes; j++) {
	    glVertex3fv(lgeom[SphereLowRes * i + j]);
	    glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
	  }
	  for (j = 0; j <= baseIndex; j++) {
	    glVertex3fv(lgeom[SphereLowRes * i + j]);
	    glVertex3fv(lgeom[SphereLowRes * i + j + SphereLowRes]);
	  }
	  glEnd();
	}
      }
    }

    if (!BZDBCache::blend && sceneNode->transparent)
      myStipple(0.5f);

  glPopMatrix();

  glDisable(GL_CLIP_PLANE0);
}

//
// SphereFragmentSceneNode
//

SphereFragmentSceneNode::SphereFragmentSceneNode(int _theta, int _phi,
					SphereSceneNode* _parentSphere) :
				parentSphere(_parentSphere),
				renderNode(_parentSphere, _theta, _phi)
{
  // position sphere fragment
  move();
}

SphereFragmentSceneNode::~SphereFragmentSceneNode()
{
  // do nothing
}

void			SphereFragmentSceneNode::move()
{
  const GLfloat* pSphere = parentSphere->getSphere();
  const GLfloat pRadius = parentSphere->getRadius();
  const GLfloat* vertex = renderNode.getVertex();
  setCenter(pSphere[0] + pRadius * vertex[0],
	    pSphere[1] + pRadius * vertex[1],
	    pSphere[2] + pRadius * vertex[2]);
  setRadius((GLfloat)(4.0 * M_PI * M_PI * pSphere[3]) /
			GLfloat(SphereLowRes * SphereLowRes));
}

void			SphereFragmentSceneNode::addRenderNodes
				(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &parentSphere->gstate);
}

void			SphereFragmentSceneNode::addShadowNodes(
				SceneRenderer& /*renderer*/)
{
  return;
/*
  renderer.addShadowNode(&renderNode);
*/
}

//
// SphereFragmentSceneNode::FragmentRenderNode
//

 SphereFragmentSceneNode::FragmentRenderNode::FragmentRenderNode(
				const SphereSceneNode* _sceneNode,
				int _theta, int _phi) :
				sceneNode(_sceneNode),
				theta(_theta),
				phi(_phi)
{
  // compute incremented theta and phi
  theta2 = (theta + 1) % SphereLowRes;
  phi2 = phi + 1;
}

SphereFragmentSceneNode::FragmentRenderNode::~FragmentRenderNode()
{
  // do nothing
}

const GLfloat*		SphereFragmentSceneNode::FragmentRenderNode::
				getVertex() const
{
  return SphereSceneNode::SphereRenderNode::lgeom[phi * SphereLowRes + theta];
}

const GLfloat*		SphereFragmentSceneNode::FragmentRenderNode::
				getPosition() const
{
  return sceneNode->getSphere();
}

void			SphereFragmentSceneNode::FragmentRenderNode::render()
{
  const GLfloat pRadius = sceneNode->getRadius();
  const GLfloat* pSphere = sceneNode->getSphere();

  glPushMatrix();
    glTranslatef(pSphere[0], pSphere[1], pSphere[2]);
    glScalef(pRadius, pRadius, pRadius);

    myColor4fv(sceneNode->color);
    if (!BZDBCache::blend && sceneNode->transparent)
      myStipple(sceneNode->color[3]);
    glBegin(GL_QUADS);
      if (sceneNode->lighting) {
	glNormal3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi + theta]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi + theta]);
	glNormal3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi2 + theta]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi2 + theta]);
	glNormal3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi2 + theta2]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi2 + theta2]);
	glNormal3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi + theta2]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi + theta2]);
      }
      else {
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi + theta]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi2 + theta]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi2 + theta2]);
	glVertex3fv(SphereSceneNode::SphereRenderNode::lgeom[SphereLowRes * phi + theta2]);
      }
    glEnd();

  glPopMatrix();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

