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
#include "LuaFloat.h"

// system headers
#include <math.h>
#include <string>

// local headers
#include "LuaHeader.h"


const char* LuaFloat::metaName = "Float";


//============================================================================//
//============================================================================//

bool LuaFloat::IsFloat(lua_State* L, int index)
{
  return (luaL_testudata(L, index, metaName) != NULL);
}


float* LuaFloat::TestFloat(lua_State* L, int index)
{
  return (float*)luaL_testudata(L, index, metaName);
}


float* LuaFloat::TestNumber(lua_State* L, int index)
{
  static float value = 0.0f;
  if (lua_israwnumber(L, index)) {
    value = lua_tofloat(L, 1);
    return &value;
  }
  return TestFloat(L, index);
}


float LuaFloat::CheckFloat(lua_State* L, int index)
{
  return *((float*)luaL_checkudata(L, index, metaName));
}


float LuaFloat::CheckNumber(lua_State* L, int index)
{
  if (lua_israwnumber(L, index)) {
    return lua_tofloat(L, index);
  }
  return CheckFloat(L, index);
}


//============================================================================//
//============================================================================//

int LuaFloat::MetaIndex(lua_State* L)
{
  const float f1 = CheckFloat(L, 1);
  const std::string key = luaL_checkstring(L, 2);

  if (key == "number") {
    lua_pushfloat(L, f1);
    return 1;
  }
  else if (key == "string") {
    return MetaToString(L);
  }
  else if (key == "floor") { PushFloat(L, floorf(f1)); return 1; }
  else if (key == "ceil")  { PushFloat(L, ceilf(f1));  return 1; }
  else if (key == "sqrt")  { PushFloat(L, sqrtf(f1));  return 1; }
  else if (key == "abs")   { PushFloat(L, fabsf(f1));  return 1; }
  else if (key == "cos")   { PushFloat(L, cosf(f1));   return 1; }
  else if (key == "sin")   { PushFloat(L, sinf(f1));   return 1; }
  else if (key == "tan")   { PushFloat(L, tanf(f1));   return 1; }
  else if (key == "acos")  { PushFloat(L, acosf(f1));  return 1; }
  else if (key == "asin")  { PushFloat(L, asinf(f1));  return 1; }
  else if (key == "atan")  { PushFloat(L, atanf(f1));  return 1; }
  else if (key == "exp")   { PushFloat(L, expf(f1));   return 1; }
  else if (key == "log")   { PushFloat(L, logf(f1));   return 1; }
  else if (key == "log10") { PushFloat(L, log10f(f1)); return 1; }
  else if (key == "cosh")  { PushFloat(L, coshf(f1));  return 1; }
  else if (key == "sinh")  { PushFloat(L, sinhf(f1));  return 1; }
  else if (key == "tanh")  { PushFloat(L, tanhf(f1));  return 1; }

  return 0;
}


int LuaFloat::MetaToString(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  char buf[128];
  snprintf(buf, sizeof(buf), "%.14g", f1);
  lua_pushstring(L, buf);
  return 1;
}


int LuaFloat::MetaADD(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  PushFloat(L, f1 + f2);
  return 1;
}


int LuaFloat::MetaSUB(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  PushFloat(L, f1 - f2);
  return 1;
}


int LuaFloat::MetaMUL(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  PushFloat(L, f1 * f2);
  return 1;
}


int LuaFloat::MetaDIV(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  PushFloat(L, f1 / f2);
  return 1;
}


int LuaFloat::MetaMOD(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  PushFloat(L, fmod(f1, f2));
  return 1;
}


int LuaFloat::MetaPOW(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  PushFloat(L, pow(f1, f2));
  return 1;
}


int LuaFloat::MetaUNM(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  PushFloat(L, -f1);
  return 1;
}


int LuaFloat::MetaEQ(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  lua_pushboolean(L, f1 == f2);
  return 1;
}


int LuaFloat::MetaLT(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  lua_pushboolean(L, f1 < f2);
  return 1;
}


int LuaFloat::MetaLE(lua_State* L)
{
  const float f1 = CheckNumber(L, 1);
  const float f2 = CheckNumber(L, 2);
  lua_pushboolean(L, f1 <= f2);
  return 1;
}


//============================================================================//
//============================================================================//

static void PushNamedFunction(lua_State* L, const char* name,
                              int (*func)(lua_State*))
{
  lua_pushstring(L, name);
  lua_pushcfunction(L, func);
  lua_rawset(L, -3);
}


bool LuaFloat::CreateMetatable(lua_State* L)
{
  luaL_newmetatable(L, metaName);

  PushNamedFunction(L, "__index",    MetaIndex);
  PushNamedFunction(L, "__tostring", MetaToString);

  PushNamedFunction(L, "__add",  MetaADD);
  PushNamedFunction(L, "__sub",  MetaSUB);
  PushNamedFunction(L, "__mul",  MetaMUL);
  PushNamedFunction(L, "__div",  MetaDIV);
  PushNamedFunction(L, "__mod",  MetaMOD);
  PushNamedFunction(L, "__pow",  MetaPOW);
  PushNamedFunction(L, "__unm",  MetaUNM);
  PushNamedFunction(L, "__eq",   MetaEQ);
  PushNamedFunction(L, "__lt",   MetaLT);
  PushNamedFunction(L, "__le",   MetaLE);

  lua_pushliteral(L, "__metatable");
  lua_pushliteral(L, "no access");
  lua_rawset(L, -3);

  lua_pop(L, 1);

  return true;
}

//============================================================================//
//============================================================================//

int LuaFloat::IsFloat(lua_State* L)
{
  lua_pushboolean(L, TestFloat(L, 1) != NULL);
  return 1;
}


int LuaFloat::CreateFloat(lua_State* L)
{
  float _value = 0.0f;
  if (lua_israwnumber(L, 1)) {
    _value = lua_tofloat(L, 1);
  }
  else if (lua_israwstring(L, 1)) {
    const char* start = lua_tostring(L, 1);
    char* end;
    _value = (float)strtod(start, &end);
    if (start == end) {
      luaL_argerror(L, 1, "invalid numeric string");
    }
  }
  else {
    _value = CheckFloat(L, 1);
  }

  float* floatPtr = (float*)lua_newuserdata(L, sizeof(float));
  *floatPtr = _value;

  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);

  return 1;
}


void LuaFloat::PushFloat(lua_State* L, float value)
{
  float* floatPtr = (float*)lua_newuserdata(L, sizeof(float));
  *floatPtr = value;

  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);
}


//============================================================================//
//============================================================================//

bool LuaFloat::PushEntries(lua_State* L)
{
  CreateMetatable(L);

  PushNamedFunction(L, "float", CreateFloat);
  PushNamedFunction(L, "isfloat", IsFloat);

  return true;
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
