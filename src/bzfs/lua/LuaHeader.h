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

#ifndef LUA_INCLUDE_H
#define LUA_INCLUDE_H


#include <string>

#include "../../other/lua/src/lua.h"
#include "../../other/lua/src/lualib.h"
#include "../../other/lua/src/lauxlib.h"


//============================================================================//

#define PUSH_LUA_CFUNC(L, x)  \
  lua_pushliteral(L, #x);  \
  lua_pushcfunction(L, x); \
  lua_rawset(L, -3)


//============================================================================//

inline bool lua_israwnumber(lua_State* L, int index)
{
  return (lua_type(L, index) == LUA_TNUMBER);
}


inline bool lua_israwstring(lua_State* L, int index)
{
  return (lua_type(L, index) == LUA_TSTRING);
}


inline int lua_checkgeti(lua_State* L, int idx, int n)
{
  lua_rawgeti(L, idx, n);
  if (lua_isnoneornil(L, -1)) {
    lua_pop(L, 1);
    return 0;
  }
  return 1;
}


inline int lua_toint(lua_State* L, int idx) {
  return (int)lua_tointeger(L, idx);
}

inline bool lua_tobool(lua_State* L, int index) {
  return lua_toboolean(L, index) != 0;
}

inline float lua_tofloat(lua_State* L, int idx) {
  return (float)lua_tonumber(L, idx);
}

inline double lua_todouble(lua_State* L, int idx) {
  return (double)lua_tonumber(L, idx);
}


inline void lua_pushfloat(lua_State* L, float value) {
  lua_pushnumber(L, (lua_Number)value);
}

inline void lua_pushdouble(lua_State* L, double value) {
  lua_pushnumber(L, (lua_Number)value);
}


inline float luaL_checkfloat(lua_State* L, int idx) {
  return (float)luaL_checknumber(L, idx);
}

inline double luaL_checkdouble(lua_State* L, int idx) {
  return (double)luaL_checknumber(L, idx);
}


inline float luaL_optfloat(lua_State* L, int idx, float def) {
  return (float)luaL_optnumber(L, idx, def);
}

inline double luaL_optdouble(lua_State* L, int idx, double def) {
  return (double)luaL_optnumber(L, idx, def);
}


inline void lua_pushstdstring(lua_State* L, const std::string& value)
{
  lua_pushlstring(L, value.data(), value.size());
}


//============================================================================//


#endif // LUA_INCLUDE_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
