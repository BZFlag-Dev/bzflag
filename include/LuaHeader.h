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

#ifndef __LUA_HEADER_H__
#define __LUA_HEADER_H__


// system headers
#include <string>

// common headers
#include "vectors.h"

// lua headers
#include "../src/other/lua/src/lua.h"
#include "../src/other/lua/src/lualib.h"
#include "../src/other/lua/src/lauxlib.h"


//============================================================================//

class LuaHandle;


struct LuaVfsModes {
  const char* readDefault;
  const char* readAllowed;
  const char* writeDefault;
  const char* writeAllowed;
};


// returns 'true' if the named variable can be accessed
typedef bool (*LuaBzdbCheckFunc)(const std::string& name);


// prepended to the beginning of lua_State's  (see lua_extra.h)
struct LuaExtraSpace {
  LuaHandle*  handle;
  LuaHandle** handlePtr;
  const LuaVfsModes* vfsModes;
  LuaBzdbCheckFunc bzdbReadCheck;  // NULL allows access
  LuaBzdbCheckFunc bzdbWriteCheck; // NULL allows access
};


// L2ES(L)  --  lua_State* to LuaExtraSpace*
static inline LuaExtraSpace* L2ES(lua_State* L)
{
  return (LuaExtraSpace*)((char*)L - LUAI_EXTRASPACE);
}


// L2H(L)  --  lua_State* to LuaHandle*
static inline LuaHandle* L2H(lua_State* L)
{
  return L2ES(L)->handle;
}


// L2HP(L)  --  lua_State* to LuaHandle**
static inline LuaHandle** L2HP(lua_State* L)
{
  return L2ES(L)->handlePtr;
}


//============================================================================//

extern void LuaLog(int debugLevel, const std::string& msg);
extern void LuaLog(int debugLevel, const char* fmt, ...);


//============================================================================//

#define PUSH_LUA_CFUNC(L, x)  \
  lua_pushliteral(L, #x);  \
  lua_pushcfunction(L, x); \
  lua_rawset(L, -3)


//============================================================================//

inline bool lua_israwnumber(lua_State* L, int idx)
{
  return (lua_type(L, idx) == LUA_TNUMBER);
}


inline bool lua_israwstring(lua_State* L, int idx)
{
  return (lua_type(L, idx) == LUA_TSTRING);
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


inline int luaL_pushnil(lua_State* L)
{
  lua_pushnil(L);
  return 1;
}


inline int luaL_pushnilstring(lua_State* L, const char* str)
{
  lua_pushnil(L);
  lua_pushstring(L, str);
  return 2;
}


//============================================================================//

inline int lua_toint(lua_State* L, int idx) {
  return (int)lua_tointeger(L, idx);
}

inline bool lua_tobool(lua_State* L, int idx) {
  return lua_toboolean(L, idx) != 0;
}

inline float lua_tofloat(lua_State* L, int idx) {
  return (float)lua_tonumber(L, idx);
}

inline double lua_todouble(lua_State* L, int idx) {
  return (double)lua_tonumber(L, idx);
}

inline std::string lua_tostdstring(lua_State* L, int idx) {
  size_t len;
  const char* s = lua_tolstring(L, idx, &len);
  return std::string(s, len);
}


inline void lua_pushint(lua_State* L, int value) {
  lua_pushinteger(L, (lua_Integer)value);
}

inline void lua_pushbool(lua_State* L, bool value) {
  lua_pushboolean(L, value);
}

inline void lua_pushfloat(lua_State* L, float value) {
  lua_pushnumber(L, (lua_Number)value);
}

inline void lua_pushdouble(lua_State* L, double value) {
  lua_pushnumber(L, (lua_Number)value);
}

inline void lua_pushstdstring(lua_State* L, const std::string& value)
{
  lua_pushlstring(L, value.data(), value.size());
}


inline float luaL_checkfloat(lua_State* L, int idx) {
  return (float)luaL_checknumber(L, idx);
}

inline double luaL_checkdouble(lua_State* L, int idx) {
  return (double)luaL_checknumber(L, idx);
}

inline std::string luaL_checkstdstring(lua_State* L, int idx) {
  size_t len;
  const char* s = luaL_checklstring(L, idx, &len);
  return std::string(s, len);
}


inline float luaL_optfloat(lua_State* L, int idx, float def) {
  return (float)luaL_optnumber(L, idx, (lua_Number)def);
}

inline double luaL_optdouble(lua_State* L, int idx, double def) {
  return (double)luaL_optnumber(L, idx, (lua_Number)def);
}

inline std::string luaL_optstdstring(lua_State* L, int idx,
                                     const std::string& def) {
  size_t len;
  const char* s = luaL_optlstring(L, idx, def.c_str(), &len);
  return std::string(s, len);
}


//============================================================================//

inline fvec2 luaL_checkfvec2(lua_State* L, int idx)
{
  return fvec2((float)luaL_checknumber(L, idx + 0),
               (float)luaL_checknumber(L, idx + 1));
}

inline fvec3 luaL_checkfvec3(lua_State* L, int idx)
{
  return fvec3((float)luaL_checknumber(L, idx + 0),
               (float)luaL_checknumber(L, idx + 1),
               (float)luaL_checknumber(L, idx + 2));
}

inline fvec4 luaL_checkfvec4(lua_State* L, int idx)
{
  return fvec4((float)luaL_checknumber(L, idx + 0),
               (float)luaL_checknumber(L, idx + 1),
               (float)luaL_checknumber(L, idx + 2),
               (float)luaL_checknumber(L, idx + 3));
}


inline fvec2 luaL_optfvec2(lua_State* L, int idx, const fvec2& def)
{
  return fvec2((float)luaL_optnumber(L, idx + 0, def.x),
               (float)luaL_optnumber(L, idx + 1, def.y));
}

inline fvec3 luaL_optfvec3(lua_State* L, int idx, const fvec3& def)
{
  return fvec3((float)luaL_optnumber(L, idx + 0, def.x),
               (float)luaL_optnumber(L, idx + 1, def.y),
               (float)luaL_optnumber(L, idx + 2, def.z));
}

inline fvec4 luaL_optfvec4(lua_State* L, int idx, const fvec4& def)
{
  return fvec4((float)luaL_optnumber(L, idx + 0, def.x),
               (float)luaL_optnumber(L, idx + 1, def.y),
               (float)luaL_optnumber(L, idx + 2, def.z),
               (float)luaL_optnumber(L, idx + 3, def.w));
}


inline void lua_pushfvec2(lua_State* L, const fvec2& v)
{
  lua_pushnumber(L, v.x);
  lua_pushnumber(L, v.y);
}

inline void lua_pushfvec3(lua_State* L, const fvec3& v)
{
  lua_pushnumber(L, v.x);
  lua_pushnumber(L, v.y);
  lua_pushnumber(L, v.z);
}

inline void lua_pushfvec4(lua_State* L, const fvec4& v)
{
  lua_pushnumber(L, v.x);
  lua_pushnumber(L, v.y);
  lua_pushnumber(L, v.z);
  lua_pushnumber(L, v.w);
}


//============================================================================//

inline void luaset_strstr(lua_State* L, const std::string& k, const std::string& v)
{
  lua_pushstdstring(L, k); lua_pushstdstring(L, v); lua_rawset(L, -3);
}

inline void luaset_strnum(lua_State* L, const std::string& k, lua_Number v)
{
  lua_pushstdstring(L, k); lua_pushnumber(L, v); lua_rawset(L, -3);
}

inline void luaset_strint(lua_State* L, const std::string& k, int v)
{
  lua_pushstdstring(L, k); lua_pushint(L, v); lua_rawset(L, -3);
}

inline void luaset_strbool(lua_State* L, const std::string& k, bool v)
{
  lua_pushstdstring(L, k); lua_pushboolean(L, v); lua_rawset(L, -3);
}

inline void luaset_strfunc(lua_State* L, const std::string& k, lua_CFunction v)
{
  lua_pushstdstring(L, k); lua_pushcfunction(L, v); lua_rawset(L, -3);
}


inline void luaset_numstr(lua_State* L, lua_Number k, const std::string& v)
{
  lua_pushnumber(L, k); lua_pushstdstring(L, v); lua_rawset(L, -3);
}

inline void luaset_numnum(lua_State* L, lua_Number k, lua_Number v)
{
  lua_pushnumber(L, k); lua_pushnumber(L, v); lua_rawset(L, -3);
}

inline void luaset_numint(lua_State* L, lua_Number k, int v)
{
  lua_pushnumber(L, k); lua_pushint(L, v); lua_rawset(L, -3);
}

inline void luaset_numbool(lua_State* L, lua_Number k, bool v)
{
  lua_pushnumber(L, k); lua_pushboolean(L, v); lua_rawset(L, -3);
}

inline void luaset_numfunc(lua_State* L, lua_Number k, lua_CFunction v)
{
  lua_pushnumber(L, k); lua_pushcfunction(L, v); lua_rawset(L, -3);
}


inline void luaset_intstr(lua_State* L, int k, const std::string& v)
{
  lua_pushstdstring(L, v); lua_rawseti(L, -2, k);
}

inline void luaset_intnum(lua_State* L, int k, lua_Number v)
{
  lua_pushnumber(L, v); lua_rawseti(L, -2, k);
}

inline void luaset_intint(lua_State* L, int k, int v)
{
  lua_pushint(L, v); lua_rawseti(L, -2, k);
}

inline void luaset_intbool(lua_State* L, int k, bool v)
{
  lua_pushboolean(L, v); lua_rawseti(L, -2, k);
}

inline void luaset_intfunc(lua_State* L, int k, lua_CFunction v)
{
  lua_pushcfunction(L, v); lua_rawseti(L, -2, k);
}


//============================================================================//


#endif // __LUA_HEADER_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
