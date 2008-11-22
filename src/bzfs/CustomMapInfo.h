/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_MAP_INFO_H__
#define __CUSTOM_MAP_INFO_H__

/* interface header */
#include "WorldFileObstacle.h"

/* local interface headers */
#include "WorldInfo.h"

/* common interface headers */
#include "BzMaterial.h"


class CustomMapInfo : public WorldFileObject {
  public:
    CustomMapInfo();
    ~CustomMapInfo();
    virtual bool read(const char* cmd, std::istream& input);
    
    bool usesManager() { return false; }
    bool usesGroupDef() { return false; }

    void writeToWorld(WorldInfo*) const;

  private:
    std::vector<std::string> lines;
};


#endif  /* __CUSTOM_MAP_INFO_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
