#ifndef LUA_SCREAM_H
#define LUA_SCREAM_H

struct lua_State;


class LuaScream {
	public:
		LuaScream();
		~LuaScream();

		static bool PushEntries(lua_State* L);

	public:
		static const char* metaName;

	private: // metatable methods
		static bool CreateMetatable(lua_State* L);
		static int MetaGC(lua_State* L);
		static int MetaIndex(lua_State* L);
		static int MetaNewindex(lua_State* L);

	private:
		static int* GetScreamRef(lua_State* L, int index);

	private: // call-outs
		static int CreateScream(lua_State* L);
};


#endif // LUA_SCREAM_H
