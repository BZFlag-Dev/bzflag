/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _TARGETINGUTILS__H_
#define _TARGETINGUTILS__H_

// common - 1st
#include "common.h"

// common headers
#include "vectors.h"

class TargetingUtils {
  public:
    static float normalizeAngle(float ang);
    static void  getUnitVector(const fvec3& src, const fvec3& target, fvec3& unitVector);
    static void  get3DUnitVector(const fvec3& src, const fvec3& target, fvec3& unitVector);
    static float getTargetDistance(const fvec3& src, const fvec3& target);
    static float getTargetAzimuth(const fvec3& src, const fvec3& target);
    static float getTargetRotation(float startAzimuth, float targetAzimuth);
    static float getTargetAngleDifference(const fvec3& src, float srcAzimuth, const fvec3& target);
    static bool  isLocationObscured(const fvec3& src, const fvec3& target);
    static float getOpenDistance(const fvec3& src, const float azimuth);
    static bool  getFirstCollisionPoint(const fvec3& src, const fvec3& target, fvec3& collisionPt);
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab expandtab
