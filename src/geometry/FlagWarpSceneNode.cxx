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

#include <stdlib.h>
#include <math.h>
#include "FlagWarpSceneNode.h"
#include "SceneRenderer.h"
#include "ViewFrustum.h"

const float		FlagWarpSize =	7.5;		// meters
const GLfloat		FlagWarpAlpha = 0.5f;
const GLfloat		FlagWarpSceneNode::color[7][3] = {
				{ 0.25, 1.0, 0.25 },
				{ 0.25, 0.25, 1.0 },
				{ 1.0, 0.0, 1.0 },
				{ 1.0, 0.25, 0.25 },
				{ 1.0, 0.5, 0.0 },
				{ 1.0, 1.0, 0.0 },
				{ 1.0, 1.0, 1.0 }
			};

FlagWarpSceneNode::FlagWarpSceneNode(const GLfloat pos[3]) :
				renderNode(this)
{
  move(pos);
  setRadius(1.25f * FlagWarpSize * FlagWarpSize);
  size = 1.0f;
}

FlagWarpSceneNode::~FlagWarpSceneNode()
{
  // do nothing
}

void			FlagWarpSceneNode::setSizeFraction(GLfloat _size)
{
  size = _size;
}

void			FlagWarpSceneNode::move(const GLfloat pos[3])
{
  setCenter(pos);
}

GLfloat			FlagWarpSceneNode::getDistance(const GLfloat* eye) const
{
  // shift position of warp down a little because a flag and it's warp
  // are at the same position but we want the warp to appear below the
  // flag.
  const GLfloat* sphere = getSphere();
  return (eye[0] - sphere[0]) * (eye[0] - sphere[0]) +
	 (eye[1] - sphere[1]) * (eye[1] - sphere[1]) +
	 (eye[2] - sphere[2] + 0.2f) * (eye[2] - sphere[2] + 0.2f);
}

void			FlagWarpSceneNode::notifyStyleChange(
				const SceneRenderer& renderer)
{
  OpenGLGStateBuilder builder(gstate);
  if (renderer.useBlending()) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  }
  else {
    builder.resetBlending();
    builder.setStipple(FlagWarpAlpha);
  }
  gstate = builder.getState();
}

void			FlagWarpSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

//
// FlagWarpSceneNode::FlagWarpRenderNode
//

FlagWarpSceneNode::FlagWarpRenderNode::FlagWarpRenderNode(
				const FlagWarpSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  // do nothing
}

FlagWarpSceneNode::FlagWarpRenderNode::~FlagWarpRenderNode()
{
  // do nothing
}

void			FlagWarpSceneNode::FlagWarpRenderNode::render()
{
  // make a perturbed ring
  GLfloat geom[12][2];
  for (int i = 0; i < 12; i++) {
    const GLfloat r = FlagWarpSize * (0.9f + 0.2f * (float)bzfrand());
    geom[i][0] = r * cosf(2.0f * M_PI * float(i) / 12.0f);
    geom[i][1] = r * sinf(2.0f * M_PI * float(i) / 12.0f);
  }

  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);

    if (sphere[2] > SceneRenderer::getInstance()->getViewFrustum().getEye()[2]){
      for (int i = 0; i < 7; i++) {
	GLfloat s = sceneNode->size - 0.05f * float(i);
	if (s < 0.0f) break;
	myColor4f(color[i][0], color[i][1], color[i][2], FlagWarpAlpha);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(s * geom[0][0], s * geom[0][1]);
	glVertex2f(s * geom[11][0], s * geom[11][1]);
	glVertex2f(s * geom[10][0], s * geom[10][1]);
	glVertex2f(s * geom[9][0], s * geom[9][1]);
	glVertex2f(s * geom[8][0], s * geom[8][1]);
	glVertex2f(s * geom[7][0], s * geom[7][1]);
	glVertex2f(s * geom[6][0], s * geom[6][1]);
	glVertex2f(s * geom[5][0], s * geom[5][1]);
	glVertex2f(s * geom[4][0], s * geom[4][1]);
	glVertex2f(s * geom[3][0], s * geom[3][1]);
	glVertex2f(s * geom[2][0], s * geom[2][1]);
	glVertex2f(s * geom[1][0], s * geom[1][1]);
	glVertex2f(s * geom[0][0], s * geom[0][1]);
	glEnd();
	glTranslatef(0.0f, 0.0f, -0.01f);
      }
    }
    else {
      for (int i = 0; i < 7; i++) {
	GLfloat s = sceneNode->size - 0.05f * float(i);
	if (s < 0.0f) break;
	myColor4f(color[i][0], color[i][1], color[i][2], FlagWarpAlpha);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(s * geom[0][0], s * geom[0][1]);
	glVertex2f(s * geom[1][0], s * geom[1][1]);
	glVertex2f(s * geom[2][0], s * geom[2][1]);
	glVertex2f(s * geom[3][0], s * geom[3][1]);
	glVertex2f(s * geom[4][0], s * geom[4][1]);
	glVertex2f(s * geom[5][0], s * geom[5][1]);
	glVertex2f(s * geom[6][0], s * geom[6][1]);
	glVertex2f(s * geom[7][0], s * geom[7][1]);
	glVertex2f(s * geom[8][0], s * geom[8][1]);
	glVertex2f(s * geom[9][0], s * geom[9][1]);
	glVertex2f(s * geom[10][0], s * geom[10][1]);
	glVertex2f(s * geom[11][0], s * geom[11][1]);
	glVertex2f(s * geom[0][0], s * geom[0][1]);
	glEnd();
	glTranslatef(0.0f, 0.0f, 0.01f);
      }
    }

  glPopMatrix();
}
// ex: shiftwidth=2 tabstop=8
