
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

#ifndef _TARGETINGUTILS__H_
#define _TARGETINGUTILS__H_

class TargetingUtils
{
public:
  static void getUnitVector( const float *src, const float *target, float unitVector[3] );
  static float getTargetDistance( const float *src, const float *target );
  static float getTargetAzimuth( const float *src, const float *target );
  static float getTargetRotation( const float startAzimuth, float targetAzimuth );
  static float getTargetAngleDifference( const float *src, float srcAzimuth, const float *target );
  static bool isLocationObscured( const float *src, const float *target );
  static float getOpenDistance( const float *src, const float azimuth );
  static bool getFirstCollisionPoint( const float *src, const float *target, float *collisionPt );
};

#endif
