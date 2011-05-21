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
#include "SlashCmd.h"

// system headers
#include <string>
#include <map>

// common headers
#include "bzfsAPI.h"
#include "TextUtils.h"

// local headers
#include "LuaHeader.h"
#include "LuaServer.h"


static std::map<std::string, class SlashCmdHandler*> slashHandlers;

static int AttachSlashCommand(lua_State* L);
static int DetachSlashCommand(lua_State* L);


//============================================================================//
//============================================================================//

class SlashCmdHandler : public bz_CustomSlashCommandHandler {
  public:
    SlashCmdHandler(const std::string& c, const std::string& h);
    ~SlashCmdHandler();

    bool handle(int playerID, bz_ApiString command,
                bz_ApiString message, bz_APIStringList* params);

    const char* help(bz_ApiString /* command */) { return helpTxt.c_str(); }

  private:
    std::string cmd;
    std::string helpTxt;
    int luaRef;
};


SlashCmdHandler::SlashCmdHandler(const std::string& c, const std::string& h)
  : cmd(c)
  , helpTxt(h)
  , luaRef(LUA_NOREF) {
  slashHandlers[cmd] = this;

  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return;
  }
  if (!lua_isfunction(L, -1)) {
    return;
  }
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
}


SlashCmdHandler::~SlashCmdHandler() {
  slashHandlers.erase(cmd);

  lua_State* L = LuaServer::GetL();
  if (L == NULL) {
    return;
  }
  luaL_unref(L, LUA_REGISTRYINDEX, luaRef);
}


bool SlashCmdHandler::handle(int playerID, bz_ApiString /*command*/,
                             bz_ApiString message, bz_APIStringList* /*params*/) {
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

  lua_pushinteger(L, playerID);
  lua_pushstring(L, TextUtils::tolower(cmd).c_str());
  lua_pushstring(L, message.c_str());

  if (lua_pcall(L, 3, 1, 0) != 0) {
    bz_debugMessagef(0, "LuaSlashCmd callback error (%s): %s\n",
                     cmd.c_str(), lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }

  if (lua_isboolean(L, -1) && !lua_tobool(L, -1)) {
    lua_pop(L, 1);
    return false;
  }

  return true;
}


//============================================================================//
//============================================================================//

bool SlashCmd::PushEntries(lua_State* L) {
  lua_pushliteral(L, "AttachSlashCommand");
  lua_pushcfunction(L, AttachSlashCommand);
  lua_rawset(L, -3);

  lua_pushliteral(L, "DetachSlashCommand");
  lua_pushcfunction(L, DetachSlashCommand);
  lua_rawset(L, -3);

  return true;
}


bool SlashCmd::CleanUp(lua_State* /*_L*/) {
  std::map<std::string, SlashCmdHandler*>::const_iterator it, nextIT;

  for (it = slashHandlers.begin(); it != slashHandlers.end(); /* noop */) {
    nextIT = it;
    nextIT++;
    delete it->second; // deletes itself from slashHandlers
    it = nextIT;
  }

  slashHandlers.clear();

  return true; // do nothing
}


//============================================================================//
//============================================================================//

static int AttachSlashCommand(lua_State* L) {
  const std::string cmd = TextUtils::tolower(luaL_checkstring(L, 1));
  const char* help = luaL_checkstring(L, 2);
  if (!lua_isfunction(L, 3)) {
    luaL_error(L, "expected a function");
  }
  lua_settop(L, 3); // function is the third param

  if (help[0] == 0) {
    luaL_error(L, "empty slash command help");
  }

  if (slashHandlers.find(cmd) != slashHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  SlashCmdHandler* handler = new SlashCmdHandler(cmd, help);
  if (bz_registerCustomSlashCommand(cmd.c_str(), handler)) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushboolean(L, false);
    delete handler;
  }

  return 1;
}


static int DetachSlashCommand(lua_State* L) {
  const std::string cmd = TextUtils::tolower(luaL_checkstring(L, 1));

  std::map<std::string, SlashCmdHandler*>::iterator it = slashHandlers.find(cmd);
  if (it == slashHandlers.end()) {
    lua_pushboolean(L, false);
    return 1;
  }

  delete it->second;

  if (bz_removeCustomSlashCommand(cmd.c_str())) {
    lua_pushboolean(L, true);
  }
  else {
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
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
