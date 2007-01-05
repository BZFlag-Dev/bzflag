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
#include "TankGeometryMgr.h"
using namespace TankGeometryUtils;

int TankGeometryUtils::buildHighRCasing ( void )
{
  int tris = 0;
  glShadeModel(GL_FLAT);
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
    doVertex3f(-2.740f, -1.400f, 0.528f);
    doTexCoord2f(0.720f, 0.489f);
    doVertex3f(-2.740f, -0.875f, 0.528f);
    doNormal3f(-0.426419f, 0.000000f, -0.904526f);
    doTexCoord2f(-0.026f, 0.803f);
    doVertex3f(-1.620f, -1.400f, 0.000f);
    doTexCoord2f(0.690f, 0.279f);
    doVertex3f(-1.620f, -0.875f, 0.000f);
    doNormal3f(0.000000f, 0.000000f, -1.000000f);
    doTexCoord2f(-0.383f, 0.314f);
    doVertex3f(1.990f, -1.400f, 0.000f);
    doTexCoord2f(0.332f, -0.209f);
    doVertex3f(1.990f, -0.875f, 0.000f);
    doNormal3f(0.454326f, 0.000000f, -0.890835f);
    doTexCoord2f(-0.415f, 0.172f);
    doVertex3f(2.790f, -1.400f, 0.408f);
    doTexCoord2f(0.206f, -0.283f);
    doVertex3f(2.790f, -0.875f, 0.408f);
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
    doVertex3f(-2.740f, -1.400f, 0.528f);
    doTexCoord2f(-0.026f, 0.803f);
    doVertex3f(-1.620f, -1.400f, 0.000f);
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
    doVertex3f(1.990f, -1.400f, 0.000f);
    doTexCoord2f(-0.415f, 0.172f);
    doVertex3f(2.790f, -1.400f, 0.408f);
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
    doVertex3f(-1.620f, -1.400f, 0.000f);
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
    doVertex3f(-2.740f, -0.875f, 0.528f);
    doTexCoord2f(0.511f, 0.690f);
    doVertex3f(-2.970f, -0.875f, 1.410f);
    doTexCoord2f(0.362f, 0.486f);
    doVertex3f(-1.460f, -0.875f, 1.400f);
    doTexCoord2f(0.324f, 0.422f);
    doVertex3f(-1.020f, -0.875f, 1.320f);
    doTexCoord2f(0.690f, 0.279f);
    doVertex3f(-1.620f, -0.875f, 0.000f);
  glEnd(); // 5 verts -> 3 tris
  tris += 3;

  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.000000f, 1.000000f, 0.000000f);
    doTexCoord2f(0.332f, -0.209f);
    doVertex3f(1.990f, -0.875f, 0.000f);
    doTexCoord2f(0.690f, 0.279f);
    doVertex3f(-1.620f, -0.875f, 0.000f);
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
    doVertex3f(2.790f, -0.875f, 0.408f);
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

  return tris;
}
/*
 * Local Variables: ***
 * mode:C ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
