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

#ifndef LUA_UTILS_H
#define LUA_UTILS_H
// LuaUtils.h: lua utility routines
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include "LuaHeader.h"


class LuaUtils {
  public:
    static int CopyData(lua_State* dst, lua_State* src, int count);

    static void PushCurrentFuncEnv(lua_State* L, const char* caller);

    // lower case all keys in the table, with recursion
    static bool LowerKeys(lua_State* L, int tableIndex);

    static int Print(lua_State* L);

    static void PrintStack(lua_State* L);

    static bool FormatArgs(lua_State* L, bool expandTables,
                           std::vector<std::string>& result,
                           const char* caller);

    // check for client BZDB write permission
    static bool ClientWriteCheck(const std::string& varName);

    // not implemented...
    static int ParseIntArray(lua_State* L, int tableIndex,
                             int* array, int arraySize);
    static int ParseFloatArray(lua_State* L, int tableIndex,
                               float* array, int arraySize);
    static int ParseStringArray(lua_State* L, int tableIndex,
                                std::string* array, int arraySize);

    static int ParseIntVector(lua_State* L, int tableIndex,
                              std::vector<int>& vec);
    static int ParseFloatVector(lua_State* L, int tableIndex,
                                std::vector<float>& vec);
    static int ParseStringVector(lua_State* L, int tableIndex,
                                 std::vector<std::string>& vec);
};


inline void LuaSetDualPair(lua_State* L, const std::string& s, int i) {
  luaset_strint(L, s, i);
  luaset_intstr(L, i, s);
}


#endif // LUA_UTILS_H



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
