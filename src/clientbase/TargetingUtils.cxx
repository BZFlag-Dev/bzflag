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

// interface header
#include "TargetingUtils.h"

// system headers
#include <math.h>

// common headers
#include "Ray.h"

// local headers
#include "ShotStrategy.h"

// These routines are 2 dimensional

float TargetingUtils::normalizeAngle(float ang)
{
  if (ang < (float)-M_PI) { ang += (float)(2.0 * M_PI); }
  if (ang > (float)+M_PI) { ang -= (float)(2.0 * M_PI); }
  return ang;
}


void TargetingUtils::getUnitVector(const fvec3& src, const fvec3& target, fvec3& unitVector)
{
  unitVector.xy() = target.xy() - src.xy();
  unitVector.z = 0.0f;
  fvec2::normalize(unitVector.xy());
}


void TargetingUtils::get3DUnitVector(const fvec3& src,
                                     const fvec3& target, fvec3& unitVector)
{
  unitVector = (target - src).normalize();
}


float TargetingUtils::getTargetDistance(const fvec3& src, const fvec3& target)
{
  return (src.xy() - target.xy()).length();
}


float TargetingUtils::getTargetAzimuth(const fvec3& src, const fvec3& target)
{
  const fvec2 diff = (target.xy() - src.xy());
  return atan2f(diff.y, diff.x);
}


float TargetingUtils::getTargetRotation(float startAzimuth,
                                        float targetAzimuth)
{
  float targetRotation = targetAzimuth - startAzimuth;
  if (targetRotation < (float)-M_PI) { targetRotation += (float)(2.0 * M_PI); }
  if (targetRotation > (float)+M_PI) { targetRotation -= (float)(2.0 * M_PI); }

  return targetRotation;
}


float TargetingUtils::getTargetAngleDifference(const fvec3& src,
                                               float srcAzimuth,
                                               const fvec3& target)
{
  fvec3 targetUnitVector;
  fvec3 srcUnitVector;
  getUnitVector(src, target, targetUnitVector);

  srcUnitVector.x = cosf(srcAzimuth);
  srcUnitVector.y = sinf(srcAzimuth);
  srcUnitVector.z = 0.0f;

  return acos(fvec2::dot(targetUnitVector.xy(), srcUnitVector.xy()));
}


bool TargetingUtils::isLocationObscured(const fvec3& src, const fvec3& target)
{
  fvec3 dir;
  getUnitVector(src, target, dir);

  Ray tankRay(src, dir);
  float targetDistance = getTargetDistance(src, target);
  const Obstacle *building =
    ShotStrategy::getFirstBuilding(tankRay, -0.5f, targetDistance);
  return building != NULL;
}


float TargetingUtils::getOpenDistance(const fvec3& src, const float azimuth)
{

  fvec3 dir(cosf(azimuth), sinf(azimuth), 0.0f);

  const_cast<float&>(src.z) += 0.1f; //Don't hit building because your sitting on one
  Ray tankRay(src, dir);
  const_cast<float&>(src.z) -= 0.1f;

  float t = MAXFLOAT; //Some constant?
  ShotStrategy::getFirstBuilding(tankRay, -0.5f, t);
  return t;
}


bool TargetingUtils::getFirstCollisionPoint(const fvec3& src,
                                            const fvec3& target,
                                            fvec3& collisionPt)
{
  float t = MAXFLOAT;
  fvec3 dir;
  get3DUnitVector(src, target, dir);

  Ray tankRay(src, dir);
  const Obstacle *building = ShotStrategy::getFirstBuilding(tankRay, 0.0f, t);
  if (building == NULL) {
    return false;
  }

  collisionPt = src + (t * dir);
  return true;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
