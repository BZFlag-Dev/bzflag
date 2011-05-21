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
#include "LuaVFS.h"

// system headers
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;

// common headers
#include "BzVFS.h"

// local headers
#include "LuaHeader.h"
#include "LuaHandle.h"
#include "LuaUtils.h"


//============================================================================//
//============================================================================//

bool LuaVFS::PushEntries(lua_State* L) {
  PUSH_LUA_CFUNC(L, FileExists);
  PUSH_LUA_CFUNC(L, FileSize);
  PUSH_LUA_CFUNC(L, ReadFile);
  PUSH_LUA_CFUNC(L, WriteFile);
  PUSH_LUA_CFUNC(L, AppendFile);
  PUSH_LUA_CFUNC(L, RemoveFile);
  PUSH_LUA_CFUNC(L, RenameFile);
  PUSH_LUA_CFUNC(L, Include);
  PUSH_LUA_CFUNC(L, CreateDir);
  PUSH_LUA_CFUNC(L, DirList);
  PUSH_LUA_CFUNC(L, GetModes);

  lua_pushliteral(L, "MODES");
  lua_newtable(L);
  {
    luaset_strstr(L, "CONFIG",          BZVFS_CONFIG);
    luaset_strstr(L, "DATA",            BZVFS_DATA);
    luaset_strstr(L, "DATA_DEFAULT",    BZVFS_DATA_DEFAULT);
    luaset_strstr(L, "FTP",             BZVFS_FTP);
    luaset_strstr(L, "HTTP",            BZVFS_HTTP);
    luaset_strstr(L, "LUA_USER",        BZVFS_LUA_USER);
    luaset_strstr(L, "LUA_BZORG",       BZVFS_LUA_BZORG);
    luaset_strstr(L, "LUA_WORLD",       BZVFS_LUA_WORLD);
    luaset_strstr(L, "LUA_RULES",       BZVFS_LUA_RULES);
    luaset_strstr(L, "LUA_USER_WRITE",  BZVFS_LUA_USER_WRITE);
    luaset_strstr(L, "LUA_BZORG_WRITE", BZVFS_LUA_BZORG_WRITE);
    luaset_strstr(L, "LUA_WORLD_WRITE", BZVFS_LUA_WORLD_WRITE);
    luaset_strstr(L, "LUA_RULES_WRITE", BZVFS_LUA_RULES_WRITE);
    luaset_strstr(L, "BASIC",           BZVFS_BASIC);
    luaset_strstr(L, "ALL",             BZVFS_ALL);
  }
  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//
//
//  call('path',   nil)   -> (defModes)       & allModes
//  call('path',   modes) -> (modes)    & allModes
//  call(':pmodes:path', nil)   -> (pmodes)  & allModes
//  call(':pmodes:path', modes) -> (pmodes & modes) & allModes
//

const char* GetReadFilter(const std::string& path, lua_State* L) {
  if (!path.empty() && (path[0] == ':')) {
    return L2ES(L)->vfsModes->readAllowed;
  }
  else {
    return L2ES(L)->vfsModes->readDefault;
  }
}


//============================================================================//
//============================================================================//

int LuaVFS::FileExists(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  string modes = luaL_optstring(L, 2, GetReadFilter(path, L));
  modes = BzVFS::allowModes(modes, L2ES(L)->vfsModes->readAllowed);

  lua_pushboolean(L, bzVFS.fileExists(path, modes));

  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::FileSize(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  string modes = luaL_optstring(L, 2, GetReadFilter(path, L));
  modes = BzVFS::allowModes(modes, L2ES(L)->vfsModes->readAllowed);

  const int size = bzVFS.fileSize(path, modes);
  if (size >= 0) {
    lua_pushinteger(L, size);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::ReadFile(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  string modes = luaL_optstring(L, 2, GetReadFilter(path, L));
  modes = BzVFS::allowModes(modes, L2ES(L)->vfsModes->readAllowed);

  string data;
  if (bzVFS.readFile(path, modes, data)) {
    lua_pushstdstring(L, data);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

static bool ParseWriteData(lua_State* L, int index, string& data) {
  data.clear();

  if (lua_israwstring(L, index)) {
    size_t len;
    const char* c = lua_tolstring(L, index, &len);
    data.assign(c, len);
    return true;
  }
  else if (lua_istable(L, index)) { // a table of strings
    const string suffix = luaL_optstring(L, index + 1, ""); // optional suffix
    const int table = (index > 0) ? index : (lua_gettop(L) + index + 1);
    vector<string> dataVec;
    size_t total = 0;
    for (int i = 1; lua_checkgeti(L, table, i) != 0; lua_pop(L, 1), i++) {
      if (lua_israwstring(L, -1)) {
        size_t len;
        const char* c = lua_tolstring(L, -1, &len);
        dataVec.push_back(string(c, len) + suffix);
        total += len;
      }
    }
    data.resize(total);
    size_t offset = 0;
    for (size_t s = 0; s < dataVec.size(); s++) {
      const string& d = dataVec[s];
      data.replace(offset, d.size(), d);
      offset += d.size();
    }
    return true;
  }

  return false;
}


//============================================================================//
//============================================================================//

int LuaVFS::WriteFile(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const string modes = L2ES(L)->vfsModes->writeAllowed;

  string data;
  if (!ParseWriteData(L, 2, data)) {
    luaL_error(L, "bad data");
  }

  if (bzVFS.writeFile(path, modes, data)) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::AppendFile(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const string modes = L2ES(L)->vfsModes->writeAllowed;

  string data;
  if (!ParseWriteData(L, 2, data)) {
    luaL_error(L, "%s: bad data");
  }

  if (bzVFS.appendFile(path, modes, data)) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::RemoveFile(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const string modes = L2ES(L)->vfsModes->writeAllowed;

  if (bzVFS.removeFile(path, modes)) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::RenameFile(lua_State* L) {
  const char* oldPath = luaL_checkstring(L, 1);
  const char* newPath = luaL_checkstring(L, 2);
  const string modes = L2ES(L)->vfsModes->writeAllowed;

  if (bzVFS.renameFile(oldPath, modes, newPath)) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::Include(lua_State* L) {
  lua_settop(L, 3);

  const char* path = luaL_checkstring(L, 1);
  string modes = luaL_optstring(L, 2, GetReadFilter(path, L));
  modes = BzVFS::allowModes(modes, L2ES(L)->vfsModes->readAllowed);

  string code;
  if (!bzVFS.readFile(path, modes, code)) {
    luaL_error(L, "file not found");
  }

  // compile the code
  int error = luaL_loadbuffer(L, code.data(), code.size(), path);
  if (error != 0) {
    lua_error(L);
  }
  const int funcIndex = 4;

  // setup the function's environment
  if (lua_istable(L, 3)) {
    lua_pushvalue(L, 3);
  }
  else if (lua_isnil(L, 3)) {
    LuaUtils::PushCurrentFuncEnv(L, "Include");
  }
  else {
    luaL_error(L, "bad fenv parameter");
  }

  if (lua_setfenv(L, -2) == 0) {
    luaL_error(L, "Include: error with setfenv");
  }

  // execute the code
  error = lua_pcall(L, 0, LUA_MULTRET, 0);
  if (error != 0) {
    lua_error(L);
  }

  return lua_gettop(L) - (funcIndex - 1);
}


//============================================================================//
//============================================================================//

int LuaVFS::CreateDir(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  const string modes = L2ES(L)->vfsModes->writeAllowed;

  if (bzVFS.createDir(path, modes)) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}


//============================================================================//
//============================================================================//

int LuaVFS::DirList(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  string modes = luaL_optstring(L, 2, GetReadFilter(path, L));
  modes = BzVFS::allowModes(modes, L2ES(L)->vfsModes->readAllowed);

  const bool recursive = lua_isboolean(L, 3) && lua_tobool(L, 3);

  vector<string> files;
  vector<string> dirs;
  if (!bzVFS.dirList(path, modes, recursive, dirs, files)) {
    lua_pushnil(L);
    return 1;
  }

  lua_createtable(L, files.size(), 0);
  for (size_t i = 0; i < files.size(); i++) {
    const string& fpath = files[i];
    lua_pushinteger(L, i + 1);
    lua_pushstdstring(L, fpath);
    lua_rawset(L, -3);
  }

  lua_createtable(L, dirs.size(), 0);
  for (size_t i = 0; i < dirs.size(); i++) {
    const string& dpath = dirs[i];
    lua_pushinteger(L, i + 1);
    lua_pushstdstring(L, dpath);
    lua_rawset(L, -3);
  }

  return 2;
}


//============================================================================//
//============================================================================//

int LuaVFS::GetModes(lua_State* L) {
  lua_createtable(L, 0, 2); {

    lua_pushliteral(L, "read");
    lua_createtable(L, 0, 2); {
      luaset_strstr(L, "default", L2ES(L)->vfsModes->readDefault);
      luaset_strstr(L, "allowed", L2ES(L)->vfsModes->readAllowed);
    }
    lua_rawset(L, -3);

    lua_pushliteral(L, "write");
    lua_createtable(L, 0, 2); {
      luaset_strstr(L, "default", L2ES(L)->vfsModes->writeDefault);
      luaset_strstr(L, "allowed", L2ES(L)->vfsModes->writeAllowed);
    }
    lua_rawset(L, -3);
  }
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
