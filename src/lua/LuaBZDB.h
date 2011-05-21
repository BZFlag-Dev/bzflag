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

#ifndef LUA_BZDB_H
#define LUA_BZDB_H


#include <string>

struct lua_State;


class LuaBZDB {
  public:
    static bool PushEntries(lua_State* L);

  private:
    static int GetMap(lua_State* L);
    static int GetList(lua_State* L);

    static int Exists(lua_State* L);
    static int IsPersistent(lua_State* L);
    static int GetDefault(lua_State* L);
    static int GetPermission(lua_State* L);

    static int GetInt(lua_State* L);
    static int GetBool(lua_State* L);
    static int GetFloat(lua_State* L);
    static int GetString(lua_State* L);

    static int SetInt(lua_State* L);
    static int SetBool(lua_State* L);
    static int SetFloat(lua_State* L);
    static int SetString(lua_State* L);

    static int Unset(lua_State* L);
};


#endif // LUA_BZDB_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
