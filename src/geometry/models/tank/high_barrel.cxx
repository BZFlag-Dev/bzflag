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

int TankGeometryUtils::buildHighBarrel ( void )
{
  glShadeModel(GL_FLAT);
  glBegin(GL_TRIANGLE_FAN);
    doNormal3f(1.000000f, 0.000000f, 0.000000f);
    doVertex3f(4.940f, 0.0f, 1.410f);
    doVertex3f(4.940f, 0.089f, 1.440f);
    doVertex3f(4.940f, 0.126f, 1.530f);
    doVertex3f(4.940f, 0.089f, 1.620f);
    doVertex3f(4.940f, 0.0f, 1.660f);
    doVertex3f(4.940f, -0.09f, 1.620f);
    doVertex3f(4.940f, -0.126f, 1.530f);
    doVertex3f(4.940f, -0.09f, 1.440f);
  glEnd(); // 8 verts -> 6 tris
  glShadeModel(GL_SMOOTH);
  glBegin(GL_TRIANGLE_STRIP);
    doNormal3f(0.015873f, -0.999874f, 0.000000f);
    doVertex3f(1.570f, -0.18f, 1.530f);
    doVertex3f(4.940f, -0.126f, 1.530f);
    doNormal3f(0.016407f, -0.704331f, 0.709682f);
    doVertex3f(1.570f, -0.128f, 1.660f);
    doNormal3f(0.016398f, -0.718536f, 0.695296f);
    doVertex3f(4.940f, -0.09f, 1.620f);
    doNormal3f(0.014835f, -0.000267f, 0.999890f);
    doVertex3f(1.570f, 0.0f, 1.710f);
    doVertex3f(4.940f, 0.0f, 1.660f);
    doNormal3f(0.016367f, 0.702262f, 0.711730f);
    doVertex3f(1.570f, 0.126f, 1.660f);
    doNormal3f(0.016356f, 0.717625f, 0.696238f);
    doVertex3f(4.940f, 0.089f, 1.620f);
    doNormal3f(0.016022f, 0.999872f, 0.000000f);
    doVertex3f(1.570f, 0.18f, 1.530f);
    doVertex3f(4.940f, 0.126f, 1.530f);
    doNormal3f(0.016367f, 0.702262f, -0.711730f);
    doVertex3f(1.570f, 0.126f, 1.400f);
    doNormal3f(0.016369f, 0.683073f, -0.730167f);
    doVertex3f(4.940f, 0.089f, 1.440f);
    doNormal3f(0.017801f, -0.000267f, -0.999841f);
    doVertex3f(1.570f, 0.0f, 1.350f);
    doVertex3f(4.940f, 0.0f, 1.410f);
    doNormal3f(0.016407f, -0.704331f, -0.709683f);
    doVertex3f(1.570f, -0.128f, 1.400f);
    doNormal3f(0.016410f, -0.683912f, -0.729379f);
    doVertex3f(4.940f, -0.09f, 1.440f);
    doNormal3f(0.015873f, -0.999874f, 0.000000f);
    doVertex3f(1.570f, -0.18f, 1.530f);
    doVertex3f(4.940f, -0.126f, 1.530f);
  glEnd(); // 18 verts -> 16 tris

  return 22;
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
