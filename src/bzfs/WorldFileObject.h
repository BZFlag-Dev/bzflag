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
#ifndef __WORLDFILEOBJECT_H__
#define __WORLDFILEOBJECT_H__

#include "common.h"

// system headers
#include <iostream>
#include <vector>
#include <string>


class WorldInfo;
class GroupDefinition;


class WorldFileObject {
  public:
    WorldFileObject();
    virtual ~WorldFileObject() { }

    virtual bool read(const char *cmd, std::istream&);

    virtual bool usesManager() { return false; }
    virtual bool usesGroupDef() { return true; }
    virtual void writeToWorld(WorldInfo*) const;
    virtual void writeToManager() const;
    virtual void writeToGroupDef(GroupDefinition*) const;

  protected:
    std::string name;
};


void emptyWorldFileObjectList(std::vector<WorldFileObject*>& wlist);

#endif  /* __WORLDFILEOBJECT_H__ */

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
