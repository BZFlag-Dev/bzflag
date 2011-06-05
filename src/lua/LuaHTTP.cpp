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
#include "LuaHTTP.h"

// system headers
#include <new>
#include <time.h>
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "bzfio.h"
#include "network.h"
#include "AccessList.h"
#include "CacheManager.h"

// local headers
#include "LuaHeader.h"
#include "LuaDouble.h"


const char* LuaHTTPMgr::metaName = "HTTP";

AccessList* LuaHTTPMgr::accessList = NULL;


//============================================================================//
//============================================================================//

LuaHTTP::LuaHTTP(lua_State* LS, const std::string& _url)
  : httpL(LS)
  , active(false)
  , success(false)
  , fileSize(-1.0)
  , fileRemoteSize(-1.0)
  , fileTime(0)
  , httpCode(0)
  , url(_url)
  , postData("")
  , funcRef(LUA_NOREF)
  , selfRef(LUA_NOREF) {
  selfRef = luaL_ref(httpL, LUA_REGISTRYINDEX);

  long timeout = 0;
  bool noBody = false;
  bool header = false;
  std::vector<std::string> httpHeaders;

  int funcArg = 2;
  if (lua_israwstring(httpL, 2)) {
    postData = lua_tostring(httpL, 2);
    funcArg++;
  }
  else if (lua_istable(httpL, 2)) {
    const int table = funcArg;
    funcArg++;
    for (lua_pushnil(httpL); lua_next(httpL, table) != 0; lua_pop(httpL, 1)) {
      if (lua_israwstring(httpL, -2)) {
        const std::string key = lua_tostring(httpL, -2);
        if (key == "post") {
          postData = luaL_checkstring(httpL, -1);
        }
        else if (key == "timeout") {
          timeout = (long)luaL_checkint(httpL, -1);
        }
        else if (key == "noBody") {
          luaL_checktype(httpL, -1, LUA_TBOOLEAN);
          if (lua_tobool(httpL, -1)) {
            noBody = true;
          }
        }
        else if (key == "header") {
          luaL_checktype(httpL, -1, LUA_TBOOLEAN);
          if (lua_tobool(httpL, -1)) {
            header = true;
          }
        }
        else if (key == "failOnError") {
          luaL_checktype(httpL, -1, LUA_TBOOLEAN);
          if (lua_tobool(httpL, -1)) {
            setFailOnError();
          }
        }
        else if (key == "httpHeader") {
          if (lua_israwstring(httpL, -1)) {
            httpHeaders.push_back(lua_tostring(httpL, -1));
          }
          else if (lua_type(httpL, -1) == LUA_TTABLE) {
            const int t = lua_gettop(httpL);
            for (int i = 1; lua_checkgeti(httpL, t, i) != 0; lua_pop(httpL, 1), i++) {
              httpHeaders.push_back(luaL_checkstring(httpL, -1));
            }
          }
          else {
            luaL_error(httpL, "'httpHeaders' expects a string or table");
          }
        }
      }
    }
  }

  if (lua_isfunction(httpL, funcArg)) {
    lua_pushvalue(httpL, funcArg);
    funcRef = luaL_ref(httpL, LUA_REGISTRYINDEX);
  }

  setURL(url);

  if (postData.empty()) {
    setGetMode();
  }
  else {
    setHTTPPostMode();
    setPostMode(postData);
  }

  setRequestFileTime(true);

  if (timeout != 0) {
    setTimeout(timeout);
  }

  if (noBody) {
    setNoBody();
  }

  if (header) {
    setIncludeHeader();
  }

  if (!httpHeaders.empty()) {
    setHttpHeader(httpHeaders);
  }

  addHandle();

  active = true;
}


LuaHTTP::~LuaHTTP() {
  debugf(6, "LuaHTTP: deleting %s userdata\n", url.c_str());
  if (active) {
    removeHandle();
  }
  ClearRefs();
}


void LuaHTTP::ClearRefs() {
  if (funcRef != LUA_NOREF) {
    luaL_unref(httpL, LUA_REGISTRYINDEX, funcRef);
    funcRef = LUA_NOREF;
  }
  if (selfRef != LUA_NOREF) {
    luaL_unref(httpL, LUA_REGISTRYINDEX, selfRef);
    selfRef = LUA_NOREF;
  }
}


void LuaHTTP::finalization(char* data, unsigned int length, bool good) {
  active = false;

  good = good && (data != NULL);

  // note the lower-case get()'s
  getFileTime(fileTime);
  getFileSize(fileSize);
  getFileRemoteSize(fileRemoteSize);
  getHttpCode(httpCode);

  if ((funcRef == LUA_NOREF) || (selfRef == LUA_NOREF)) {
    ClearRefs();
    return;
  }

  lua_checkstack(httpL, 4);
  lua_rawgeti(httpL, LUA_REGISTRYINDEX, funcRef);
  if (!lua_isfunction(httpL, -1)) {
    lua_pop(httpL, 1);
    ClearRefs();
    return;
  }

  // callback(thisFetch, <data | nil> [, errMsg, errCode ])

  int args = 1;
  lua_rawgeti(httpL, LUA_REGISTRYINDEX, selfRef);
  if (!LuaHTTPMgr::TestHTTP(httpL, -1)) {
    lua_pop(httpL, 2);
    ClearRefs();
    return;
  }

  ClearRefs();

  if (good) {
    success = true;
    args += 1;
    lua_pushlstring(httpL, data, length);
  }
  else {
    args += 3;
    lua_pushnil(httpL);
    lua_pushliteral(httpL, "failed");
    lua_pushinteger(httpL, httpCode);
  }

  // call the function
  if (lua_pcall(httpL, args, 0, 0) != 0) {
    debugf(0, "LuaHTTP callback error (%s): %s\n",
                    url.c_str(), lua_tostring(httpL, -1));
    lua_pop(httpL, 1);
  }
}


bool LuaHTTP::Cancel() {
  if (!active) {
    return false;
  }
  active = false;
  removeHandle();
  ClearRefs();
  return true;
}


bool LuaHTTP::PushFunc(lua_State* L) const {
  if (httpL != L) {
    return false;
  }
  lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }
  return true;
}


time_t LuaHTTP::GetFileTime() {
  if (!active) {
    return fileTime;
  }
  time_t tmp;
  return getFileTime(tmp) ? tmp : 0;
}

double LuaHTTP::GetFileSize() {
  if (!active) {
    return fileSize;
  }
  double tmp;
  return getFileSize(tmp) ? tmp : -1.0;
}


double LuaHTTP::GetFileRemoteSize() {
  if (!active) {
    return fileRemoteSize;
  }
  double tmp;
  return getFileRemoteSize(tmp) ? tmp : -1.0;
}


bool LuaHTTP::GetFileData(unsigned int offset, std::string& data) const {
  if (!theData || (theLen < offset)) {
    return false;
  }
  data.assign((char*)theData + offset, theLen - offset);
  return true;
}



//============================================================================//
//============================================================================//
//
//  LuaHTTPMgr
//

bool LuaHTTPMgr::PushEntries(lua_State* L) {
  CreateMetatable(L);

  PUSH_LUA_CFUNC(L, Fetch);
  PUSH_LUA_CFUNC(L, Cancel);
  PUSH_LUA_CFUNC(L, Length);
  PUSH_LUA_CFUNC(L, Success);
  PUSH_LUA_CFUNC(L, IsActive);
  PUSH_LUA_CFUNC(L, GetURL);
  PUSH_LUA_CFUNC(L, GetPostData);
  PUSH_LUA_CFUNC(L, GetCallback);
  PUSH_LUA_CFUNC(L, GetFileSize);
  PUSH_LUA_CFUNC(L, GetFileRemoteSize);
  PUSH_LUA_CFUNC(L, GetFileTime);
  PUSH_LUA_CFUNC(L, GetFileData);
  PUSH_LUA_CFUNC(L, GetHttpCode);
  PUSH_LUA_CFUNC(L, TestAccess);

  return true;
}


void LuaHTTPMgr::SetAccessList(AccessList* list) {
  accessList = list;
}


//============================================================================//
//============================================================================//

const LuaHTTP* LuaHTTPMgr::TestHTTP(lua_State* L, int index) {
  return (LuaHTTP*)luaL_testudata(L, index, metaName);
}


const LuaHTTP* LuaHTTPMgr::CheckHTTP(lua_State* L, int index) {
  return (LuaHTTP*)luaL_checkudata(L, index, metaName);
}


LuaHTTP* LuaHTTPMgr::GetURL(lua_State* L, int index) {
  return (LuaHTTP*)luaL_testudata(L, index, metaName);
}


//============================================================================//
//============================================================================//

bool LuaHTTPMgr::CreateMetatable(lua_State* L) {
  luaL_newmetatable(L, metaName);

  luaset_strfunc(L, "__gc",       MetaGC);
  luaset_strfunc(L, "__tostring", MetaToString);

  lua_pushliteral(L, "__index");
  lua_newtable(L);
  {
    PUSH_LUA_CFUNC(L, Cancel);
    PUSH_LUA_CFUNC(L, Length);
    PUSH_LUA_CFUNC(L, Success);
    PUSH_LUA_CFUNC(L, IsActive);
    PUSH_LUA_CFUNC(L, GetURL);
    PUSH_LUA_CFUNC(L, GetPostData);
    PUSH_LUA_CFUNC(L, GetCallback);
    PUSH_LUA_CFUNC(L, GetFileSize);
    PUSH_LUA_CFUNC(L, GetFileRemoteSize);
    PUSH_LUA_CFUNC(L, GetFileTime);
    PUSH_LUA_CFUNC(L, GetFileData);
    PUSH_LUA_CFUNC(L, GetHttpCode);
  }
  lua_rawset(L, -3);

  luaset_strstr(L, "__metatable", "no access");

  lua_pop(L, 1); // pop the metatable
  return true;
}


int LuaHTTPMgr::MetaGC(lua_State* L) {
  LuaHTTP* fetch = GetURL(L, 1);
  fetch->~LuaHTTP();
  return 0;
}


int LuaHTTPMgr::MetaToString(lua_State* L) {
  LuaHTTP* fetch = GetURL(L, 1);
  lua_pushfstring(L, "http(%p,%s)", (void*)fetch, fetch->GetURL().c_str());
  return 1;
}


//============================================================================//
//============================================================================//

static int ParseURL(lua_State* L, const std::string& url, std::string& hostname) {
  std::string protocol, path;
  int port;
  if (!BzfNetwork::parseURL(url, protocol, hostname, port, path)) {
    lua_pushnil(L);
    lua_pushliteral(L, "bad URL, http:// or ftp:// is required");
    return 2;
  }
  if ((protocol != "http") && (protocol != "ftp")) {
    lua_pushnil(L);
    lua_pushliteral(L, "bad URL, http:// or ftp:// is required");
    return 2;
  }
  return 0;
}


//============================================================================//
//============================================================================//

int LuaHTTPMgr::Fetch(lua_State* L) {
  std::string url = luaL_checkstring(L, 1);

  // default to http
  if (url.find("://") == std::string::npos) {
    url = "http://" + url;
  }

  std::string hostname;
  const int parseArgs = ParseURL(L, url, hostname);
  if (parseArgs) {
    return parseArgs;
  }

  if (accessList && !accessList->alwaysAuthorized()) {
    std::vector<std::string> hostnames;
    hostnames.push_back(hostname);
    if (!accessList->authorized(hostnames)) {
      lua_pushnil(L);
      lua_pushliteral(L, "site blocked by DownloadAccess.txt");
      return 2;
    }
  }

  // create the userdata
  void* data = lua_newuserdata(L, sizeof(LuaHTTP));
  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);

  lua_pushvalue(L, -1); // push a copy of the userdata for
  // the LuaHTTP constructor to reference

  LuaHTTP* fetch = new(data) LuaHTTP(L, url);
  if (!fetch->GetActive()) {
    fetch->~LuaHTTP();
    return luaL_pushnil(L);
  }

  return 1;
}


//============================================================================//
//============================================================================//

int LuaHTTPMgr::Cancel(lua_State* L) {
  LuaHTTP* fetch = GetURL(L, 1);
  lua_pushboolean(L, fetch->Cancel());
  return 1;
}


int LuaHTTPMgr::Length(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);
  lua_pushinteger(L, fetch->GetLength());
  return 1;
}


int LuaHTTPMgr::Success(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);
  lua_pushboolean(L, fetch->GetSuccess());
  return 1;
}


int LuaHTTPMgr::IsActive(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);
  lua_pushboolean(L, fetch->GetActive());
  return 1;
}


int LuaHTTPMgr::GetURL(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);
  lua_pushstdstring(L, fetch->GetURL());
  return 1;
}


int LuaHTTPMgr::GetPostData(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);
  const std::string& postData = fetch->GetPostData();
  if (postData.empty()) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, postData);
  return 1;
}


int LuaHTTPMgr::GetCallback(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);
  if (!fetch->PushFunc(L)) {
    return luaL_pushnil(L);
  }
  return 1;
}


int LuaHTTPMgr::GetFileSize(lua_State* L) {
  LuaHTTP* fetch = const_cast<LuaHTTP*>(CheckHTTP(L, 1));
  LuaDouble::PushDouble(L, fetch->GetFileSize());
  return 1;
}


int LuaHTTPMgr::GetFileRemoteSize(lua_State* L) {
  LuaHTTP* fetch = const_cast<LuaHTTP*>(CheckHTTP(L, 1));
  LuaDouble::PushDouble(L, fetch->GetFileRemoteSize());
  return 1;
}


int LuaHTTPMgr::GetFileTime(lua_State* L) {
  LuaHTTP* fetch = const_cast<LuaHTTP*>(CheckHTTP(L, 1));

  const time_t t = fetch->GetFileTime();
  struct tm* tmPtr = gmtime(&t);
  if (tmPtr == NULL) {
    return luaL_pushnil(L);
  }

  char buf[256];
  int len;

  len = strftime(buf, sizeof(buf), "%Y-%m-%d/%H:%M:%S", tmPtr);
  lua_pushlstring(L, buf, len);

  snprintf(buf, sizeof(buf), "%lu", t);
  lua_pushstring(L, buf);

  return 2;
}


int LuaHTTPMgr::GetFileData(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);

  double offset = 0.0;
  double* offPtr = LuaDouble::TestNumber(L, 2);
  if (offPtr) {
    offset = *offPtr;
  }

  std::string data;
  if (!fetch->GetFileData((unsigned int) offset, data)) {
    lua_pushnil(L);
    return 0;
  }

  lua_pushstdstring(L, data);
  return 1;
}


int LuaHTTPMgr::GetHttpCode(lua_State* L) {
  const LuaHTTP* fetch = CheckHTTP(L, 1);

  const long code = fetch->GetHttpCode();
  lua_pushinteger(L, (int)code);

  return 1;
}


int LuaHTTPMgr::TestAccess(lua_State* L) {
  const std::string hostname = luaL_checkstring(L, 1);
  if ((accessList == NULL) || accessList->alwaysAuthorized()) {
    lua_pushboolean(L, 1);
  }
  else {
    std::vector<std::string> hostnames;
    hostnames.push_back(hostname);
    lua_pushboolean(L, accessList->authorized(hostnames));
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

