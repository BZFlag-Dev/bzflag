#ifndef LUA_PHYDRV_H
#define LUA_PHYDRV_H

struct lua_State;


class LuaPhyDrv {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int GetPhyDrvName(lua_State* L);
		static int GetPhyDrvDeath(lua_State* L);
		static int GetPhyDrvSlideTime(lua_State* L);
		static int GetPhyDrvVelocity(lua_State* L);
		static int GetPhyDrvRadialPos(lua_State* L);
		static int GetPhyDrvRadialVel(lua_State* L);
		static int GetPhyDrvAngularPos(lua_State* L);
		static int GetPhyDrvAngularVel(lua_State* L);
};


#endif // LUA_PHYDRV_H
