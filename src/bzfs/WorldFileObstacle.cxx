/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include <math.h>
#include <string.h>

#include "WorldFileObstacle.h"

WorldFileObstacle::WorldFileObstacle()
{
    driveThrough = false;
    shootThrough = false;
    ricochet     = false;
}


bool WorldFileObstacle::read(const char *cmd, std::istream& input)
{
    if (strcasecmp(cmd, "drivethrough") == 0)
        driveThrough = true;
    else if (strcasecmp(cmd, "shootthrough") == 0)
        shootThrough = true;
    else if (strcasecmp(cmd, "passable") == 0)
        driveThrough = shootThrough = true;
    else if (strcasecmp(cmd, "ricochet") == 0)
        ricochet = true;
    else
        return WorldFileLocation::read(cmd, input);
    return true;
}

// Local variables: ***
// mode: C++ ***
// tab-width: 4***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
