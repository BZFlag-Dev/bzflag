#ifndef LUA_BZDB_H
#define LUA_BZDB_H

struct lua_State;


class LuaBZDB {
	public:
		static bool PushEntries(lua_State* L);

	private:
		static int GetMap(lua_State* L);
		static int GetList(lua_State* L);

		static int Exists(lua_State* L);
		static int IsPersistent(lua_State* L);
		static int GetDefault(lua_State* L);
		static int GetPermission(lua_State* L);

		static int GetInt(lua_State* L);
		static int GetBool(lua_State* L);
		static int GetFloat(lua_State* L);
		static int GetString(lua_State* L);

		static int SetInt(lua_State* L);
		static int SetBool(lua_State* L);
		static int SetFloat(lua_State* L);
		static int SetString(lua_State* L);
};


#endif // LUA_BZDB_H
