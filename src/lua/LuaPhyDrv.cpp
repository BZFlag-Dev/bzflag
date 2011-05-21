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
#include "LuaPhyDrv.h"

// system headers
#include <string>
using std::string;

// common headers
#include "PhysicsDriver.h"

// local headers
#include "LuaHeader.h"


//============================================================================//
//============================================================================//

bool LuaPhyDrv::PushEntries(lua_State* L) {
  PUSH_LUA_CFUNC(L, GetPhyDrvID);
  PUSH_LUA_CFUNC(L, GetPhyDrvName);
  PUSH_LUA_CFUNC(L, GetPhyDrvDeath);
  PUSH_LUA_CFUNC(L, GetPhyDrvSlideTime);
  PUSH_LUA_CFUNC(L, GetPhyDrvVelocity);
  PUSH_LUA_CFUNC(L, GetPhyDrvRadialPos);
  PUSH_LUA_CFUNC(L, GetPhyDrvRadialVel);
  PUSH_LUA_CFUNC(L, GetPhyDrvAngularPos);
  PUSH_LUA_CFUNC(L, GetPhyDrvAngularVel);

  return true;
}


//============================================================================//
//============================================================================//

static inline const PhysicsDriver* ParsePhyDrv(lua_State* L, int index) {
  const int phydrvIndex = luaL_checkint(L, index);
  return PHYDRVMGR.getDriver(phydrvIndex);
}


//============================================================================//
//============================================================================//

int LuaPhyDrv::GetPhyDrvID(lua_State* L) {
  const std::string name = luaL_checkstring(L, 1);
  const int phydrvID = PHYDRVMGR.findDriver(name);
  if (phydrvID < 0) {
    lua_pushboolean(L, false);
  }
  else {
    lua_pushint(L, phydrvID);
  }
  return 1;
}


int LuaPhyDrv::GetPhyDrvName(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, phydrv->getName());
  return 1;
}


int LuaPhyDrv::GetPhyDrvDeath(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  if (!phydrv->getIsDeath()) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, phydrv->getDeathMsg());
  return 1;
}


int LuaPhyDrv::GetPhyDrvSlideTime(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  if (!phydrv->getIsSlide()) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, phydrv->getSlideTime());
  return 1;
}


int LuaPhyDrv::GetPhyDrvVelocity(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, phydrv->getLinearVel());
  return 3;
}


int LuaPhyDrv::GetPhyDrvRadialPos(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec2(L, phydrv->getRadialPos());
  return 2;
}


int LuaPhyDrv::GetPhyDrvRadialVel(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, phydrv->getRadialVel());
  return 1;
}


int LuaPhyDrv::GetPhyDrvAngularPos(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec2(L, phydrv->getAngularPos());
  return 2;
}


int LuaPhyDrv::GetPhyDrvAngularVel(lua_State* L) {
  const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
  if (phydrv == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, phydrv->getAngularVel());
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
// ex: shiftwidth=2 tabstop=8 expandtab
