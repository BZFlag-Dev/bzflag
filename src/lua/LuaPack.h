#ifndef LUA_PACK_H
#define LUA_PACK_H

struct lua_State;


class LuaPack {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int PackU8(lua_State* L);
		static int PackU16(lua_State* L);
		static int PackU32(lua_State* L);
		static int PackU64(lua_State* L);
		static int PackI8(lua_State* L);
		static int PackI16(lua_State* L);
		static int PackI32(lua_State* L);
		static int PackI64(lua_State* L);
		static int PackF32(lua_State* L);
		static int PackF64(lua_State* L);

		static int UnpackU8(lua_State* L);
		static int UnpackU16(lua_State* L);
		static int UnpackU32(lua_State* L);
		static int UnpackU64(lua_State* L);
		static int UnpackI8(lua_State* L);
		static int UnpackI16(lua_State* L);
		static int UnpackI32(lua_State* L);
		static int UnpackI64(lua_State* L);
		static int UnpackF32(lua_State* L);
		static int UnpackF64(lua_State* L);

		static int SwapBy2(lua_State* L);
		static int SwapBy4(lua_State* L);
		static int SwapBy8(lua_State* L);

		static int GetEndian(lua_State* L);
};


#endif // LUA_PACK_H
