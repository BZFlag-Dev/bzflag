/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_GROUP_H__
#define __CUSTOM_GROUP_H__

#include "common.h"

/* system interface headers */
#include <string>
#include <vector>
#include <iostream>

/* local interface headers */
#include "WorldInfo.h"
#include "WorldFileLocation.h"


class CustomGroup : public WorldFileLocation {
  public:
    CustomGroup(const std::string& groupdef);
    virtual bool read(const char *cmd, std::istream&);
    virtual void writeToGroupDef(GroupDefinition*) const;

  protected:
    std::string groupdef;
};

#endif  /* __CUSTOM_GROUP_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
