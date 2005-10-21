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

#include "common.h"
// system headers
#include <assert.h>
#include <math.h>

// local implementation headers
#include "TankSceneNode.h"
#include "TankGeometryMgr.h"

using namespace TankGeometryUtils;


static int treadStyle = TankGeometryUtils::Covered;

// setup in setTreadStyle()
static int treadCount;
static float fullLength;
static float treadHeight;
static float treadInside;
static float treadOutside;
static float treadThickness;
static float treadWidth;
static float treadRadius;
static float treadYCenter;
static float treadLength;
static float treadTexCoordLen;
static float wheelRadius;
static float wheelWidth;
static float wheelSpacing;
static float wheelTexCoordLen;
static float casingWidth;
static float wheelInsideTexRad;
static float wheelOutsideTexRad;


void TankGeometryUtils::setTreadStyle(int style)
{
  if (style == TankGeometryUtils::Exposed) {
    fullLength = 6.0f;
    treadHeight = 1.2f;
    treadInside = 0.875f;
    treadOutside = 1.4f;
    treadStyle = TankGeometryUtils::Exposed;
  } else {
    fullLength = 5.4f;
    treadHeight = 1.1f;
    treadInside = 0.877f;
    treadOutside = 1.38f;
    treadStyle = TankGeometryUtils::Covered;
  }

  treadCount = 1;

  treadThickness = 0.15f;
  treadWidth = treadOutside - treadInside;
  treadRadius = 0.5f * treadHeight;
  treadYCenter = treadInside + (0.5f * treadWidth);
  treadLength = (float)(((fullLength - treadHeight) * 2.0) +
				   (M_PI * treadHeight));
  treadTexCoordLen = (float)treadCount;

  wheelRadius = treadRadius - (0.7f * treadThickness);
  wheelWidth = treadWidth * 0.9f;
  wheelSpacing = (fullLength - treadHeight) / 3.0f;
  wheelTexCoordLen = 1.0f;

  casingWidth = treadWidth * 0.6f;

  wheelInsideTexRad = 0.4f;
  wheelOutsideTexRad = 0.5f;

  return;
}


float TankGeometryUtils::getWheelScale()
{
  // degrees / meter
  return (float)(360.0 / (treadHeight * M_PI));
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


static int buildCasing(float Yoffset)
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

  return 4;
}


static int buildTread(float Yoffset, int divisions)
{
  int i;
  const float divs = (float)((divisions / 2) * 2); // even number
  const float divScale = 2.0f / divs;
  const float astep = (float)((M_PI * 2.0) / divs);
  const float yLeft = Yoffset + (0.5f * treadWidth);
  const float yRight = Yoffset - (0.5f * treadWidth);
  float x, z;
  float tx;
  // setup some basic texture coordinates
  const float txScale = treadTexCoordLen / treadLength;
  const float tx0 = 0.0f;
  const float tx1 = (float)(txScale * (treadRadius * M_PI));
  const float tx2 = (float)(txScale * ((treadRadius * M_PI) + (fullLength - treadHeight)));
  const float tx3 = (float)(txScale * ((treadHeight * M_PI) + (fullLength - treadHeight)));
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
      const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
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
      const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
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
      const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
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
      const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
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
	const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
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
	const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
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
	const float ang = (float)((astep * (double)i) - (M_PI / 2.0));
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
	const float ang = (float)((astep * (double)i) + (M_PI / 2.0));
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

  return (2 * 4 * (divisions + 2));
}


static int buildWheel(const float pos[3], float angle, int divisions)
{
  int i;
  int tris = 0;
  const float divs = (float)divisions;
  const float astep = (float)((M_PI * 2.0) / (double)divs);
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
  tris += (2 * divisions);

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
    tris += (divisions - 2);

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
    tris += (divisions - 2);
  }
  glShadeModel(GL_SMOOTH);

  return tris;
}


int TankGeometryUtils::buildHighLCasingAnim()
{
  int tris = 0;
  
  tris += buildCasing(+treadYCenter);

  if (treadStyle == TankGeometryUtils::Covered) {
    glShadeModel(GL_FLAT);
    {
      //draw the left tread cover
      glBegin(GL_TRIANGLE_STRIP);
	doNormal3f(0.984696f, 0.000000f, 0.174282f);
	doTexCoord2f(-0.193f, 0.727f);
	doVertex3f(3.000f, 0.875f, 0.770f);
	doTexCoord2f(0.009f, 0.356f);
	doVertex3f(3.000f, 1.400f, 0.770f);
	doTexCoord2f(-0.164f, 0.679f);
	doVertex3f(2.980f, 0.875f, 0.883f);
	doTexCoord2f(-0.015f, 0.407f);
	doVertex3f(2.980f, 1.400f, 0.883f);
	doNormal3f(0.519720f, 0.000000f, 0.854336f);
	doTexCoord2f(-0.134f, 0.666f);
	doVertex3f(2.860f, 0.875f, 0.956f);
	doTexCoord2f(-0.010f, 0.439f);
	doVertex3f(2.860f, 1.400f, 0.956f);
	doNormal3f(0.748075f, 0.000000f, 0.663614f);
	doTexCoord2f(-0.102f, 0.647f);
	doVertex3f(2.750f, 0.875f, 1.080f);
	doTexCoord2f(-0.009f, 0.477f);
	doVertex3f(2.750f, 1.400f, 1.080f);
	doNormal3f(0.049938f, 0.000000f, 0.998752f);
	doTexCoord2f(-0.041f, 0.675f);
	doVertex3f(2.350f, 0.875f, 1.100f);
	doTexCoord2f(0.048f, 0.512f);
	doVertex3f(2.350f, 1.400f, 1.100f);
	doNormal3f(0.455876f, 0.000000f, 0.890043f);
	doTexCoord2f(0.033f, 0.684f);
	doVertex3f(1.940f, 0.875f, 1.310f);
	doTexCoord2f(0.095f, 0.570f);
	doVertex3f(1.940f, 1.400f, 1.310f);
	doNormal3f(0.003378f, 0.000000f, 0.999994f);
	doTexCoord2f(0.468f, 0.920f);
	doVertex3f(-1.020f, 0.875f, 1.320f);
	doTexCoord2f(0.529f, 0.808f);
	doVertex3f(-1.020f, 1.400f, 1.320f);
	doNormal3f(0.178885f, 0.000000f, 0.983870f);
	doTexCoord2f(0.536f, 0.949f);
	doVertex3f(-1.460f, 0.875f, 1.400f);
	doTexCoord2f(0.591f, 0.849f);
	doVertex3f(-1.460f, 1.400f, 1.400f);
	doNormal3f(0.006622f, 0.000000f, 0.999978f);
	doTexCoord2f(0.759f, 1.070f);
	doVertex3f(-2.970f, 0.875f, 1.410f);
	doTexCoord2f(0.813f, 0.970f);
	doVertex3f(-2.970f, 1.400f, 1.410f);
	doNormal3f(-0.967641f, 0.000000f, -0.252333f);
	doTexCoord2f(0.587f, 1.300f);
	doVertex3f(-2.740f, 0.875f, 0.628f);
	doTexCoord2f(0.917f, 0.700f);
	doVertex3f(-2.740f, 1.400f, 0.628f);
	doNormal3f(-0.426419f, 0.000000f, -0.904526f);
	doTexCoord2f(0.375f, 1.300f);
	doVertex3f(-1.620f, 0.875f, 0.500f);
	doTexCoord2f(0.800f, 0.523f);
	doVertex3f(-1.620f, 1.400f, 0.500f);
	doNormal3f(0.000000f, 0.000000f, -1.000000f);
	doTexCoord2f(-0.156f, 1.010f);
	doVertex3f(1.990f, 0.875f, 0.500f);
	doTexCoord2f(0.268f, 0.233f);
	doVertex3f(1.990f, 1.400f, 0.500f);
	doNormal3f(0.454326f, 0.000000f, -0.890835f);
	doTexCoord2f(-0.246f, 0.896f);
	doVertex3f(2.790f, 0.875f,0.608f);
	doTexCoord2f(0.123f, 0.220f);
	doVertex3f(2.790f, 1.400f,0.608f);
	doNormal3f(0.978361f, 0.000000f, -0.206904f);
	doTexCoord2f(-0.182f, 0.754f);
	doVertex3f(2.860f, 0.875f, 0.739f);
	doTexCoord2f(0.038f, 0.352f);
	doVertex3f(2.860f, 1.400f, 0.739f);
	doNormal3f(0.216192f, 0.000000f, -0.976351f);
	doTexCoord2f(-0.193f, 0.727f);
	doVertex3f(3.000f, 0.875f, 0.770f);
	doTexCoord2f(0.009f, 0.356f);
	doVertex3f(3.000f, 1.400f, 0.770f);
      glEnd(); // 30 verts -> 28 tris
      tris += 28;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	doTexCoord2f(0.587f, 1.300f);
	doVertex3f(-2.740f, 0.875f, 0.628f);
	doTexCoord2f(0.375f, 1.300f);
	doVertex3f(-1.620f, 0.875f, 0.500f);
	doTexCoord2f(0.468f, 0.920f);
	doVertex3f(-1.020f, 0.875f, 1.320f);
	doTexCoord2f(0.536f, 0.949f);
	doVertex3f(-1.460f, 0.875f, 1.400f);
	doTexCoord2f(0.759f, 1.070f);
	doVertex3f(-2.970f, 0.875f, 1.410f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	doTexCoord2f(-0.156f, 1.010f);
	doVertex3f(1.990f, 0.875f, 0.500f);
	doTexCoord2f(-0.246f, 0.896f);
	doVertex3f(2.790f, 0.875f,0.608f);
	doTexCoord2f(-0.182f, 0.754f);
	doVertex3f(2.860f, 0.875f, 0.739f);
	doTexCoord2f(-0.102f, 0.647f);
	doVertex3f(2.750f, 0.875f, 1.080f);
	doTexCoord2f(-0.041f, 0.675f);
	doVertex3f(2.350f, 0.875f, 1.100f);
	doTexCoord2f(0.033f, 0.684f);
	doVertex3f(1.940f, 0.875f, 1.310f);
	doTexCoord2f(0.468f, 0.920f);
	doVertex3f(-1.020f, 0.875f, 1.320f);
	doTexCoord2f(0.375f, 1.300f);
	doVertex3f(-1.620f, 0.875f, 0.500f);
      glEnd(); // 8 verts -> 6 tris
      tris += 6;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	doTexCoord2f(-0.182f, 0.754f);
	doVertex3f(2.860f, 0.875f, 0.739f);
	doTexCoord2f(-0.193f, 0.727f);
	doVertex3f(3.000f, 0.875f, 0.770f);
	doTexCoord2f(-0.164f, 0.679f);
	doVertex3f(2.980f, 0.875f, 0.883f);
	doTexCoord2f(-0.134f, 0.666f);
	doVertex3f(2.860f, 0.875f, 0.956f);
	doTexCoord2f(-0.102f, 0.647f);
	doVertex3f(2.750f, 0.875f, 1.080f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	doTexCoord2f(0.917f, 0.700f);
	doVertex3f(-2.740f, 1.400f, 0.628f);
	doTexCoord2f(0.813f, 0.970f);
	doVertex3f(-2.970f, 1.400f, 1.410f);
	doTexCoord2f(0.591f, 0.849f);
	doVertex3f(-1.460f, 1.400f, 1.400f);
	doTexCoord2f(0.529f, 0.808f);
	doVertex3f(-1.020f, 1.400f, 1.320f);
	doTexCoord2f(0.800f, 0.523f);
	doVertex3f(-1.620f, 1.400f, 0.500f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	doTexCoord2f(0.268f, 0.233f);
	doVertex3f(1.990f, 1.400f, 0.500f);
	doTexCoord2f(0.800f, 0.523f);
	doVertex3f(-1.620f, 1.400f, 0.500f);
	doTexCoord2f(0.529f, 0.808f);
	doVertex3f(-1.020f, 1.400f, 1.320f);
	doTexCoord2f(0.095f, 0.570f);
	doVertex3f(1.940f, 1.400f, 1.310f);
	doTexCoord2f(0.048f, 0.512f);
	doVertex3f(2.350f, 1.400f, 1.100f);
	doTexCoord2f(-0.009f, 0.477f);
	doVertex3f(2.750f, 1.400f, 1.080f);
	doTexCoord2f(0.038f, 0.352f);
	doVertex3f(2.860f, 1.400f, 0.739f);
	doTexCoord2f(0.123f, 0.220f);
	doVertex3f(2.790f, 1.400f,0.608f);
      glEnd(); // 8 verts -> 6 tris
      tris += 6;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	doTexCoord2f(0.038f, 0.352f);
	doVertex3f(2.860f, 1.400f, 0.739f);
	doTexCoord2f(-0.009f, 0.477f);
	doVertex3f(2.750f, 1.400f, 1.080f);
	doTexCoord2f(-0.010f, 0.439f);
	doVertex3f(2.860f, 1.400f, 0.956f);
	doTexCoord2f(-0.015f, 0.407f);
	doVertex3f(2.980f, 1.400f, 0.883f);
	doTexCoord2f(0.009f, 0.356f);
	doVertex3f(3.000f, 1.400f, 0.770f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;
    }
    glShadeModel(GL_SMOOTH);
  }
  
  return tris;
}

int TankGeometryUtils::buildHighRCasingAnim()
{
  int tris = 0;

  tris += buildCasing(-treadYCenter);

  if (treadStyle == TankGeometryUtils::Covered) {
    glShadeModel(GL_FLAT);
    {
      //draw the right tread cover
      glBegin(GL_TRIANGLE_STRIP);
	doNormal3f(0.984696f, 0.000000f, 0.174282f);
	doTexCoord2f(-0.295f, 0.041f);
	doVertex3f(3.000f, -1.400f, 0.770f);
	doTexCoord2f(0.045f, -0.208f);
	doVertex3f(3.000f, -0.875f, 0.770f);
	doTexCoord2f(-0.248f, 0.010f);
	doVertex3f(2.980f, -1.400f, 0.883f);
	doTexCoord2f(0.002f, -0.173f);
	doVertex3f(2.980f, -0.875f, 0.883f);
	doNormal3f(0.519720f, 0.000000f, 0.854336f);
	doTexCoord2f(-0.216f, 0.011f);
	doVertex3f(2.860f, -1.400f, 0.956f);
	doTexCoord2f(-0.007f, -0.141f);
	doVertex3f(2.860f, -0.875f, 0.956f);
	doNormal3f(0.748075f, 0.000000f, 0.663614f);
	doTexCoord2f(-0.179f, 0.008f);
	doVertex3f(2.750f, -1.400f, 1.080f);
	doTexCoord2f(-0.022f, -0.107f);
	doVertex3f(2.750f, -0.875f, 1.080f);
	doNormal3f(0.049938f, 0.000000f, 0.998752f);
	doTexCoord2f(-0.136f, 0.059f);
	doVertex3f(2.350f, -1.400f, 1.100f);
	doTexCoord2f(0.014f, -0.050f);
	doVertex3f(2.350f, -0.875f, 1.100f);
	doNormal3f(0.455876f, 0.000000f, 0.890043f);
	doTexCoord2f(-0.072f, 0.099f);
	doVertex3f(1.940f, -1.400f, 1.310f);
	doTexCoord2f(0.032f, 0.022f);
	doVertex3f(1.940f, -0.875f, 1.310f);
	doNormal3f(0.003378f, 0.000000f, 0.999994f);
	doTexCoord2f(0.221f, 0.497f);
	doVertex3f(-1.020f, -1.400f, 1.320f);
	doTexCoord2f(0.324f, 0.422f);
	doVertex3f(-1.020f, -0.875f, 1.320f);
	doNormal3f(0.178885f, 0.000000f, 0.983870f);
	doTexCoord2f(0.270f, 0.553f);
	doVertex3f(-1.460f, -1.400f, 1.400f);
	doTexCoord2f(0.362f, 0.486f);
	doVertex3f(-1.460f, -0.875f, 1.400f);
	doNormal3f(0.006622f, 0.000000f, 0.999978f);
	doTexCoord2f(0.419f, 0.757f);
	doVertex3f(-2.970f, -1.400f, 1.410f);
	doTexCoord2f(0.511f, 0.690f);
	doVertex3f(-2.970f, -0.875f, 1.410f);
	doNormal3f(-0.967641f, 0.000000f, -0.252333f);
	doTexCoord2f(0.165f, 0.896f);
	doVertex3f(-2.740f, -1.400f, 0.628f);
	doTexCoord2f(0.720f, 0.489f);
	doVertex3f(-2.740f, -0.875f, 0.628f);
	doNormal3f(-0.426419f, 0.000000f, -0.904526f);
	doTexCoord2f(-0.026f, 0.803f);
	doVertex3f(-1.620f, -1.400f, 0.500f);
	doTexCoord2f(0.690f, 0.279f);
	doVertex3f(-1.620f, -0.875f, 0.500f);
	doNormal3f(0.000000f, 0.000000f, -1.000000f);
	doTexCoord2f(-0.383f, 0.314f);
	doVertex3f(1.990f, -1.400f, 0.500f);
	doTexCoord2f(0.332f, -0.209f);
	doVertex3f(1.990f, -0.875f, 0.500f);
	doNormal3f(0.454326f, 0.000000f, -0.890835f);
	doTexCoord2f(-0.415f, 0.172f);
	doVertex3f(2.790f, -1.400f,0.608f);
	doTexCoord2f(0.206f, -0.283f);
	doVertex3f(2.790f, -0.875f,0.608f);
	doNormal3f(0.978361f, 0.000000f, -0.206904f);
	doTexCoord2f(-0.296f, 0.070f);
	doVertex3f(2.860f, -1.400f, 0.739f);
	doTexCoord2f(0.073f, -0.200f);
	doVertex3f(2.860f, -0.875f, 0.739f);
	doNormal3f(0.216192f, 0.000000f, -0.976351f);
	doTexCoord2f(-0.295f, 0.041f);
	doVertex3f(3.000f, -1.400f, 0.770f);
	doTexCoord2f(0.045f, -0.208f);
	doVertex3f(3.000f, -0.875f, 0.770f);
      glEnd(); // 30 verts -> 28 tris
      tris += 28;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	doTexCoord2f(0.165f, 0.896f);
	doVertex3f(-2.740f, -1.400f, 0.628f);
	doTexCoord2f(-0.026f, 0.803f);
	doVertex3f(-1.620f, -1.400f, 0.500f);
	doTexCoord2f(0.221f, 0.497f);
	doVertex3f(-1.020f, -1.400f, 1.320f);
	doTexCoord2f(0.270f, 0.553f);
	doVertex3f(-1.460f, -1.400f, 1.400f);
	doTexCoord2f(0.419f, 0.757f);
	doVertex3f(-2.970f, -1.400f, 1.410f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	doTexCoord2f(-0.383f, 0.314f);
	doVertex3f(1.990f, -1.400f, 0.500f);
	doTexCoord2f(-0.415f, 0.172f);
	doVertex3f(2.790f, -1.400f,0.608f);
	doTexCoord2f(-0.296f, 0.070f);
	doVertex3f(2.860f, -1.400f, 0.739f);
	doTexCoord2f(-0.179f, 0.008f);
	doVertex3f(2.750f, -1.400f, 1.080f);
	doTexCoord2f(-0.136f, 0.059f);
	doVertex3f(2.350f, -1.400f, 1.100f);
	doTexCoord2f(-0.072f, 0.099f);
	doVertex3f(1.940f, -1.400f, 1.310f);
	doTexCoord2f(0.221f, 0.497f);
	doVertex3f(-1.020f, -1.400f, 1.320f);
	doTexCoord2f(-0.026f, 0.803f);
	doVertex3f(-1.620f, -1.400f, 0.500f);
      glEnd(); // 8 verts -> 6 tris
      tris += 6;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, -1.000000f, 0.000000f);
	doTexCoord2f(-0.296f, 0.070f);
	doVertex3f(2.860f, -1.400f, 0.739f);
	doTexCoord2f(-0.295f, 0.041f);
	doVertex3f(3.000f, -1.400f, 0.770f);
	doTexCoord2f(-0.248f, 0.010f);
	doVertex3f(2.980f, -1.400f, 0.883f);
	doTexCoord2f(-0.216f, 0.011f);
	doVertex3f(2.860f, -1.400f, 0.956f);
	doTexCoord2f(-0.179f, 0.008f);
	doVertex3f(2.750f, -1.400f, 1.080f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	doTexCoord2f(0.720f, 0.489f);
	doVertex3f(-2.740f, -0.875f, 0.628f);
	doTexCoord2f(0.511f, 0.690f);
	doVertex3f(-2.970f, -0.875f, 1.410f);
	doTexCoord2f(0.362f, 0.486f);
	doVertex3f(-1.460f, -0.875f, 1.400f);
	doTexCoord2f(0.324f, 0.422f);
	doVertex3f(-1.020f, -0.875f, 1.320f);
	doTexCoord2f(0.690f, 0.279f);
	doVertex3f(-1.620f, -0.875f, 0.500f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	doTexCoord2f(0.332f, -0.209f);
	doVertex3f(1.990f, -0.875f, 0.500f);
	doTexCoord2f(0.690f, 0.279f);
	doVertex3f(-1.620f, -0.875f, 0.500f);
	doTexCoord2f(0.324f, 0.422f);
	doVertex3f(-1.020f, -0.875f, 1.320f);
	doTexCoord2f(0.032f, 0.022f);
	doVertex3f(1.940f, -0.875f, 1.310f);
	doTexCoord2f(0.014f, -0.050f);
	doVertex3f(2.350f, -0.875f, 1.100f);
	doTexCoord2f(-0.022f, -0.107f);
	doVertex3f(2.750f, -0.875f, 1.080f);
	doTexCoord2f(0.073f, -0.200f);
	doVertex3f(2.860f, -0.875f, 0.739f);
	doTexCoord2f(0.206f, -0.283f);
	doVertex3f(2.790f, -0.875f,0.608f);
      glEnd(); // 8 verts -> 6 tris
      tris += 6;

      glBegin(GL_TRIANGLE_FAN);
	doNormal3f(0.000000f, 1.000000f, 0.000000f);
	doTexCoord2f(0.073f, -0.200f);
	doVertex3f(2.860f, -0.875f, 0.739f);
	doTexCoord2f(-0.022f, -0.107f);
	doVertex3f(2.750f, -0.875f, 1.080f);
	doTexCoord2f(-0.007f, -0.141f);
	doVertex3f(2.860f, -0.875f, 0.956f);
	doTexCoord2f(0.002f, -0.173f);
	doVertex3f(2.980f, -0.875f, 0.883f);
	doTexCoord2f(0.045f, -0.208f);
	doVertex3f(3.000f, -0.875f, 0.770f);
      glEnd(); // 5 verts -> 3 tris
      tris += 3;
    }
    glShadeModel(GL_SMOOTH);
  }
  
  return tris;
}


int TankGeometryUtils::buildHighLTread(int divs)
{
  return buildTread(+treadYCenter, divs);
}

int TankGeometryUtils::buildHighRTread(int divs)
{
  return buildTread(-treadYCenter, divs);
}


int TankGeometryUtils::buildHighLWheel(int number, float angle, int divs)
{
  assert ((number >= 0) && (number < 4));
  float pos[3];
  pos[0] = wheelSpacing * (-1.5f + (float)number);
  pos[1] = +treadYCenter;
  pos[2] = treadRadius;
  return buildWheel(pos, angle, divs);
}

int TankGeometryUtils::buildHighRWheel(int number, float angle, int divs)
{
  assert ((number >= 0) && (number < 4));
  float pos[3];
  pos[0] = wheelSpacing * (-1.5f + (float)number);
  pos[1] = -treadYCenter;
  pos[2] = treadRadius;
  return buildWheel(pos, angle, divs);
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
