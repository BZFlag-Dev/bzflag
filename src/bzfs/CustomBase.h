/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOMBASE_H__
#define __CUSTOMBASE_H__

#include "common.h"

/* interface header */
#include "WorldFileObstacle.h"

/* system interface headers */
#include <iostream>

/* local interface headers */
#include "WorldInfo.h"


class CustomBase : public WorldFileObstacle {
  public:
    CustomBase();
    virtual bool read(const char *cmd, std::istream&);
    virtual void writeToGroupDef(GroupDefinition*) const;

  protected:
    int color;
    bool triggerWorldWep;
    std::string worldWepType;
};

#endif  /* __CUSTOMBASE_H__ */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
