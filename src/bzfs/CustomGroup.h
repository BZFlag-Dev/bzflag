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

#ifndef __CUSTOM_GROUP_H__
#define __CUSTOM_GROUP_H__

// Inherits from
#include "WorldFileObstacle.h"

/* system interface headers */
#include <string>
#include <vector>
#include <iostream>

/* local interface headers */
#include "WorldInfo.h"

class GroupInstance;

class CustomGroup : public WorldFileObstacle
{
public:
    CustomGroup(const std::string& groupdef);
    ~CustomGroup();
    bool read(const char *cmd, std::istream&) override;
    void writeToGroupDef(GroupDefinition*) override;

protected:
    mutable GroupInstance* group;
};

#endif  /* __CUSTOM_GROUP_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 4***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
