/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOMGATE_H__
#define __CUSTOMGATE_H__

/* interface header */
#include "WorldFileObstacle.h"

/* system interface headers */
#include <string>
#include <iostream>

/* local interface headers */
#include "WorldInfo.h"


class CustomGate : public WorldFileObstacle {
  public:
    CustomGate(const char* telename);
    virtual bool read(const char *cmd, std::istream&);
    virtual void writeToGroupDef(GroupDefinition*) const;

  protected:
    std::string telename;
    float border;
    bool horizontal;
};

#endif  /* __CUSTOMGATE_H__ */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
