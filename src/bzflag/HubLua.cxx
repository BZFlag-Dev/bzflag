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
#include "HubLua.h" // redirects to "HubLink.h"

// system headers
#include <errno.h>
#include <stdarg.h>
#include <string>
#include <set>

// common headers
#include "AnsiCodes.h"
#include "BZDBCache.h"
#include "HubComposeKey.h"
#include "HUDRenderer.h"
#include "HUDui.h"
#include "Pack.h"
#include "TimeKeeper.h"
#include "bzfio.h"
#include "bz_md5.h"
#include "version.h"

// local headers
#include "LuaHeader.h"
#include "guiplaying.h"


static int thisIndex = -123456; // LUA_REGISTRYINDEX for the HuaLink pointer


//============================================================================//
//============================================================================//
//
//  Lua routines
//

static void PushNamedString(lua_State* L, const char* key,
                                          const char* value)
{
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_rawset(L, -3);
}


static void limitMembers(lua_State* L, const char* table,
                         const std::vector<std::string>& functions)
{
  lua_newtable(L);
  const int nt = lua_gettop(L); // new table
  lua_getglobal(L, table);
  for (size_t i = 0; i < functions.size(); i++) {
    lua_getfield(L, -1, functions[i].c_str());
    lua_setfield(L, nt, functions[i].c_str());
  }
  lua_pop(L, 1);
  lua_setglobal(L, table);
}


bool HubLink::createLua(const std::string& code)
{
  L = luaL_newstate();
  if (L == NULL) {
    fail("luaL_newstate() error");
    return false;
  }

  lua_pushlightuserdata(L, this);
  lua_rawseti(L, LUA_REGISTRYINDEX, thisIndex);

  luaL_openlibs(L);

  // remove the "io" and "package" libraries
  lua_pushnil(L); lua_setfield(L, LUA_GLOBALSINDEX, "io");
  lua_pushnil(L); lua_setfield(L, LUA_GLOBALSINDEX, "package");

  // limit { os } table members
  std::vector<std::string> osFuncs;
  osFuncs.push_back("clock");
  osFuncs.push_back("date");
  osFuncs.push_back("time");
  osFuncs.push_back("difftime");
  limitMembers(L, "os", osFuncs);

  // limit { debug } table members
  std::vector<std::string> debugFuncs;
  debugFuncs.push_back("traceback");
  limitMembers(L, "debug", debugFuncs);

  // remove dofile() and loadfile()
  lua_pushnil(L); lua_setglobal(L, "dofile");
  lua_pushnil(L); lua_setglobal(L, "loadfile");

  if (!pushAnsiCodes()) {
    fail("pushAnsiCodes() error");
    return false;
  }
  if (!pushCallOuts()) {
    fail("pushCallOuts() error");
    return false;
  }
  if (!pushConstants()) {
    fail("pushConstants() error");
    return false;
  }

  const char* chunkName = codeFileName.c_str();
  if (luaL_loadbuffer(L, code.c_str(), code.size(), chunkName) != 0) {
    std::string msg= "error: ";
    msg += lua_tostring(L, -1);
    fail(msg);
    return false;
  }

  if (lua_pcall(L, 0, 0, 0) != 0) {
    std::string msg = "lua_pcall() error: ";
    msg += lua_tostring(L, -1);
    fail(msg);
    return false;
  }

  lua_gc(L, LUA_GCCOLLECT, 0);

  // create ServerJoined call-in
  const ServerLink* srvLink = ServerLink::getServer();
  if ((srvLink != NULL) &&
      (srvLink->getState() == ServerLink::Okay)) {
    serverJoined(srvLink->getJoinServer(),
                 srvLink->getJoinPort(),
                 srvLink->getJoinCallsign());
  }

  return true;
}


bool HubLink::pushAnsiCodes()
{
  lua_pushvalue(L, LUA_GLOBALSINDEX);

  lua_pushliteral(L, "ANSI");
  lua_newtable(L);

  PushNamedString(L, "RESET",        ANSI_STR_RESET_FINAL);
  PushNamedString(L, "RESET_BRIGHT", ANSI_STR_RESET);
  PushNamedString(L, "BRIGHT",       ANSI_STR_BRIGHT);
  PushNamedString(L, "DIM",          ANSI_STR_DIM);
  PushNamedString(L, "UNDERLINE",    ANSI_STR_UNDERLINE);
  PushNamedString(L, "NO_UNDERLINE", ANSI_STR_NO_UNDERLINE);
  PushNamedString(L, "BLINK",        ANSI_STR_PULSATING);
  PushNamedString(L, "NO_BLINK",     ANSI_STR_NO_PULSATE);
  PushNamedString(L, "REVERSE",      ANSI_STR_REVERSE);
  PushNamedString(L, "NO_REVERSE",   ANSI_STR_NO_REVERSE);
  PushNamedString(L, "BLACK",   ANSI_STR_FG_BLACK);
  PushNamedString(L, "RED",     ANSI_STR_FG_RED);
  PushNamedString(L, "GREEN",   ANSI_STR_FG_GREEN);
  PushNamedString(L, "YELLOW",  ANSI_STR_FG_YELLOW);
  PushNamedString(L, "BLUE",    ANSI_STR_FG_BLUE);
  PushNamedString(L, "MAGENTA", ANSI_STR_FG_MAGENTA);
  PushNamedString(L, "CYAN",    ANSI_STR_FG_CYAN);
  PushNamedString(L, "WHITE",   ANSI_STR_FG_WHITE);
  PushNamedString(L, "ORANGE",  ANSI_STR_FG_ORANGE);
  lua_rawset(L, -3);

  lua_pop(L, 1); // pop _G

  return true;
}


//============================================================================//

bool HubLink::pushCallIn(const char* funcName, int inArgs)
{
  if (L == NULL) {
    return false;
  }
  if (!lua_checkstack(L, inArgs + 2)) {
    return false;
  }
  lua_getglobal(L, funcName);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }
  return true;
  
}


bool HubLink::runCallIn(int inArgs, int outArgs)
{
  if (lua_pcall(L, inArgs, outArgs, 0) != 0) {
    debugf(2, "error, %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
  }
  return true;
}


//============================================================================//
//
//  Lua Call-Ins
//

void HubLink::shutdown()
{
  if (!pushCallIn("Shutdown", 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::updateLua()
{
  if (!pushCallIn("Update", 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::recvCommand(const std::string& cmd)
{
  if (!pushCallIn("RecvCommand", 1)) {
    return;
  }

  lua_pushstdstring(L, cmd);

  runCallIn(1, 0);
}


bool HubLink::recvData(const std::string& data)
{
  if (!pushCallIn("RecvData", 1)) {
    return false;
  }

  lua_pushstdstring(L, data);

  if (!runCallIn(1, 1)) {
    return false;
  }

  bool eatData = true;
  if (lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
    eatData = false;
  }

  lua_pop(L, 1);

  return eatData;
}


void HubLink::serverJoined(const std::string& location, int port,
                           const std::string& callsign)
{
  if (!pushCallIn("ServerJoined", 3)) {
    return;
  }

  lua_pushstdstring(L, location);
  lua_pushinteger(L,   port);
  lua_pushstdstring(L, callsign);

  runCallIn(3, 0);
}


void HubLink::serverParted()
{
  if (!pushCallIn("ServerParted", 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::wordComplete(const std::string& line,
                           std::set<std::string>& matches)
{
  if (!pushCallIn("WordComplete", 1)) {
    return;
  }

  lua_pushstdstring(L, line);

  if (!runCallIn(1, 1)) {
    return;
  }

  const int table = lua_gettop(L);
  if (!lua_istable(L, table)) {
    lua_pop(L, 1);
    return;
  }

  for (int i = 1; lua_checkgeti(L, table, i++); lua_pop(L, 1)) {
    if (lua_israwstring(L, -1)) {
      matches.insert(lua_tostring(L, -1));
    }
  }
}


void HubLink::activeTabChanged()
{
  if (!pushCallIn("ActiveTabChanged", 0)) {
    return;
  }

  runCallIn(0, 0);
}


void HubLink::bzdbChange(const std::string& varName)
{
  if (!pushCallIn("BZDBChange", 1)) {
    return;
  }

  lua_pushstdstring(L, varName);

  runCallIn(1, 0);
}


void HubLink::startComposing()
{
  if (!pushCallIn("StartComposing", 0)) {
    return;
  }

  runCallIn(0, 0);
}


//============================================================================//
//
//  Lua Call-Outs
//

static inline HubLink* GetLink(lua_State* L)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, thisIndex);
  if (!lua_isuserdata(L, -1)) {
    luaL_error(L, "Internal error -- missing 'thisIndex'");
  }
  HubLink* link = (HubLink*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  return link;
}


static int ParseTab(lua_State* L, int index)
{
  if (controlPanel == NULL) {
    return -1;
  }
  switch (lua_type(L, index)) {
    case LUA_TNUMBER: {
      return lua_tonumber(L, index);
    }
    case LUA_TSTRING: {
      return controlPanel->getTabID(lua_tostring(L, index));
    }
  }
  return -1;
}


#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
static int ReadStdin(lua_State* L);
#endif


bool HubLink::pushCallOuts()
{
  lua_newtable(L);

  PUSH_LUA_CFUNC(L, Reload);
  PUSH_LUA_CFUNC(L, Disable);

  PUSH_LUA_CFUNC(L, SendData);
  PUSH_LUA_CFUNC(L, ReadDataSize);
  PUSH_LUA_CFUNC(L, ReadData);
  PUSH_LUA_CFUNC(L, PeekData);

  PUSH_LUA_CFUNC(L, Print);
  PUSH_LUA_CFUNC(L, SetAlert);

  PUSH_LUA_CFUNC(L, AddTab);
  PUSH_LUA_CFUNC(L, RemoveTab);
  PUSH_LUA_CFUNC(L, ShiftTab);
  PUSH_LUA_CFUNC(L, ClearTab);
  PUSH_LUA_CFUNC(L, RenameTab);
  PUSH_LUA_CFUNC(L, GetTabCount);
  PUSH_LUA_CFUNC(L, GetTabLabel);
  PUSH_LUA_CFUNC(L, GetActiveTab);
  PUSH_LUA_CFUNC(L, SetActiveTab);

  PUSH_LUA_CFUNC(L, GetComposePrompt);
  PUSH_LUA_CFUNC(L, SetComposePrompt);
  PUSH_LUA_CFUNC(L, GetComposeString);
  PUSH_LUA_CFUNC(L, SetComposeString);

  PUSH_LUA_CFUNC(L, CalcMD5);
  PUSH_LUA_CFUNC(L, StripAnsiCodes);
  PUSH_LUA_CFUNC(L, SetBZDB);
  PUSH_LUA_CFUNC(L, GetBZDB);
  PUSH_LUA_CFUNC(L, GetCode);
  PUSH_LUA_CFUNC(L, GetTime);
  PUSH_LUA_CFUNC(L, GetVersion);
  PUSH_LUA_CFUNC(L, GetHubServer);
  PUSH_LUA_CFUNC(L, GetServerInfo);

  PUSH_LUA_CFUNC(L, PackInt8);
  PUSH_LUA_CFUNC(L, PackInt16);
  PUSH_LUA_CFUNC(L, PackInt32);
  PUSH_LUA_CFUNC(L, PackInt64);
  PUSH_LUA_CFUNC(L, PackUInt8);
  PUSH_LUA_CFUNC(L, PackUInt16);
  PUSH_LUA_CFUNC(L, PackUInt32);
  PUSH_LUA_CFUNC(L, PackUInt64);
  PUSH_LUA_CFUNC(L, PackFloat);
  PUSH_LUA_CFUNC(L, PackDouble);
  PUSH_LUA_CFUNC(L, UnpackInt8);
  PUSH_LUA_CFUNC(L, UnpackInt16);
  PUSH_LUA_CFUNC(L, UnpackInt32);
  PUSH_LUA_CFUNC(L, UnpackInt64);
  PUSH_LUA_CFUNC(L, UnpackUInt8);
  PUSH_LUA_CFUNC(L, UnpackUInt16);
  PUSH_LUA_CFUNC(L, UnpackUInt32);
  PUSH_LUA_CFUNC(L, UnpackUInt64);
  PUSH_LUA_CFUNC(L, UnpackFloat);
  PUSH_LUA_CFUNC(L, UnpackDouble);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  PUSH_LUA_CFUNC(L, ReadStdin);
#endif

  lua_setglobal(L, "bz");

  return true;
}


bool HubLink::pushConstants()
{
#define PUSH_LUA_STRING_NUMBER(s, n) \
  lua_pushstring(L, s); \
  lua_pushnumber(L, n); \
  lua_rawset(L, -3);

  lua_newtable(L);
  PUSH_LUA_STRING_NUMBER("ALLTABS", ControlPanel::MessageAllTabs);
  PUSH_LUA_STRING_NUMBER("CURRENT", ControlPanel::MessageCurrent);
  PUSH_LUA_STRING_NUMBER("ALL",     ControlPanel::MessageAll);
  PUSH_LUA_STRING_NUMBER("CHAT",    ControlPanel::MessageChat);
  PUSH_LUA_STRING_NUMBER("SERVER",  ControlPanel::MessageServer);
  PUSH_LUA_STRING_NUMBER("MISC",    ControlPanel::MessageMisc);
  PUSH_LUA_STRING_NUMBER("DEBUG",   ControlPanel::MessageDebug);
  lua_setglobal(L, "TABS");

#undef PUSH_LUA_STRING_NUMBER

  return true;
}


int HubLink::Reload(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  link->reloadHostPort = lua_israwstring(L, 1) ? lua_tostring(L, 1)
                                               : link->getHostPort();
  link->reloadLuaCode  = luaL_optstring(L, 2, "");
  return 0;
}


int HubLink::Disable(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  link->wantDisable = true;
  return 0;
}


int HubLink::AddTab(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  if (!controlPanel) {
    return 0;
  }
  const std::string label = luaL_checkstring(L, 1);
  const bool allSrc = !lua_isboolean(L, 2) || lua_toboolean(L, 2);
  const bool allDst = !lua_isboolean(L, 3) || lua_toboolean(L, 3);
  const bool success = controlPanel->addTab(label, allSrc, allDst);
  if (success) {
    link->tabs.insert(label);
  }
  lua_pushboolean(L, success);
  return 1;
}


int HubLink::RemoveTab(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  if (!controlPanel) {
    return 0;
  }
  const std::string label = luaL_checkstring(L, 1);
  link->tabs.erase(label);
  lua_pushboolean(L, controlPanel->removeTab(label));
  return 1;
}


int HubLink::ShiftTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = ParseTab(L, 1);
  const int distance  = luaL_checkint(L, 2);
  lua_pushboolean(L, controlPanel->shiftTab(tabID, distance));
  return 1;
}


int HubLink::ClearTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = ParseTab(L, 1);
  lua_pushboolean(L, controlPanel->clearTab(tabID));
  return 1;
}


int HubLink::RenameTab(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  if (!controlPanel) {
    return 0;
  }
  const std::string oldLabel = luaL_checkstring(L, 1);
  const std::string newLabel = luaL_checkstring(L, 2);
  const bool success = controlPanel->renameTab(oldLabel, newLabel);
  if (success) {
    link->tabs.erase(oldLabel);
    link->tabs.insert(newLabel);
  }
  lua_pushboolean(L, success);
  return 1;
}


int HubLink::GetTabCount(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  lua_pushinteger(L, controlPanel->getTabCount());
  return 1;
}


int HubLink::GetTabLabel(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = luaL_checkint(L, 1);
  const std::string& label = controlPanel->getTabLabel(tabID);
  if (label.empty()) {
    return 0;
  }
  lua_pushstdstring(L, label);
  return 1;
}


int HubLink::GetActiveTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = controlPanel->getActiveTab();
  if (tabID < 0) {
    return 0;
  }
  const std::string& label = controlPanel->getTabLabel(tabID);
  if (label.empty()) {
    return 0;
  }
  lua_pushstdstring(L, label);
  return 1;
}


int HubLink::GetComposePrompt(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  lua_pushstdstring(L, hud->getComposePrompt());
  return 1;
}


int HubLink::SetComposePrompt(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  const std::string prompt = luaL_checkstring(L, 1);
  hud->setComposing(prompt);
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::GetComposeString(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  lua_pushstdstring(L, hud->getComposeString());
  return 1;
}


int HubLink::SetComposeString(lua_State* L)
{
  if (hud == NULL) {
    return 0;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    return 0;
  }
  if (!hud->getComposing()) {
    return 0;
  }
  const std::string prompt = luaL_checkstring(L, 1);
  hud->setComposeString(prompt);
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::SetActiveTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = ParseTab(L, 1);
  lua_pushboolean(L, controlPanel->setActiveTab(tabID));
  return 1;
}


int HubLink::SendData(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  const std::string data = luaL_checkstdstring(L, 1);
  link->sendData(data);
  return 0;
}


int HubLink::ReadDataSize(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  lua_pushinteger(L, link->recvTotal);
  return 1;
}


int HubLink::ReadData(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  const int bytes = luaL_optint(L, 1, link->recvTotal);
  std::string data; 
  if (!link->readData(bytes, data)) {
    return 0;
  }
  lua_pushstdstring(L, data);
  return 1;
}


int HubLink::PeekData(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  const int bytes = luaL_optint(L, 1, link->recvTotal);
  std::string data;
  if (!link->peekData(bytes, data)) {
    return 0;
  }
  lua_pushstdstring(L, data);
  return 1;
}


int HubLink::Print(lua_State* L)
{
  const std::string msg = luaL_checkstring(L, 1);

  switch (lua_type(L, 2)) {
    case LUA_TNUMBER: {
      controlPanel->addMessage(msg, lua_tointeger(L, 2));
      break;
    }
    case LUA_TSTRING: {
      controlPanel->addMessage(msg, lua_tostring(L, 2));
      break;
    }
    default: {
      controlPanel->addMessage(msg);
      break;
    }
  }
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::SetAlert(lua_State* L)
{
  const std::string msg = luaL_checkstring(L, 1);
  const int   priority  = luaL_optint(L, 2, 0);
  const float duration  = luaL_optfloat(L, 3, 3.0f);
  const bool  warning   = lua_isboolean(L, 4) && lua_toboolean(L, 4);

  hud->setAlert(priority, msg.c_str(), duration, warning);
  return 0;
}


int HubLink::CalcMD5(lua_State* L)
{
  const std::string text = luaL_checkstdstring(L, 1);
  lua_pushstdstring(L, MD5(text).hexdigest());
  return 1;
}


int HubLink::StripAnsiCodes(lua_State* L)
{
  const std::string text = luaL_checkstring(L, 1);
  lua_pushstdstring(L, stripAnsiCodes(text));
  return 1;
}


int HubLink::SetBZDB(lua_State* L)
{
  const std::string key    = luaL_checkstring(L, 1);
  const std::string value  = luaL_checkstring(L, 2);
  const bool usePersistent = lua_isboolean(L, 3);
  const bool isPersistent  = usePersistent && lua_toboolean(L, 3);

  if (BZDB.isSet(key) &&
      (BZDB.getPermission(key) != StateDatabase::Client)) {
    lua_pushboolean(L, false);
    return 1;
  }

  BZDB.set(key, value);

  if (usePersistent) {
    BZDB.setPersistent(key, isPersistent);
  }

  lua_pushboolean(L, true);
  return 1;
}


int HubLink::GetBZDB(lua_State* L)
{
  const std::string key = luaL_checkstring(L, 1);
  if (!BZDB.isSet(key)) {
    return 0;
  }
  lua_pushstdstring(L, BZDB.get(key));
  return 1;
}


int HubLink::GetCode(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  lua_pushstdstring(L, link->getLuaCode());
  return 1;
}


int HubLink::GetTime(lua_State* L)
{
  lua_pushdouble(L, TimeKeeper::getCurrent().getSeconds());
  return 1;
}


int HubLink::GetVersion(lua_State* L)
{
  lua_pushstring(L, getMajorMinorRevVersion());
  lua_pushstring(L, getAppVersion());
  return 2;
}


int HubLink::GetHubServer(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    return 0;
  }
  lua_pushstdstring(L, link->getHostPort());
  return 1;
}


int HubLink::GetServerInfo(lua_State* L)
{
  const ServerLink* srvLink = ServerLink::getServer();
  if ((srvLink == NULL) ||
      (srvLink->getState() != ServerLink::Okay)) {
    return 0;
  }

  lua_pushstdstring(L, srvLink->getJoinServer());
  lua_pushinteger(L,   srvLink->getJoinPort());
  lua_pushstdstring(L, srvLink->getJoinCallsign());

  return 3;
}

//============================================================================//

#undef  PACK_TYPE
#define PACK_TYPE(label, type) \
  const type value = luaL_checkint(L, 1); \
  char buf[sizeof(type)]; \
  nboPack ## label(buf, value); \
  lua_pushlstring(L, buf, sizeof(type)); \
  return 1;
int HubLink::PackInt8(lua_State* L)   { PACK_TYPE(Int8,   int8_t)   }
int HubLink::PackInt16(lua_State* L)  { PACK_TYPE(Int16,  int16_t)  }
int HubLink::PackInt32(lua_State* L)  { PACK_TYPE(Int32,  int32_t)  }
int HubLink::PackInt64(lua_State* L)  { PACK_TYPE(Int64,  int64_t)  }
int HubLink::PackUInt8(lua_State* L)  { PACK_TYPE(UInt8,  uint8_t)  }
int HubLink::PackUInt16(lua_State* L) { PACK_TYPE(UInt16, uint16_t) }
int HubLink::PackUInt32(lua_State* L) { PACK_TYPE(UInt32, uint32_t) }
int HubLink::PackUInt64(lua_State* L) { PACK_TYPE(UInt64, uint64_t) }
int HubLink::PackFloat(lua_State* L)  { PACK_TYPE(Float,  float)    }
int HubLink::PackDouble(lua_State* L) { PACK_TYPE(Double, double)   }
#undef PACK_TYPE

#undef  UNPACK_TYPE
#define UNPACK_TYPE(label, type) \
  size_t len; \
  const char* s = luaL_checklstring(L, 1, &len); \
  if (len < sizeof(type)) { \
    return 0; \
  } \
  type value; \
  nboUnpack ## label((void*)s, value); \
  lua_pushnumber(L, (lua_Number)value); \
  return 1;
int HubLink::UnpackInt8(lua_State* L)   { UNPACK_TYPE(Int8,   int8_t)   }
int HubLink::UnpackInt16(lua_State* L)  { UNPACK_TYPE(Int16,  int16_t)  } 
int HubLink::UnpackInt32(lua_State* L)  { UNPACK_TYPE(Int32,  int32_t)  }
int HubLink::UnpackInt64(lua_State* L)  { UNPACK_TYPE(Int64,  int64_t)  }
int HubLink::UnpackUInt8(lua_State* L)  { UNPACK_TYPE(UInt8,  uint8_t)  }
int HubLink::UnpackUInt16(lua_State* L) { UNPACK_TYPE(UInt16, uint16_t) } 
int HubLink::UnpackUInt32(lua_State* L) { UNPACK_TYPE(UInt32, uint32_t) }
int HubLink::UnpackUInt64(lua_State* L) { UNPACK_TYPE(UInt64, uint64_t) }
int HubLink::UnpackFloat(lua_State* L)  { UNPACK_TYPE(Float,  float)    }
int HubLink::UnpackDouble(lua_State* L) { UNPACK_TYPE(Double, double)   }
#undef UNPACK_TYPE


//============================================================================//
//============================================================================//

// whacky bit of dev'ing fun
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  #include <unistd.h>
  #include <fcntl.h>
  static int ReadStdin(lua_State* L)
  {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    char buf[4096];
    const int r = read(STDIN_FILENO, buf, sizeof(buf));
    if (r <= 0) {
      return 0;
    }
    lua_pushlstring(L, buf, r);
    fcntl(STDIN_FILENO, F_SETFL, 0);
    return 1;
  }
#endif


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
