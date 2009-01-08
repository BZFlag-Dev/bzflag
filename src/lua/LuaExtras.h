#ifndef LUA_EXTRAS_H
#define LUA_EXTRAS_H
// LuaExtras.h: extra simple lua functions
//
//////////////////////////////////////////////////////////////////////

struct lua_State;


class LuaExtras {
	public:
		static bool PushEntries(lua_State* L);

	public:
		static int tobool(lua_State* L);
		static int isnil(lua_State* L);
		static int isbool(lua_State* L);
		static int isnumber(lua_State* L);
		static int isstring(lua_State* L);
		static int istable(lua_State* L);
		static int isthread(lua_State* L);
		static int isfunction(lua_State* L);
		static int isuserdata(lua_State* L);

		static int traceback(lua_State* L);

		static int dump(lua_State* L); // can strip as well
		static int listing(lua_State* L);
};


#endif // LUA_EXTRAS_H
