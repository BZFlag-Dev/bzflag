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

int TankGeometryUtils::buildMedTurret ( void )
{
	int count = 0;
	if (buildGeoFromObj("/medium/turret.obj",count))
		return count;

  glShadeModel(GL_FLAT);
  glBegin(GL_TRIANGLE_STRIP);
    doNormal3f(-0.235964f, 0.967658f, 0.089216f);
    doTexCoord2f(0.812f, -0.134f);
    doVertex3f(0.007f, 1.110f, 1.940f);
    doTexCoord2f(0.996f, -0.132f);
    doVertex3f(-0.456f, 1.080f, 1.040f);
    doTexCoord2f(1.080f, 0.169f);
    doVertex3f(-1.370f, 0.764f, 2.050f);
    doNormal3f(-0.741466f, 0.000000f, -0.670990f);
    doTexCoord2f(0.559f, 0.339f);
    doVertex3f(-0.456f, -1.060f, 1.040f);
    doTexCoord2f(0.866f, 0.402f);
    doVertex3f(-1.370f, -0.765f, 2.050f);
    doNormal3f(-0.238331f, -0.968849f, 0.067303f);
    doTexCoord2f(0.546f, 0.156f);
    doVertex3f(0.001f, -1.110f, 1.940f);
    doNormal3f(0.079953f, 0.000000f, 0.996799f);
    doTexCoord2f(1.080f, 0.169f);
    doVertex3f(-1.370f, 0.764f, 2.050f);
    doTexCoord2f(0.812f, -0.134f);
    doVertex3f(0.007f, 1.110f, 1.940f);
  glEnd(); // 6 tris
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
    doNormal3f(0.094595f, 0.000000f, 0.995516f);
    doTexCoord2f(0.546f, 0.156f);
    doVertex3f(0.001f, -1.110f, 1.940f);
    doTexCoord2f(0.812f, -0.134f);
    doVertex3f(0.007f, 1.110f, 1.940f);
  glEnd(); // 4 tris
  glShadeModel(GL_SMOOTH);
  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.149229f, -0.988713f, 0.013310f);
    doTexCoord2f(0.458f, 0.244f);
    doVertex3f(0.014f, -1.100f, 1.030f);
    doNormal3f(0.369158f, -0.927898f, 0.052229f);
    doTexCoord2f(0.107f, -0.009f);
    doVertex3f(1.480f, -0.516f, 1.040f);
    doNormal3f(0.381395f, -0.924109f, 0.023687f);
    doTexCoord2f(0.224f, -0.178f);
    doVertex3f(1.580f, -0.434f, 1.790f);
    doNormal3f(-0.085139f, -0.996296f, -0.012079f);
    doTexCoord2f(0.546f, 0.156f);
    doVertex3f(0.001f, -1.110f, 1.940f);
    doTexCoord2f(0.559f, 0.339f);
    doVertex3f(-0.456f, -1.060f, 1.040f);
  glEnd(); // 3 tris
  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(0.222340f, 0.974751f, 0.020603f);
    doTexCoord2f(0.895f, -0.226f);
    doVertex3f(0.014f, 1.100f, 1.030f);
    doNormal3f(-0.042797f, 0.999020f, -0.011269f);
    doTexCoord2f(0.996f, -0.132f);
    doVertex3f(-0.456f, 1.080f, 1.040f);
    doNormal3f(0.179990f, 0.983622f, -0.009585f);
    doTexCoord2f(0.812f, -0.134f);
    doVertex3f(0.007f, 1.110f, 1.940f);
    doNormal3f(0.369189f, 0.927954f, 0.050994f);
    doTexCoord2f(0.457f, -0.429f);
    doVertex3f(1.580f, 0.435f, 1.790f);
    doTexCoord2f(0.617f, -0.559f);
    doVertex3f(1.480f, 0.516f, 1.040f);
  glEnd(); // 3 tris

  return 16;
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
