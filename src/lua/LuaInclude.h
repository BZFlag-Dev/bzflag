#ifndef LUA_INCLUDE_H
#define LUA_INCLUDE_H

extern "C" {
  #include "../other/lua/src/lua.h"
  #include "../other/lua/src/lualib.h"
  #include "../other/lua/src/lauxlib.h"
}

#include <string>   // FIXME -- LuaLog ?
#include <stdarg.h> // FIXME -- LuaLog ?

/******************************************************************************/

#define PUSH_LUA_CFUNC(L, x)  \
  lua_pushliteral(L, #x);  \
  lua_pushcfunction(L, x); \
  lua_rawset(L, -3)

/******************************************************************************/

struct LuaHeader {
  class LuaHandle* lh;
  bool hasFullRead;
  bool hasInputCtrl;
};


inline LuaHeader* luaBZ_toheader(lua_State* L)
{
  return (LuaHeader*)((char*)L - LUAI_EXTRASPACE);
}


/******************************************************************************/

inline void LuaLog(const std::string& msg) // FIXME
{
  printf(msg.c_str());
  fflush(stdout);
}


inline void LuaLog(const char* fmt, ...) // FIXME
{
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  fflush(stdout);
}


/******************************************************************************/

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


inline int lua_toint(lua_State* L, int idx)
{
  return (int)lua_tointeger(L, idx);
}


inline bool lua_tobool(lua_State* L, int index)
{
  return lua_toboolean(L, index) != 0;
}


inline float lua_tofloat(lua_State* L, int idx)
{
  return (float)lua_tonumber(L, idx);
}


inline float luaL_checkfloat(lua_State* L, int idx)
{
  return (float)luaL_checknumber(L, idx);
}


inline float luaL_optfloat(lua_State* L, int idx, float def)
{
  return (float)luaL_optnumber(L, idx, def);
}


inline void lua_pushstdstring(lua_State* L, const std::string& s)
{
  lua_pushlstring(L, s.data(), s.size());
}


/******************************************************************************/

// FIXME -- IntToString(), move it
static inline std::string IntToString(int i, const std::string& format = "%i")
{
  char buf[64];
  snprintf(buf, sizeof(buf), format.c_str(), i);
  return std::string(buf);  
}
      

/******************************************************************************/

#endif // LUA_INCLUDE_H
