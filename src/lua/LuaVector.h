#ifndef LUA_VECTOR_H
#define LUA_VECTOR_H
// LuaVector.h: extra simple lua functions
//
//////////////////////////////////////////////////////////////////////

struct lua_State;


class LuaVector {
	public:
		static bool PushEntries(lua_State* L);

	public:
		static int dot(lua_State* L);
		static int cross(lua_State* L);
		static int normdot(lua_State* L);
		static int normdotdegs(lua_State* L);
		static int normdotrads(lua_State* L);
		static int length(lua_State* L);
		static int normalize(lua_State* L);
};


#endif // LUA_VECTOR_H
