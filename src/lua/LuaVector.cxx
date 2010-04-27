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
#include "LuaVector.h"

// system headers
#include <vector>
using std::vector;

// local headers
#include "LuaHeader.h"


//============================================================================//
//============================================================================//

bool LuaVector::PushEntries(lua_State* L)
{
  lua_pushliteral(L, "vector");
  lua_newtable(L);

  PUSH_LUA_CFUNC(L, dot);
  PUSH_LUA_CFUNC(L, cross);
  PUSH_LUA_CFUNC(L, normdot);
  PUSH_LUA_CFUNC(L, normdotdegs);
  PUSH_LUA_CFUNC(L, normdotrads);
  PUSH_LUA_CFUNC(L, length);
  PUSH_LUA_CFUNC(L, normalize);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

int LuaVector::dot(lua_State* L)
{
  const int pairs = (lua_gettop(L) / 2);
  if (pairs < 1) {
    return luaL_pushnil(L);
  }
  float sum = 0.0f;
  for (int p = 1; p <= pairs; p++) {
    const float a = luaL_checkfloat(L, p);
    const float b = luaL_checkfloat(L, p + pairs);
    sum += a * b;
  }
  lua_checkstack(L, 1);
  lua_pushfloat(L, sum);
  return 1;
}


int LuaVector::cross(lua_State* L)
{
  if (lua_gettop(L) != 6) {
    return luaL_pushnil(L);
  }
  const fvec3 a = luaL_checkfvec3(L, 1);
  const fvec3 b = luaL_checkfvec3(L, 4);
  lua_pushfvec3(L, fvec3::cross(a, b));
  return 3;
}


static inline bool NormalizedDot(lua_State* L, float& cosval)
{
  const int pairs = (lua_gettop(L) / 2);
  if (pairs < 1) {
    return false;
  }
  float sum  = 0.0f;
  float sumA = 0.0f;
  float sumB = 0.0f;
  for (int p = 1; p <= pairs; p++) {
    const float a = luaL_checkfloat(L, p);
    const float b = luaL_checkfloat(L, p + pairs);
    sum  += a * b;
    sumA += a * a;
    sumB += b * b;
  }
  const float sqrtAB = sqrtf(sumA) * sqrtf(sumB);
  if (sqrtAB == 0.0f) {
    cosval = 0.0f;
  } else {
    cosval = sum / (sqrtAB);
  }
  return true;
}


int LuaVector::normdot(lua_State* L)
{
  float cosval;
  if (!NormalizedDot(L, cosval)) {
    return luaL_pushnil(L);
  }
  lua_checkstack(L, 1);
  lua_pushfloat(L, cosval);
  return 1;
}


int LuaVector::normdotdegs(lua_State* L)
{
  float cosval;
  if (!NormalizedDot(L, cosval)) {
    return luaL_pushnil(L);
  }
  lua_checkstack(L, 1);
  lua_pushfloat(L, (lua_Number)(acos(cosval) * (180.0 / M_PI)));
  return 1;
}


int LuaVector::normdotrads(lua_State* L)
{
  float cosval;
  if (!NormalizedDot(L, cosval)) {
    return luaL_pushnil(L);
  }
  lua_checkstack(L, 1);
  lua_pushfloat(L, acos(cosval));
  return 1;
}


int LuaVector::length(lua_State* L)
{
  const int count = lua_gettop(L);
  float sum = 0.0f;
  for (int i = 1; i <= count; i++) {
    const float v = luaL_checkfloat(L, i);
    sum += v * v;
  }
  lua_checkstack(L, 1);
  lua_pushfloat(L, sqrtf(sum));
  return 1;
}


int LuaVector::normalize(lua_State* L)
{
  static vector<float> data;
  const int count = lua_gettop(L);
  data.resize(count + 1); // not using data[0]

  float sum = 0.0f;
  for (int i = 1; i <= count; i++) {
    const float v = luaL_checkfloat(L, i);
    data[i] = v;
    sum += (v * v);
  }
  sum = sqrtf(sum);
  if (sum == 0.0f) {
    return count;
  }
  lua_checkstack(L, count);
  const float scale = (1.0f / sum);
  for (int i = 1; i <= count; i++) {
    lua_pushfloat(L, data[i] * scale);
  }
  return count;
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
