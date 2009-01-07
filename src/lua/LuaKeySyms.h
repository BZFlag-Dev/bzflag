#ifndef LUA_KEY_SYMS_H
#define LUA_KEY_SYMS_H

struct lua_State;


class LuaKeySyms {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int CreateScream(lua_State* L);
};


#endif // LUA_KEY_SYMS_H
