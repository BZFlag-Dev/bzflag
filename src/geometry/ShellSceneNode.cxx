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
#include "ShellSceneNode.h"

// system headers
#include <math.h>

// common implementation headers
#include "OpenGLMaterial.h"
#include "StateDatabase.h"

// FIXME (SceneRenderer.cxx is in src/bzflag)
#include "SceneRenderer.h"

#define	ShellRadius1_2	((GLfloat)(M_SQRT1_2 * ShellRadius))

const GLfloat		ShellSceneNode::shellVertex[9][3] = {
				{ 3.0f * ShellRadius, 0.0f, 0.0f },
				{ 0.0f, -ShellRadius, 0.0f },
				{ 0.0f, -ShellRadius1_2, -ShellRadius1_2 },
				{ 0.0f, 0.0f, -ShellRadius },
				{ 0.0f, ShellRadius1_2, -ShellRadius1_2 },
				{ 0.0f, ShellRadius, 0.0f },
				{ 0.0f, ShellRadius1_2, ShellRadius1_2 },
				{ 0.0f, 0.0f, ShellRadius },
				{ 0.0f, -ShellRadius1_2, ShellRadius1_2 }
			};
const GLfloat		ShellSceneNode::shellNormal[10][3] = {
				{ 1.0f, 0.0f, 0.0f },
				{ 0.0f, -1.0f, 0.0f },
				{ 0.0f, (float)-M_SQRT1_2, (float)-M_SQRT1_2 },
				{ 0.0f, 0.0f, -1.0f },
				{ 0.0f, (float)M_SQRT1_2, (float)-M_SQRT1_2 },
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, (float)M_SQRT1_2, (float)M_SQRT1_2 },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, (float)-M_SQRT1_2, (float)M_SQRT1_2 },
				{-1.0f, 0.0f, 0.0f }
			};

ShellSceneNode::ShellSceneNode(const GLfloat pos[3],
				const GLfloat forward[3]) :
				renderNode(this)
{
  static const GLfloat specular[3] = { 1.0f, 1.0f, 1.0f };
  static const GLfloat emissive[3] = { 0.0f, 0.0f, 0.0f };

  move(pos, forward);
  setRadius(9.0f * ShellRadius * ShellRadius);

  OpenGLGStateBuilder builder(gstate);
  builder.setMaterial(OpenGLMaterial(specular, emissive, 20.0f));
  gstate = builder.getState();
}

ShellSceneNode::~ShellSceneNode()
{
  // do nothing
}

void			ShellSceneNode::move(const GLfloat pos[3],
						const GLfloat forward[3])
{
  setCenter(pos);
  azimuth = (float)(180.0 / M_PI*atan2f(forward[1], forward[0]));
  elevation = (float)(-180.0 / M_PI*atan2f(forward[2], hypotf(forward[0],forward[1])));
}

void			ShellSceneNode::notifyStyleChange()
{
  OpenGLGStateBuilder builder(gstate);
  const bool lighting = BZDB.isTrue("lighting");
  builder.enableMaterial(lighting);
  builder.setShading(lighting ? GL_SMOOTH : GL_FLAT);
  renderNode.setLighting(lighting);
  gstate = builder.getState();
}

void			ShellSceneNode::addRenderNodes(SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

void			ShellSceneNode::addShadowNodes(SceneRenderer& renderer)
{
  renderer.addShadowNode(&renderNode);
}

//
// ShellSceneNode::ShellRenderNode
//

ShellSceneNode::ShellRenderNode::ShellRenderNode(
				const ShellSceneNode* _sceneNode) :
				sceneNode(_sceneNode),
				lighted(false)
{
  // do nothing
}

ShellSceneNode::ShellRenderNode::~ShellRenderNode()
{
  // do nothing
}

void			ShellSceneNode::ShellRenderNode::
				setLighting(bool _lighted)
{
  lighted = _lighted;
}

void			ShellSceneNode::ShellRenderNode::render()
{
  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);
    glRotatef(sceneNode->azimuth, 0.0f, 0.0f, 1.0f);
    glRotatef(sceneNode->elevation, 0.0f, 1.0f, 0.0f);

    myColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_TRIANGLE_FAN);
    if (lighted) {
      glNormal3fv(shellNormal[0]);
      glVertex3fv(shellVertex[0]);
      glNormal3fv(shellNormal[1]);
      glVertex3fv(shellVertex[1]);
      glNormal3fv(shellNormal[2]);
      glVertex3fv(shellVertex[2]);
      glNormal3fv(shellNormal[3]);
      glVertex3fv(shellVertex[3]);
      glNormal3fv(shellNormal[4]);
      glVertex3fv(shellVertex[4]);
      glNormal3fv(shellNormal[5]);
      glVertex3fv(shellVertex[5]);
      glNormal3fv(shellNormal[6]);
      glVertex3fv(shellVertex[6]);
      glNormal3fv(shellNormal[7]);
      glVertex3fv(shellVertex[7]);
      glNormal3fv(shellNormal[8]);
      glVertex3fv(shellVertex[8]);
      glNormal3fv(shellNormal[1]);
      glVertex3fv(shellVertex[1]);
    }
    else {
      glVertex3fv(shellVertex[0]);
      glVertex3fv(shellVertex[1]);
      glVertex3fv(shellVertex[2]);
      glVertex3fv(shellVertex[3]);
      glVertex3fv(shellVertex[4]);
      glVertex3fv(shellVertex[5]);
      glVertex3fv(shellVertex[6]);
      glVertex3fv(shellVertex[7]);
      glVertex3fv(shellVertex[8]);
      glVertex3fv(shellVertex[1]);
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
