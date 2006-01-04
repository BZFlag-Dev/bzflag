/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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

int TankGeometryUtils::buildLowBody ( void )
{
  glShadeModel(GL_FLAT);

  glBegin(GL_TRIANGLE_STRIP);
    doNormal3f(0.023598f, 0.000000f, 0.999722f);
    doTexCoord2f(1.360f, 1.970f);
    doVertex3f(2.575f, -0.877f, 1.111f);
    doTexCoord2f(0.822f, 1.710f);
    doVertex3f(2.575f, 0.877f, 1.111f);
    doTexCoord2f(2.030f, 0.541f);
    doVertex3f(-2.835f, -0.877f, 1.238f);
    doTexCoord2f(1.490f, 0.289f);
    doVertex3f(-2.835f, 0.877f, 1.238f);
    doNormal3f(-0.898134f, 0.000000f, -0.439723f);
    doTexCoord2f(1.840f, 0.932f);
    doVertex3f(-2.229f, -0.877f, 0.200f);
    doTexCoord2f(1.310f, 0.680f);
    doVertex3f(-2.229f, 0.877f, 0.200f);
    doNormal3f(0.000000f, 0.000000f, -1.000000f);
    doTexCoord2f(1.350f, 1.980f);
    doVertex3f(2.430f, -0.877f, 0.200f);
    doTexCoord2f(0.815f, 1.730f);
    doVertex3f(2.430f, 0.877f, 0.200f);
    doNormal3f(0.991585f, 0.000000f, -0.129459f);
    doTexCoord2f(1.360f, 1.970f);
    doVertex3f(2.575f, -0.877f, 1.111f);
    doTexCoord2f(0.822f, 1.710f);
    doVertex3f(2.575f, 0.877f, 1.111f);
  glEnd();

  return 8;
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
