/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include "TracerSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "StateDatabase.h"

// local implemenation headers
#include "ViewFrustum.h"
#include "ShellSceneNode.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

#define	ShellRadius1_2	((float)(M_SQRT1_2 * ShellRadius))

const GLfloat		TracerSceneNode::TailLength = 10.0f;
const GLfloat		TracerSceneNode::tailVertex[9][3] = {
				{-0.5f * TailLength, 0.0f, 0.0f },
				{ 0.5f * TailLength, -ShellRadius, 0.0f },
				{ 0.5f * TailLength, -ShellRadius1_2, -ShellRadius1_2 },
				{ 0.5f * TailLength, 0.0f, -ShellRadius },
				{ 0.5f * TailLength, ShellRadius1_2, -ShellRadius1_2 },
				{ 0.5f * TailLength, ShellRadius, 0.0f },
				{ 0.5f * TailLength, ShellRadius1_2, ShellRadius1_2 },
				{ 0.5f * TailLength, 0.0f, ShellRadius },
				{ 0.5f * TailLength, -ShellRadius1_2, ShellRadius1_2 }
			};

TracerSceneNode::TracerSceneNode(const GLfloat pos[3],
				const GLfloat forward[3]) :
				renderNode(this)
{
  // prepare light
  light.setAttenuation(0, 0.0667f);
  light.setAttenuation(1, 0.0f);
  light.setAttenuation(2, 0.0667f);
  light.setColor(2.0f, 2.0f, 0.0f);

  // prepare geometry
  move(pos, forward);
  setRadius(0.25f * TailLength * TailLength);
}

TracerSceneNode::~TracerSceneNode()
{
  // do nothing
}

void			TracerSceneNode::move(const GLfloat pos[3],
						const GLfloat forward[3])
{
  const GLfloat d = 1.0f / sqrtf(forward[0] * forward[0] +
				forward[1] * forward[1] +
				forward[2] * forward[2]);
  azimuth = (GLfloat)(180.0 / M_PI * atan2f(forward[1], forward[0]));
  elevation = (GLfloat)(-180.0 / M_PI * atan2f(forward[2], hypotf(forward[0],forward[1])));
  setCenter(pos[0] - 0.5f * TailLength * d * forward[0],
	    pos[1] - 0.5f * TailLength * d * forward[1],
	    pos[2] - 0.5f * TailLength * d * forward[2]);
  light.setPosition(getSphere());
}

void			TracerSceneNode::addLight(SceneRenderer& renderer)
{
  renderer.addLight(light);
}

void			TracerSceneNode::notifyStyleChange()
{
  OpenGLGStateBuilder builder(gstate);
  if (BZDB.isTrue("blend")) {
    // add tail contribution instead of regular blend
    builder.setBlending(GL_SRC_ALPHA, GL_ONE);
    builder.setShading(GL_SMOOTH);
    builder.setStipple(1.0f);
    style = 1;
  }
  else {
    builder.resetBlending();
    builder.setShading(GL_FLAT);
    builder.setStipple(0.7f);
    style = 0;
  }
  gstate = builder.getState();
}

void			TracerSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// TracerSceneNode::TracerRenderNode
//

TracerSceneNode::TracerRenderNode::TracerRenderNode(
				const TracerSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  // do nothing
}

TracerSceneNode::TracerRenderNode::~TracerRenderNode()
{
  // do nothing
}

void			TracerSceneNode::TracerRenderNode::render()
{
  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);
      myColor4f(1.0f, 1.0f, 0.0f, 0.0f);
      glVertex3fv(tailVertex[0]);
      myColor4f(1.0f, 1.0f, 0.0f, 0.7f);
      glVertex3fv(tailVertex[8]);
      glVertex3fv(tailVertex[7]);
      glVertex3fv(tailVertex[6]);
      glVertex3fv(tailVertex[5]);
      glVertex3fv(tailVertex[4]);
      glVertex3fv(tailVertex[3]);
      glVertex3fv(tailVertex[2]);
      glVertex3fv(tailVertex[1]);
      glVertex3fv(tailVertex[8]);
    glEnd();

    if (sceneNode->style == 0) myStipple(1.0f);
    else glDisable(GL_BLEND);
    glBegin(GL_TRIANGLE_FAN);
      glVertex3fv(tailVertex[8]);
      glVertex3fv(tailVertex[7]);
      glVertex3fv(tailVertex[6]);
      glVertex3fv(tailVertex[5]);
      glVertex3fv(tailVertex[4]);
      glVertex3fv(tailVertex[3]);
      glVertex3fv(tailVertex[2]);
      glVertex3fv(tailVertex[1]);
    glEnd();
    if (sceneNode->style == 0) myStipple(0.7f);
    else glEnable(GL_BLEND);

  glPopMatrix();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

