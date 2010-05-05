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

#ifndef LUA_DOUBLE_H
#define LUA_DOUBLE_H

struct lua_State;


class LuaDouble {
  public:
    static bool PushEntries(lua_State* L);

    static bool      IsDouble(lua_State* L, int index);
    static double* TestDouble(lua_State* L, int index);
    static double* TestNumber(lua_State* L, int index);
    static double CheckDouble(lua_State* L, int index);
    static double CheckNumber(lua_State* L, int index);

    static void PushDouble(lua_State* L, double value);

  public:
    static const char* metaName;

  private:
    static bool CreateMetatable(lua_State* L);
    static int CreateDouble(lua_State* L);
    static int IsDouble(lua_State* L);
    static int Date(lua_State* L);
    static int MetaIndex(lua_State* L);
    static int MetaToString(lua_State* L);
    static int MetaADD(lua_State* L);
    static int MetaSUB(lua_State* L);
    static int MetaMUL(lua_State* L);
    static int MetaDIV(lua_State* L);
    static int MetaMOD(lua_State* L);
    static int MetaPOW(lua_State* L);
    static int MetaUNM(lua_State* L);
    static int MetaEQ(lua_State* L);
    static int MetaLT(lua_State* L);
    static int MetaLE(lua_State* L);
};


#endif // LUA_DOUBLE_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
