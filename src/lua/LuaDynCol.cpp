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
#include "LuaDynCol.h"

// system headers
#include <string>
using std::string;

// common headers
#include "DynamicColor.h"

// local headers
#include "LuaHeader.h"


//============================================================================//
//============================================================================//

bool LuaDynCol::PushEntries(lua_State* L) {
  PUSH_LUA_CFUNC(L, GetDynColName);
  PUSH_LUA_CFUNC(L, GetDynCol);
  PUSH_LUA_CFUNC(L, SetDynCol);
  PUSH_LUA_CFUNC(L, GetDynColCanHaveAlpha);

  return true;
}


//============================================================================//
//============================================================================//

static inline const DynamicColor* ParseDynCol(lua_State* L, int index) {
  const int dyncolIndex = luaL_checkint(L, index);
  return DYNCOLORMGR.getColor(dyncolIndex);
}


//============================================================================//
//============================================================================//

int LuaDynCol::GetDynColName(lua_State* L) {
  const DynamicColor* dyncol = ParseDynCol(L, 1);
  if (dyncol == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, dyncol->getName());
  return 1;
}


int LuaDynCol::GetDynCol(lua_State* L) {
  const DynamicColor* dyncol = ParseDynCol(L, 1);
  if (dyncol == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec4(L, dyncol->getColor());
  return 4;
}


int LuaDynCol::SetDynCol(lua_State* L) {
  DynamicColor* dyncol = const_cast<DynamicColor*>(ParseDynCol(L, 1));
  if (dyncol == NULL) {
    return luaL_pushnil(L);
  }
  const fvec4& oldColor = dyncol->getColor();
  const fvec4 newColor = luaL_optfvec4(L, 2, oldColor);
  dyncol->setColor(newColor);
  lua_pushboolean(L, true);
  return 1;
}


int LuaDynCol::GetDynColCanHaveAlpha(lua_State* L) {
  const DynamicColor* dyncol = ParseDynCol(L, 1);
  if (dyncol == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, dyncol->canHaveAlpha());
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
