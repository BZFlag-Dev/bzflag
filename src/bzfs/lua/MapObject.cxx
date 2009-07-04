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
#include "MapObject.h"

// system headers
#include <string>
#include <map>
using std::string;
using std::map;

// common headers
#include "bzfsAPI.h"
#include "TextUtils.h"

// local headers
#include "LuaHeader.h"
#include "LuaServer.h"


static map<string, class MapHandler*> mapHandlers;

static int AttachMapObject(lua_State* L);
static int DetachMapObject(lua_State* L);


//============================================================================//
//============================================================================//

class MapHandler : public bz_CustomMapObjectHandler {
  public:
    MapHandler(const string& objName);
    ~MapHandler();

    bool handle(bz_ApiString object, bz_CustomMapObjectInfo *data);

  private:
    string objName;
    int luaRef;
};


MapHandler::MapHandler(const string& name)
: objName(name)
, luaRef(LUA_NOREF)
{
  mapHandlers[objName] = this;

  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return;
  }
  if (!lua_isfunction(L, -1)) {
    return;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


MapHandler::~MapHandler()
{
  mapHandlers.erase(objName);

  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return;
  }
  luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
}


bool MapHandler::handle(bz_ApiString objToken, bz_CustomMapObjectInfo *info)
{
  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return false;
  }

  lua_checkstack(L, 5);

  lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    delete this;
    return false;
  }

  lua_pushstring(L, TextUtils::tolower(objToken.c_str()).c_str());

  lua_pushstring(L, info->args.c_str());

  lua_newtable(L);
  bz_APIStringList& list = info->data;
  for (size_t i = 0; i < list.size(); i++) {
    lua_pushstring(L, list[i].c_str());
    lua_rawseti(L, -2, i + 1);
  }

  lua_pushstring(L, info->fileName.c_str());

  lua_pushinteger(L, info->lineNum);

  if (lua_pcall(L, 5, 1, 0) != 0) {
    bz_debugMessagef(0, "lua call-in mapobject error (%s): %s\n",
                     objToken.c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  string newData = info->newData.c_str();

  if (lua_israwstring(L, -1)) {
    newData += lua_tostring(L, -1);
  }
  else if (lua_istable(L, -1)) {
    const int table = lua_gettop(L);
    for (int i = 1; lua_checkgeti(L, table, i) != 0; lua_pop(L, 1), i++) {
      if (lua_israwstring(L, -1)) {
        newData += lua_tostring(L, -1);
        newData += "\n";
      }
    }
  }

  info->newData = newData;

  lua_pop(L, 1);

  return true;
}


//============================================================================//
//============================================================================//

bool MapObject::PushEntries(lua_State* L)
{
  lua_pushliteral(L, "AttachMapObject");
  lua_pushcfunction(L, AttachMapObject);
  lua_rawset(L, -3);

  lua_pushliteral(L, "DetachMapObject");
  lua_pushcfunction(L, DetachMapObject);
  lua_rawset(L, -3);

  return true;
}


bool MapObject::CleanUp(lua_State* /*L*/)
{
  map<string, MapHandler*>::const_iterator it, nextIT;

  for (it = mapHandlers.begin(); it != mapHandlers.end(); /* noop */) {
    nextIT = it;
    nextIT++;
    delete it->second; // deletes itself from mapHandlers
    it = nextIT;
  }

  mapHandlers.clear();

  return true; // do nothing
}


//============================================================================//
//============================================================================//

static int AttachMapObject(lua_State* L)
{
  int funcIndex = 2;
  const string objName  = TextUtils::tolower(luaL_checkstring(L, 1));
  const char* endToken = NULL;
  if (lua_israwstring(L, 2)) {
    funcIndex++;
    endToken = lua_tostring(L, 2);
  }
  if (!lua_isfunction(L, funcIndex)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, funcIndex); // discard any extras

  if (mapHandlers.find(objName) != mapHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  MapHandler* handler = new MapHandler(objName);
  if (bz_registerCustomMapObject2(objName.c_str(), endToken, handler)) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
    delete handler;
  }

  return 1;
}


static int DetachMapObject(lua_State* L)
{
  const string objName = TextUtils::tolower(luaL_checkstring(L, 1));

  map<string, MapHandler*>::iterator it = mapHandlers.find(objName);
  if (it == mapHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  delete it->second;

  if (bz_removeCustomMapObject(objName.c_str())) {
    lua_pushboolean(L, true);
  } else {
    lua_pushboolean(L, false);
  }

  return 1;
}


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
