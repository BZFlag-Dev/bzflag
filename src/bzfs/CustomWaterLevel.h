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

#ifndef __CUSTOM_WATER_LEVEL_H__
#define __CUSTOM_WATER_LEVEL_H__

/* interface header */
#include "WorldFileObject.h"

/* common headers */
#include "BzMaterial.h"

class CustomWaterLevel : public WorldFileObject {
  public:
    CustomWaterLevel();
    ~CustomWaterLevel();
    virtual bool read(const char *cmd, std::istream& input);
    virtual void writeToWorld(WorldInfo*) const;
    virtual bool usesGroupDef() { return false; }
  private:
    float height;
    bool modedMaterial;
    mutable BzMaterial material;
};

#endif  /* __CUSTOM_WATER_LEVEL_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
