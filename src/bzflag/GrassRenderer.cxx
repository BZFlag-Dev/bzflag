/* bzflag
 * Copyright (c) 1993-2016 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// system headers
#include <algorithm>
#include <vector>

// interface header
#include "GrassRenderer.h"

// common impl headers
#include "TextureManager.h"
#include "StateDatabase.h"
#include "BZDBCache.h"
#include "TimeKeeper.h"
#include "TextUtils.h"
#include "ParseColor.h"
#include "Intersect.h"
#include "Extents.h"

bool sortDistance(GLfloat *a, GLfloat *b)
{
  GLfloat distA = std::max(fabs(a[0]), fabs(a[1]));
  GLfloat distB = std::max(fabs(b[0]), fabs(b[1]));

  return distA > distB;
}

GrassRenderer::GrassRenderer()
{
  grassHeight = 1.4f;
}

GrassRenderer::~GrassRenderer()
{
  freeContext(); // free the display lists
}


void GrassRenderer::init(void)
{
  OpenGLGStateBuilder gstate;
  TextureManager &tm = TextureManager::instance();
  
  // create grass material
  gstate.reset();
  gstate.setShading();
  gstate.setBlending();
  gstate.setAlphaFunc();
  gstate.enableTexture(1);
  gstate.setTexture(tm.getTextureID("3dgrass"));
  
  grassGState = gstate.getState();
  
  buildGrassList();
}

void GrassRenderer::draw(const SceneRenderer& sr)
{
  if (!BZDBCache::grass3D)
    return;

  const GLfloat *pos = sr.getViewFrustum().getEye();

  GLfloat i, j, x, y, bbyaw, alpha, burrowFade;

  GLfloat grassRadius = 120.0f;
  GLfloat grassRadScale = 1.0f / grassRadius;

  GLfloat grassDensity = (GLfloat) (11.0f - (BZDBCache::grass3D / 2.0f));

  GLfloat offX = floorf(pos[0] / grassDensity) * grassDensity;
  GLfloat offY = floorf(pos[1] / grassDensity) * grassDensity;

  std::vector<GLfloat*> grasses;

  int n = 0;
  for (i = -grassRadius; i <= grassRadius; i += grassDensity) {
    for (j = -grassRadius; j <= grassRadius; j += grassDensity) {
      x = i + fmod(((i + offX) + ((j + offY) * 0.4519f)) * 0.6316f, grassDensity);
      y = j + fmod(((i + offX) + ((j + offY) * 0.8742f)) * 0.8642f, grassDensity);

      GLfloat *gPos = (GLfloat*) malloc(sizeof(GLfloat) * 2);
      gPos[0] = x;
      gPos[1] = y;
      grasses.push_back(gPos);
    }
  }

  std::sort(grasses.begin(), grasses.end(), sortDistance);
  
  glEnable(GL_CULL_FACE);
  glMatrixMode(GL_MODELVIEW);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  glDepthMask(GL_FALSE);

  grassGState.setState();

  for(std::vector<GLfloat*>::iterator it = grasses.begin(); it != grasses.end(); ++it) {
    x = offX + (*it)[0];
    y = offY + (*it)[1];

    bbyaw = -atan2(x - pos[0], y - pos[1]) * (180.0f / M_PI);
      
    alpha = (1.0f - std::max(fabs(x - pos[0]),
			     fabs(y - pos[1])) * grassRadScale);

    burrowFade = std::max(0.0f, std::min(1.0f, pos[2]));

    glColor4f(1.0f, 1.0f, 1.0f, alpha * burrowFade);

    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(bbyaw, 0.0f, 0.0f, 1.0f);

    glCallList(grassList);

    glPopMatrix();    
  }

  glEnable(GL_CULL_FACE);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  glDepthMask(GL_TRUE);
}


void GrassRenderer::freeContext(void)
{
  if (grassList != INVALID_GL_LIST_ID) {
    glDeleteLists(grassList, 1);
    grassList = INVALID_GL_LIST_ID;
  }

  return;
}


void GrassRenderer::rebuildContext(void)
{
  buildGrassList();

  return;
}

void GrassRenderer::buildGrassList(bool _draw)
{
  if (!_draw) {
    if (grassList != INVALID_GL_LIST_ID) {
      glDeleteLists(grassList, 1);
      grassList = INVALID_GL_LIST_ID;
    }
    grassList = glGenLists(1);
    glNewList(grassList, GL_COMPILE);
  }

  glBegin(GL_TRIANGLE_FAN);

  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-5.0f, 0.0f, 0.0f);

  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(5.0f, 0.0f, 0.0f);

  glTexCoord2f(1.0f, 0.9f);
  glVertex3f(5.0f, 0.0f, grassHeight);

  glTexCoord2f(0.0f, 0.9f);
  glVertex3f(-5.0f, 0.0f, grassHeight);

  glEnd();

  if (!_draw) {
    glEndList();
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
