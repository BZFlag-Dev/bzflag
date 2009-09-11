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


#include "common.h"

// interface header
#include "BZDB.h"

// system headers
#include <algorithm>
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;

// common headers
#include "StateDatabase.h"
#include "Protocol.h"

// bzfs headers
#include "../GameKeeper.h"

// local headers
#include "LuaHeader.h"


//============================================================================//

static int GetMap(lua_State* L);
static int GetList(lua_State* L);

static int Exists(lua_State* L);
static int GetDefault(lua_State* L);
static int IsPersistent(lua_State* L);
static int GetPermission(lua_State* L);
static int SetPersistent(lua_State* L);

static int GetInt(lua_State* L);
static int GetBool(lua_State* L);
static int GetFloat(lua_State* L);
static int GetString(lua_State* L);

static int SetInt(lua_State* L);
static int SetBool(lua_State* L);
static int SetFloat(lua_State* L);
static int SetString(lua_State* L);

static int SendPlayerVariables(lua_State* L);


//============================================================================//

bool LuaBZDB::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, GetMap);
  PUSH_LUA_CFUNC(L, GetList);

  PUSH_LUA_CFUNC(L, Exists);
  PUSH_LUA_CFUNC(L, GetDefault);
  PUSH_LUA_CFUNC(L, IsPersistent);
  PUSH_LUA_CFUNC(L, GetPermission);

  PUSH_LUA_CFUNC(L, GetInt);
  PUSH_LUA_CFUNC(L, GetBool);
  PUSH_LUA_CFUNC(L, GetFloat);
  PUSH_LUA_CFUNC(L, GetString);

  PUSH_LUA_CFUNC(L, SetInt);
  PUSH_LUA_CFUNC(L, SetBool);
  PUSH_LUA_CFUNC(L, SetFloat);
  PUSH_LUA_CFUNC(L, SetString);

  PUSH_LUA_CFUNC(L, SetPersistent);

  PUSH_LUA_CFUNC(L, SendPlayerVariables);

  lua_pushliteral(L, "PERMISSIONS");
  lua_newtable(L); {
    // permission levels
    lua_pushliteral(L, "READ_WRITE");
    lua_pushinteger(L, StateDatabase::ReadWrite);
    lua_rawset(L, -3);
    lua_pushliteral(L, "READ_ONLY");
    lua_pushinteger(L, StateDatabase::ReadOnly);
    lua_rawset(L, -3);
    lua_pushliteral(L, "LOCKED");
    lua_pushinteger(L, StateDatabase::Locked);
    lua_rawset(L, -3);
    // access levels
    lua_pushliteral(L, "USER");
    lua_pushinteger(L, StateDatabase::User);
    lua_rawset(L, -3);
    lua_pushliteral(L, "CLIENT");
    lua_pushinteger(L, StateDatabase::Client);
    lua_rawset(L, -3);
    lua_pushliteral(L, "SERVER");
    lua_pushinteger(L, StateDatabase::Server);
    lua_rawset(L, -3);
  }
  lua_rawset(L, -3);

  return true;
}


//============================================================================//

static void VarCallback(const string& key, void* userData)
{
  vector<string>* names = (vector<string>*) userData;
  names->push_back(key);
}


//============================================================================//

static int GetMap(lua_State* L)
{
  vector<string> names;
  BZDB.iterate(VarCallback, &names);

  lua_createtable(L, 0, (int)names.size());
  for (size_t i = 0; i < names.size(); i++) {
    lua_pushstdstring(L, names[i]);
    lua_pushstdstring(L, BZDB.get(names[i]));
    lua_rawset(L, -3);
  }

  return 1;
}


static int GetList(lua_State* L)
{
  vector<string> names;
  BZDB.iterate(VarCallback, &names);

  std::sort(names.begin(), names.end());

  lua_createtable(L, (int)names.size(), 0);
  for (size_t i = 0; i < names.size(); i++) {
    lua_pushstdstring(L, names[i]);
    lua_rawseti(L, -2, i + 1);
  }

  return 1;
}


//============================================================================//

static int Exists(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  lua_pushboolean(L, BZDB.isSet(key));
  return 1;
}


static int GetDefault(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  lua_pushstdstring(L, BZDB.getDefault(key));
  return 1;
}


static int IsPersistent(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  lua_pushboolean(L, BZDB.isPersistent(key));
  return 1;
}


static int GetPermission(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  lua_pushinteger(L, BZDB.getPermission(key));
  return 1;
}


static int SetPersistent(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  const bool value = lua_tobool(L, 2);

  BZDB.setPersistent(key, value);
  
  lua_pushboolean(L, true);
  return 1;
}


//============================================================================//
//
//  Get()'s
//

static int GetInt(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushinteger(L, BZDB.evalInt(key));
  return 1;
}


static int GetBool(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushboolean(L, BZDB.isTrue(key));
  return 1;
}


static int GetFloat(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushfloat(L, BZDB.eval(key));
  return 1;
}


static int GetString(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  lua_pushstdstring(L, BZDB.get(key));
  return 1;
}


//============================================================================//
//
//  Set()'s
//

static int SetInt(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  const int value = luaL_checkint(L, 2);
  const int perms = luaL_optint(L, 3, BZDB.getPermission(key));

  BZDB.setInt(key, value, (StateDatabase::Permission) perms);
  lua_pushboolean(L, true);
  return 1;
}


static int SetBool(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  const bool value = lua_tobool(L, 2);
  const int  perms = luaL_optint(L, 3, BZDB.getPermission(key));

  BZDB.setBool(key, value, (StateDatabase::Permission) perms);

  lua_pushboolean(L, true);
  return 1;
}


static int SetFloat(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  const float value = luaL_checkfloat(L, 2);
  const int   perms = luaL_optint(L, 3, BZDB.getPermission(key));

  BZDB.setFloat(key, value, (StateDatabase::Permission) perms);

  lua_pushboolean(L, true);
  return 1;
}


static int SetString(lua_State* L)
{
  const char* key = luaL_checkstring(L, 1);
  if (strlen(key) <= 0) {
    return 0;
  }
  const char* value = luaL_checkstring(L, 2);
  const int   perms = luaL_optint(L, 3, BZDB.getPermission(key));

  BZDB.set(key, value, (StateDatabase::Permission) perms);

  lua_pushboolean(L, true);
  return 1;
}


//============================================================================//

static int SendPlayerVariables(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  GameKeeper::Player* gkPlayer = GameKeeper::Player::getPlayerByIndex(playerID);
  if ((gkPlayer == NULL) || (gkPlayer->netHandler == NULL)) {
    lua_pushboolean(L, false);
    return 1;
  }

  const int tableIndex = 2;
  luaL_checktype(L, tableIndex, LUA_TTABLE);

  std::map<string, string> varMap;
  for (lua_pushnil(L); lua_next(L, tableIndex) != 0; lua_pop(L, 1)) {
    if (lua_israwstring(L, -2) && lua_isstring(L, -1)) {
      const string key = lua_tostring(L, -2);
      if (!key.empty()) {
        varMap[key] = lua_tostring(L, -1);
      }
    }
  }

  if (varMap.empty()) {
    lua_pushboolean(L, false);
    return 1;
  }

  NetMsg msg = MSGMGR.newMessage();
  msg->packUInt16(varMap.size());
  std::map<string, string>::const_iterator it;
  for (it = varMap.begin(); it != varMap.end(); ++it) {
    msg->packStdString(it->first);
    msg->packStdString(it->second);
  }
  msg->send(gkPlayer->netHandler, MsgSetVar);

  lua_pushinteger(L, (int)varMap.size());
  return 1;
}


//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
