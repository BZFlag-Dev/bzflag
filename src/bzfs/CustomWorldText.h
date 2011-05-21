/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CUSTOM_WORLD_TEXT__
#define __CUSTOM_WORLD_TEXT__

/* interface header */
#include "WorldFileLocation.h"

/* common headers */
#include "WorldText.h"
#include "BzMaterial.h"

class CustomWorldText : public WorldFileLocation {
  public:
    CustomWorldText();
    ~CustomWorldText();
    virtual bool read(const char* cmd, std::istream& input);
    virtual void writeToGroupDef(GroupDefinition*) const;

  private:
    mutable WorldText* text;
    BzMaterial material;
    bool upright;
};

#endif  /* __CUSTOM_WORLD_TEXT__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
