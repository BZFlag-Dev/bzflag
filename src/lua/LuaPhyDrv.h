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

#ifndef LUA_PHYDRV_H
#define LUA_PHYDRV_H

struct lua_State;


class LuaPhyDrv {
  public:
    static bool PushEntries(lua_State* L);

  private: // call-outs
    static int GetPhyDrvID(lua_State* L);
    static int GetPhyDrvName(lua_State* L);
    static int GetPhyDrvDeath(lua_State* L);
    static int GetPhyDrvSlideTime(lua_State* L);
    static int GetPhyDrvVelocity(lua_State* L);
    static int GetPhyDrvRadialPos(lua_State* L);
    static int GetPhyDrvRadialVel(lua_State* L);
    static int GetPhyDrvAngularPos(lua_State* L);
    static int GetPhyDrvAngularVel(lua_State* L);
};


#endif // LUA_PHYDRV_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
