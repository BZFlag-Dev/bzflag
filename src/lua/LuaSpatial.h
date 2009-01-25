#ifndef LUA_SPATIAL_H
#define LUA_SPATIAL_H


struct lua_State;


class LuaSpatial {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int RayTrace(lua_State* L);
		static int RayTeleport(lua_State* L);

		static int IsPointInView(lua_State* L);
		static int IsSphereInView(lua_State* L);
		static int IsAABBInView(lua_State* L); // Axis Aligned Bounding Box
		static int IsOBBInView(lua_State* L);  // Oriented Bounding Box
		static int IsLSSInView(lua_State* L);  // Line Swept Sphere

		static int IsPointInRadar(lua_State* L);
		static int IsSphereInRadar(lua_State* L);
		static int IsAABBInRadar(lua_State* L); // Axis Aligned Bounding Box
		static int IsOBBInRadar(lua_State* L);  // Oriented Bounding Box
		static int IsLSSInRadar(lua_State* L);  // Line Swept Sphere

		static int GetPlayersInPlanes(lua_State* L);
		static int GetPlayersInSphere(lua_State* L);
		static int GetPlayersInCylinder(lua_State* L);
		static int GetPlayersInBox(lua_State* L);
		static int GetVisiblePlayers(lua_State* L);
		static int GetRadarPlayers(lua_State* L);

		static int GetFlagsInPlanes(lua_State* L);
		static int GetFlagsInSphere(lua_State* L);
		static int GetFlagsInCylinder(lua_State* L);
		static int GetFlagsInBox(lua_State* L);
		static int GetVisibleFlags(lua_State* L);
		static int GetRadarFlags(lua_State* L);

		static int GetShotsInPlanes(lua_State* L);
		static int GetShotsInSphere(lua_State* L);
		static int GetShotsInCylinder(lua_State* L);
		static int GetShotsInBox(lua_State* L);
		static int GetVisibleShots(lua_State* L);
		static int GetRadarShots(lua_State* L);

		static int GetObstaclesInPlanes(lua_State* L);
		static int GetObstaclesInSphere(lua_State* L);
		static int GetObstaclesInCylinder(lua_State* L);
		static int GetObstaclesInBox(lua_State* L);
		static int GetVisibleObstacles(lua_State* L);
		static int GetRadarObstacles(lua_State* L);
};


#endif // LUA_SPATIAL_H
