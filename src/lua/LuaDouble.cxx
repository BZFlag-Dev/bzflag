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
#include "LuaDouble.h"

// system headers
#include <math.h>
#include <string>
#include <string.h>
using std::string;

// local headers
#include "LuaHeader.h"


const char* LuaDouble::metaName = "Double";


//============================================================================//
//============================================================================//

bool LuaDouble::IsDouble(lua_State* L, int index)
{
  return (lua_getuserdataextra(L, index) == metaName);
}


double* LuaDouble::TestDouble(lua_State* L, int index)
{
  if (lua_getuserdataextra(L, index) != metaName) {
    return NULL;
  }
  double* doublePtr = (double*)lua_touserdata(L, index);
  return doublePtr;
}


double* LuaDouble::TestNumber(lua_State* L, int index)
{
  static double value = 0.0f;
  if (lua_israwnumber(L, index)) {
    value = (double)lua_tonumber(L, index);
    return &value;
  }
  if (lua_getuserdataextra(L, index) != metaName) {
    return NULL;
  }
  double* doublePtr = (double*)lua_touserdata(L, index);
  return doublePtr;
}


double LuaDouble::CheckDouble(lua_State* L, int index)
{
  if (lua_getuserdataextra(L, index) != metaName) {
    luaL_typerror(L, index, "Double");
  }
  const double* doublePtr = (double*)lua_touserdata(L, index);
  return *doublePtr;
}


double LuaDouble::CheckDouble(lua_State* L, int index, const char* type)
{
  if (lua_getuserdataextra(L, index) != metaName) {
    luaL_typerror(L, index, type);
  }
  const double* doublePtr = (double*)lua_touserdata(L, index);
  return *doublePtr;
}


double LuaDouble::CheckNumber(lua_State* L, int index)
{
  if (lua_israwnumber(L, index)) {
    return (double)lua_tonumber(L, index);
  }
  return CheckDouble(L, index, "Double or number");
}


double LuaDouble::CheckNumber(lua_State* L, int index, const char* type)
{
  if (lua_israwnumber(L, index)) {
    return (double)lua_tonumber(L, index);
  }
  return CheckDouble(L, index, type);
}


void LuaDouble::PushDouble(lua_State* L, double value)
{
  double* doublePtr = (double*)lua_newuserdata(L, sizeof(double));
  *doublePtr = value;

  lua_setuserdataextra(L, -1, (void*)metaName);
  luaL_getmetatable(L, metaName);
  lua_setmetatable(L, -2);
}


int LuaDouble::CreateDouble(lua_State* L, int index)
{
  if (!lua_israwstring(L, index)) {
    PushDouble(L, CheckNumber(L, index, "Double, number, or string"));
    return 1;
  }

  const char* start = lua_tostring(L, index);
  char* end;
  double value = strtod(start, &end);
  if (start == end) {
    luaL_argerror(L, 1, "invalid numeric string");
  }

  PushDouble(L, value);
  return 1;
}


int LuaDouble::CallCreate(lua_State* L)
{
  return CreateDouble(L, 2);
}


//============================================================================//
//============================================================================//

bool LuaDouble::CreateMetatable(lua_State* L)
{
  const int doubleTable = lua_gettop(L);

  luaL_newmetatable(L, metaName); {
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, doubleTable);
    lua_rawset(L, -3);

    luaset_strfunc(L, "__tostring", MetaTOSTRING);
    luaset_strfunc(L, "__concat",   MetaCONCAT);

    luaset_strfunc(L, "__add", MetaADD);
    luaset_strfunc(L, "__sub", MetaSUB);
    luaset_strfunc(L, "__mul", MetaMUL);
    luaset_strfunc(L, "__div", MetaDIV);
    luaset_strfunc(L, "__mod", MetaMOD);
    luaset_strfunc(L, "__pow", MetaPOW);
    luaset_strfunc(L, "__unm", MetaUNM);
    luaset_strfunc(L, "__eq",  MetaEQ);
    luaset_strfunc(L, "__lt",  MetaLT);
    luaset_strfunc(L, "__le",  MetaLE);

    luaset_strstr(L, "__metatable", "no access");
  }
  lua_pop(L, 1);

  return true;
}

//============================================================================//
//============================================================================//

static inline void PushDoubleString(lua_State* L, double d)
{
  static char buf[LUAI_MAXNUMBER2STR];
  snprintf(buf, LUAI_MAXNUMBER2STR, LUA_NUMBER_FMT, d);
  lua_pushstring(L, buf);
}


int LuaDouble::MetaTOSTRING(lua_State* L)
{
  PushDoubleString(L, CheckDouble(L, 1));
  return 1;
}


int LuaDouble::MetaCONCAT(lua_State* L)
{
  const double* dptr1 = TestDouble(L, 1);
  if (dptr1 != NULL) {
    PushDoubleString(L, *dptr1);
  } else {
    lua_pushvalue(L, 1);
  }

  const double* dptr2 = TestDouble(L, 2);
  if (dptr2 != NULL) {
    PushDoubleString(L, *dptr2);
  } else {
    lua_pushvalue(L, 2);
  }

  lua_concat(L, 2);
  return 1;
}


//============================================================================//
//============================================================================//

int LuaDouble::MetaADD(lua_State* L) {
  PushDouble(L, CheckNumber(L, 1) + CheckNumber(L, 2)); return 1;
}

int LuaDouble::MetaSUB(lua_State* L) {
  PushDouble(L, CheckNumber(L, 1) - CheckNumber(L, 2)); return 1;
}

int LuaDouble::MetaMUL(lua_State* L) {
  PushDouble(L, CheckNumber(L, 1) * CheckNumber(L, 2)); return 1;
}

int LuaDouble::MetaDIV(lua_State* L) {
  PushDouble(L, CheckNumber(L, 1) / CheckNumber(L, 2)); return 1;
}

int LuaDouble::MetaMOD(lua_State* L) {
  PushDouble(L, ::fmod(CheckNumber(L, 1), CheckNumber(L, 2))); return 1;
}

int LuaDouble::MetaPOW(lua_State* L) {
  PushDouble(L, ::pow(CheckNumber(L, 1), CheckNumber(L, 2))); return 1;
}

int LuaDouble::MetaUNM(lua_State* L) {
  PushDouble(L, -CheckDouble(L, 1)); return 1;
}

int LuaDouble::MetaEQ(lua_State* L) {
  lua_pushboolean(L, CheckNumber(L, 1) == CheckNumber(L, 2)); return 1;
}

int LuaDouble::MetaLT(lua_State* L) {
  lua_pushboolean(L, CheckNumber(L, 1) < CheckNumber(L, 2)); return 1;
}

int LuaDouble::MetaLE(lua_State* L) {
  lua_pushboolean(L, CheckNumber(L, 1) <= CheckNumber(L, 2)); return 1;
}


//============================================================================//
//============================================================================//

int LuaDouble::create(lua_State* L)
{
  return CreateDouble(L, 1);
}


int LuaDouble::isdouble(lua_State* L)
{
  lua_pushboolean(L, lua_getuserdataextra(L, 1) == metaName);
  return 1;
}


int LuaDouble::tonumber(lua_State* L)
{
  lua_pushnumber(L, (lua_Number) CheckNumber(L, 1));
  return 1;
}


int LuaDouble::tostring(lua_State* L) // FIXME -- optional format
{
  const double d1 = CheckNumber(L, 1);
  char buf[128];
  snprintf(buf, sizeof(buf), LUA_NUMBER_FMT, d1);
  lua_pushstring(L, buf);
  return 1;
}


int LuaDouble::pack(lua_State* L)
{
  const double d1 = CheckNumber(L, 1);
  lua_pushlstring(L, (char*)&d1, sizeof(double));
  return 1;
}


int LuaDouble::unpack(lua_State* L)
{
  size_t len;
  const char* s = luaL_checklstring(L, 1, &len);
  const size_t offset = luaL_optint(L, 2, 1) - 1;
  if ((sizeof(double) + offset) > len) {
    luaL_error(L, "not enough data");
  }
  PushDouble(L, *((double*)(s + offset)));
  return 1;
}


//============================================================================//
//============================================================================//

int LuaDouble::date(lua_State* L)
{
  // NOTE: heavily copied from lua/src/loslib.cxx : os_date()

  const double d = LuaDouble::CheckDouble(L, 1);
  const char*  s = luaL_optstring(L, 2, "%c");
  const time_t t = (time_t) d;

  struct tm* stm = NULL;
  if (*s == '!') {  // UTC?
    stm = gmtime(&t);
    s++;  // skip `!'
  }
  else {
    stm = localtime(&t);
  }

  if (stm == NULL) { // invalid date?
    lua_pushnil(L);
  }
  else if (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  // 9 = number of fields
    luaset_strint(L,  "sec",   stm->tm_sec);
    luaset_strint(L,  "min",   stm->tm_min);
    luaset_strint(L,  "hour",  stm->tm_hour);
    luaset_strint(L,  "day",   stm->tm_mday); 
    luaset_strint(L,  "month", stm->tm_mon  + 1);
    luaset_strint(L,  "year",  stm->tm_year + 1900);
    luaset_strint(L,  "wday",  stm->tm_wday + 1);   
    luaset_strint(L,  "yday",  stm->tm_yday + 1);   
    luaset_strbool(L, "isdst", stm->tm_isdst);
  }
  else {
    char cc[3];
    luaL_Buffer b;
    cc[0] = '%'; cc[2] = '\0';
    luaL_buffinit(L, &b);
    for (; *s; s++) {
      if (*s != '%' || *(s + 1) == '\0')  // no conversion specifier?
        luaL_addchar(&b, *s);
      else {
        size_t reslen;
        char buff[256];  // should be big enough for any conversion result
        cc[1] = *(++s);
        reslen = strftime(buff, sizeof(buff), cc, stm);
        luaL_addlstring(&b, buff, reslen);
      }
    }  
    luaL_pushresult(&b);
  }

  return 1;
}


//============================================================================//
//============================================================================//

static inline bool double_isnan(double d)
{
  return (d != d);
}


static inline int double_isinf(double d)
{
  if (d != (d + d)) {
    return 0;
  }
  if (d > +1.0) { return +1; }
  if (d < -1.0) { return -1; }
  return 0;
}


//============================================================================//
//============================================================================//

int LuaDouble::ld_isnan(lua_State* L) {
  lua_pushboolean(L, double_isnan(CheckNumber(L, 1)));
  return 1;
}

int LuaDouble::ld_isinf(lua_State* L) {
  const int infState = double_isinf(CheckNumber(L, 1));
  if (infState) {
    lua_pushint(L, infState);
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

int LuaDouble::ld_isfinite(lua_State* L) {
  const double d = CheckNumber(L, 1);
  lua_pushboolean(L, !double_isnan(d) && !double_isinf(d));
  return 1;
}


int LuaDouble::ld_abs(lua_State* L) {
  PushDouble(L, ::fabs(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_floor(lua_State* L) {
  PushDouble(L, ::floor(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_ceil(lua_State* L) {
  PushDouble(L, ::ceil(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_fmod(lua_State* L) {
  PushDouble(L, ::fmod(CheckNumber(L, 1), CheckNumber(L, 2))); return 1;
}

int LuaDouble::ld_modf(lua_State* L) {
  const double value = CheckNumber(L, 1);
  double ival;
  PushDouble(L, ::modf(value, &ival));
  PushDouble(L, ival);
  return 2;
}

int LuaDouble::ld_frexp(lua_State* L) {
  int e;
  PushDouble(L, ::frexp(CheckNumber(L, 1), &e));
  lua_pushint(L, e);
  return 2;
}

int LuaDouble::ld_ldexp(lua_State* L) {
  PushDouble(L, ::ldexp(CheckNumber(L, 1), (int)CheckNumber(L, 2))); return 1;
}


int LuaDouble::ld_sqrt(lua_State* L) {
  PushDouble(L, ::sqrt(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_pow(lua_State* L) {
  PushDouble(L, ::pow(CheckNumber(L, 1), CheckNumber(L, 2))); return 1;
}

int LuaDouble::ld_exp(lua_State* L) {
  PushDouble(L, ::exp(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_log(lua_State* L) {
  PushDouble(L, ::log(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_log10(lua_State* L) {
  PushDouble(L, ::log10(CheckNumber(L, 1))); return 1;
}


int LuaDouble::ld_cos(lua_State* L) {
  PushDouble(L, ::cos(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_sin(lua_State* L) {
  PushDouble(L, ::sin(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_tan(lua_State* L) {
  PushDouble(L, ::tan(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_acos(lua_State* L) {
  PushDouble(L, ::acos(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_asin(lua_State* L) {
  PushDouble(L, ::asin(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_atan(lua_State* L) {
  PushDouble(L, ::atan(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_atan2(lua_State* L) {
  PushDouble(L, ::atan2(CheckNumber(L, 1), CheckNumber(L, 2))); return 1;
}


int LuaDouble::ld_cosh(lua_State* L) {
  PushDouble(L, ::cosh(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_sinh(lua_State* L) {
  PushDouble(L, ::sinh(CheckNumber(L, 1))); return 1;
}

int LuaDouble::ld_tanh(lua_State* L) {
  PushDouble(L, ::tanh(CheckNumber(L, 1))); return 1;
}


//============================================================================//
//============================================================================//

bool LuaDouble::PushEntries(lua_State* L)
{
  lua_newtable(L);

  lua_pushliteral(L, "double");
  lua_pushvalue(L, -2); {
    luaset_strfunc(L, "create",   create);
    luaset_strfunc(L, "isdouble", isdouble);

    luaset_strfunc(L, "tonumber", tonumber);
    luaset_strfunc(L, "tostring", tostring);

    luaset_strfunc(L, "date", date);

    luaset_strfunc(L, "pack",   pack);
    luaset_strfunc(L, "unpack", unpack);

    luaset_strfunc(L, "isnan",    ld_isnan);
    luaset_strfunc(L, "isinf",    ld_isinf);
    luaset_strfunc(L, "isfinite", ld_isfinite);

    luaset_strfunc(L, "abs",   ld_abs);
    luaset_strfunc(L, "floor", ld_floor);
    luaset_strfunc(L, "ceil",  ld_ceil);
    luaset_strfunc(L, "fmod",  ld_fmod);
    luaset_strfunc(L, "modf",  ld_modf);
    luaset_strfunc(L, "frexp", ld_frexp);
    luaset_strfunc(L, "ldexp", ld_ldexp);
    luaset_strfunc(L, "sqrt",  ld_sqrt);
    luaset_strfunc(L, "pow",   ld_pow);
    luaset_strfunc(L, "exp",   ld_exp);
    luaset_strfunc(L, "log",   ld_log);
    luaset_strfunc(L, "log10", ld_log10);
    luaset_strfunc(L, "cos",   ld_cos);
    luaset_strfunc(L, "sin",   ld_sin);
    luaset_strfunc(L, "tan",   ld_tan);
    luaset_strfunc(L, "acos",  ld_acos);
    luaset_strfunc(L, "asin",  ld_asin);
    luaset_strfunc(L, "atan",  ld_atan);
    luaset_strfunc(L, "atan2", ld_atan2);
    luaset_strfunc(L, "cosh",  ld_cosh);
    luaset_strfunc(L, "sinh",  ld_sinh);
    luaset_strfunc(L, "tanh",  ld_tanh);
  }
  lua_rawset(L, -4); // NOTE: -4 is because we are working on a copy

  // copy isdouble() to the global table
  lua_getfield(L, -1, "isdouble");
  lua_setfield(L, -3, "isdouble");

  // convenience metatable, 'double(x)' calls 'double.create(x)'
  lua_newtable(L);
  luaset_strfunc(L, "__call", CallCreate);
  lua_setmetatable(L, -2);

  // the 'double' table must be on the top of the stack
  CreateMetatable(L);

  // add the 'pi' constant
  lua_pushliteral(L, "pi");
  PushDouble(L, M_PI);
  lua_rawset(L, -3);

  // FIXME -- need 'huge' ?

  lua_pop(L, 1); // pop the 'double' table

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
