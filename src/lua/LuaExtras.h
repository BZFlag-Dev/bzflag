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

#ifndef LUA_EXTRAS_H
#define LUA_EXTRAS_H
// LuaExtras.h: extra simple lua functions
//
//////////////////////////////////////////////////////////////////////

// remove non-standard conflicting ctype.h defines
#include <ctype.h> // make sure it's been loaded
#undef isnumber    // kill the deprecated macro


struct lua_State;


class LuaExtras {
  public:
    static bool PushEntries(lua_State* L);

  public:
    static int tobool(lua_State* L);
    static int isnil(lua_State* L);
    static int isbool(lua_State* L);
    static int isnumber(lua_State* L);
    static int isstring(lua_State* L);
    static int istable(lua_State* L);
    static int isthread(lua_State* L);
    static int isfunction(lua_State* L);
    static int isuserdata(lua_State* L);

    static int flush(lua_State* L);
    static int stderr_write(lua_State* L);

    static int traceback(lua_State* L);

    static int dump(lua_State* L); // can strip as well
    static int listing(lua_State* L);
};


#endif // LUA_EXTRAS_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
