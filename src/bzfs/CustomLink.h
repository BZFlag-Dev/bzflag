/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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

// interface header
#include "WorldFileObject.h"

// system headers
#include <iostream>
#include <string>

// common headers
#include "ObstacleMgr.h"
#include "LinkDef.h"

// local headers
#include "WorldInfo.h"


class CustomLink : public WorldFileObject {
  public:
    CustomLink(bool linkSet);
    virtual bool read(const char *cmd, std::istream& input);
    virtual void writeToGroupDef(GroupDefinition*) const;

  protected:
    LinkDef linkDef;

    bool linkSet;
    std::vector<LinkDef> linkDefVec;
};

#endif  /* __CUSTOMLINK_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
