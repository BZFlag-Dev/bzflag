#ifndef LUA_OBSTACLE_H
#define LUA_OBSTACLE_H

struct lua_State;

class LuaObstacle {
	public:
		static bool PushEntries(lua_State* L);

	private: // call-outs
		static int GetObstacleList(lua_State* L);
		static int GetObstacleName(lua_State* L);
		static int GetObstacleType(lua_State* L);

		static int GetObstacleDriveThrough(lua_State* L);
		static int GetObstacleShootThrough(lua_State* L);
		static int GetObstacleRicochet(lua_State* L);

		static int GetObstaclePosition(lua_State* L);
		static int GetObstacleSize(lua_State* L);
		static int GetObstacleRotation(lua_State* L);

		static int GetObstacleFlatTop(lua_State* L);
		static int GetObstacleExtents(lua_State* L);
		static int GetObstacleTeam(lua_State* L);
		static int GetObstacleFlipZ(lua_State* L);
		static int GetObstacleBorder(lua_State* L);

		static int GetObstacleFaceCount(lua_State* L);
		static int GetFaceMesh(lua_State* L);
		static int GetFaceElementCount(lua_State* L);
		static int GetFaceVerts(lua_State* L);
		static int GetFaceNorms(lua_State* L);
		static int GetFaceTxcds(lua_State* L);
		static int GetFacePlane(lua_State* L);
		static int GetFacePhyDrv(lua_State* L);
		static int GetFaceMaterial(lua_State* L);
		static int GetFaceSmoothBounce(lua_State* L);

		static int GetTeleByName(lua_State* L);
		static int GetTeleBorder(lua_State* L);
		static int GetTeleLinks(lua_State* L);
		static int GetLinkDestinations(lua_State* L);

		static int HasMeshDrawInfo(lua_State* L);
		static int GetMeshDrawInfo(lua_State* L);
};


#endif // LUA_OBSTACLE_H
