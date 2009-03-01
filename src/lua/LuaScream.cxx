
#include "common.h"

// interface header
#include "LuaScream.h"

// system headers
#include <string>
using std::string;

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


const char* LuaScream::metaName = "Scream";


//============================================================================//
//============================================================================//

LuaScream::LuaScream()
{
}


LuaScream::~LuaScream()
{
}


//============================================================================//
//============================================================================//

bool LuaScream::PushEntries(lua_State* L)
{
	CreateMetatable(L);

	PUSH_LUA_CFUNC(L, CreateScream);

	return true;
}


//============================================================================//
//============================================================================//

bool LuaScream::CreateMetatable(lua_State* L)
{
	luaL_newmetatable(L, "Scream");
	HSTR_PUSH_CFUNC(L,  "__gc",       MetaGC);
	HSTR_PUSH_CFUNC(L,  "__index",    MetaIndex);
	HSTR_PUSH_CFUNC(L,  "__newindex", MetaNewindex);
	HSTR_PUSH_STRING(L, "__metatable", "no access");
	lua_pop(L, 1);
	return true;
}


int LuaScream::MetaGC(lua_State* L)
{
	int* refPtr = GetScreamRef(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, *refPtr);
	if (lua_isfunction(L, -1)) {
		const int error = lua_pcall(L, 0, 0, 0);
		if (error != 0) {
			LuaLog(1, "Scream: error(%i) = %s", error, lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
	else if (lua_israwstring(L, -1)) {
		LuaLog(0, "SCREAM: %s", lua_tostring(L, -1));
	}
	luaL_unref(L, LUA_REGISTRYINDEX, *refPtr);
	return 0;
}


int LuaScream::MetaIndex(lua_State* L)
{
	int* refPtr = GetScreamRef(L, 1);
	const string key = luaL_checkstring(L, 2);
	if (key == "func") {
		lua_rawgeti(L, LUA_REGISTRYINDEX, *refPtr);
		return 1;
	}
	return 0;
}


int LuaScream::MetaNewindex(lua_State* L)
{
	int* refPtr = GetScreamRef(L, 1);
	const string key = luaL_checkstring(L, 2);
	if (key == "func") {
		lua_pushvalue(L, 3);
		lua_rawseti(L, LUA_REGISTRYINDEX, *refPtr);
	}
	return 0;
}

//============================================================================//
//============================================================================//

int* LuaScream::GetScreamRef(lua_State* L, int index)
{
	if (lua_getuserdataextra(L, index) != metaName) {
		luaL_argerror(L, index, "expected Scream");
	}
	return (int*)lua_touserdata(L, index);
}


//============================================================================//
//============================================================================//

int LuaScream::CreateScream(lua_State* L)
{
	if (lua_isnoneornil(L, 1)) {
		luaL_error(L, "Incorrect arguments to CreateScream()");
	}

	int* refPtr = (int*)lua_newuserdata(L, sizeof(int));
	lua_setuserdataextra(L, -1, (void*)metaName);
	luaL_getmetatable(L, metaName);
	lua_setmetatable(L, -2);

	lua_pushvalue(L, 1);
	*refPtr = luaL_ref(L, LUA_REGISTRYINDEX);

	return 1;
}


//============================================================================//
//============================================================================//
