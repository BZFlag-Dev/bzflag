/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "common.h"
#include <math.h>
#include "TargetingUtils.h"
#include "ShotStrategy.h"
#include "Ray.h"

// These routines are 2 dimensional

void TargetingUtils::getUnitVector( const float *src, const float *target, float unitVector[3] )
{
  unitVector[0] = target[0] - src[0];
  unitVector[1] = target[1] - src[1];
  unitVector[2] = 0.0f;

  float len = (float) sqrt(unitVector[0] * unitVector[0] + 
	                   unitVector[1] * unitVector[1]);
  unitVector[0] /= len;
  unitVector[1] /= len;
}

float TargetingUtils::getTargetDistance( const float *src, const float *target )
{
  float vec[2];

  vec[0] = target[0] - src[0];
  vec[1] = target[1] - src[1];
 
  return (float) sqrt(vec[0] * vec[0] + 
	              vec[1] * vec[1]);
}

float TargetingUtils::getTargetAzimuth( const float *src, const float *target )
{
  return atan2f((target[1] - src[1]), (target[0] - src[0]));
}

float TargetingUtils::getTargetRotation( const float startAzimuth, float targetAzimuth )
{
  float targetRotation = targetAzimuth - startAzimuth;
  if (targetRotation < -1.0f * M_PI) targetRotation += 2.0f * M_PI;
  if (targetRotation > 1.0f * M_PI) targetRotation -= 2.0f * M_PI;

  return targetRotation;
}

float TargetingUtils::getTargetAngleDifference( const float *src, float srcAzimuth, const float *target )
{
  float targetUnitVector[3];
  float srcUnitVector[3];

  getUnitVector(src, target, targetUnitVector);

  srcUnitVector[0] = cos(srcAzimuth);
  srcUnitVector[1] = sin(srcAzimuth);
  srcUnitVector[2] = 0.0f;

  return acos( targetUnitVector[0]*srcUnitVector[0] + targetUnitVector[1]*srcUnitVector[1] );
}

bool TargetingUtils::isLocationObscured( const float *src, const float *target )
{
  float dir[3];
  
  getUnitVector(src, target, dir);

  Ray tankRay( src, dir );
  float targetDistance = getTargetDistance(src, target);
  const Obstacle *building = ShotStrategy::getFirstBuilding(tankRay, -0.5f, targetDistance);
  return building != NULL;
}

float TargetingUtils::getOpenDistance( const float *src, const float azimuth )
{
  float t = MAXFLOAT; //Some constant?

  float dir[3] = { cosf(azimuth), sinf(azimuth), 0.0f };
  *((float *) &src[2]) += 0.1f; //Don't hit building because your sitting on one
  Ray tankRay( src, dir );
  *((float *) &src[2]) -= 0.1f;
  ShotStrategy::getFirstBuilding(tankRay, -0.5f, t);
  return t;
}
