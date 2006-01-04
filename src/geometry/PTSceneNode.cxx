/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// BZFlag common header
#include "common.h"

// interface header
#include "PTSceneNode.h"

// system headers
#include <stdlib.h>
#include <math.h>

// common implementation headers
#include "StateDatabase.h"
#include "BZDBCache.h"

// local implementation headers
#include "ViewFrustum.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

const GLfloat		PhotonTorpedoSceneNode::CoreSize = 0.125f;
const GLfloat		PhotonTorpedoSceneNode::CoronaSize = 1.0f;
const GLfloat		PhotonTorpedoSceneNode::FlareSize = 1.0f;
const GLfloat		PhotonTorpedoSceneNode::FlareSpread = 0.08f;

PhotonTorpedoSceneNode::PhotonTorpedoSceneNode(const GLfloat pos[3]) :
				renderNode(this)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setShading(GL_SMOOTH);
  gstate = builder.getState();

  // prepare light
  light.setAttenuation(0, 0.05f);
  light.setAttenuation(1, 0.0f);
  light.setAttenuation(2, 0.05f);
  light.setColor(2.0f, 0.4f, 0.0f);

  // prepare geometry
  move(pos, NULL);
  setRadius(FlareSize * FlareSize);
}

PhotonTorpedoSceneNode::~PhotonTorpedoSceneNode()
{
  // do nothing
}

void			PhotonTorpedoSceneNode::move(const GLfloat pos[3],
							const GLfloat[3])
{
  setCenter(pos);
  light.setPosition(pos);
}

void			PhotonTorpedoSceneNode::addLight(
				SceneRenderer& renderer)
{
  renderer.addLight(light);
}

void			PhotonTorpedoSceneNode::notifyStyleChange()
{
  OpenGLGStateBuilder builder(gstate);
  if (BZDBCache::blend) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(0.5f);
  }
  gstate = builder.getState();
}

void			PhotonTorpedoSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// PhotonTorpedoSceneNode::PTRenderNode
//

GLfloat			PhotonTorpedoSceneNode::PTRenderNode::core[9][2];
GLfloat			PhotonTorpedoSceneNode::PTRenderNode::corona[8][2];
const GLfloat		PhotonTorpedoSceneNode::PTRenderNode::ring[8][2] = {
				{ 1.0f, 0.0f },
				{ (float)M_SQRT1_2, (float)M_SQRT1_2 },
				{ 0.0f, 1.0f },
				{ (float)-M_SQRT1_2, (float)M_SQRT1_2 },
				{ -1.0f, 0.0f },
				{ (float)-M_SQRT1_2, (float)-M_SQRT1_2 },
				{ 0.0f, -1.0f },
				{ (float)M_SQRT1_2, (float)-M_SQRT1_2 }
			};

PhotonTorpedoSceneNode::PTRenderNode::PTRenderNode(
				const PhotonTorpedoSceneNode* _sceneNode) :
				sceneNode(_sceneNode),
				numFlares(0)
{
  // initialize core and corona if not already done
  static bool init = false;
  if (!init) {
    init = true;
    core[0][0] = 0.0f;
    core[0][1] = 0.0f;
    for (int i = 0; i < 8; i++) {
      core[i+1][0] = CoreSize * ring[i][0];
      core[i+1][1] = CoreSize * ring[i][1];
      corona[i][0] = CoronaSize * ring[i][0];
      corona[i][1] = CoronaSize * ring[i][1];
    }
  }
}

PhotonTorpedoSceneNode::PTRenderNode::~PTRenderNode()
{
  // do nothing
}

void			PhotonTorpedoSceneNode::PTRenderNode::render()
{
  static const GLfloat mainColor[3]  = { 1.0f, 0.2f, 0.0f };
  static const GLfloat innerColor[3] = { 1.0f, 0.5f, 0.375f };
  static const GLfloat outerColor[4] = { 1.0f, 0.2f, 0.0f, 0.0f};
  static const GLfloat flareColor[4] = { 1.0f, 0.2f, 0.0f, 0.667f};
  static const GLfloat coronaColor[4] = { 1.0f, 0.2f, 0.0f, 0.333f};

  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    RENDERER.getViewFrustum().executeBillboard();

    if (!RENDERER.isSameFrame()) {
      numFlares = 3 + int(3.0f * (float)bzfrand());
      for (int i = 0; i < numFlares; i++) {
	theta[i] = (float)(2.0 * M_PI * bzfrand());
	phi[i] = (float)(bzfrand() - 0.5);
	phi[i] *= (float)(2.0 * M_PI * fabsf(phi[i]));
      }
    }

    // draw some flares
    myColor4fv(flareColor);
    if (!BZDBCache::blend) myStipple(flareColor[3]);
    glBegin(GL_QUADS);
    for (int i = 0; i < numFlares; i++) {
      // pick random direction in 3-space.  picking a random theta with
      // a uniform distribution is fine, but doing so with phi biases
      // the directions toward the poles.  my correction doesn't remove
      // the bias completely, but moves it towards the equator, which is
      // really where i want it anyway cos the flares are more noticeable
      // there.
      const GLfloat c = FlareSize * GLfloat(cosf(phi[i]));
      const GLfloat s = FlareSize * GLfloat(sinf(phi[i]));
      glVertex3fv(core[0]);
      glVertex3f(c * cosf(theta[i]-FlareSpread), c * sinf(theta[i]-FlareSpread), s);
      glVertex3f(2.0f * c * cosf(theta[i]), 2.0f * c * sinf(theta[i]), 2.0f * s);
      glVertex3f(c * cosf(theta[i]+FlareSpread), c * sinf(theta[i]+FlareSpread), s);
    }
    glEnd();

    if (BZDBCache::blend) {
      // draw corona
      glBegin(GL_QUAD_STRIP);
      myColor3fv(mainColor);
      glVertex2fv(core[1]);
      myColor4fv(outerColor);
      glVertex2fv(corona[0]);
      myColor3fv(mainColor);
      glVertex2fv(core[2]);
      myColor4fv(outerColor);
      glVertex2fv(corona[1]);
      myColor3fv(mainColor);
      glVertex2fv(core[3]);
      myColor4fv(outerColor);
      glVertex2fv(corona[2]);
      myColor3fv(mainColor);
      glVertex2fv(core[4]);
      myColor4fv(outerColor);
      glVertex2fv(corona[3]);
      myColor3fv(mainColor);
      glVertex2fv(core[5]);
      myColor4fv(outerColor);
      glVertex2fv(corona[4]);
      myColor3fv(mainColor);
      glVertex2fv(core[6]);
      myColor4fv(outerColor);
      glVertex2fv(corona[5]);
      myColor3fv(mainColor);
      glVertex2fv(core[7]);
      myColor4fv(outerColor);
      glVertex2fv(corona[6]);
      myColor3fv(mainColor);
      glVertex2fv(core[8]);
      myColor4fv(outerColor);
      glVertex2fv(corona[7]);
      myColor3fv(mainColor);
      glVertex2fv(core[1]);
      myColor4fv(outerColor);
      glVertex2fv(corona[0]);
      glEnd();

      // draw core
      glBegin(GL_TRIANGLE_FAN);
      myColor3fv(innerColor);
      glVertex2fv(core[0]);
      myColor3fv(mainColor);
      glVertex2fv(core[1]);
      glVertex2fv(core[2]);
      glVertex2fv(core[3]);
      glVertex2fv(core[4]);
      glVertex2fv(core[5]);
      glVertex2fv(core[6]);
      glVertex2fv(core[7]);
      glVertex2fv(core[8]);
      glVertex2fv(core[1]);
      glEnd();
    }
    else {
      // draw corona
      myColor3fv(coronaColor);
      myStipple(coronaColor[3]);
      glBegin(GL_QUAD_STRIP);
      glVertex2fv(core[1]);
      glVertex2fv(corona[0]);
      glVertex2fv(core[2]);
      glVertex2fv(corona[1]);
      glVertex2fv(core[3]);
      glVertex2fv(corona[2]);
      glVertex2fv(core[4]);
      glVertex2fv(corona[3]);
      glVertex2fv(core[5]);
      glVertex2fv(corona[4]);
      glVertex2fv(core[6]);
      glVertex2fv(corona[5]);
      glVertex2fv(core[7]);
      glVertex2fv(corona[6]);
      glVertex2fv(core[8]);
      glVertex2fv(corona[7]);
      glVertex2fv(core[1]);
      glVertex2fv(corona[0]);
      glEnd();

      // draw core
      myStipple(1.0f);
      glBegin(GL_TRIANGLE_FAN);
      myColor3fv(innerColor);
      glVertex2fv(core[0]);
      myColor3fv(mainColor);
      glVertex2fv(core[1]);
      glVertex2fv(core[2]);
      glVertex2fv(core[3]);
      glVertex2fv(core[4]);
      glVertex2fv(core[5]);
      glVertex2fv(core[6]);
      glVertex2fv(core[7]);
      glVertex2fv(core[8]);
      glVertex2fv(core[1]);
      glEnd();

      myStipple(0.5f);
    }

  glPopMatrix();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

