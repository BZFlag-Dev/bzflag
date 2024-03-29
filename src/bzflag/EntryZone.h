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

#ifndef __ENTRYZONE_H__
#define __ENTRYZONE_H__

// common should be the 1st one
#include "common.h"

// system headers
#include <vector>

// common headers
#include "Flag.h"
#include "global.h"

class EntryZone
{
public:
    EntryZone();
    const void* unpack(const void*);

public:
    float pos[3];
    float size[3];
    float rot;
    std::vector<FlagType*> flags;
    std::vector<TeamColor> teams;
    std::vector<TeamColor> safety;
};


#endif /* __ENTRYZONE_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
