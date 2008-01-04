/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CrackedGlass.h"

/* common implementation headers */
#include "bzfgl.h"

/* local implementation headers */
#include "MainWindow.h"


float CrackedGlass::cracks[NUM_CRACKS][(1 << NUM_CRACKLEVELS) + 1][2];

CrackedGlass::CrackedGlass()
{
}

CrackedGlass::~CrackedGlass()
{
}


void
CrackedGlass::InitCracks(int maxMotionSize)
{
  for (int i = 0; i < NUM_CRACKS; i++) {
    const float d = 0.90f * float(maxMotionSize) * ((float)bzfrand() + 0.90f);
    const float a = (float)(2.0 * M_PI * (double(i) + bzfrand()) / double(NUM_CRACKS));
    CrackedGlass::cracks[i][0][0] = 0.0f;
    CrackedGlass::cracks[i][0][1] = 0.0f;
    CrackedGlass::cracks[i][1][0] = d * cosf(a);
    CrackedGlass::cracks[i][1][1] = d * sinf(a);
    MakeCrack(maxMotionSize, CrackedGlass::cracks, i, 1, a);
  }
}

void
CrackedGlass::MakeCrack(int maxMotionSize, float crackpattern[NUM_CRACKS][(1 << NUM_CRACKLEVELS) + 1][2], int n, int l, float a)
{
  if (l >= (1 << (NUM_CRACKLEVELS - 1))) {
    return;
  }
  float d = 0.5f * float(maxMotionSize) * ((float)bzfrand() + 0.5f) * powf(0.5f, 0.69f * logf(float(l)));
  float newAngle = (float)(a + M_PI * bzfrand() / double(NUM_CRACKS));
  crackpattern[n][2*l][0] = crackpattern[n][l][0] + d * cosf(newAngle);
  crackpattern[n][2*l][1] = crackpattern[n][l][1] + d * sinf(newAngle);
  MakeCrack(maxMotionSize, crackpattern, n, 2*l, newAngle);

  d = 0.5f * float(maxMotionSize) * ((float)bzfrand() + 0.5f) * powf(0.5f, 0.69f * logf(float(l)));
  newAngle = (float)(a - M_PI * bzfrand() / double(NUM_CRACKS));
  crackpattern[n][2*l+1][0] = crackpattern[n][l][0] + d * cosf(newAngle);
  crackpattern[n][2*l+1][1] = crackpattern[n][l][1] + d * sinf(newAngle);
  MakeCrack(maxMotionSize, crackpattern, n, 2*l+1, newAngle);
}


void
CrackedGlass::Render(SceneRenderer& renderer)
{
  if (renderer.useQuality() >= _EXPERIMENTAL_QUALITY ) {
    RenderHighResCracks(renderer);
  } else {
    RenderClassicCracks(renderer);
  }
}

void
CrackedGlass::RenderHighResCracks(SceneRenderer& renderer)
{
  MainWindow& window = renderer.getWindow();
  int maxLevels = (int) NUM_CRACKLEVELS;

  glEnable(GL_BLEND);
  glPushMatrix();
  glTranslatef(GLfloat(window.getWidth() >> 1), GLfloat(window.getViewHeight() >> 1), -0.02f);

  glLineWidth(5.0);
  glColor4f(1.0f, 1.0f, 1.0f,0.25f);
  for (int i = 0; i < NUM_CRACKS; i++) {
    glLineWidth(5.0);
    glBegin(GL_LINES);
    glVertex3f(CrackedGlass::cracks[i][0][0],CrackedGlass::cracks[i][0][1],0);
    glVertex3f(CrackedGlass::cracks[i][1][0],CrackedGlass::cracks[i][1][1],0);
    glEnd();

    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3f(CrackedGlass::cracks[i][0][0],CrackedGlass::cracks[i][0][1],0.01f);
    glVertex3f(CrackedGlass::cracks[i][1][0],CrackedGlass::cracks[i][1][1],0.01f);
    glEnd();

    glLineWidth(5.0);
    for (int j = 0; j < maxLevels-1; j++) {
      int num = 1 << j;
      for (int k = 0; k < num; k++) {
	glBegin(GL_LINES);
	glVertex2fv(CrackedGlass::cracks[i][num + k]);
	glVertex2fv(CrackedGlass::cracks[i][2 * (num + k)]);
	glVertex2fv(CrackedGlass::cracks[i][num + k]);
	glVertex2fv(CrackedGlass::cracks[i][2 * (num + k) + 1]);
	glEnd();
      }
    }
  }

  glLineWidth(1.0);
  glPopMatrix();
  glDisable(GL_BLEND);
}

void
CrackedGlass::RenderClassicCracks(SceneRenderer& renderer)
{
  MainWindow& window = renderer.getWindow();
  int maxLevels = (int)NUM_CRACKLEVELS;

  glPushMatrix();
  glTranslatef(GLfloat(window.getWidth() >> 1),
	       GLfloat(window.getViewHeight() >> 1), 0.0f);
  glLineWidth(3.0);
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_LINES);
  for (int i = 0; i < NUM_CRACKS; i++) {
    glVertex2fv(CrackedGlass::cracks[i][0]);
    glVertex2fv(CrackedGlass::cracks[i][1]);
    for (int j = 0; j < maxLevels-1; j++) {
      const int num = 1 << j;
      for (int k = 0; k < num; k++) {
	glVertex2fv(CrackedGlass::cracks[i][num + k]);
	glVertex2fv(CrackedGlass::cracks[i][2 * (num + k)]);
	glVertex2fv(CrackedGlass::cracks[i][num + k]);
	glVertex2fv(CrackedGlass::cracks[i][2 * (num + k) + 1]);
      }
    }
  }
  glEnd();
  glLineWidth(1.0);
  glPopMatrix();
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
