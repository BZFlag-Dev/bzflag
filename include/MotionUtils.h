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

#endif  /*__MOTION_UTILS_H_ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
