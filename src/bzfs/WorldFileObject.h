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
#ifndef __WORLDFILEOBJECT_H__
#define __WORLDFILEOBJECT_H__

// system headers
#include <iostream>
#include <vector>
#include <string>

class WorldInfo;


class WorldFileObject {
  public:
    WorldFileObject();
    virtual ~WorldFileObject() { }

    virtual bool read(const char *cmd, std::istream&);
    virtual void write(WorldInfo*) const = 0;
    virtual bool writeImmediately() { return false; }
    
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
