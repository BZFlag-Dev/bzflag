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
#include "TankGeometryMgr.h"
using namespace TankGeometryUtils;

int TankGeometryUtils::buildHighLCasing( void )
{
  int count = 0;
  if (buildGeoFromObj("geometry/tank/std/lcasing.obj",count))
    return count;

  int tris = 0;
  glShadeModel(GL_FLAT);
  glBegin(GL_TRIANGLE_STRIP);
    doNormal3f(0.984696f, 0.000000f, 0.174282f);
    doTexCoord2f(-0.193f, 0.727f);
    doVertex3f(3.000f, 0.877f, 0.770f);
    doTexCoord2f(0.009f, 0.356f);
    doVertex3f(3.000f, 1.400f, 0.770f);
    doTexCoord2f(-0.164f, 0.679f);
    doVertex3f(2.980f, 0.877f, 0.883f);
    doTexCoord2f(-0.015f, 0.407f);
    doVertex3f(2.980f, 1.400f, 0.883f);
    doNormal3f(0.519720f, 0.000000f, 0.854336f);
    doTexCoord2f(-0.134f, 0.666f);
    doVertex3f(2.860f, 0.877f, 0.956f);
    doTexCoord2f(-0.010f, 0.439f);
    doVertex3f(2.860f, 1.400f, 0.956f);
    doNormal3f(0.748075f, 0.000000f, 0.663614f);
    doTexCoord2f(-0.102f, 0.647f);
    doVertex3f(2.750f, 0.877f, 1.080f);
    doTexCoord2f(-0.009f, 0.477f);
    doVertex3f(2.750f, 1.400f, 1.080f);
    doNormal3f(0.049938f, 0.000000f, 0.998752f);
    doTexCoord2f(-0.041f, 0.675f);
    doVertex3f(2.350f, 0.877f, 1.100f);
    doTexCoord2f(0.048f, 0.512f);
    doVertex3f(2.350f, 1.400f, 1.100f);
    doNormal3f(0.455876f, 0.000000f, 0.890043f);
    doTexCoord2f(0.033f, 0.684f);
    doVertex3f(1.940f, 0.877f, 1.310f);
    doTexCoord2f(0.095f, 0.570f);
    doVertex3f(1.940f, 1.400f, 1.310f);
    doNormal3f(0.003378f, 0.000000f, 0.999994f);
    doTexCoord2f(0.468f, 0.920f);
    doVertex3f(-1.020f, 0.877f, 1.320f);
    doTexCoord2f(0.529f, 0.808f);
    doVertex3f(-1.020f, 1.400f, 1.320f);
    doNormal3f(0.178885f, 0.000000f, 0.983870f);
    doTexCoord2f(0.536f, 0.949f);
    doVertex3f(-1.460f, 0.877f, 1.400f);
    doTexCoord2f(0.591f, 0.849f);
    doVertex3f(-1.460f, 1.400f, 1.400f);
    doNormal3f(0.006622f, 0.000000f, 0.999978f);
    doTexCoord2f(0.759f, 1.070f);
    doVertex3f(-2.970f, 0.877f, 1.410f);
    doTexCoord2f(0.813f, 0.970f);
    doVertex3f(-2.970f, 1.400f, 1.410f);
    doNormal3f(-0.967641f, 0.000000f, -0.252333f);
    doTexCoord2f(0.587f, 1.300f);
    doVertex3f(-2.740f, 0.877f, 0.528f);
    doTexCoord2f(0.917f, 0.700f);
    doVertex3f(-2.740f, 1.400f, 0.528f);
    doNormal3f(-0.426419f, 0.000000f, -0.904526f);
    doTexCoord2f(0.375f, 1.300f);
    doVertex3f(-1.620f, 0.877f, 0.000f);
    doTexCoord2f(0.800f, 0.523f);
    doVertex3f(-1.620f, 1.400f, 0.000f);
    doNormal3f(0.000000f, 0.000000f, -1.000000f);
    doTexCoord2f(-0.156f, 1.010f);
    doVertex3f(1.990f, 0.877f, 0.000f);
    doTexCoord2f(0.268f, 0.233f);
    doVertex3f(1.990f, 1.400f, 0.000f);
    doNormal3f(0.454326f, 0.000000f, -0.890835f);
    doTexCoord2f(-0.246f, 0.896f);
    doVertex3f(2.790f, 0.877f, 0.408f);
    doTexCoord2f(0.123f, 0.220f);
    doVertex3f(2.790f, 1.400f, 0.408f);
    doNormal3f(0.978361f, 0.000000f, -0.206904f);
    doTexCoord2f(-0.182f, 0.754f);
    doVertex3f(2.860f, 0.877f, 0.739f);
    doTexCoord2f(0.038f, 0.352f);
    doVertex3f(2.860f, 1.400f, 0.739f);
    doNormal3f(0.216192f, 0.000000f, -0.976351f);
    doTexCoord2f(-0.193f, 0.727f);
    doVertex3f(3.000f, 0.877f, 0.770f);
    doTexCoord2f(0.009f, 0.356f);
    doVertex3f(3.000f, 1.400f, 0.770f);
  glEnd(); // 30 verts -> 28 tris
  tris += 28;

  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.000000f, -1.000000f, 0.000000f);
    doTexCoord2f(0.587f, 1.300f);
    doVertex3f(-2.740f, 0.877f, 0.528f);
    doTexCoord2f(0.375f, 1.300f);
    doVertex3f(-1.620f, 0.877f, 0.000f);
    doTexCoord2f(0.468f, 0.920f);
    doVertex3f(-1.020f, 0.877f, 1.320f);
    doTexCoord2f(0.536f, 0.949f);
    doVertex3f(-1.460f, 0.877f, 1.400f);
    doTexCoord2f(0.759f, 1.070f);
    doVertex3f(-2.970f, 0.877f, 1.410f);
  glEnd(); // 5 verts -> 3 tris
  tris += 3;

  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.000000f, -1.000000f, 0.000000f);
    doTexCoord2f(-0.156f, 1.010f);
    doVertex3f(1.990f, 0.877f, 0.000f);
    doTexCoord2f(-0.246f, 0.896f);
    doVertex3f(2.790f, 0.877f, 0.408f);
    doTexCoord2f(-0.182f, 0.754f);
    doVertex3f(2.860f, 0.877f, 0.739f);
    doTexCoord2f(-0.102f, 0.647f);
    doVertex3f(2.750f, 0.877f, 1.080f);
    doTexCoord2f(-0.041f, 0.675f);
    doVertex3f(2.350f, 0.877f, 1.100f);
    doTexCoord2f(0.033f, 0.684f);
    doVertex3f(1.940f, 0.877f, 1.310f);
    doTexCoord2f(0.468f, 0.920f);
    doVertex3f(-1.020f, 0.877f, 1.320f);
    doTexCoord2f(0.375f, 1.300f);
    doVertex3f(-1.620f, 0.877f, 0.000f);
  glEnd(); // 8 verts -> 6 tris
  tris += 6;

  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.000000f, -1.000000f, 0.000000f);
    doTexCoord2f(-0.182f, 0.754f);
    doVertex3f(2.860f, 0.877f, 0.739f);
    doTexCoord2f(-0.193f, 0.727f);
    doVertex3f(3.000f, 0.877f, 0.770f);
    doTexCoord2f(-0.164f, 0.679f);
    doVertex3f(2.980f, 0.877f, 0.883f);
    doTexCoord2f(-0.134f, 0.666f);
    doVertex3f(2.860f, 0.877f, 0.956f);
    doTexCoord2f(-0.102f, 0.647f);
    doVertex3f(2.750f, 0.877f, 1.080f);
  glEnd(); // 5 verts -> 3 tris
  tris += 3;

  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.000000f, 1.000000f, 0.000000f);
    doTexCoord2f(0.917f, 0.700f);
    doVertex3f(-2.740f, 1.400f, 0.528f);
    doTexCoord2f(0.813f, 0.970f);
    doVertex3f(-2.970f, 1.400f, 1.410f);
    doTexCoord2f(0.591f, 0.849f);
    doVertex3f(-1.460f, 1.400f, 1.400f);
    doTexCoord2f(0.529f, 0.808f);
    doVertex3f(-1.020f, 1.400f, 1.320f);
    doTexCoord2f(0.800f, 0.523f);
    doVertex3f(-1.620f, 1.400f, 0.000f);
  glEnd(); // 5 verts -> 3 tris
  tris += 3;

  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.000000f, 1.000000f, 0.000000f);
    doTexCoord2f(0.268f, 0.233f);
    doVertex3f(1.990f, 1.400f, 0.000f);
    doTexCoord2f(0.800f, 0.523f);
    doVertex3f(-1.620f, 1.400f, 0.000f);
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
    doVertex3f(2.790f, 1.400f, 0.408f);
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
