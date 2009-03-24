
#include "common.h"

// interface header
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
#include "LuaDouble.h"


//============================================================================//
//============================================================================//

bool LuaPack::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, PackU8);
	PUSH_LUA_CFUNC(L, PackU16);
	PUSH_LUA_CFUNC(L, PackU32);
	PUSH_LUA_CFUNC(L, PackU64);
	PUSH_LUA_CFUNC(L, PackI8);
	PUSH_LUA_CFUNC(L, PackI16);
	PUSH_LUA_CFUNC(L, PackI32);
	PUSH_LUA_CFUNC(L, PackI64);
	PUSH_LUA_CFUNC(L, PackF32);
	PUSH_LUA_CFUNC(L, PackF64); // LuaDouble

	PUSH_LUA_CFUNC(L, UnpackU8);
	PUSH_LUA_CFUNC(L, UnpackU16);
	PUSH_LUA_CFUNC(L, UnpackU32);
	PUSH_LUA_CFUNC(L, UnpackU64);
	PUSH_LUA_CFUNC(L, UnpackI8);
	PUSH_LUA_CFUNC(L, UnpackI16);
	PUSH_LUA_CFUNC(L, UnpackI32);
	PUSH_LUA_CFUNC(L, UnpackI64);
	PUSH_LUA_CFUNC(L, UnpackF32);
	PUSH_LUA_CFUNC(L, UnpackF64); // LuaDouble

	PUSH_LUA_CFUNC(L, SwapBy2);
	PUSH_LUA_CFUNC(L, SwapBy4);
	PUSH_LUA_CFUNC(L, SwapBy8);

	PUSH_LUA_CFUNC(L, GetEndian);

	return true;
}


//============================================================================//
//============================================================================//

template <typename T>
int PackType(lua_State* L)
{
	vector<T> vals;

	// collect the values
	if (lua_istable(L, 1)) {
		for (int i = 1; lua_checkgeti(L, 1, i); lua_pop(L, 1), i++) {
			if (lua_israwnumber(L, -1)) {
				vals.push_back((T)lua_tonumber(L, -1));
			}
			else {
				const double* dptr = LuaDouble::TestDouble(L, -1);
				if (dptr != NULL) {
					vals.push_back((T)(*dptr));
				} else {
					break; // not a lua_Number or a LuaDouble
				}
			}
		}
	}
	else {
		const int args = lua_gettop(L);
		for (int i = 1; i <= args; i++) {
			if (lua_israwnumber(L, i)) {
				vals.push_back((T)lua_tonumber(L, i));
			}
			else {
				const double* dptr = LuaDouble::TestDouble(L, i);
				if (dptr != NULL) {
					vals.push_back((T)(*dptr));
				} else {
					break; // not a lua_Number or a LuaDouble
				}
			}
		}
	}

	if (vals.empty()) {
		return 0;
	}

	// push the result
	const int bufSize = sizeof(T) * vals.size();
	char* buf = new char[bufSize];
	for (int i = 0; i < (int)vals.size(); i++) {
		memcpy(buf + (i * sizeof(T)), &vals[i], sizeof(T));
	}
	lua_pushlstring(L, buf, bufSize);
	delete[] buf;

	return 1;
}


int LuaPack::PackU8(lua_State*  L) { return PackType<uint8_t> (L); }
int LuaPack::PackU16(lua_State* L) { return PackType<uint16_t>(L); }
int LuaPack::PackU32(lua_State* L) { return PackType<uint32_t>(L); }
int LuaPack::PackU64(lua_State* L) { return PackType<uint64_t>(L); }
int LuaPack::PackI8(lua_State*  L) { return PackType<int8_t>  (L); }
int LuaPack::PackI16(lua_State* L) { return PackType<int16_t> (L); }
int LuaPack::PackI32(lua_State* L) { return PackType<int32_t> (L); }
int LuaPack::PackI64(lua_State* L) { return PackType<int64_t> (L); }
int LuaPack::PackF32(lua_State* L) { return PackType<float>   (L); }
int LuaPack::PackF64(lua_State* L) { return PackType<double>  (L); }


//============================================================================//
//============================================================================//

template <typename T>
int UnpackType(lua_State* L, bool defaultToDoubles = false)
{
	int index = 1;
	bool pushDoubles = defaultToDoubles;
	if (lua_isboolean(L, index)) {
		pushDoubles = lua_tobool(L, index);
		index++;
	}

	if (!lua_israwstring(L, index)) {
		return 0;
	}
	size_t len;
	const char* str = lua_tolstring(L, index, &len);

	// apply the offset
	if (lua_israwnumber(L, index + 1)) {
		const size_t offset = lua_toint(L, index + 1) - 1;
		if (offset >= len) {
			return 0;
		}
		str += offset;
		len -= offset;
	}

	const size_t eSize = sizeof(T);
	if (len < eSize) {
		return 0;
	}

	// return a single value
	if (!lua_israwnumber(L, index + 2)) {
		const T value = *((T*)str);
		if (pushDoubles) {
			LuaDouble::PushDouble(L, (double)(static_cast<lua_Number>(value)));
		} else {
			lua_pushnumber(L, static_cast<lua_Number>(value));
		}
		return 1;
	}

	// return a table
	const int maxCount = (len / eSize);
	int tableCount = lua_toint(L, 3);
	if (tableCount < 0) {
		tableCount = maxCount;
	}
	tableCount = min(maxCount, tableCount);
	lua_newtable(L);
	if (pushDoubles) {
		for (int i = 0; i < tableCount; i++) {
			const T value = *(((T*)str) + i);
			LuaDouble::PushDouble(L, (double)(static_cast<lua_Number>(value)));
			lua_rawseti(L, -2, (i + 1));
		}
	} else {
		for (int i = 0; i < tableCount; i++) {
			const T value = *(((T*)str) + i);
			lua_pushnumber(L, static_cast<lua_Number>(value));
			lua_rawseti(L, -2, (i + 1));
		}
	}
	return 1;
}


int LuaPack::UnpackU8(lua_State*  L) { return UnpackType<uint8_t> (L, false); }
int LuaPack::UnpackU16(lua_State* L) { return UnpackType<uint16_t>(L, false); }
int LuaPack::UnpackU32(lua_State* L) { return UnpackType<uint32_t>(L, true);  }
int LuaPack::UnpackU64(lua_State* L) { return UnpackType<uint64_t>(L, true);  }
int LuaPack::UnpackI8(lua_State*  L) { return UnpackType<int8_t>  (L, false); }
int LuaPack::UnpackI16(lua_State* L) { return UnpackType<int16_t> (L, false); }
int LuaPack::UnpackI32(lua_State* L) { return UnpackType<int32_t> (L, true);  }
int LuaPack::UnpackI64(lua_State* L) { return UnpackType<int64_t> (L, true);  }
int LuaPack::UnpackF32(lua_State* L) { return UnpackType<float>   (L, false); }
int LuaPack::UnpackF64(lua_State* L) { return UnpackType<double>  (L, true);  }


//============================================================================//
//============================================================================//

int LuaPack::SwapBy2(lua_State* L)
{
	size_t size;
	const char* data = lua_tolstring(L, 1, &size);
	if ((size % 2) != 0) {
		return 0;
	}
	char* output = new char[size];
	for (size_t i = 0; i < size; i += 2) {
		output[i + 0] = data[i + 1];
		output[i + 1] = data[i + 0];
	}
	lua_pushlstring(L, output, size);
	delete[] output;
	return 1;
}


int LuaPack::SwapBy4(lua_State* L)
{
	size_t size;
	const char* data = lua_tolstring(L, 1, &size);
	if ((size % 4) != 0) {
		return 0;
	}
	char* output = new char[size];
	for (size_t i = 0; i < size; i += 4) {
		output[i + 0] = data[i + 3];
		output[i + 1] = data[i + 2];
		output[i + 2] = data[i + 1];
		output[i + 3] = data[i + 0];
	}
	lua_pushlstring(L, output, size);
	delete[] output;
	return 1;
}



int LuaPack::SwapBy8(lua_State* L)
{
	size_t size;
	const char* data = lua_tolstring(L, 1, &size);
	if ((size % 8) != 0) {
		return 0;
	}
	char* output = new char[size];
	for (size_t i = 0; i < size; i += 8) {
		output[i + 0] = data[i + 7];
		output[i + 1] = data[i + 6];
		output[i + 2] = data[i + 5];
		output[i + 3] = data[i + 4];
		output[i + 4] = data[i + 3];
		output[i + 5] = data[i + 2];
		output[i + 6] = data[i + 1];
		output[i + 7] = data[i + 0];
	}
	lua_pushlstring(L, output, size);
	delete[] output;
	return 1;
}


//============================================================================//
//============================================================================//

int LuaPack::GetEndian(lua_State* L)
{
	static unsigned char endianArray[4] = { 0x11, 0x22, 0x33, 0x44 };
	static uint32_t& endian = *((uint32_t*)endianArray);

	switch (endian) {
		case 0x44332211: { HSTR_PUSH(L, "little");  break; }
		case 0x11223344: { HSTR_PUSH(L, "big");     break; }
		case 0x22114433: { HSTR_PUSH(L, "pdp");     break; }
		default:         { HSTR_PUSH(L, "unknown"); break; }
	}

	return 1;
}


//============================================================================//
//============================================================================//
