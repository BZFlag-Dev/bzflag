#ifndef LUA_BIT_OPS_H
#define LUA_BIT_OPS_H

struct lua_State;

class LuaBitOps {
	public:
		static bool PushEntries(lua_State* L);

	private:
		static int bit_or(lua_State* L);
		static int bit_and(lua_State* L);
		static int bit_xor(lua_State* L);
		static int bit_inv(lua_State* L);
		static int bit_bits(lua_State* L);
};


#endif // LUA_BIT_OPS_H
