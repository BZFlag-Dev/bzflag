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
#include "LuaControl.h"

// common headers
#include "CommandManager.h"

// bzflag headers
#include "bzflag/guiplaying.h"
#include "clientbase/playing.h"
#include "clientbase/LocalPlayer.h"

// local headers
#include "LuaHandle.h"
#include "LuaHeader.h"
#include "LuaBzOrg.h"


//============================================================================//
//============================================================================//

bool LuaControl::PushEntries(lua_State* L)
{
  PUSH_LUA_CFUNC(L, Move);
  PUSH_LUA_CFUNC(L, Fire);
  PUSH_LUA_CFUNC(L, Jump);
  PUSH_LUA_CFUNC(L, Spawn);
  PUSH_LUA_CFUNC(L, Pause);
  PUSH_LUA_CFUNC(L, DropFlag);
  PUSH_LUA_CFUNC(L, SetTarget);

  return true;
}


//============================================================================//
//============================================================================//

static inline bool ValidScript(lua_State* L)
{
  return ((L2H(L) == luaBzOrg) || LuaHandle::GetDevMode());
}


//============================================================================//
//============================================================================//

int LuaControl::Move(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control movement");
  }
  const float speed  = luaL_optfloat(L, 1, 0.0f);
  const float angVel = luaL_optfloat(L, 2, 0.0f);
  forceControls(true, speed, angVel);
  return 0;
}


int LuaControl::Fire(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control firing");
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    lua_pushboolean(L, false);
    return 1;
  }
  lua_pushboolean(L, myTank->fireShot() != NULL);
  return 1;
}


int LuaControl::Jump(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control jumps");
  }
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return 0;
  }
  myTank->doJump();
  return 0;
}


int LuaControl::Spawn(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control spawns");
  }
  CMDMGR.run("restart");
  return 0;
}


int LuaControl::Pause(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control pausing");
  }
  CMDMGR.run("pause");
  return 0;
}


int LuaControl::DropFlag(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control flag drops");
  }
  CMDMGR.run("drop");
  return 0;
}


int LuaControl::SetTarget(lua_State* L)
{
  if (!ValidScript(L)) {
    luaL_error(L, "this script can not control target locks");
  }
  CMDMGR.run("identify");
  return 0;
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
