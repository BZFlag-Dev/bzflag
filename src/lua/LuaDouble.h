#ifndef LUA_DOUBLE_H
#define LUA_DOUBLE_H

struct lua_State;


class LuaDouble {
	public:
		static bool PushEntries(lua_State* L);

		static bool      IsDouble(lua_State* L, int index);
		static double* TestDouble(lua_State* L, int index);
		static double* TestNumber(lua_State* L, int index);
		static double CheckDouble(lua_State* L, int index);
		static double CheckNumber(lua_State* L, int index);

	public:
		static const char* metaName;

	// FIXME -- tonumber() ?  double(1234)() ? return  number, numberDiff ?

	private:
		static bool CreateMetatable(lua_State* L);
		static int CreateDouble(lua_State* L, double value);
		static int CreateDouble(lua_State* L);
		static int IsDouble(lua_State* L);
		static int MetaIndex(lua_State* L);
		static int MetaToString(lua_State* L);
		static int MetaADD(lua_State* L);
		static int MetaSUB(lua_State* L);
		static int MetaMUL(lua_State* L);
		static int MetaDIV(lua_State* L);
		static int MetaMOD(lua_State* L);
		static int MetaPOW(lua_State* L);
		static int MetaUNM(lua_State* L);
		static int MetaEQ(lua_State* L);
		static int MetaLT(lua_State* L);
		static int MetaLE(lua_State* L);
};


#endif // LUA_DOUBLE_H
