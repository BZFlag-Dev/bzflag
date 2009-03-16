#ifndef LUA_UTILS_H
#define LUA_UTILS_H
// LuaUtils.h: lua utility routines
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include "LuaHashString.h"
#include "LuaInclude.h"

class LuaUtils {
	public:
		static int CopyData(lua_State* dst, lua_State* src, int count);

		static void PushCurrentFuncEnv(lua_State* L, const char* caller);

		static void* TestUserData(lua_State* L, int index, const std::string& type);

		// lower case all keys in the table, with recursion
		static bool LowerKeys(lua_State* L, int tableIndex);

		static int Print(lua_State* L);

		static void PrintStack(lua_State* L);

		static bool FormatArgs(lua_State* L, bool expandTables,
												   std::vector<std::string>& result,
												   const char* caller);

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


inline void LuaPushDualPair(lua_State* L, const char* name, int code)
{
	lua_pushstring(L, name);
	lua_pushinteger(L, code);
	lua_rawset(L, -3);
	lua_pushinteger(L, code);
	lua_pushstring(L, name);
	lua_rawset(L, -3);
}


inline void LuaPushNamedBool(lua_State* L,
                             const std::string& key, bool value)
{
	lua_pushstring(L, key.c_str());
	lua_pushboolean(L, value);
	lua_rawset(L, -3);
}


inline void LuaPushNamedInt(lua_State* L,
                            const std::string& key, int value)
{
	lua_pushstring(L, key.c_str());
	lua_pushinteger(L, value);
	lua_rawset(L, -3);
}


inline void LuaPushNamedNumber(lua_State* L,
                               const std::string& key, lua_Number value)
{
	lua_pushstring(L, key.c_str());
	lua_pushnumber(L, value);
	lua_rawset(L, -3);
}


inline void LuaPushNamedString(lua_State* L,
                               const std::string& key, const std::string& value)
{
	lua_pushstring(L, key.c_str());
	lua_pushstring(L, value.c_str());
	lua_rawset(L, -3);
}


inline void LuaPushNamedCFunc(lua_State* L,
                              const std::string& key, int (*func)(lua_State*))
{
	lua_pushstring(L, key.c_str());
	lua_pushcfunction(L, func);
	lua_rawset(L, -3);
}


#endif // LUA_UTILS_H

