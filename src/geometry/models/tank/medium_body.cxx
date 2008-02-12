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

int TankGeometryUtils::buildMedBody ( void )
{
	int count = 0;
	if (buildGeoFromObj("/medium/body.obj",count))
		return count;

  glShadeModel(GL_FLAT);
  glBegin(GL_TRIANGLE_STRIP);
    doNormal3f(0.997647f, 0.000000f, 0.068567f);
    doTexCoord2f(1.240f, 2.230f);
    doVertex3f(2.610f, -0.877f, 0.408f);
    doTexCoord2f(0.700f, 1.970f);
    doVertex3f(2.610f, 0.877f, 0.408f);
    doTexCoord2f(1.240f, 2.210f);
    doVertex3f(2.570f, -0.877f, 0.990f);
    doTexCoord2f(0.705f, 1.960f);
    doVertex3f(2.570f, 0.877f, 0.990f);
    doNormal3f(0.170314f, 0.000000f, 0.985390f);
    doTexCoord2f(1.360f, 1.970f);
    doVertex3f(1.760f, -0.877f, 1.130f);
    doTexCoord2f(0.822f, 1.710f);
    doVertex3f(1.760f, 0.877f, 1.130f);
    doNormal3f(0.023599f, 0.000000f, 0.999722f);
    doTexCoord2f(2.030f, 0.541f);
    doVertex3f(-2.900f, -0.877f, 1.240f);
    doTexCoord2f(1.490f, 0.289f);
    doVertex3f(-2.900f, 0.877f, 1.240f);
    doNormal3f(-0.975668f, 0.000000f, -0.219251f);
    doTexCoord2f(2.000f, 0.590f);
    doVertex3f(-2.740f, -0.877f, 0.528f);
    doTexCoord2f(1.470f, 0.338f);
    doVertex3f(-2.740f, 0.877f, 0.528f);
    doNormal3f(-0.426419f, 0.000000f, -0.904526f);
    doTexCoord2f(1.840f, 0.932f);
    doVertex3f(-1.620f, -0.877f, 0.200f);
    doTexCoord2f(1.310f, 0.680f);
    doVertex3f(-1.620f, 0.877f, 0.200f);
    doNormal3f(0.000000f, 0.000000f, -1.000000f);
    doTexCoord2f(1.350f, 1.980f);
    doVertex3f(1.810f, -0.877f, 0.200f);
    doTexCoord2f(0.815f, 1.730f);
    doVertex3f(1.810f, 0.877f, 0.200f);
    doNormal3f(0.454326f, 0.000000f, -0.890835f);
    doTexCoord2f(1.240f, 2.230f);
    doVertex3f(2.610f, -0.877f, 0.408f);
    doTexCoord2f(0.700f, 1.970f);
    doVertex3f(2.610f, 0.877f, 0.408f);
  glEnd();

  return 14;
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
