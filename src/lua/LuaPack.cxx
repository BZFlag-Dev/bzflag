
#include "common.h"

// implementation header
#include "LuaPack.h"

// system headers
#include <string.h>
#include <string>
#include <vector>
using std::string;
using std::vector;
#include <algorithm>
using std::min;
using std::max;

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

bool LuaPack::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, PackU8);
	PUSH_LUA_CFUNC(L, PackU16);
	PUSH_LUA_CFUNC(L, PackU32);
	PUSH_LUA_CFUNC(L, PackS8);
	PUSH_LUA_CFUNC(L, PackS16);
	PUSH_LUA_CFUNC(L, PackS32);
	PUSH_LUA_CFUNC(L, PackF32);

	PUSH_LUA_CFUNC(L, UnpackU8);
	PUSH_LUA_CFUNC(L, UnpackU16);
	PUSH_LUA_CFUNC(L, UnpackU32);
	PUSH_LUA_CFUNC(L, UnpackS8);
	PUSH_LUA_CFUNC(L, UnpackS16);
	PUSH_LUA_CFUNC(L, UnpackS32);
	PUSH_LUA_CFUNC(L, UnpackF32);

	return true;
}


/******************************************************************************/
/******************************************************************************/
//
//  FIXME: Endianess should be handled -- also, unsigned & signed required?
//

template <typename T>
int PackType(lua_State* L)
{
	vector<T> vals;

	if (lua_istable(L, 1)) {
		for (int i = 1;
		     lua_rawgeti(L, 1, i), lua_israwnumber(L, -1);
		     lua_pop(L, 1), i++) {
			vals.push_back((T)lua_tonumber(L, i));
		}
		return 1;
	}
	else {
		const int args = lua_gettop(L);
		for (int i = 1; i <= args; i++) {
			if (!lua_israwnumber(L, i)) {
				break;
			}
			vals.push_back((T)lua_tonumber(L, i));
		}
	}

	if (vals.empty()) {
		return 0;
	}

	const int bufSize = sizeof(T) * vals.size();
	char* buf = new char[bufSize];
	for (int i = 0; i < (int)vals.size(); i++) {
		memcpy(buf + (i * sizeof(T)), &vals[i], sizeof(T));
	}
	lua_pushlstring(L, buf, bufSize);
	delete[] buf;
	
	return 1;
}


int LuaPack::PackU8(lua_State*  L) { return PackType<uint8_t>(L);  }
int LuaPack::PackU16(lua_State* L) { return PackType<uint16_t>(L); }
int LuaPack::PackU32(lua_State* L) { return PackType<uint32_t>(L); }
int LuaPack::PackS8(lua_State*  L) { return PackType<char>(L);   }
int LuaPack::PackS16(lua_State* L) { return PackType<int16_t>(L);  }
int LuaPack::PackS32(lua_State* L) { return PackType<int32_t>(L);  }
int LuaPack::PackF32(lua_State* L) { return PackType<float>(L);    }


/******************************************************************************/

template <typename T>
int UnpackType(lua_State* L)
{
	if (!lua_israwstring(L, 1)) {
		return 0;
	}
	size_t len;
	const char* str = lua_tolstring(L, 1, &len);

	if (lua_israwnumber(L, 2)) {
		const size_t pos = lua_toint(L, 2);
		if ((pos < 1) || (pos >= len)) {
			return 0;
		}
		const size_t offset = (pos - 1);
		str += offset;
		len -= offset;
	}
	
	const size_t eSize = sizeof(T);
	if (len < eSize) {
		return 0;
	}

	if (!lua_israwnumber(L, 3)) {
		const T value = *((T*)str);
		lua_pushnumber(L, value);
		return 1;
	}
	else {
		const int maxCount = (len / eSize);
		int tableCount = lua_toint(L, 3);
		if (tableCount < 0) {
			tableCount = maxCount;
		}
		tableCount = min(maxCount, tableCount);
		lua_newtable(L);
		for (int i = 0; i < tableCount; i++) {
			const T value = *(((T*)str) + i);
			lua_pushnumber(L, value);
			lua_rawseti(L, -2, (i + 1));
		}
		lua_pushstring(L, "n");
		lua_pushnumber(L, tableCount);
		lua_rawset(L, -3);
		return 1;
	}			

	return 0;
}


int LuaPack::UnpackU8(lua_State*  L) { return UnpackType<uint8_t>(L);  }
int LuaPack::UnpackU16(lua_State* L) { return UnpackType<uint16_t>(L); }
int LuaPack::UnpackU32(lua_State* L) { return UnpackType<uint32_t>(L); }
int LuaPack::UnpackS8(lua_State*  L) { return UnpackType<char>(L);   }
int LuaPack::UnpackS16(lua_State* L) { return UnpackType<int16_t>(L);  }
int LuaPack::UnpackS32(lua_State* L) { return UnpackType<int32_t>(L);  }
int LuaPack::UnpackF32(lua_State* L) { return UnpackType<float>(L);    }


/******************************************************************************/
/******************************************************************************/
