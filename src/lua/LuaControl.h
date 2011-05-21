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

#ifndef LUA_CONTROL_H
#define LUA_CONTROL_H

struct lua_State;


class LuaControl {
  public:
    static bool PushEntries(lua_State* L);

  private:
    static int Move(lua_State* L);
    static int Fire(lua_State* L);
    static int Jump(lua_State* L);
    static int Spawn(lua_State* L);
    static int Pause(lua_State* L);
    static int DropFlag(lua_State* L);
    static int SetTarget(lua_State* L);
};


#endif // LUA_CONTROL_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
