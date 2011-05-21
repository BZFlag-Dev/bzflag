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
#include "LuaServerPing.h"

// system headers
#include <new>
#include <string>
#include <vector>
#include <set>

// common headers
#include "network.h"
#include "Address.h"
#include "ServerPing.h"

// local headers
#include "LuaHeader.h"
#include "LuaDouble.h"


const char* LuaServerPingMgr::metaName = "SPing";

LuaServerPingMgr::PingSet LuaServerPingMgr::pings;


//============================================================================//
//============================================================================//
//
//  LuaServerPing
//

LuaServerPing::LuaServerPing(lua_State* L, int _funcRef, int _selfRef,
                             double _addr, int _port)
  : serverPing(NULL)
  , pingL(L)
  , funcRef(_funcRef)
  , selfRef(_selfRef)
  , done(false)
  , lag(-1)
  , addr(_addr)
  , port(_port) {
  LuaServerPingMgr::pings.insert(this);

  InAddr inAddr;
  inAddr.s_addr = htonl((uint32_t)addr);

  const size_t samples  =   4;
  const double interval = 1.0;
  const double timeout  = 1.0;

  serverPing = new ServerPing(inAddr, port, samples, interval, timeout);
  serverPing->start();
}


LuaServerPing::~LuaServerPing() {
  Cancel();
}


void LuaServerPing::Update() {
  if (!serverPing || done) {
    return;
  }

  serverPing->doPings();

  if (!serverPing->done()) {
    return;
  }

  done = true;

  lag = serverPing->calcLag();

  if ((funcRef != LUA_NOREF) && (selfRef != LUA_NOREF)) {
    lua_checkstack(pingL, 2);
    lua_rawgeti(pingL, LUA_REGISTRYINDEX, funcRef);
    lua_rawgeti(pingL, LUA_REGISTRYINDEX, selfRef);
    const int error = lua_pcall(pingL, 1, 0, 0);
    if (error != 0) {
      LuaLog(1, "LuaServerPing error(%i) = %s", error, lua_tostring(pingL, -1));
      lua_pop(pingL, 1);
    }
  }

  Cancel();
}


void LuaServerPing::Cancel() {
  if (funcRef != LUA_NOREF) {
    luaL_unref(pingL, LUA_REGISTRYINDEX, funcRef);
    funcRef = LUA_NOREF;
  }
  if (selfRef != LUA_NOREF) {
    luaL_unref(pingL, LUA_REGISTRYINDEX, selfRef);
    selfRef = LUA_NOREF;
  }

  delete serverPing;
  serverPing = NULL;

  LuaServerPingMgr::pings.erase(this);
}


//============================================================================//
//============================================================================//
//
//  LuaServerPingMgr
//

bool LuaServerPingMgr::PushEntries(lua_State* L) {
  CreateMetatable(L);
  PUSH_LUA_CFUNC(L, SendServerPing);
  return true;
}


void LuaServerPingMgr::Update() {
  PingSet::iterator it, nextIt;
  for (it = pings.begin(); it != pings.end(); it = nextIt) {
    nextIt = it;
    nextIt++;
    (*it)->Update();
  }
}


//============================================================================//
//============================================================================//

bool LuaServerPingMgr::CreateMetatable(lua_State* L) {
  luaL_newmetatable(L, metaName);

  lua_pushliteral(L, "__gc"); // garbage collection
  lua_pushcfunction(L, MetaGC);
  lua_rawset(L, -3);

  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, MetaToString);
  lua_rawset(L, -3);

  lua_pushliteral(L, "__index");
  lua_newtable(L);
  {
    PUSH_LUA_CFUNC(L, Cancel);
    PUSH_LUA_CFUNC(L, IsDone);
    PUSH_LUA_CFUNC(L, GetLag);
    PUSH_LUA_CFUNC(L, GetAddress);
  }
  lua_rawset(L, -3);

  lua_pop(L, 1); // pop the metatable
  return true;
}


int  LuaServerPingMgr::MetaGC(lua_State* L) {
  LuaServerPing* ping = CheckServerPing(L, 1);
  ping->~LuaServerPing();
  return 0;
}


int LuaServerPingMgr::MetaToString(lua_State* L) {
  LuaServerPing* ping = CheckServerPing(L, 1);
  lua_pushfstring(L, "ping(%p,%g:%i)", (void*)ping,
                  ping->GetAddress(), ping->GetPort());
  return 1;
}


//============================================================================//
//============================================================================//

LuaServerPing* LuaServerPingMgr::CheckServerPing(lua_State* L, int index) {
  return (LuaServerPing*)luaL_checkudata(L, index, metaName);
}


//============================================================================//
//============================================================================//

int LuaServerPingMgr::SendServerPing(lua_State* L) {
  lua_settop(L, 3);

  const double addr = LuaDouble::CheckDouble(L, 1);
  const int    port = luaL_checkint(L, 2);
  const int funcRef = luaL_ref(L, LUA_REGISTRYINDEX);

  // create the userdata
  void* data = lua_newuserdata(L, sizeof(LuaServerPing));
  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);

  lua_pushvalue(L, -1);
  const int selfRef = luaL_ref(L, LUA_REGISTRYINDEX);

  LuaServerPing* ping =
    new(data) LuaServerPing(L, funcRef, selfRef, addr, port);
  ping->Update();
  if (!ping->IsValid()) {
    ping->~LuaServerPing();
    return luaL_pushnil(L);
  }

  return 1;
}


//============================================================================//
//============================================================================//

int LuaServerPingMgr::Cancel(lua_State* L) {
  LuaServerPing* ping = CheckServerPing(L, 1);
  ping->Cancel();
  return 0;
}


int LuaServerPingMgr::IsDone(lua_State* L) {
  LuaServerPing* ping = CheckServerPing(L, 1);
  lua_pushboolean(L, ping->IsDone());
  return 1;
}


int LuaServerPingMgr::GetLag(lua_State* L) {
  LuaServerPing* ping = CheckServerPing(L, 1);
  lua_pushint(L, ping->GetLag());
  return 1;
}


int LuaServerPingMgr::GetAddress(lua_State* L) {
  LuaServerPing* ping = CheckServerPing(L, 1);
  LuaDouble::PushDouble(L, ping->GetAddress());
  lua_pushint(L, ping->GetPort());
  return 2;
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
