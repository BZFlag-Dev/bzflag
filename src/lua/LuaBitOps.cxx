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
#include "LuaBitOps.h"

// local headers
#include "LuaHeader.h"
#include "LuaUtils.h"


const int mask = 0x00FFFFFF; // 2^24, a float's contiguous integer range


//============================================================================//
//============================================================================//

bool LuaBitOps::PushEntries(lua_State* L)
{
  lua_pushliteral(L, "bit");
  lua_newtable(L);

  luaset_strfunc(L, "bit_test", bit_test);
  luaset_strfunc(L, "bit_or",   bit_or);
  luaset_strfunc(L, "bit_and",  bit_and);
  luaset_strfunc(L, "bit_xor",  bit_xor);
  luaset_strfunc(L, "bit_inv",  bit_inv);
  luaset_strfunc(L, "bit_bits", bit_bits);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static inline unsigned int luaL_checkuint(lua_State* L, int index)
{
  return (unsigned int)luaL_checkint(L, index);
}


int LuaBitOps::bit_test(lua_State* L)
{
  const unsigned int value = luaL_checkuint(L, 1) & mask;
  const unsigned int bit   = luaL_checkuint(L, 2) & mask;
  lua_pushboolean(L, value & bit);
  return 1;
}


int LuaBitOps::bit_or(lua_State* L)
{
  unsigned int result = 0x00000000;
  for (int i = 1; !lua_isnone(L, i); i++) {
    result = result | luaL_checkuint(L, i);
  }
  lua_pushinteger(L, result & mask);
  return 1;
}


int LuaBitOps::bit_and(lua_State* L)
{
  unsigned int result = 0xFFFFFFFF;
  for (int i = 1; !lua_isnone(L, i); i++) {
    result = result & luaL_checkuint(L, i);
  }
  lua_pushinteger(L, result & mask);
  return 1;
}


int LuaBitOps::bit_xor(lua_State* L)
{
  unsigned int result = 0x00000000;
  for (int i = 1; !lua_isnone(L, i); i++) {
    result = result ^ luaL_checkuint(L, i);
  }
  lua_pushinteger(L, result & mask);
  return 1;
}


int LuaBitOps::bit_inv(lua_State* L)
{
  const unsigned int result = ~luaL_checkuint(L, 1);
  lua_pushinteger(L, result & mask);
  return 1;
}


int LuaBitOps::bit_bits(lua_State* L)
{
  unsigned int result = 0x00000000;
  for (int i = 1; !lua_isnone(L, i); i++) {
    const int bit = (unsigned int)luaL_checkint(L, i);
    result = result | (1 << bit);
  }
  lua_pushinteger(L, result & mask);
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
