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

#include "BeOSMedia.h"
#include "BzfMedia.h"
#include <sys/time.h>

BeOSMedia::BeOSMedia() : BzfMedia(), stopWatchStart(0)
{
    // do nothing
}

BeOSMedia::~BeOSMedia()
{
    // do nothing
}

double                  BeOSMedia::stopwatch(bool start)
{
    return BzfMedia::stopwatch(start); // not better
    if (start)
    {
        stopWatchStart = system_time();
        return 0.0;
    }
    return ((double)(system_time() - stopWatchStart)) / 1.0e6;
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
