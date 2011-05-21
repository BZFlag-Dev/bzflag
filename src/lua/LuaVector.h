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

#ifndef LUA_VECTOR_H
#define LUA_VECTOR_H
// LuaVector.h: extra simple lua functions
//
//////////////////////////////////////////////////////////////////////

struct lua_State;


class LuaVector {
  public:
    static bool PushEntries(lua_State* L);

  public:
    static int dot(lua_State* L);
    static int cross(lua_State* L);
    static int normdot(lua_State* L);
    static int normdotdegs(lua_State* L);
    static int normdotrads(lua_State* L);
    static int length(lua_State* L);
    static int normalize(lua_State* L);
};


#endif // LUA_VECTOR_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
