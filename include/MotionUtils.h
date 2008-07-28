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

#ifndef __MOTION_UTILS_H_
#define __MOTION_UTILS_H_

#include "common.h"
#include "Flag.h"

float computeAngleVelocity(float old, float desired, float dt );
float computeMaxAngleVelocity ( FlagType *flag, float z );
float computeMaxLinVelocity (  FlagType *flag, float z );
void computeMomentum(float dt, FlagType *flag, float& speed, float& angVel, const float lastSpeed, const float lastAngVel );
void computeFriction(float dt, FlagType *flag, const float *oldVelocity, float *newVelocity);
float computeJumpVelocity ( FlagType *flag );
float computeGroundLimit ( FlagType *flag );

void vecFromAngle2d ( float ang, float vec[3], float mag = 1.0f );

float getMagnitude ( float v[3] );
float getMagnitude ( float p1[3], float p2[3] );
float getMagnitude2d ( float v[2] );
float getMagnitude2d ( float p1[2], float p2[2] );

float getMagnitudeSquare ( float v[3] );
float getMagnitudeSquare ( float p1[3], float p2[3] );
float getMagnitude2dSquare ( float v[2] );
float getMagnitude2dSquare ( float p1[2], float p2[2] );

float getShotVel (ShotType type);

#endif  /*__MOTION_UTILS_H_ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
