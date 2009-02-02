
#include "common.h"

// implementation header
#include "LuaBZDB.h"

// system headers
#include <algorithm>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

// common headers
#include "StateDatabase.h"

// local headers
#include "LuaInclude.h"


/******************************************************************************/
/******************************************************************************/

bool LuaBZDB::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, GetMap);
	PUSH_LUA_CFUNC(L, GetList);

	PUSH_LUA_CFUNC(L, Exists);
	PUSH_LUA_CFUNC(L, IsPersistent);
	PUSH_LUA_CFUNC(L, GetDefault);
	PUSH_LUA_CFUNC(L, GetPermission);

	PUSH_LUA_CFUNC(L, GetInt);
	PUSH_LUA_CFUNC(L, GetBool);
	PUSH_LUA_CFUNC(L, GetFloat);
	PUSH_LUA_CFUNC(L, GetString);

	PUSH_LUA_CFUNC(L, SetInt);
	PUSH_LUA_CFUNC(L, SetBool);
	PUSH_LUA_CFUNC(L, SetFloat);
	PUSH_LUA_CFUNC(L, SetString);

	return true;
}


/******************************************************************************/
/******************************************************************************/
//
//  GetMap
//

static void mapCallback(const std::string& name, void* data)
{
	map<string, string>& bzdbMap = *((map<string, string>*)data);
	bzdbMap[name] = BZDB.get(name);  
}


int LuaBZDB::GetMap(lua_State* L)
{
	map<string, string> bzdbMap;
	BZDB.iterate(mapCallback, &bzdbMap);

	map<string, string>::const_iterator it;
	lua_createtable(L, 0, bzdbMap.size());
	for (it = bzdbMap.begin(); it != bzdbMap.end(); ++it) {
		lua_pushstdstring(L, it->first);
		lua_pushstdstring(L, it->second);
		lua_rawset(L, -3);
	}
	return 1;
}


/******************************************************************************/
/******************************************************************************/
//
//  GetList
//

static void vectorCallback(const std::string& name, void* data)
{
	vector<string>& bzdbVec = *((vector<string>*)data);
	bzdbVec.push_back(name);
}


int LuaBZDB::GetList(lua_State* L)
{
	vector<string> bzdbVec;
	BZDB.iterate(vectorCallback, &bzdbVec);
	std::sort(bzdbVec.begin(), bzdbVec.end()); // safety

	vector<string>::const_iterator it;
	lua_createtable(L, bzdbVec.size(), 0);
	for (size_t i = 0; i < bzdbVec.size(); i++) {
		lua_pushinteger(L, i + 1);
		lua_pushstdstring(L, bzdbVec[i]);
		lua_rawset(L, -3);
	}
	return 1;
}


/******************************************************************************/
/******************************************************************************/
//
//  INFO call-outs
//

int LuaBZDB::Exists(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushboolean(L, true);
	return 1;
}


int LuaBZDB::IsPersistent(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushboolean(L, BZDB.isPersistent(key));
	return 1;
}


int LuaBZDB::GetDefault(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushstdstring(L, BZDB.getDefault(key));
	return 1;
}


int LuaBZDB::GetPermission(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushinteger(L, BZDB.getPermission(key));
	return 1;
}


/******************************************************************************/
/******************************************************************************/
//
//  GET call-outs
//

int LuaBZDB::GetInt(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushinteger(L, BZDB.evalInt(key));
	return 1;
}


int LuaBZDB::GetBool(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushboolean(L, BZDB.isTrue(key));
	return 1;
}


int LuaBZDB::GetFloat(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushnumber(L, BZDB.eval(key));
	return 1;
}


int LuaBZDB::GetString(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!BZDB.isSet(key)) {
		return 0;
	}
	lua_pushstdstring(L, BZDB.get(key));
	return 1;
}


/******************************************************************************/
/******************************************************************************/
//
//  SET call-outs
//

static inline bool CheckPermission(const char* name)
{
	return (BZDB.getPermission(name) == StateDatabase::ReadWrite);
}


int LuaBZDB::SetInt(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	const int value = luaL_checkint(L, 2);

	if (!CheckPermission(key)) {
		lua_pushboolean(L, false);
		return 1;
	}

	BZDB.setInt(key, value);

	lua_pushboolean(L, true);
	return 1;
}


int LuaBZDB::SetBool(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	if (!lua_isboolean(L, 2)) {
		luaL_error(L, "expected boolean argument for arg 2");
	}
	const bool value = lua_tobool(L, 2);

	if (!CheckPermission(key)) {
		lua_pushboolean(L, false);
		return 1;
	}

	BZDB.setBool(key, value);

	lua_pushboolean(L, true);
	return 1;
}


int LuaBZDB::SetFloat(lua_State* L)
{
	const char* key   = luaL_checkstring(L, 1);
	const float value = luaL_checkfloat(L, 2);

	if (!CheckPermission(key)) {
		lua_pushboolean(L, false);
		return 1;
	}

	BZDB.setFloat(key, value);

	lua_pushboolean(L, true);
	return 1;
}


int LuaBZDB::SetString(lua_State* L)
{
	const char* key   = luaL_checkstring(L, 1);
	const char* value = luaL_checkstring(L, 2);

	if (!CheckPermission(key)) {
		lua_pushboolean(L, false);
		return 1;
	}

	BZDB.set(key, value);

	lua_pushboolean(L, true);
	return 1;
}


/******************************************************************************/
/******************************************************************************/
