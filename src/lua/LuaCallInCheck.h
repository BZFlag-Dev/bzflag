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

#ifndef LUA_CALL_IN_CHECK_H
#define LUA_CALL_IN_CHECK_H


struct lua_State;


class LuaCallInCheck {
  public:
    LuaCallInCheck(lua_State* L, const char* funcName);
    ~LuaCallInCheck();

  private:
    lua_State* L;
    int startTop;
    const char* funcName;
};


#ifdef DEBUG_LUA
#  define LUA_CALL_IN_CHECK(L, args) \
    LuaCallInCheck ciCheck((L), __FUNCTION__); \
    lua_checkstack(L, args + 2);
#else
#  define LUA_CALL_IN_CHECK(L, args) \
    lua_checkstack(L, args + 2);
#endif


#endif // LUA_CALL_IN_CHECK_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
