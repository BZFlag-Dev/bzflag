#ifndef LUA_INCLUDE_H
#define LUA_INCLUDE_H

#include "common.h"

// system headers
#include <string>

// common headers
#include "vectors.h"

// liblua headers
#include "../other/lua/src/lua.h"
#include "../other/lua/src/lualib.h"
#include "../other/lua/src/lauxlib.h"


//============================================================================//

#define PUSH_LUA_CFUNC(L, x)  \
	lua_pushliteral(L, #x);  \
	lua_pushcfunction(L, x); \
	lua_rawset(L, -3)


//============================================================================//

extern void LuaLog(int debugLevel, const std::string& msg);
extern void LuaLog(int debugLevel, const char* fmt, ...);


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


inline void lua_pushstdstring(lua_State* L, const std::string& s)
{
	lua_pushlstring(L, s.data(), s.size());
}


//============================================================================//

#endif // LUA_INCLUDE_H
