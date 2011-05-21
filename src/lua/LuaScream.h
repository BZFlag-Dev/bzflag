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

#ifndef LUA_SCREAM_H
#define LUA_SCREAM_H

struct lua_State;


class LuaScream {
  public:
    LuaScream();
    ~LuaScream();

    static bool PushEntries(lua_State* L);

  public:
    static const char* metaName;

  private: // metatable methods
    static bool CreateMetatable(lua_State* L);
    static int MetaGC(lua_State* L);
    static int MetaIndex(lua_State* L);
    static int MetaNewindex(lua_State* L);

  private:
    static int* GetScreamRef(lua_State* L, int index);

  private: // call-outs
    static int CreateScream(lua_State* L);
};


#endif // LUA_SCREAM_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
