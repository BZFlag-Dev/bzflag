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

#ifndef LUA_VFS_H
#define LUA_VFS_H

#include "common.h"


struct lua_State;


class LuaVFS {
  public:
    LuaVFS();
    ~LuaVFS();
    static bool PushEntries(lua_State* L);

  private:
    static int FileExists(lua_State* L);
    static int FileSize(lua_State* L);
    static int ReadFile(lua_State* L);
    static int WriteFile(lua_State* L);
    static int AppendFile(lua_State* L);
    static int RemoveFile(lua_State* L);
    static int RenameFile(lua_State* L);
    static int Include(lua_State* L);
    static int CreateDir(lua_State* L);
    static int DirList(lua_State* L);
    static int GetModes(lua_State* L);
};


#endif // LUA_VFS_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
