
#include "common.h"

// implementation header
#include "LuaScream.h"

// system headers
#include <string>
using std::string;

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

LuaScream::LuaScream()
{
}


LuaScream::~LuaScream()
{
}


/******************************************************************************/
/******************************************************************************/

bool LuaScream::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateScream);

	return true;
}


/******************************************************************************/
/******************************************************************************/

bool LuaScream::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, "Scream");
	HSTR_PUSH_CFUNC(L, "__gc",       meta_gc);
	HSTR_PUSH_CFUNC(L, "__index",    meta_index);
	HSTR_PUSH_CFUNC(L, "__newindex", meta_newindex);
	lua_pop(L, 1);
	return true;
}


int LuaScream::meta_gc(lua_State* L)
{
	int* refPtr = (int*)luaL_checkudata(L, 1, "Scream");
	lua_rawgeti(L, LUA_REGISTRYINDEX, *refPtr);
	if (lua_isfunction(L, -1)) {
		const int error = lua_pcall(L, 0, 0, 0);
		if (error != 0) {
			LuaLog("Scream: error(%i) = %s", error, lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
	else if (lua_israwstring(L, -1)) {
		LuaLog("SCREAM: %s", lua_tostring(L, -1));
	}
	luaL_unref(L, LUA_REGISTRYINDEX, *refPtr);
	return 0;
}


int LuaScream::meta_index(lua_State* L)
{
	int* refPtr = (int*)luaL_checkudata(L, 1, "Scream");
	const string key = luaL_checkstring(L, 2);
	if (key == "func") {
		lua_rawgeti(L, LUA_REGISTRYINDEX, *refPtr);
		return 1;
	}
	return 0;
}


int LuaScream::meta_newindex(lua_State* L)
{
	int* refPtr = (int*)luaL_checkudata(L, 1, "Scream");
	const string key = luaL_checkstring(L, 2);
	if (key == "func") {
		lua_pushvalue(L, 3);
		lua_rawseti(L, LUA_REGISTRYINDEX, *refPtr);
	}
	return 0;
}


/******************************************************************************/
/******************************************************************************/

int LuaScream::CreateScream(lua_State* L)
{
	if (lua_isnoneornil(L, 1)) {
		luaL_error(L, "Incorrect arguments to CreateScream()");
	}

	int* refPtr = (int*)lua_newuserdata(L, sizeof(int));
	luaL_getmetatable(L, "Scream");
	lua_setmetatable(L, -2);

	lua_pushvalue(L, 1);
	*refPtr = luaL_ref(L, LUA_REGISTRYINDEX);

	return 1;
}


/******************************************************************************/
/******************************************************************************/
