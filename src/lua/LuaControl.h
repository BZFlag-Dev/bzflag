#ifndef LUA_CONTROL_H
#define LUA_CONTROL_H

struct lua_State;


class LuaControl {
	public:
		static bool PushEntries(lua_State* L);

	private:
		static int Move(lua_State* L);
		static int Fire(lua_State* L);
		static int Jump(lua_State* L);
		static int Spawn(lua_State* L);
		static int Pause(lua_State* L);
		static int DropFlag(lua_State* L);
		static int SetTarget(lua_State* L);
};


#endif // LUA_CONTROL_H
