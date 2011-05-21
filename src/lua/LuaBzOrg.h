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

#ifndef LUA_BZORG_H
#define LUA_BZORG_H

#include "LuaHandle.h"

#include <string>


class LuaBzOrg : public LuaHandle {
    friend class CodeFetch;

  public:
    static void LoadHandler();
    static void FreeHandler();

    static bool IsActive();

  private:
    LuaBzOrg(const std::string& code, const std::string& url);
    ~LuaBzOrg();
};


extern LuaBzOrg* luaBzOrg;


#endif // LUA_BZORG_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
