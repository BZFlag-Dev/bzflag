#pragma once
/* bzflag
* Copyright (c) 1993-2018 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _ROBOTUTILS__H_
#define _ROBOTUTILS__H_

namespace BotUtils
{
    float normalizeAngle(float ang);
    void getUnitVector(const float *src, const float *target, float unitVector[3]);
    void get3DUnitVector(const float *src, const float *target, float unitVector[3]);
    float getTargetDistance(const float *src, const float *target);
    float getTargetAzimuth(const float *src, const float *target);
    float getTargetRotation(const float startAzimuth, float targetAzimuth);
    float getTargetAngleDifference(const float *src, float srcAzimuth, const float *target);
    bool isLocationObscured(const float *src, const float *target);
    float getOpenDistance(const float *src, const float azimuth);
    bool getFirstCollisionPoint(const float *src, const float *target, float *collisionPt);
}

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
