#ifndef LUA_DYNCOL_H
#define LUA_DYNCOL_H

struct lua_State;


class LuaDynCol {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int GetDynColName(lua_State* L);
		static int GetDynCol(lua_State* L);
		static int SetDynCol(lua_State* L);
};


#endif // LUA_DYNCOL_H
