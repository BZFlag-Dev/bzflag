/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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

// system headers
#include <iostream>
#include <vector>
#include <string>


class WorldInfo;
class GroupDefinition;


class WorldFileObject
{
public:
    WorldFileObject();
    virtual ~WorldFileObject() {}

    virtual bool read(const char *cmd, std::istream&);

    virtual bool usesManager()
    {
        return false;
    }
    virtual bool usesGroupDef()
    {
        return true;
    }
    virtual void writeToWorld(WorldInfo*) const;
    virtual void writeToManager() const;
    virtual void writeToGroupDef(GroupDefinition*) const;

    virtual int getLineCount() const;

protected:
    std::string name;
    int lines;
};

void emptyWorldFileObjectList(std::vector<WorldFileObject*>& wlist);

#endif  /* __WORLDFILEOBJECT_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 4***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
