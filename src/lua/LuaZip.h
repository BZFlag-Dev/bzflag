#ifndef LUA_ZIP_H
#define LUA_ZIP_H

struct lua_State;


class LuaZip {
	public:
		static bool PushEntries(lua_State* L);

	private: // metatable methods
		static int Zip(lua_State* L);
		static int Unzip(lua_State* L);
};


#endif // LUA_ZIP_H
