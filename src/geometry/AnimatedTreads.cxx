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


#include <assert.h>
#include <math.h>
#include "TankSceneNode.h"
#include "TankGeometryMgr.h"

using namespace TankGeometryUtils;


static const int treadCount = 64;

static const float fullLength = 6.0f;

static const float treadHeight = 1.2f;
static const float treadInside = 0.875f;
static const float treadOutside = 1.4f;
static const float treadThickness = 0.15f;
static const float treadWidth = treadOutside - treadInside;
static const float treadRadius = 0.5f * treadHeight;
static const float treadYCenter = treadInside + (0.5f * treadWidth);
static const float treadLength = ((fullLength - (2.0f * treadRadius)) * 2.0f) +
                                 (M_PI * 2.0f * treadRadius);
static const float treadTexCoordLen = (float)treadCount;
static const float treadTexCoordScale = treadLength / treadTexCoordLen;

static const float wheelRadius = treadRadius - (0.7f * treadThickness);
static const float wheelWidth = treadWidth * 0.9f;
static const float wheelSpacing = (fullLength - treadHeight) / 3.0f;
static const float wheelTexCoordLen = 1.0f;

static const float casingWidth = treadWidth * 0.6f;

static const float wheelInsideTexRad = 0.4f;
static const float wheelOutsideTexRad = 0.5f;



static void buildCasing(float Yoffset, int /*divisions*/)
{
  const float yLeft = Yoffset + (0.5f * casingWidth);
  const float yRight = Yoffset - (0.5f * casingWidth);
  
  glShadeModel(GL_FLAT);
  {  
    const float xc = wheelSpacing * 1.5f;
    const float zb = treadThickness;
    const float zt = treadHeight - treadThickness;
    const float ty = 0.25f; // effective, the texture scale factor
    const float tx = (2.0f * ty) * (xc / (zt - zb));
    
    // the left and right quad surface
    glBegin(GL_QUADS);
    {
      // the right side
      doTexCoord2f(-tx, -ty);
      doVertex3f(-xc, yRight, zb);
      doTexCoord2f(+tx, -ty);
      doVertex3f(+xc, yRight, zb);
      doTexCoord2f(+tx, +ty);
      doVertex3f(+xc, yRight, zt);
      doTexCoord2f(-tx, +ty);
      doVertex3f(-xc, yRight, zt);
      // the left side
      doTexCoord2f(-tx, -ty);
      doVertex3f(+xc, yLeft, zb);
      doTexCoord2f(+tx, -ty);
      doVertex3f(-xc, yLeft, zb);
      doTexCoord2f(+tx, +ty);
      doVertex3f(-xc, yLeft, zt);
      doTexCoord2f(-tx, +ty);
      doVertex3f(+xc, yLeft, zt);
    }
    glEnd();
  }
  glShadeModel(GL_SMOOTH);
  
  return;
}


static void buildTread(float Yoffset, int divisions)
{
  int i;
  const float divs = (float)((divisions / 2) * 2); // even number
  const float astep = (M_PI * 2.0f) / divs;
  const float yLeft = Yoffset + (0.5f * treadWidth);
  const float yRight = Yoffset - (0.5f * treadWidth);
  float x, z;
  float nx, nz;
  float tx;//, ty;

  // the outside of the tread
  glBegin(GL_QUAD_STRIP);
  {  
    // first curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) - (M_PI / 2.0f);
      const float cos_val = cos(ang);
      const float sin_val = sin(ang);
      nx = cos_val;
      nz = sin_val;
      doNormal3f(nx, 0.0f, nz);
      tx = (float)i / divs;
      doTexCoord2f(tx, 1.0f);
      x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
      z = (sin_val * treadRadius) + treadRadius;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx, 0.0f);
      doVertex3f(x, yLeft, z);
    }
    // top of the tread
    doNormal3f(0.0f, 0.0f, 1.0f);
    doTexCoord2f(2.0f, 1.0f);
    x = -wheelSpacing * 1.5f;
    z = treadHeight;
    doVertex3f(x, yRight, z);
    doTexCoord2f(2.0f, 0.0f);
    doVertex3f(x, yLeft, z);
    // second curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) + (M_PI / 2.0f);
      const float cos_val = cos(ang);
      const float sin_val = sin(ang);
      nx = cos_val;
      nz = sin_val;
      doNormal3f(nx, 0.0f, nz);
      tx = (float)i / divs;
      doTexCoord2f(tx, 1.0f);
      x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
      z = (sin_val * treadRadius) + treadRadius;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx, 0.0f);
      doVertex3f(x, yLeft, z);
    }
    // bottom of the tread
    doNormal3f(0.0f, 0.0f, 1.0f);
    doTexCoord2f(2.0f, 1.0f);
    x = wheelSpacing * 1.5f;
    z = 0.0f;
    doVertex3f(x, yRight, z);
    doTexCoord2f(2.0f, 0.0f);
    doVertex3f(x, yLeft, z);
  }  
  glEnd();
  
  // the inside of the tread
  glBegin(GL_QUAD_STRIP);
  {  
    // first curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) - (M_PI / 2.0f);
      const float cos_val = cos(-ang);
      const float sin_val = sin(-ang);
      nx = cos_val;
      nz = sin_val;
      doNormal3f(nx, 0.0f, nz);
      tx = (float)i / divs;
      doTexCoord2f(tx, 1.0f);
      x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
      z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx, 0.0f);
      doVertex3f(x, yLeft, z);
    }
    // bottom of the tread
    doNormal3f(0.0f, 0.0f, 1.0f);
    doTexCoord2f(2.0f, 1.0f);
    x = -wheelSpacing * 1.5f;
    z = treadThickness;
    doVertex3f(x, yRight, z);
    doTexCoord2f(2.0f, 0.0f);
    doVertex3f(x, yLeft, z);
    // second curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) + (M_PI / 2.0f);
      const float cos_val = cos(-ang);
      const float sin_val = sin(-ang);
      nx = cos_val;
      nz = sin_val;
      doNormal3f(nx, 0.0f, nz);
      tx = (float)i / divs;
      doTexCoord2f(tx, 1.0f);
      x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
      z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx, 0.0f);
      doVertex3f(x, yLeft, z);
    }
    // bottom of the tread
    doNormal3f(0.0f, 0.0f, 1.0f);
    doTexCoord2f(2.0f, 1.0f);
    x = wheelSpacing * 1.5f;
    z = treadHeight - treadThickness;
    doVertex3f(x, yRight, z);
    doTexCoord2f(2.0f, 0.0f);
    doVertex3f(x, yLeft, z);
  }  
  glEnd();

  glShadeModel(GL_FLAT);
  {  
    // the right edge
    glBegin(GL_QUAD_STRIP);
    {  
      // first outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) - (M_PI / 2.0f);
        const float cos_val = cos(ang);
        const float sin_val = sin(ang);
        tx = (float)i / divs;
        doTexCoord2f(tx, 1.0f);
        x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yRight, z);
        doTexCoord2f(tx, 0.0f);
        x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yRight, z);
      }
      // top edge
      doTexCoord2f(2.0f, 1.0f);
      x = -wheelSpacing * 1.5f;
      z = treadHeight - treadThickness;
      doVertex3f(x, yRight, z);
      doTexCoord2f(2.0f, 0.0f);
      z = treadHeight;
      doVertex3f(x, yLeft, z);
      // second outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) + (M_PI / 2.0f);
        const float cos_val = cos(ang);
        const float sin_val = sin(ang);
        tx = (float)i / divs;
        doTexCoord2f(tx, 1.0f);
        x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yRight, z);
        doTexCoord2f(tx, 0.0f);
        x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yRight, z);
      }
      // bottom edge
      doTexCoord2f(2.0f, 1.0f);
      x = wheelSpacing * 1.5f;
      z = treadThickness;
      doVertex3f(x, yRight, z);
      doTexCoord2f(2.0f, 0.0f);
      z = 0.0f;
      doVertex3f(x, yRight, z);
    }  
    glEnd();
    
    // the left edge
    glBegin(GL_QUAD_STRIP);
    {  
      // first outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) - (M_PI / 2.0f);
        const float cos_val = cos(-ang);
        const float sin_val = sin(-ang);
        tx = (float)i / divs;
        doTexCoord2f(-tx, 1.0f);
        x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yLeft, z);
        doTexCoord2f(-tx, 0.0f);
        x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yLeft, z);
      }
      // bottom edge
      doTexCoord2f(-2.0f, 1.0f);
      x = -wheelSpacing * 1.5f;
      z = treadThickness;
      doVertex3f(x, yLeft, z);
      doTexCoord2f(-2.0f, 0.0f);
      z = 0.0f;
      doVertex3f(x, yLeft, z);
      // second outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) + (M_PI / 2.0f);
        const float cos_val = cos(-ang);
        const float sin_val = sin(-ang);
        tx = (float)i / divs;
        doTexCoord2f(-tx, 1.0f);
        x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yLeft, z);
        doTexCoord2f(-tx, 0.0f);
        x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yLeft, z);
      }
      // top edge
      doTexCoord2f(-2.0f, 1.0f);
      x = wheelSpacing * 1.5f;
      z = treadHeight - treadThickness;
      doVertex3f(x, yLeft, z);
      doTexCoord2f(-2.0f, 0.0f);
      z = treadHeight;
      doVertex3f(x, yLeft, z);
    }  
    glEnd();
  }
  glShadeModel(GL_SMOOTH);
  
  return;
}


static void buildWheel(const float pos[3], float angle, int divisions)
{
  int i;
  const float divs = (float)divisions;
  const float astep = (M_PI * 2.0f) / (float)divs;
  const float yLeft = pos[1] + (0.5f * wheelWidth);
  const float yRight = pos[1] - (0.5f * wheelWidth);
  float x, z;
  float tx, ty;

  // the edge loop
  glBegin(GL_QUAD_STRIP);
  doNormal3f(0.0f, +1.0f, 0.0f);
  for (i = 0; i < (divisions + 1); i++) {
    const float ang = astep * (float)i;
    const float cos_val = cos(ang);
    const float sin_val = sin(ang);
    doNormal3f(cos_val, 0.0f, sin_val);
    tx = 0.5f + (cos(angle + ang) * 0.4f);
    ty = 0.5f + (sin(angle + ang) * 0.4f);
    doTexCoord2f(tx, ty);
    x = (cos_val * wheelRadius) + pos[0];
    z = (sin_val * wheelRadius) + pos[2];
    doVertex3f(x, yRight, z);
    tx = 0.5f + (cos(angle + ang) * 0.5f);
    ty = 0.5f + (sin(angle + ang) * 0.5f);
    doTexCoord2f(tx, ty);
    doVertex3f(x, yLeft, z);
  }
  glEnd();

  glShadeModel(GL_FLAT);
  {
    // the left face
    glBegin(GL_TRIANGLE_FAN);
    for (i = 0; i < divisions; i++) {
      const float ang = astep * (float)i;
      const float cos_val = cos(-ang);
      const float sin_val = sin(-ang);
      tx = 0.5f + (cos(angle - ang) * 0.4f);
      ty = 0.5f + (sin(angle - ang) * 0.4f);
      doTexCoord2f(tx, ty);
      x = (cos_val * wheelRadius) + pos[0];
      z = (sin_val * wheelRadius) + pos[2];
      doVertex3f(x, yLeft, z);
      
    }
    glEnd();
    
    // the right face
    glBegin(GL_TRIANGLE_FAN);
    for (i = 0; i < divisions; i++) {
      const float ang = astep * (float)i;
      const float cos_val = cos(+ang);
      const float sin_val = sin(+ang);
      tx = 0.5f + (cos(angle + ang) * 0.4f);
      ty = 0.5f + (sin(angle + ang) * 0.4f);
      doTexCoord2f(tx, ty);
      x = (cos_val * wheelRadius) + pos[0];
      z = (sin_val * wheelRadius) + pos[2];
      doVertex3f(x, yRight, z);
    }
    glEnd();
  }
  glShadeModel(GL_SMOOTH);

  return;
}


void TankGeometryUtils::buildHighLCasing(int divs)
{
  buildCasing(+treadYCenter, divs);
  return;
}

void TankGeometryUtils::buildHighRCasing(int divs)
{
  buildCasing(-treadYCenter, divs);
  return;
}



void TankGeometryUtils::buildHighLTread(int divs)
{
  buildTread(+treadYCenter, divs);
  return;
}

void TankGeometryUtils::buildHighRTread(int divs)
{
  buildTread(-treadYCenter, divs);
  return;
}


void TankGeometryUtils::buildHighLWheel(int number, float angle, int divs)
{
  assert ((number >= 0) && (number < 4));
  float pos[3];
  pos[0] = wheelSpacing * (-1.5f + (float)number);
  pos[1] = +treadYCenter;
  pos[2] = treadRadius;
  buildWheel(pos, angle, divs);
  return;
}

void TankGeometryUtils::buildHighRWheel(int number, float angle, int divs)
{
  assert ((number >= 0) && (number < 4));
  float pos[3];
  pos[0] = wheelSpacing * (-1.5f + (float)number);
  pos[1] = -treadYCenter;
  pos[2] = treadRadius;
  buildWheel(pos, angle, divs);
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
