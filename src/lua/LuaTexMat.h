#ifndef LUA_TEXMAT_H
#define LUA_TEXMAT_H

struct lua_State;


class LuaTexMat {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int GetTexMatName(lua_State* L);
		static int GetTexMat(lua_State* L);
		static int SetTexMat(lua_State* L);
};


#endif // LUA_TEXMAT_H
