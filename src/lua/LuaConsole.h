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

#ifndef LUA_CONSOLE_H
#define LUA_CONSOLE_H

struct lua_State;


class LuaConsole {
  public:
    static bool PushEntries(lua_State* L);

  private: // call-outs
    static int GetTabMessageCount(lua_State* L);
    static int GetTabMessages(lua_State* L);
    static int GetTabTopic(lua_State* L);
    static int SetTabTopic(lua_State* L);
    static int GetActiveTab(lua_State* L);
    static int SetActiveTab(lua_State* L);
    static int AddTab(lua_State* L);
    static int RemoveTab(lua_State* L);
    static int RenameTab(lua_State* L);
    static int ClearTab(lua_State* L);
    static int SwapTabs(lua_State* L);
    static int GetTabID(lua_State* L);
    static int GetTabLabel(lua_State* L);
    static int IsValidTab(lua_State* L);
    static int IsTabLocked(lua_State* L);
    static int IsTabUnread(lua_State* L);
    static int IsTabVisible(lua_State* L);
};


#endif // LUA_CONSOLE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
