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
		static int PackS8(lua_State* L);
		static int PackS16(lua_State* L);
		static int PackS32(lua_State* L);
		static int PackF32(lua_State* L);

		static int UnpackU8(lua_State* L);
		static int UnpackU16(lua_State* L);
		static int UnpackU32(lua_State* L);
		static int UnpackS8(lua_State* L);
		static int UnpackS16(lua_State* L);
		static int UnpackS32(lua_State* L);
		static int UnpackF32(lua_State* L);

		static int SwapBy2(lua_State* L);
		static int SwapBy4(lua_State* L);
		static int SwapBy8(lua_State* L);

		static int GetEndian(lua_State* L);
};


#endif // LUA_PACK_H
