/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOMLINK_H__
#define __CUSTOMLINK_H__

/* interface header */
#include "WorldFileObject.h"

/* system interface headers */
#include <iostream>
#include <string>

/* local interface headers */
#include "WorldInfo.h"


class CustomLink final : public WorldFileObject
{
public:
    CustomLink();
    bool read(const char *cmd, std::istream& input) override;
    void writeToWorld(WorldInfo*) const override;
    bool usesGroupDef() override
    {
        return false;
    }

protected:
    std::string from;
    std::string to;
};

#endif  /* __CUSTOMLINK_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 4***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
