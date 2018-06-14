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

#include "SolarisMedia.h"
//#include "TimeKeeper.h"

//
// SolarisMedia
//

SolarisMedia::SolarisMedia() : BzfMedia()
{
    // do nothing
}

SolarisMedia::~SolarisMedia()
{
    // do nothing
}

double          SolarisMedia::stopwatch(bool start)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    if (start)
    {
        stopwatchTime = (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec;
        return 0.0;
    }
    return (double)tv.tv_sec + 1.0e-6 * (double)tv.tv_usec - stopwatchTime;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
