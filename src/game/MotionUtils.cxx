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

/* interface header */

#include "MotionUtils.h"

float computeAngleVelocity(float old, float desired, float dt )
{
  float newAngVel;
  float frames = 1.0f;

  // keyboard users
  if ((old * desired < 0.0f) || // reversed direction
    (NEAR_ZERO(desired, ZERO_TOLERANCE)) || // stopped
    (NEAR_ZERO(old - desired, ZERO_TOLERANCE)))
  { // close enough
	newAngVel = desired;
  }
  else 
  {
    /* dampened turning for aim control */

    /* mildly fudgey factor controls turn rate dampening.  should
    * converge roughly within converge and converge*converge
    * seconds (assuming converge is < 1).
    *
    * it would converge within .5 but we didn't accelerate
    * non-linearly by combining additional previous velocity, so
    * it's generally around sqrt(converge) * converge seconds
    * (i.e., around .35 sec for converge of .5)
    */
    static const float converge = 0.5f;

    /* spread out dampening over this many frames */
    frames = converge / dt;
    if (frames < 1.0f)
	frames = 1.0f; // framerate too low

    /* accelerate towards desired */
    newAngVel = (old + (old / frames)) + (desired / frames);

    // if reached desired, clamp it
    if (desired > 0) 
    {
	if (newAngVel > desired) 
	  newAngVel = desired;
    } 
    else
    {
	if (newAngVel < desired) 
	  newAngVel = desired;
    }
  }
  return newAngVel;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
