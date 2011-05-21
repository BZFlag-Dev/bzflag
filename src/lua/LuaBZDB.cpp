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


#include "common.h"

// interface header
#include "LuaBZDB.h"

// system headers
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::map;
using std::set;

// common headers
#include "StateDatabase.h"

// local headers
#include "LuaHeader.h"
#include "LuaUtils.h"


//============================================================================//
//============================================================================//

bool LuaBZDB::PushEntries(lua_State* L) {
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

  PUSH_LUA_CFUNC(L, Unset);

  lua_pushliteral(L, "ACCESS");
  lua_newtable(L);
  LuaSetDualPair(L, "USER",   StateDatabase::User);
  LuaSetDualPair(L, "CLIENT", StateDatabase::Client);
  LuaSetDualPair(L, "SERVER", StateDatabase::Server);
  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool ValidWriteString(const string& str) {
  return (str.size() <= 256) &&
         (str.find_first_of("\r\n") == string::npos);
}


static inline bool ReadCheck(lua_State* L, const string& varName) {
  LuaBzdbCheckFunc func = L2ES(L)->bzdbReadCheck;
  return (!func || func(varName));
}


static inline bool WriteCheck(lua_State* L, const string& varName) {
  if (!ValidWriteString(varName)) {
    return false;
  }
  LuaBzdbCheckFunc func = L2ES(L)->bzdbWriteCheck;
  return (!func || func(varName));
}


#define BZDB_READ_CHECK(L, name) \
  if (!ReadCheck(L, name)) { \
    lua_pushnil(L); \
    lua_pushliteral(L, "variable can not be read"); \
    return 2; \
  }


#define BZDB_WRITE_CHECK(L, name) \
  if (!WriteCheck(L, name)) { \
    lua_pushnil(L); \
    lua_pushliteral(L, "variable can not be written"); \
    return 2; \
  }


//============================================================================//
//============================================================================//
//
//  GetMap
//

static void mapCallback(const string& name, void* data) {
  map<string, string>& bzdbMap = *((map<string, string>*)data);
  bzdbMap[name] = BZDB.get(name);
}


int LuaBZDB::GetMap(lua_State* L) {
  map<string, string> bzdbMap;

  BZDB.iterate(mapCallback, &bzdbMap);

  map<string, string>::const_iterator it;
  lua_createtable(L, 0, bzdbMap.size());
  for (it = bzdbMap.begin(); it != bzdbMap.end(); ++it) {
    if (ReadCheck(L, it->first)) {
      luaset_strstr(L, it->first, it->second);
    }
  }
  return 1;
}


//============================================================================//
//============================================================================//
//
//  GetList
//

static void vectorCallback(const string& name, void* data) {
  vector<string>& bzdbVec = *((vector<string>*)data);
  bzdbVec.push_back(name);
}


int LuaBZDB::GetList(lua_State* L) {
  vector<string> bzdbVec;

  BZDB.iterate(vectorCallback, &bzdbVec);
  std::sort(bzdbVec.begin(), bzdbVec.end());

  lua_createtable(L, bzdbVec.size(), 0);
  for (size_t i = 0; i < bzdbVec.size(); i++) {
    lua_pushstdstring(L, bzdbVec[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


//============================================================================//
//============================================================================//
//
//  INFO call-outs
//

int LuaBZDB::Exists(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  lua_pushboolean(L, BZDB.isSet(key));
  return 1;
}


int LuaBZDB::IsPersistent(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  if (!BZDB.isSet(key)) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushboolean(L, BZDB.isPersistent(key));
  return 1;
}


int LuaBZDB::GetDefault(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  if (!BZDB.isSet(key)) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstdstring(L, BZDB.getDefault(key));
  return 1;
}


int LuaBZDB::GetPermission(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  if (!BZDB.isSet(key)) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushinteger(L, BZDB.getPermission(key));
  return 1;
}


//============================================================================//
//============================================================================//
//
//  GET call-outs
//

int LuaBZDB::GetInt(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  lua_pushinteger(L, BZDB.evalInt(key));
  return 1;
}


int LuaBZDB::GetBool(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  lua_pushboolean(L, BZDB.isTrue(key));
  return 1;
}


int LuaBZDB::GetFloat(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  lua_pushfloat(L, BZDB.eval(key));
  return 1;
}


int LuaBZDB::GetString(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  BZDB_READ_CHECK(L, key)
  lua_pushstdstring(L, BZDB.get(key));
  return 1;
}


//============================================================================//
//============================================================================//
//
//  SET call-outs
//

int LuaBZDB::SetInt(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  const int value  = luaL_checkint(L, 2);

  BZDB_WRITE_CHECK(L, key)

  BZDB.setInt(key, value);

  lua_pushboolean(L, true);
  return 1;
}


int LuaBZDB::SetBool(lua_State* L) {
  const string key = luaL_checkstring(L, 1);
  if (!lua_isboolean(L, 2)) {
    luaL_error(L, "expected boolean argument for arg 2");
  }
  const bool value = lua_tobool(L, 2);

  BZDB_WRITE_CHECK(L, key)

  BZDB.setBool(key, value);

  lua_pushboolean(L, true);
  return 1;
}


int LuaBZDB::SetFloat(lua_State* L) {
  const string key  = luaL_checkstring(L, 1);
  const float value = luaL_checkfloat(L, 2);

  BZDB_WRITE_CHECK(L, key)

  BZDB.setFloat(key, value);

  lua_pushboolean(L, true);
  return 1;
}


int LuaBZDB::SetString(lua_State* L) {
  const string key   = luaL_checkstring(L, 1);
  const string value = luaL_checkstring(L, 2);

  BZDB_WRITE_CHECK(L, key)

  if (!ValidWriteString(value)) {
    lua_pushboolean(L, false);
    return 1;
  }

  BZDB.set(key, value);

  lua_pushboolean(L, true);
  return 1;
}


int LuaBZDB::Unset(lua_State* L) {
  const string key = luaL_checkstring(L, 1);

  BZDB_WRITE_CHECK(L, key)

  BZDB.unset(key);

  lua_pushboolean(L, true);
  return 1;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
