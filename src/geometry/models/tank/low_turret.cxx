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

int TankGeometryUtils::buildLowTurret( void )
{
	int count = 0;
	if (buildGeoFromObj("/low/turret.obj",count))
		return count;

  glShadeModel(GL_FLAT);
  glBegin(GL_TRIANGLE_STRIP);
    doNormal3f(0.991228f, 0.000000f, -0.132164f);
    doTexCoord2f(0.107f, -0.009f);
    doVertex3f(1.480f, -0.516f, 1.040f);
    doTexCoord2f(0.617f, -0.559f);
    doVertex3f(1.480f, 0.516f, 1.040f);
    doTexCoord2f(0.224f, -0.178f);
    doVertex3f(1.580f, -0.434f, 1.790f);
    doTexCoord2f(0.457f, -0.429f);
    doVertex3f(1.580f, 0.435f, 1.790f);
    doNormal3f(0.087795f, 0.000000f, 0.996139f);
    doTexCoord2f(0.866f, 0.402f);
    doVertex3f(-1.370f, -0.765f, 2.050f);
    doTexCoord2f(1.080f, 0.169f);
    doVertex3f(-1.370f, 0.764f, 2.050f);
    doNormal3f(-0.741466f, 0.000000f, -0.670990f);
    doTexCoord2f(0.559f, 0.339f);
    doVertex3f(-0.456f, -1.060f, 1.040f);
    doTexCoord2f(0.996f, -0.132f);
    doVertex3f(-0.456f, 1.080f, 1.040f);
  glEnd(); // 6 tris
  glShadeModel(GL_SMOOTH);
  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.183504f, -0.940289f, 0.286676f);
    doTexCoord2f(0.559f, 0.339f);
    doVertex3f(-0.456f, -1.060f, 1.040f);
    doNormal3f(0.269870f, -0.960420f, 0.069023f);
    doTexCoord2f(0.107f, -0.009f);
    doVertex3f(1.480f, -0.516f, 1.040f);
    doNormal3f(0.136533f, -0.910814f, 0.389585f);
    doTexCoord2f(0.224f, -0.178f);
    doVertex3f(1.580f, -0.434f, 1.790f);
    doTexCoord2f(0.866f, 0.402f);
    doVertex3f(-1.370f, -0.765f, 2.050f);
  glEnd(); // 2 tris
  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.235235f, 0.954665f, 0.182426f);
    doTexCoord2f(0.996f, -0.132f);
    doVertex3f(-0.456f, 1.080f, 1.040f);
    doNormal3f(0.136569f, 0.903492f, 0.406265f);
    doTexCoord2f(1.080f, 0.169f);
    doVertex3f(-1.370f, 0.764f, 2.050f);
    doNormal3f(0.279081f, 0.957980f, 0.066251f);
    doTexCoord2f(0.457f, -0.429f);
    doVertex3f(1.580f, 0.435f, 1.790f);
    doTexCoord2f(0.617f, -0.559f);
    doVertex3f(1.480f, 0.516f, 1.040f);
  glEnd(); // 2 tris

  return 10;
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
