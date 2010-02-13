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

#ifndef __MOTION_UTILS_H_
#define __MOTION_UTILS_H_

#include "common.h"

// common headers
#include "Flag.h"
#include "vectors.h"

// FIXME -- why global? use a namespace or class

float computeAngleVelocity(float old, float desired, float dt);
float computeMaxAngleVelocity(FlagType* flag, float z);
float computeMaxLinVelocity( FlagType* flag, float z);
void  computeMomentum(float dt, FlagType* flag,
                      float& speed, float& angVel,
                      const float lastSpeed, const float lastAngVel);
void  computeFriction(float dt, FlagType* flag,
                      const fvec3& oldVelocity, fvec3& newVelocity);
float computeJumpVelocity(FlagType* flag);
float computeGroundLimit(FlagType* flag);

#endif  /*__MOTION_UTILS_H_ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
