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
#include "LuaConsole.h"

// system headers
#include <string>
using std::string;

// common headers

// bzflag headers
#include "bzflag/guiplaying.h"
#include "bzflag/ControlPanel.h"

// local headers
#include "LuaHeader.h"


typedef ControlPanel::MessageQueue MessageQueue;


//============================================================================//
//============================================================================//

bool LuaConsole::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, GetTabMessageCount);
  PUSH_LUA_CFUNC(L, GetTabMessages);
  PUSH_LUA_CFUNC(L, GetTabTopic);
  PUSH_LUA_CFUNC(L, SetTabTopic);
  PUSH_LUA_CFUNC(L, GetActiveTab);
  PUSH_LUA_CFUNC(L, SetActiveTab);
  PUSH_LUA_CFUNC(L, AddTab);
  PUSH_LUA_CFUNC(L, RemoveTab);
  PUSH_LUA_CFUNC(L, ClearTab);
  PUSH_LUA_CFUNC(L, SwapTabs);
  PUSH_LUA_CFUNC(L, GetTabID);
  PUSH_LUA_CFUNC(L, GetTabLabel);
  PUSH_LUA_CFUNC(L, IsValidTab);
  PUSH_LUA_CFUNC(L, IsTabLocked);
  PUSH_LUA_CFUNC(L, IsTabUnread);
  PUSH_LUA_CFUNC(L, IsTabVisible);

  return true;
}


//============================================================================//
//============================================================================//

static inline int ParseTab(lua_State* L, int index)
{
  if (controlPanel == NULL) {
    return -1;
  }

  int tabID = -1;

  if (lua_israwnumber(L, index)) {
    tabID = lua_toint(L, index);
  }
  else if (lua_israwstring(L, index)) {
    const string label = lua_tostring(L, index);
    tabID = controlPanel->getTabID(label);
  }
  else {
    luaL_error(L, "expected string or number");
  }

  return controlPanel->validTab(tabID) ? tabID : -1;
}


//============================================================================//
//============================================================================//

int LuaConsole::GetTabMessageCount(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  const MessageQueue* msgs = controlPanel->getTabMessages(tabID);
  if (msgs == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, (int)msgs->size());
  return 1;
}


int LuaConsole::GetTabMessages(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  const MessageQueue* msgs = controlPanel->getTabMessages(tabID);
  if (msgs == NULL) {
    return luaL_pushnil(L);
  }

  const size_t msgSize = msgs->size();
  size_t count = (size_t)luaL_optint(L, 2, (int)msgSize);
  if (count > msgSize) {
    count = msgSize;
  }

  const size_t start = msgSize - count;

  lua_createtable(L, count, 0);
  int index = 0;
  for (size_t m = start; m < msgSize; m++) {
    index++;
    const ControlPanelMessage& msg = (*msgs)[m];
    lua_pushlstring(L, msg.data.data(), msg.data.size());
    lua_rawseti(L, -2, index);
  }

  return 1;
}


//============================================================================//

int LuaConsole::GetTabTopic(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, controlPanel->getTabTopic(tabID));
  return 1;
}


int LuaConsole::SetTabTopic(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  const std::string topic = luaL_checkstring(L, 2);
  lua_pushboolean(L, controlPanel->setTabTopic(tabID, topic));
  return 1;
}


int LuaConsole::GetActiveTab(lua_State* L)
{
  if (controlPanel == NULL) {
    return luaL_pushnil(L);
  }
  const int tabID = controlPanel->getActiveTab();
  lua_pushinteger(L, tabID);
  return 1;
}


int LuaConsole::SetActiveTab(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, controlPanel->setActiveTab(tabID));
  return 1;
}


int LuaConsole::AddTab(lua_State* L)
{
  if (controlPanel == NULL) {
    return luaL_pushnil(L);
  }
  const std::string label = luaL_checkstring(L, 1);
  const bool allSrc = lua_isboolean(L, 2) && lua_tobool(L, 2);
  const bool allDst = lua_isboolean(L, 3) && lua_tobool(L, 3);
  lua_pushboolean(L, controlPanel->addTab(label, allSrc, allDst));
  return 1;
}


int LuaConsole::RemoveTab(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  const std::string label = controlPanel->getTabLabel(tabID);
  lua_pushboolean(L, controlPanel->removeTab(label));
  return 1;
}


int LuaConsole::RenameTab(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  const std::string oldLabel = controlPanel->getTabLabel(tabID);
  const std::string newLabel = luaL_checkstring(L, 2);
  lua_pushboolean(L, controlPanel->renameTab(oldLabel, newLabel));
  return 1;
}


int LuaConsole::ClearTab(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, controlPanel->clearTab(tabID));
  return 1;
}


int LuaConsole::SwapTabs(lua_State* L)
{
  const int tabID1 = ParseTab(L, 1);
  const int tabID2 = ParseTab(L, 2);
  if ((tabID1 < 0) || (tabID2 < 0)) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, controlPanel->swapTabs(tabID1, tabID2));
  return 1;
}


int LuaConsole::GetTabID(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, tabID);
  return 1;
}


int LuaConsole::GetTabLabel(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, controlPanel->getTabLabel(tabID));
  return 1;
}


int LuaConsole::IsValidTab(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  lua_pushboolean(L, tabID >= 0);
  return 1;
}


int LuaConsole::IsTabLocked(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0)  {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, controlPanel->isTabLocked(tabID));
  return 1;
}



int LuaConsole::IsTabUnread(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0)  {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, controlPanel->tabUnread(tabID));
  return 1;
}


int LuaConsole::IsTabVisible(lua_State* L)
{
  const int tabID = ParseTab(L, 1);
  if (tabID < 0)  {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, controlPanel->isTabVisible(tabID));
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
