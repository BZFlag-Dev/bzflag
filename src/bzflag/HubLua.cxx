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
#include <map>

// common headers
#include "AnsiCodes.h"
#include "BZDBCache.h"
#include "DirectoryNames.h"
#include "HubComposeKey.h"
#include "HubLink.h"
#include "HUDRenderer.h"
#include "HUDui.h"
#include "KeyManager.h"
#include "LuaHeader.h"
#include "Pack.h"
#include "ServerLink.h"
#include "TimeKeeper.h"
#include "bzfgl.h"
#include "bzfio.h"
#include "bz_md5.h"
#include "version.h"

// local headers
#include "sound.h"
#include "guiplaying.h"


static int thisIndex = -12345; // LUA_REGISTRYINDEX for the HubLink pointer

const static int CallInBase = -10000;

// LUA_REGISTRYINDEX indices for the call-in functions
enum CallInCode {
  CI_Shutdown         = CallInBase - 0,
  CI_Update           = CallInBase - 1,
  CI_RecvCommand      = CallInBase - 2,
  CI_RecvData         = CallInBase - 3,
  CI_ServerJoined     = CallInBase - 4,
  CI_ServerParted     = CallInBase - 5,
  CI_WordComplete     = CallInBase - 6,
  CI_TabAdded         = CallInBase - 7,
  CI_TabRemoved       = CallInBase - 8,
  CI_ActiveTabChanged = CallInBase - 9,
  CI_BZDBChange       = CallInBase - 10,
  CI_StartComposing   = CallInBase - 11
};

static std::map<std::string, int> name2code;
static std::map<int, std::string> code2name;


//============================================================================//
//============================================================================//
//
//  Lua routines
//

static void pushNamedString(lua_State* L, const char* key,
                                          const char* value)
{
  lua_pushstring(L, key);
  lua_pushstring(L, value);
  lua_rawset(L, -3);
}


static void pushNamedInt(lua_State* L, const char* key, int value)
{
  lua_pushstring(L, key);
  lua_pushinteger(L, value);
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


static void setupCallInMaps()
{
  if (!name2code.empty()) {
    return;
  }
#define CALLIN_PAIR(x)      \
  name2code[#x] = CI_ ## x; \
  code2name[CI_ ## x] = #x

  CALLIN_PAIR(Shutdown);
  CALLIN_PAIR(Update);
  CALLIN_PAIR(RecvCommand);
  CALLIN_PAIR(RecvData);
  CALLIN_PAIR(ServerJoined);
  CALLIN_PAIR(ServerParted);
  CALLIN_PAIR(WordComplete);
  CALLIN_PAIR(TabAdded);
  CALLIN_PAIR(TabRemoved);
  CALLIN_PAIR(ActiveTabChanged);
  CALLIN_PAIR(BZDBChange);
  CALLIN_PAIR(StartComposing);

#undef CALLIN_PAIR
}


//============================================================================//

bool HubLink::createLua(const std::string& code)
{
  setupCallInMaps();

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
  if (!pushConstants()) {
    fail("pushConstants() error");
    return false;
  }
  if (!pushCallOuts()) {
    fail("pushCallOuts() error");
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

  debugf(1, "createLua succeeded\n");  

  return true;
}


bool HubLink::pushAnsiCodes()
{
  lua_newtable(L);

  pushNamedString(L, "RESET",        ANSI_STR_RESET_FINAL);
  pushNamedString(L, "RESET_BRIGHT", ANSI_STR_RESET);
  pushNamedString(L, "BRIGHT",       ANSI_STR_BRIGHT);
  pushNamedString(L, "DIM",          ANSI_STR_DIM);
  pushNamedString(L, "NORMAL",       ANSI_STR_NORMAL);
  pushNamedString(L, "ITALIC",       ANSI_STR_ITALIC);
  pushNamedString(L, "NO_ITALIC",    ANSI_STR_NO_ITALIC);
  pushNamedString(L, "UNDERLINE",    ANSI_STR_UNDERLINE);
  pushNamedString(L, "NO_UNDERLINE", ANSI_STR_NO_UNDERLINE);
  pushNamedString(L, "BLINK",        ANSI_STR_PULSATING);
  pushNamedString(L, "NO_BLINK",     ANSI_STR_NO_PULSATE);
  pushNamedString(L, "REVERSE",      ANSI_STR_REVERSE);
  pushNamedString(L, "NO_REVERSE",   ANSI_STR_NO_REVERSE);
  pushNamedString(L, "BLACK",  ANSI_STR_FG_BLACK);
  pushNamedString(L, "RED",    ANSI_STR_FG_RED);
  pushNamedString(L, "GREEN",  ANSI_STR_FG_GREEN);
  pushNamedString(L, "YELLOW", ANSI_STR_FG_YELLOW);
  pushNamedString(L, "BLUE",   ANSI_STR_FG_BLUE);
  pushNamedString(L, "PURPLE", ANSI_STR_FG_MAGENTA);
  pushNamedString(L, "CYAN",   ANSI_STR_FG_CYAN);
  pushNamedString(L, "WHITE",  ANSI_STR_FG_WHITE);
  pushNamedString(L, "ORANGE", ANSI_STR_FG_ORANGE);

  lua_setglobal(L, "ANSI");

  return true;
}


bool HubLink::pushConstants()
{
  lua_newtable(L);

  pushNamedInt(L, "ALLTABS", ControlPanel::MessageAllTabs);
  pushNamedInt(L, "CURRENT", ControlPanel::MessageCurrent);
  pushNamedInt(L, "ALL",     ControlPanel::MessageAll);
  pushNamedInt(L, "CHAT",    ControlPanel::MessageChat);
  pushNamedInt(L, "SERVER",  ControlPanel::MessageServer);
  pushNamedInt(L, "MISC",    ControlPanel::MessageMisc);
  pushNamedInt(L, "DEBUG",   ControlPanel::MessageDebug);

  lua_setglobal(L, "TABS");

  return true;
}


//============================================================================//

bool HubLink::pushCallIn(int ciCode, int inArgs)
{
  if (L == NULL) {
    return false;
  }
  if (!lua_checkstack(L, inArgs + 2)) {
    return false;
  }
  lua_rawgeti(L, LUA_REGISTRYINDEX, ciCode);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }
  return true;
}


bool HubLink::runCallIn(int inArgs, int outArgs)
{
  if (lua_pcall(L, inArgs, outArgs, 0) != 0) {
    debugf(1, "error, %s\n", lua_tostring(L, -1));
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
  if (!pushCallIn(CI_Shutdown, 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::updateLua()
{
  if (!pushCallIn(CI_Update, 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::recvCommand(const std::string& cmd)
{
  if (!pushCallIn(CI_RecvCommand, 1)) {
    return;
  }

  lua_pushstdstring(L, cmd);

  runCallIn(1, 0);
}


bool HubLink::recvData(const std::string& data)
{
  if (!pushCallIn(CI_RecvData, 1)) {
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
  if (!pushCallIn(CI_ServerJoined, 3)) {
    return;
  }

  lua_pushstdstring(L, location);
  lua_pushinteger(L,   port);
  lua_pushstdstring(L, callsign);

  runCallIn(3, 0);
}


void HubLink::serverParted()
{
  if (!pushCallIn(CI_ServerParted, 0)) {
    return;
  }
  runCallIn(0, 0);
}


void HubLink::wordComplete(const std::string& line,
                           std::set<std::string>& matches)
{
  if (!pushCallIn(CI_WordComplete, 1)) {
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


void HubLink::tabAdded(const std::string& name)
{
  if (!pushCallIn(CI_TabAdded, 1)) {
    return;
  }

  lua_pushstdstring(L, name);

  runCallIn(1, 0);
}


void HubLink::tabRemoved(const std::string& name)
{
  if (!pushCallIn(CI_TabRemoved, 1)) {
    return;
  }

  lua_pushstdstring(L, name);

  runCallIn(1, 0);
}


void HubLink::activeTabChanged()
{
  if (!pushCallIn(CI_ActiveTabChanged, 0)) {
    return;
  }

  runCallIn(0, 0);
}


void HubLink::bzdbChange(const std::string& varName)
{
  if (!pushCallIn(CI_BZDBChange, 1)) {
    return;
  }

  lua_pushstdstring(L, varName);

  runCallIn(1, 0);
}


void HubLink::startComposing()
{
  if (!pushCallIn(CI_StartComposing, 0)) {
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


static int CheckTab(lua_State* L, int index)
{
  const int type = lua_type(L, index);
  if ((type != LUA_TNUMBER) && (type != LUA_TSTRING)) {
    luaL_error(L, "expected number or string");
  }
  if (controlPanel == NULL) {
    return -1;
  }
  if (type == LUA_TNUMBER) {
    return lua_tonumber(L, index);
  }
  return controlPanel->getTabID(lua_tostring(L, index));
}


#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
static int ReadStdin(lua_State* L);
#endif


bool HubLink::pushCallOuts()
{
  lua_newtable(L);

  PUSH_LUA_CFUNC(L, GetCallIn);
  PUSH_LUA_CFUNC(L, SetCallIn);

  PUSH_LUA_CFUNC(L, Reload);
  PUSH_LUA_CFUNC(L, Disable);

  PUSH_LUA_CFUNC(L, SendData);

  PUSH_LUA_CFUNC(L, Print);
  PUSH_LUA_CFUNC(L, Alert);
  PUSH_LUA_CFUNC(L, PlaySound);

  PUSH_LUA_CFUNC(L, AddTab);
  PUSH_LUA_CFUNC(L, RemoveTab);
  PUSH_LUA_CFUNC(L, ShiftTab);
  PUSH_LUA_CFUNC(L, ClearTab);
  PUSH_LUA_CFUNC(L, RenameTab);
  PUSH_LUA_CFUNC(L, GetTabCount);
  PUSH_LUA_CFUNC(L, GetTabIndex);
  PUSH_LUA_CFUNC(L, GetTabLabel);
  PUSH_LUA_CFUNC(L, GetActiveTab);
  PUSH_LUA_CFUNC(L, SetActiveTab);
  PUSH_LUA_CFUNC(L, GetTabTopic);
  PUSH_LUA_CFUNC(L, SetTabTopic);

  PUSH_LUA_CFUNC(L, GetComposePrompt);
  PUSH_LUA_CFUNC(L, SetComposePrompt);
  PUSH_LUA_CFUNC(L, GetComposeString);
  PUSH_LUA_CFUNC(L, SetComposeString);

  PUSH_LUA_CFUNC(L, CalcMD5);
  PUSH_LUA_CFUNC(L, StripAnsiCodes);

  PUSH_LUA_CFUNC(L, SetBZDB);
  PUSH_LUA_CFUNC(L, GetBZDB);

  PUSH_LUA_CFUNC(L, LoadFile);
  PUSH_LUA_CFUNC(L, SaveFile);
  PUSH_LUA_CFUNC(L, AppendFile);
  PUSH_LUA_CFUNC(L, RemoveFile);

  PUSH_LUA_CFUNC(L, GetCode);
  PUSH_LUA_CFUNC(L, GetTime);
  PUSH_LUA_CFUNC(L, GetVersion);
  PUSH_LUA_CFUNC(L, GetHubServer);
  PUSH_LUA_CFUNC(L, GetServerInfo);
  PUSH_LUA_CFUNC(L, GetOpenGLString);
  PUSH_LUA_CFUNC(L, GetOpenGLNumbers);

  PUSH_LUA_CFUNC(L, GetKeyBindings);

  PUSH_LUA_CFUNC(L, IsVisible);

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


//============================================================================//

int HubLink::GetCallIn(lua_State* L)
{
  const std::string ciName = luaL_checkstring(L, 1);
  std::map<std::string, int>::const_iterator it = name2code.find(ciName);
  if (it == name2code.end()) {
    lua_pushnil(L);
    lua_pushfstring(L, "unknown call-in: %s", ciName.c_str());
    return 2;
  }
  const int ciCode = it->second;

  lua_rawgeti(L, LUA_REGISTRYINDEX, ciCode);

  return 1;
}


int HubLink::SetCallIn(lua_State* L)
{
  const std::string ciName = luaL_checkstring(L, 1);
  std::map<std::string, int>::const_iterator it = name2code.find(ciName);
  if (it == name2code.end()) {
    lua_pushnil(L);
    lua_pushfstring(L, "unknown call-in: %s", ciName.c_str());
    return 2;
  }
  const int ciCode = it->second;

  const int type = lua_type(L, 2);
  if ((type != LUA_TFUNCTION) && (type != LUA_TNIL)) {
    luaL_error(L, "expected function or nil");
  }
    
  lua_rawseti(L, LUA_REGISTRYINDEX, ciCode);

  lua_pushboolean(L, true);
  return 1;
}


//============================================================================//

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


//============================================================================//

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
  const int tabID = CheckTab(L, 1);
  const int distance  = luaL_checkint(L, 2);
  lua_pushboolean(L, controlPanel->shiftTab(tabID, distance));
  return 1;
}


int HubLink::ClearTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = CheckTab(L, 1);
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


int HubLink::GetTabIndex(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = CheckTab(L, 1);
  if (!controlPanel->validTab(tabID)) {
    return 0;
  }
  lua_pushinteger(L, tabID);
  return 1;
}


int HubLink::GetTabLabel(lua_State* L)
{
  if (!controlPanel) {
    lua_pushnil(L);
    return 1;
  }
  const int tabID = CheckTab(L, 1);
  if (!controlPanel->validTab(tabID)) {
    lua_pushnil(L);
    return 1;
  }
  const std::string& label = controlPanel->getTabLabel(tabID);
  if (label.empty()) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstdstring(L, label);
  return 1;
}


int HubLink::GetActiveTab(lua_State* L)
{
  if (!controlPanel) {
    lua_pushnil(L);
    return 1;
  }
  const int tabID = controlPanel->getActiveTab();
  if (tabID < 0) {
    lua_pushnil(L);
    return 1;
  }
  const std::string& label = controlPanel->getTabLabel(tabID);
  if (label.empty()) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstdstring(L, label);
  return 1;
}


int HubLink::SetActiveTab(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = CheckTab(L, 1);
  lua_pushboolean(L, controlPanel->setActiveTab(tabID));
  return 1;
}


int HubLink::GetTabTopic(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = CheckTab(L, 1);
  lua_pushstdstring(L, controlPanel->getTabTopic(tabID));
  return 1;
}


int HubLink::SetTabTopic(lua_State* L)
{
  if (!controlPanel) {
    return 0;
  }
  const int tabID = CheckTab(L, 1);
  const std::string topic = luaL_checkstring(L, 2);
  lua_pushboolean(L, controlPanel->setTabTopic(tabID, topic));
  return 1;
}


//============================================================================//

int HubLink::GetComposePrompt(lua_State* L)
{
  if (hud == NULL) {
    lua_pushnil(L);
    return 1;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    lua_pushnil(L);
    return 1;
  }
  if (!hud->getComposing()) {
    lua_pushnil(L);
    return 1;
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
  hud->setComposePrompt(prompt);
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::GetComposeString(lua_State* L)
{
  if (hud == NULL) {
    lua_pushnil(L);
    return 1;
  }
  HUDuiDefaultKey* defKey = HUDui::getDefaultKey();
  if (!defKey || !dynamic_cast<HubComposeKey*>(defKey)) {
    lua_pushnil(L);
    return 1;
  }
  if (!hud->getComposing()) {
    lua_pushnil(L);
    return 1;
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


//============================================================================//

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


//============================================================================//

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


int HubLink::Alert(lua_State* L)
{
  const std::string msg = luaL_checkstring(L, 1);
  const int   priority  = luaL_optint(L, 2, 0);
  const float duration  = luaL_optfloat(L, 3, 3.0f);
  const bool  warning   = lua_isboolean(L, 4) && lua_toboolean(L, 4);

  hud->setAlert(priority, msg.c_str(), duration, warning);
  return 0;
}


int HubLink::PlaySound(lua_State* L)
{
  const char* soundName  = luaL_checkstring(L, 1);
  const int   soundCode  = SOUNDSYSTEM.getID(soundName);
  const bool  ignoreMute = lua_isboolean(L, 2) && lua_toboolean(L, 2);
  if (soundCode < 0) {
    lua_pushboolean(L, false);
    return 1;
  }

  SOUNDSYSTEM.play(soundCode, NULL, true, true, false, ignoreMute);

  lua_pushboolean(L, true);
  return 1;
}


//============================================================================//

int HubLink::CalcMD5(lua_State* L)
{
  const std::string text = luaL_checkstdstring(L, 1);
  lua_pushstdstring(L, MD5(text).hexdigest());
  return 1;
}


int HubLink::StripAnsiCodes(lua_State* L)
{
  const char* text = luaL_checkstring(L, 1);
  lua_pushstdstring(L, stripAnsiCodes(text));
  return 1;
}


//============================================================================//

int HubLink::SetBZDB(lua_State* L)
{
  const std::string key    = luaL_checkstring(L, 1);
  const std::string value  = luaL_checkstring(L, 2);
  const bool usePersistent = lua_isboolean(L, 3);
  const bool isPersistent  = usePersistent && lua_toboolean(L, 3);

  if (BZDB.isSet(key) &&
      (BZDB.getPermission(key) == StateDatabase::Server)) {
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
    lua_pushnil(L);
    return 1;
  }
  lua_pushstdstring(L, BZDB.get(key));
  return 1;
}

//============================================================================//

static std::string setupFilename(const char* filename)
{
  if ((strlen(filename) <= 4)
   || (strncmp(filename, "hub_", 4) != 0)
   || (strstr(filename,  "..")   != NULL)
   || (strpbrk(filename, "/:\\") != NULL)) {
    return "";
  }
  return getConfigDirName(BZ_CONFIG_DIR_VERSION) + filename;
}


int HubLink::LoadFile(lua_State* L)
{
  const std::string filename = setupFilename(luaL_checkstring(L, 1));
  if (filename.empty()) {
    return 0;
  }

  FILE* file = fopen(filename.c_str(), "rb");
  if (file == NULL) {
    const int errNo = errno;
    lua_pushnil(L);
    lua_pushstring(L, strerror(errNo));
    return 2;
  }

  std::string data;
  char buf[4096];
  while (!feof(file) && !ferror(file)) {
    const size_t bytes = fread(buf, 1, sizeof(buf), file);
    data.append(buf, bytes);
    if (ferror(file)) {
      const int errNo = errno;
      lua_pushnil(L);
      lua_pushstring(L, strerror(errNo));
      lua_pushstdstring(L, data); // partial data
      fclose(file);
      return 3;
    }
  }

  fclose(file);
  lua_pushstdstring(L, data);
  return 1;
}


static int luaSaveFile(lua_State* L, const char* mode)
{
  const std::string filename = setupFilename(luaL_checkstring(L, 1));
  const std::string data = luaL_checkstdstring(L, 2);
  if (filename.empty()) {
    lua_pushnil(L);
    lua_pushstring(L, "invalid filename");
    return 2;
  }

  FILE* file = fopen(filename.c_str(), mode);
  if (file == NULL) {
    const int errNo = errno;
    lua_pushnil(L);
    lua_pushstring(L, strerror(errNo));
    return 2;
  }

  fwrite(data.c_str(), 1, data.size(), file);
  if (ferror(file)) {
    const int errNo = errno;
    lua_pushnil(L);
    lua_pushstring(L, strerror(errNo));
    fclose(file);
    return 2;
  }

  fclose(file);
  lua_pushboolean(L, true);
  return 1;
}


int HubLink::SaveFile(lua_State* L)
{
  return luaSaveFile(L, "wb");
}


int HubLink::AppendFile(lua_State* L)
{
  return luaSaveFile(L, "ab");
}


int HubLink::RemoveFile(lua_State* L)
{
  const std::string filename = setupFilename(luaL_checkstring(L, 1));
  if (filename.empty()) {
    lua_pushnil(L);
    lua_pushstring(L, "invalid filename");
    return 2;
  }

  if (remove(filename.c_str()) != 0) {
    const int errNo = errno;
    lua_pushnil(L);
    lua_pushstring(L, strerror(errNo));
    return 2;
  }

  lua_pushboolean(L, true); 
  return 1;
}


//============================================================================//

int HubLink::GetCode(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    lua_pushnil(L);
    return 1;
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
  lua_pushstdstring(L, getOSString());
  return 3;
}


int HubLink::GetHubServer(lua_State* L)
{
  HubLink* link = GetLink(L);
  if (link == NULL) {
    lua_pushnil(L);
    return 1;
  }
  lua_pushstdstring(L, link->getHostPort());
  return 1;
}


int HubLink::GetServerInfo(lua_State* L)
{
  const ServerLink* srvLink = ServerLink::getServer();
  if ((srvLink == NULL) ||
      (srvLink->getState() != ServerLink::Okay)) {
    lua_pushnil(L);
    return 1;
  }

  lua_pushstdstring(L, srvLink->getJoinServer());
  lua_pushinteger(L,   srvLink->getJoinPort());
  lua_pushstdstring(L, srvLink->getJoinCallsign());

  return 3;
}


int HubLink::GetOpenGLString(lua_State* L)
{
  const int pname = (GLenum) luaL_checkint(L, 1);
  lua_pushstring(L, (const char*)glGetString((GLenum)pname));
  return 1;
}


int HubLink::GetOpenGLNumbers(lua_State* L)
{
  const int pname = luaL_checkint(L, 1);
  const int count = luaL_optint(L, 2, 1);
  GLdouble buf[256];
  if ((count < 0) || (count > 256)) {
    lua_pushnil(L);
    return 1;
  }
  glGetDoublev(pname, buf);
  lua_checkstack(L, count);
  for (int i = 0; i < count; i++) {
    lua_pushdouble(L, buf[i]);
  }
  return count;
}


//============================================================================//

int HubLink::GetKeyBindings(lua_State* L)
{
  const std::string command = luaL_checkstring(L, 1);
  const bool press = !lua_isboolean(L, 2) || lua_toboolean(L, 2);
  std::vector<std::string> keys = KEYMGR.getKeysFromCommand(command, press);
  lua_newtable(L);
  for (size_t i = 0; i < keys.size(); i++) {
    lua_pushstdstring(L, keys[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


int HubLink::IsVisible(lua_State* L)
{
  lua_pushboolean(L, !isUnmapped());
  return 1;
}

//============================================================================//

#undef  PACK_TYPE
#define PACK_TYPE(label, type) \
  const type value = luaL_checkint(L, 1); \
  char buf[sizeof(type)]; \
  nboPack ## label(buf, value); \
  lua_pushlstring(L, buf, sizeof(type)); \
  return 1;
int HubLink::PackInt8  (lua_State* L) { PACK_TYPE(Int8,   int8_t)   }
int HubLink::PackInt16 (lua_State* L) { PACK_TYPE(Int16,  int16_t)  }
int HubLink::PackInt32 (lua_State* L) { PACK_TYPE(Int32,  int32_t)  }
int HubLink::PackInt64 (lua_State* L) { PACK_TYPE(Int64,  int64_t)  }
int HubLink::PackUInt8 (lua_State* L) { PACK_TYPE(UInt8,  uint8_t)  }
int HubLink::PackUInt16(lua_State* L) { PACK_TYPE(UInt16, uint16_t) }
int HubLink::PackUInt32(lua_State* L) { PACK_TYPE(UInt32, uint32_t) }
int HubLink::PackUInt64(lua_State* L) { PACK_TYPE(UInt64, uint64_t) }
int HubLink::PackFloat (lua_State* L) { PACK_TYPE(Float,  float)    }
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
int HubLink::UnpackInt8  (lua_State* L) { UNPACK_TYPE(Int8,   int8_t)   }
int HubLink::UnpackInt16 (lua_State* L) { UNPACK_TYPE(Int16,  int16_t)  } 
int HubLink::UnpackInt32 (lua_State* L) { UNPACK_TYPE(Int32,  int32_t)  }
int HubLink::UnpackInt64 (lua_State* L) { UNPACK_TYPE(Int64,  int64_t)  }
int HubLink::UnpackUInt8 (lua_State* L) { UNPACK_TYPE(UInt8,  uint8_t)  }
int HubLink::UnpackUInt16(lua_State* L) { UNPACK_TYPE(UInt16, uint16_t) } 
int HubLink::UnpackUInt32(lua_State* L) { UNPACK_TYPE(UInt32, uint32_t) }
int HubLink::UnpackUInt64(lua_State* L) { UNPACK_TYPE(UInt64, uint64_t) }
int HubLink::UnpackFloat (lua_State* L) { UNPACK_TYPE(Float,  float)    }
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
    int bits = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (bits == -1) {
      return 0;
    }
    fcntl(STDIN_FILENO, F_SETFL, bits | O_NONBLOCK);
    char buf[4096];
    const int r = read(STDIN_FILENO, buf, sizeof(buf));
    fcntl(STDIN_FILENO, F_SETFL, bits & ~O_NONBLOCK);
    if (r <= 0) {
      return 0;
    }
    lua_pushlstring(L, buf, r);
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
