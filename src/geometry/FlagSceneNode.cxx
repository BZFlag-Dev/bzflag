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

/*
 *
 */

#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "FlagSceneNode.h"
#include "ViewFrustum.h"
#include "SceneRenderer.h"
#include "OpenGLTexture.h"
#include "StateDatabase.h"
#include "BZDBCache.h"

int	                flagChunks = 8;		// draw flag as 8 quads
bool                    geoPole = false;        // draw the pole as quads

const float		FlagSceneNode::RippleSpeed1 = 2.4f * M_PI;
const float		FlagSceneNode::RippleSpeed2 = 1.724f * M_PI;
const float		FlagSceneNode::DroopFactor = 0.0f;

static const GLfloat	Unit = 0.8f;		// meters
const GLfloat		FlagSceneNode::Width = 1.5f * Unit;
const GLfloat		FlagSceneNode::Height = Unit;

FlagSceneNode::FlagSceneNode(const GLfloat pos[3]) :
				billboard(true),
				angle(0.0f),
				transparent(false),
				texturing(false),
				renderNode(this)
{
  setColor(1.0f, 1.0f, 1.0f, 1.0f);
  setCenter(pos);
  setRadius(6.0f * Unit * Unit);
  ripple1 = 2.0f * M_PI * (float)bzfrand();
  ripple2 = 2.0f * M_PI * (float)bzfrand();
  geoPole = false;
}

FlagSceneNode::~FlagSceneNode()
{
  // do nothing
}

void			FlagSceneNode::waveFlag(float dt, float /*_droop*/)
{
  ripple1 += dt * RippleSpeed1;
  if (ripple1 >= 2.0f * M_PI) ripple1 -= 2.0f * M_PI;
  ripple2 += dt * RippleSpeed2;
  if (ripple2 >= 2.0f * M_PI) ripple2 -= 2.0f * M_PI;

  for (int i = 0; i <= flagChunks; i++) {
    const float x      = float(i) / float(flagChunks);
    const float damp   = 0.1f * x;
    const float angle1 = ripple1 - 4.0f * M_PI * x;

    wave0[i] = damp * sinf(angle1);
    wave1[i] = damp
      * (sinf(angle1 - 0.28f * M_PI) + sinf(ripple2 + 1.16f * M_PI));
    wave2[i] = wave0[i] + damp * sinf(ripple2);
  }
}

void			FlagSceneNode::move(const GLfloat pos[3])
{
  setCenter(pos);
}

void			FlagSceneNode::turn(GLfloat _angle)
{
  angle = _angle * 180.0f / M_PI;
}

void			FlagSceneNode::setBillboard(bool _billboard)
{
  if (billboard == _billboard) return;
  billboard = _billboard;
}

void			FlagSceneNode::setColor(
				GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  transparent = (color[3] != 1.0f);
}

void			FlagSceneNode::setColor(const GLfloat* rgba)
{
  color[0] = rgba[0];
  color[1] = rgba[1];
  color[2] = rgba[2];
  color[3] = rgba[3];
  transparent = (color[3] != 1.0f);
}

void			FlagSceneNode::setTexture(const int texture)
{
  OpenGLGStateBuilder builder(gstate);
  builder.setTexture(texture);
  builder.enableTexture(texture>=0);
  gstate = builder.getState();
}

void			FlagSceneNode::notifyStyleChange()
{
  texturing = BZDBCache::texture && BZDBCache::blend;
  OpenGLGStateBuilder builder(gstate);
  builder.enableTexture(texturing);
  if (BZDBCache::blend && (transparent || texturing)) {
    builder.setBlending(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    builder.setStipple(1.0f);
  } else if (transparent) {
    builder.resetBlending();
    builder.setStipple(0.5f);
  } else {
    builder.resetBlending();
    builder.setStipple(1.0f);
  }
  if (billboard) builder.setCulling(GL_BACK);
  else builder.setCulling(GL_NONE);
  gstate = builder.getState();

  flagChunks = (int)BZDB.eval("flagChunks");
  if (flagChunks >= maxChunks)
    flagChunks = maxChunks - 1;
  geoPole = (int)BZDB.eval("useQuality") > 2.0f;
}

void			FlagSceneNode::addRenderNodes(
				SceneRenderer& renderer)
{
  renderer.addRenderNode(&renderNode, &gstate);
}

void			FlagSceneNode::addShadowNodes(
				SceneRenderer& renderer)
{
  renderer.addShadowNode(&renderNode);
}

//
// FlagSceneNode::FlagRenderNode
//

FlagSceneNode::FlagRenderNode::FlagRenderNode(
				const FlagSceneNode* _sceneNode) :
				sceneNode(_sceneNode)
{
  // do nothing
}

FlagSceneNode::FlagRenderNode::~FlagRenderNode()
{
  // do nothing
}

void			FlagSceneNode::FlagRenderNode::render()
{
  float base = BZDBCache::flagPoleSize;
  float poleWidth = BZDBCache::flagPoleWidth;

  const GLfloat* sphere = sceneNode->getSphere();
  glPushMatrix();
    glTranslatef(sphere[0], sphere[1], sphere[2]);

    myColor4fv(sceneNode->color);

    if (!BZDBCache::blend &&
        (sceneNode->transparent || sceneNode->texturing))
      myStipple(sceneNode->color[3]);

    if (sceneNode->billboard) {
      RENDERER.getViewFrustum().executeBillboard();
      glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= flagChunks; i++) {
	  const float x = float(i) / float(flagChunks);
	  const float shift1 = sceneNode->wave0[i];
	  GLfloat v1[3], v2[3];
	  v1[0] = v2[0] = Width * x;
	  v1[1] = base + Height - shift1;
	  v2[1] = base - shift1;
	  v1[2] = sceneNode->wave1[i];
	  v2[2] = sceneNode->wave2[i];
	  glTexCoord2f(x, 1.0f);
	  glVertex3fv(v1);
	  glTexCoord2f(x, 0.0f);
	  glVertex3fv(v2);
	}
      glEnd();
    } else {
      glRotatef(sceneNode->angle + 180.0f, 0.0f, 0.0f, 1.0f);
      glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
      glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, base, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(Width, base, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(Width, base + Height, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0.0f, base + Height, 0.0f);
      glEnd();
    }

    myColor4f(0.0f, 0.0f, 0.0f, sceneNode->color[3]);
    if (sceneNode->texturing) glDisable(GL_TEXTURE_2D);

    if (geoPole) {
      glBegin(GL_QUADS);
        glVertex3f(-poleWidth, 0.0f, 0.0f);
        glVertex3f(poleWidth, 0.0f, 0.0f);
        glVertex3f(poleWidth, base + Height, 0.0f);
        glVertex3f(-poleWidth, base + Height, 0.0f);
      glEnd();
    } else {
      glBegin(GL_LINE_STRIP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, base + Height, 0.0f);
      glEnd();
    }
    if (sceneNode->texturing) glEnable(GL_TEXTURE_2D);

    if (!BZDBCache::blend && sceneNode->transparent)
      myStipple(0.5f);

  glPopMatrix();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

