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
#include "RawLink.h"

// system headers
#include <string>
#include <set>
#include <map>
using std::string;
using std::set;
using std::map;

// common headers
#include "bzfsAPI.h"

// local headers
#include "LuaHeader.h"
#include "LuaServer.h"


static int AttachRawLink(lua_State* L);
static int DetachRawLink(lua_State* L);
static int WriteRawLink(lua_State* L);
static int DisconnectRawLink(lua_State* L);
static int GetRawLinkIP(lua_State* L);
static int GetRawLinkHost(lua_State* L);
static int GetRawLinkQueued(lua_State* L);


typedef map<int, class Link*> LinkMap;
static LinkMap linkMap;


//============================================================================//
//============================================================================//

class Link : public bz_NonPlayerConnectionHandler {
  public:
    Link(lua_State* L, int linkID);
    ~Link();

    void pending(int id, void* data, unsigned int size);
    void disconnect(int id);

    bool IsValid() const { return (funcRef != LUA_NOREF); }

  private:
    int id;
    int funcRef;
};


//============================================================================//
//============================================================================//

Link::Link(lua_State* L, int _id)
: id(_id)
, funcRef(LUA_NOREF)
{
  if (linkMap.find(id) != linkMap.end()) {
    return;
  }
  if (!bz_registerNonPlayerConnectionHandler(id, this)) {
    return;
  }

  if (lua_isnil(L, 2)) {
    funcRef = LUA_REFNIL;
  }
  else if (lua_isfunction(L, 2)) {
    funcRef = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  else {
    return;
  }

  linkMap[id] = this;
}


Link::~Link()
{
  bz_removeNonPlayerConnectionHandler(id, this);
  linkMap.erase(id);

  lua_State* L = LuaServer::GetL();
  if (L != NULL) {
    luaL_unref(L, LUA_REGISTRYINDEX, funcRef);
  }
}


void Link::pending(int /*id*/, void* data, unsigned int size)
{
  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return;
  }
  if (funcRef == LUA_REFNIL) {
    return;
  }

  lua_checkstack(L, 4);

  lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    delete this;
    return;
  }

  lua_pushinteger(L, id);

  if (data == NULL) {
    lua_pushnil(L);
  } else {
    lua_pushlstring(L, (char*)data, size);
  }

  if (lua_pcall(L, 2, 0, 0) != 0) {
    bz_debugMessagef(0, "lua call-in rawlink error (%i): %s\n",
                     id, lua_tostring(L, -1));
    lua_pop(L, 1);
    return;
  }

  return;
}


void Link::disconnect(int /*id*/)
{
  pending(id, NULL, 0);
  delete this;
}


//============================================================================//
//============================================================================//

bool RawLink::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, AttachRawLink);
  PUSH_LUA_CFUNC(L, DetachRawLink);
  PUSH_LUA_CFUNC(L, WriteRawLink);
  PUSH_LUA_CFUNC(L, DisconnectRawLink);
  PUSH_LUA_CFUNC(L, GetRawLinkIP);
  PUSH_LUA_CFUNC(L, GetRawLinkHost);
  PUSH_LUA_CFUNC(L, GetRawLinkQueued);

  return true;
}


bool RawLink::CleanUp(lua_State* /*L*/)
{
  LinkMap::iterator it, next;
  for (it = linkMap.begin(); it != linkMap.end(); /* no-op */) {
    next = it;
    next++;
    delete it->second;
    it = next;
  }
  linkMap.clear();

  return true;
}


//============================================================================//
//============================================================================//

static int AttachRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);

  if (!lua_isnoneornil(L, 2) && !lua_isfunction(L, 2)) {
    return 0;
  }
  lua_settop(L, 2); // discard any extras

  Link* link = new Link(L, linkID);
  if (!link->IsValid()) {
    delete link;
    return 0;
  }
  lua_pushboolean(L, true);
  return 1;
}


static int DetachRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  LinkMap::iterator it = linkMap.find(linkID);
  if (it == linkMap.end()) {
    return 0;
  }
  Link* link = it->second;
  delete link;
  lua_pushboolean(L, true);
  return 1;
}


static int WriteRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (linkMap.find(linkID) == linkMap.end()) {
    return 0;
  }
  size_t size;
  const char* data = luaL_checklstring(L, 2, &size);
  lua_pushboolean(L, bz_sendNonPlayerData(linkID, data, size));
  return 1;
}


static int DisconnectRawLink(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (linkMap.find(linkID) == linkMap.end()) {
    return 0;
  }
  lua_pushboolean(L, bz_disconnectNonPlayerConnection(linkID));
  return 1;
}


static int GetRawLinkIP(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (linkMap.find(linkID) == linkMap.end()) {
    return 0;
  }
  lua_pushstring(L, bz_getNonPlayerConnectionIP(linkID));
  return 1;
}


static int GetRawLinkHost(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (linkMap.find(linkID) == linkMap.end()) {
    return 0;
  }
  lua_pushstring(L, bz_getNonPlayerConnectionHost(linkID));
  return 1;
}


static int GetRawLinkQueued(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  if (linkMap.find(linkID) == linkMap.end()) {
    return 0;
  }
  lua_pushinteger(L, bz_getNonPlayerConnectionOutboundPacketCount(linkID));
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
