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

// system headers
#include <assert.h>
#include <math.h>

// local implementation headers
#include "TankSceneNode.h"
#include "TankGeometryMgr.h"

using namespace TankGeometryUtils;


static const int treadCount = 1;

static const float fullLength = 6.0f;

static const float treadHeight = 1.2f;
static const float treadInside = 0.875f;
static const float treadOutside = 1.4f;
static const float treadThickness = 0.15f;
static const float treadWidth = treadOutside - treadInside;
static const float treadRadius = 0.5f * treadHeight;
static const float treadYCenter = treadInside + (0.5f * treadWidth);
static const float treadLength = ((fullLength - treadHeight) * 2.0f) +
                                 (M_PI * treadHeight);
static const float treadTexCoordLen = (float)treadCount;

static const float wheelRadius = treadRadius - (0.7f * treadThickness);
static const float wheelWidth = treadWidth * 0.9f;
static const float wheelSpacing = (fullLength - treadHeight) / 3.0f;
static const float wheelTexCoordLen = 1.0f;

static const float casingWidth = treadWidth * 0.6f;

static const float wheelInsideTexRad = 0.4f;
static const float wheelOutsideTexRad = 0.5f;


float TankGeometryUtils::getWheelScale()
{
  // degrees / meter
  return 360.0f / (treadHeight * M_PI);
}

float TankGeometryUtils::getTreadScale()
{
  // texcoords / meter
  return treadTexCoordLen / treadLength;
}

float TankGeometryUtils::getTreadTexLen()
{
  // texcoords
  return treadTexCoordLen;
}


static void buildCasing(float Yoffset)
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
      doNormal3f(0.0f, -1.0f, 0.0f);
      doTexCoord2f(-tx, -ty);
      doVertex3f(-xc, yRight, zb);
      doTexCoord2f(+tx, -ty);
      doVertex3f(+xc, yRight, zb);
      doTexCoord2f(+tx, +ty);
      doVertex3f(+xc, yRight, zt);
      doTexCoord2f(-tx, +ty);
      doVertex3f(-xc, yRight, zt);
      // the left side
      doNormal3f(0.0f, +1.0f, 0.0f);
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
  const float divScale = 2.0f / divs;
  const float astep = (M_PI * 2.0f) / divs;
  const float yLeft = Yoffset + (0.5f * treadWidth);
  const float yRight = Yoffset - (0.5f * treadWidth);
  float x, z;
  float tx;
  // setup some basic texture coordinates
  const float txScale = treadTexCoordLen / treadLength;
  const float tx0 = 0.0f;
  const float tx1 = txScale * (treadRadius * M_PI);
  const float tx2 = txScale * ((treadRadius * M_PI) + (fullLength - treadHeight));
  const float tx3 = txScale * ((treadHeight * M_PI) + (fullLength - treadHeight));
  const float tx4 = treadTexCoordLen;
  const float tyScale = 1.0f / (2.0f * (treadWidth + treadThickness));
  const float ty0 = 0.0f;
  const float ty1 = tyScale * treadWidth;
  const float ty2 = tyScale * (treadWidth + treadThickness);
  const float ty3 = tyScale * ((2.0f * treadWidth) + treadThickness);
  const float ty4 = 1.0f;

  // the outside of the tread
  glBegin(GL_QUAD_STRIP);
  {  
    // first curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) - (M_PI / 2.0f);
      const float cos_val = cosf(ang);
      const float sin_val = sinf(ang);
      doNormal3f(cos_val, 0.0f, sin_val);
      tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
      doTexCoord2f(tx, ty1);
      x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
      z = (sin_val * treadRadius) + treadRadius;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx, ty0);
      doVertex3f(x, yLeft, z);
    }
    // top of the tread
    doNormal3f(0.0f, 0.0f, 1.0f);
    doTexCoord2f(tx2, ty1);
    x = -wheelSpacing * 1.5f;
    z = treadHeight;
    doVertex3f(x, yRight, z);
    doTexCoord2f(tx2, ty0);
    doVertex3f(x, yLeft, z);
    // second curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) + (M_PI / 2.0f);
      const float cos_val = cosf(ang);
      const float sin_val = sinf(ang);
      doNormal3f(cos_val, 0.0f, sin_val);
      tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
      doTexCoord2f(tx, ty1);
      x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
      z = (sin_val * treadRadius) + treadRadius;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx, ty0);
      doVertex3f(x, yLeft, z);
    }
    // bottom of the tread
    doNormal3f(0.0f, 0.0f, -1.0f);
    doTexCoord2f(tx4, ty1);
    x = wheelSpacing * 1.5f;
    z = 0.0f;
    doVertex3f(x, yRight, z);
    doTexCoord2f(tx4, ty0);
    doVertex3f(x, yLeft, z);
  }  
  glEnd();
  
  // the inside of the tread
  glBegin(GL_QUAD_STRIP);
  {  
    // first curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) - (M_PI / 2.0f);
      const float cos_val = cosf(ang);
      const float sin_val = sinf(ang);
      doNormal3f(-cos_val, 0.0f, -sin_val);
      tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
      doTexCoord2f(tx, ty3);
      x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
      z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
      doVertex3f(x, yLeft, z);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);
    }
    // top inside of the tread
    doNormal3f(0.0f, 0.0f, -1.0f);
    doTexCoord2f(tx2, ty3);
    x = -wheelSpacing * 1.5f;
    z = treadHeight - treadThickness;
    doVertex3f(x, yLeft, z);
    doTexCoord2f(tx2, ty2);
    doVertex3f(x, yRight, z);
    // second curve
    for (i = 0; i < ((divisions / 2) + 1); i++) {
      const float ang = (astep * (float)i) + (M_PI / 2.0f);
      const float cos_val = cosf(ang);
      const float sin_val = sinf(ang);
      doNormal3f(-cos_val, 0.0f, -sin_val);
      tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
      doTexCoord2f(tx, ty3);
      x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
      z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
      doVertex3f(x, yLeft, z);
      doTexCoord2f(tx, ty2);
      doVertex3f(x, yRight, z);
    }
    // bottom inside of the tread
    doNormal3f(0.0f, 0.0f, 1.0f);
    doTexCoord2f(tx4, ty3);
    x = wheelSpacing * 1.5f;
    z = treadThickness;
    doVertex3f(x, yLeft, z);
    doTexCoord2f(tx4, ty2);
    doVertex3f(x, yRight, z);
  }  
  glEnd();

  glShadeModel(GL_FLAT);
  {  
    // the right edge
    doNormal3f(0.0f, -1.0f, 0.0f);
    glBegin(GL_QUAD_STRIP);
    {  
      // first outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) - (M_PI / 2.0f);
        const float cos_val = cosf(ang);
        const float sin_val = sinf(ang);
        tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
        doTexCoord2f(tx, ty2);
        x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yRight, z);
        doTexCoord2f(tx, ty1);
        x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yRight, z);
      }
      // top edge
      doTexCoord2f(tx2, ty2);
      x = -wheelSpacing * 1.5f;
      z = treadHeight - treadThickness;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx2, ty1);
      z = treadHeight;
      doVertex3f(x, yRight, z);
      // second outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) + (M_PI / 2.0f);
        const float cos_val = cosf(ang);
        const float sin_val = sinf(ang);
        tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
        doTexCoord2f(tx, ty2);
        x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yRight, z);
        doTexCoord2f(tx, ty1);
        x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yRight, z);
      }
      // bottom edge
      doTexCoord2f(tx4, ty2);
      x = wheelSpacing * 1.5f;
      z = treadThickness;
      doVertex3f(x, yRight, z);
      doTexCoord2f(tx4, ty1);
      z = 0.0f;
      doVertex3f(x, yRight, z);
    }  
    glEnd();
    
    // the left edge
    doNormal3f(0.0f, +1.0f, 0.0f);
    glBegin(GL_QUAD_STRIP);
    {  
      // first outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) - (M_PI / 2.0f);
        const float cos_val = cosf(ang);
        const float sin_val = sinf(ang);
        tx = tx0 + ((tx1 - tx0) * ((float)i * divScale));
        doTexCoord2f(tx, ty4);
        x = (cos_val * treadRadius) + (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yLeft, z);
        doTexCoord2f(tx, ty3);
        x = (cos_val * (treadRadius - treadThickness)) + (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yLeft, z);
      }
      // top edge
      doTexCoord2f(tx2, ty4);
      x = -wheelSpacing * 1.5f;
      z = treadHeight;
      doVertex3f(x, yLeft, z);
      doTexCoord2f(tx2, ty3);
      z = treadHeight - treadThickness;
      doVertex3f(x, yLeft, z);
      // second outside curve
      for (i = 0; i < ((divisions / 2) + 1); i++) {
        const float ang = (astep * (float)i) + (M_PI / 2.0f);
        const float cos_val = cosf(ang);
        const float sin_val = sinf(ang);
        tx = tx2 + ((tx3 - tx2) * ((float)i * divScale));
        doTexCoord2f(tx, ty4);
        x = (cos_val * treadRadius) - (wheelSpacing * 1.5f);
        z = (sin_val * treadRadius) + treadRadius;
        doVertex3f(x, yLeft, z);
        doTexCoord2f(tx, ty3);
        x = (cos_val * (treadRadius - treadThickness)) - (wheelSpacing * 1.5f);
        z = (sin_val * (treadRadius - treadThickness)) + treadRadius;
        doVertex3f(x, yLeft, z);
      }
      // bottom edge
      doTexCoord2f(tx4, ty4);
      x = wheelSpacing * 1.5f;
      z = 0.0f;
      doVertex3f(x, yLeft, z);
      doTexCoord2f(tx4, ty3);
      z = treadThickness;
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
  doNormal3f(0.0f, +1.0f, 0.0f);
  glBegin(GL_QUAD_STRIP);
  {
    for (i = 0; i < (divisions + 1); i++) {
      const float ang = astep * (float)i;
      const float cos_val = cosf(ang);
      const float sin_val = sinf(ang);
      doNormal3f(cos_val, 0.0f, sin_val);
      tx = 0.5f + (cosf(angle + ang) * wheelInsideTexRad);
      ty = 0.5f + (sinf(angle + ang) * wheelInsideTexRad);
      doTexCoord2f(tx, ty);
      x = (cos_val * wheelRadius) + pos[0];
      z = (sin_val * wheelRadius) + pos[2];
      doVertex3f(x, yRight, z);
      tx = 0.5f + (cosf(angle + ang) * wheelOutsideTexRad);
      ty = 0.5f + (sinf(angle + ang) * wheelOutsideTexRad);
      doTexCoord2f(tx, ty);
      doVertex3f(x, yLeft, z);
    }
  }
  glEnd();

  glShadeModel(GL_FLAT);
  {
    // the left face
    doNormal3f(0.0f, +1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    {
      for (i = 0; i < divisions; i++) {
        const float ang = astep * (float)i;
        const float cos_val = cosf(-ang);
        const float sin_val = sinf(-ang);
        tx = 0.5f + (cosf(angle - ang) * wheelInsideTexRad);
        ty = 0.5f + (sinf(angle - ang) * wheelInsideTexRad);
        doTexCoord2f(tx, ty);
        x = (cos_val * wheelRadius) + pos[0];
        z = (sin_val * wheelRadius) + pos[2];
        doVertex3f(x, yLeft, z);
        
      }
    }
    glEnd();
    
    // the right face
    doNormal3f(0.0f, -1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    {
      for (i = 0; i < divisions; i++) {
        const float ang = astep * (float)i;
        const float cos_val = cosf(+ang);
        const float sin_val = sinf(+ang);
        tx = 0.5f + (cosf(angle + ang) * 0.4f);
        ty = 0.5f + (sinf(angle + ang) * 0.4f);
        doTexCoord2f(tx, ty);
        x = (cos_val * wheelRadius) + pos[0];
        z = (sin_val * wheelRadius) + pos[2];
        doVertex3f(x, yRight, z);
      }
    }
    glEnd();
  }
  glShadeModel(GL_SMOOTH);

  return;
}


void TankGeometryUtils::buildHighLCasing()
{
  buildCasing(+treadYCenter);
  return;
}

void TankGeometryUtils::buildHighRCasing()
{
  buildCasing(-treadYCenter);
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
