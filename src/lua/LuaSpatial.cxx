
#include "common.h"

// interface header
#include "LuaSpatial.h"

// system headers
#include <string>
#include <vector>
using std::string;
using std::vector;

// common headers
#include "Ray.h"
#include "Extents.h"
#include "vectors.h"
#include "Obstacle.h"
#include "ObstacleMgr.h"
#include "CollisionManager.h"
#include "MeshFace.h"
#include "MeshObstacle.h"
#include "ViewFrustum.h"
#include "SceneRenderer.h"
#include "BZDBCache.h"
#include "StateDatabase.h"

// bzflag headers
#include "../bzflag/World.h"
#include "../bzflag/Roster.h"
#include "../bzflag/playing.h"
#include "../bzflag/MainWindow.h"
#include "../bzflag/LocalPlayer.h"
#include "../bzflag/ShotPath.h"
#include "../bzflag/ShotStrategy.h"
#include "../bzflag/RadarRenderer.h"

// local headers
#include "LuaInclude.h"
#include "LuaHandle.h"


//============================================================================//
//============================================================================//

bool LuaSpatial::PushEntries(lua_State* L)
{
	const bool fullRead = L2H(L)->HasFullRead();

	PUSH_LUA_CFUNC(L, RayTrace);
	PUSH_LUA_CFUNC(L, RayTeleport);

	PUSH_LUA_CFUNC(L, IsPointInView);
	PUSH_LUA_CFUNC(L, IsSphereInView);
	PUSH_LUA_CFUNC(L, IsAABBInView);
	PUSH_LUA_CFUNC(L, IsOBBInView);
	PUSH_LUA_CFUNC(L, IsLSSInView);

	PUSH_LUA_CFUNC(L, IsPointInRadar);
	PUSH_LUA_CFUNC(L, IsSphereInRadar);
	PUSH_LUA_CFUNC(L, IsAABBInRadar);
	PUSH_LUA_CFUNC(L, IsOBBInRadar);
	PUSH_LUA_CFUNC(L, IsLSSInRadar);

	if (fullRead) {
		PUSH_LUA_CFUNC(L, GetPlayersInPlanes);
		PUSH_LUA_CFUNC(L, GetPlayersInSphere);
		PUSH_LUA_CFUNC(L, GetPlayersInCylinder);
		PUSH_LUA_CFUNC(L, GetPlayersInBox);
		PUSH_LUA_CFUNC(L, GetVisiblePlayers);
		PUSH_LUA_CFUNC(L, GetRadarPlayers);

		PUSH_LUA_CFUNC(L, GetFlagsInPlanes);
		PUSH_LUA_CFUNC(L, GetFlagsInSphere);
		PUSH_LUA_CFUNC(L, GetFlagsInCylinder);
		PUSH_LUA_CFUNC(L, GetFlagsInBox);
		PUSH_LUA_CFUNC(L, GetVisibleFlags);
		PUSH_LUA_CFUNC(L, GetRadarFlags);

		PUSH_LUA_CFUNC(L, GetShotsInPlanes);
		PUSH_LUA_CFUNC(L, GetShotsInSphere);
		PUSH_LUA_CFUNC(L, GetShotsInCylinder);
		PUSH_LUA_CFUNC(L, GetShotsInBox);
		PUSH_LUA_CFUNC(L, GetVisibleShots);
		PUSH_LUA_CFUNC(L, GetRadarShots);

		PUSH_LUA_CFUNC(L, GetObstaclesInPlanes);
		PUSH_LUA_CFUNC(L, GetObstaclesInSphere);
		PUSH_LUA_CFUNC(L, GetObstaclesInCylinder);
		PUSH_LUA_CFUNC(L, GetObstaclesInBox);
		PUSH_LUA_CFUNC(L, GetVisibleObstacles);
		PUSH_LUA_CFUNC(L, GetRadarObstacles);
	}

	return true;
}


//============================================================================//
//============================================================================//

struct QueryData {
	QueryData()
	: seer(false)
	{}
	bool seer;
};


struct PlanesData : public QueryData {
	vector<fvec4> planes;
};


struct SphereData : public QueryData {
	fvec3 pos;
	float r, r2;
};


struct CylinderData : public QueryData {
	fvec2 pos;
	float r, r2;
};


struct BoxData : public QueryData {
	Extents extents;
	float radians;
};


//============================================================================//
//============================================================================//

static bool GetViewPlanes(PlanesData& planes)
{
	planes.planes.clear();
	const ViewFrustum& vf = RENDERER.getViewFrustum();
	for (int p = 0; p < vf.getPlaneCount(); p++) {
		planes.planes.push_back(vf.getSide(p));
	}
	return true;
}


static bool GetRadarBox(BoxData& box)
{
	const RadarRenderer* radar = getRadarRenderer();
	if (radar == NULL) {
		return false;
	}

	fvec3 mins, maxs;
	mins[0] = +1.0e30f;
	maxs[0] = -1.0e30f;
	mins[1] = +1.0e30f;
	maxs[1] = -1.0e30f;
	mins[2] = -1.0e30f;
	maxs[2] = +1.0e30f;

//FIXME	const float radarRange = radar->getRange();

	mins[0] = 0.0f; // FIXME -- and move the radarRange query into RadarRenderer
	mins[1] = 0.0f;
	maxs[0] = 0.0f;
	maxs[1] = 0.0f;
	box.extents.set(mins, maxs);

	return false;
}


//============================================================================//
//============================================================================//

static inline int ParsePlanes(lua_State* L, PlanesData& planes)
{
	const int table = 1;
	for (int i = 1; lua_checkgeti(L, table, i); lua_pop(L, 1), i++) {
		if (lua_istable(L, -1)) {
			fvec4 plane(0.0f, 0.0f, 0.0f, 0.0f);
			const int planeIndex = lua_gettop(L);
			int p;
			for (p = 0; p < 4; p++) {
				lua_rawgeti(L, planeIndex, p + 1);
				if (!lua_israwnumber(L, -1)) {
					lua_pop(L, 1);
					break;
				}
				plane[p] = lua_tofloat(L, -1);
				lua_pop(L, 1);
			}
			if (p == 4) {
				planes.planes.push_back(plane);
			}
		}
	}
	return 1;
}


static inline int ParseSphere(lua_State* L, SphereData& sphere)
{
	sphere.pos = luaL_checkfvec3(L, 1);
	sphere.r   = luaL_checkfloat(L, 4);
	sphere.r2 = sphere.r * sphere.r;
	return 4;
}


static inline int ParseCylinder(lua_State* L, CylinderData& cyl)
{
	cyl.pos = luaL_checkfvec2(L, 1);
	cyl.r   = luaL_checkfloat(L, 3);
	cyl.r2 = cyl.r * cyl.r;
	return 3;
}


static inline int ParseBox(lua_State* L, BoxData& box)
{
	const fvec3 mins = luaL_checkfvec3(L, 1);
	const fvec3 maxs = luaL_checkfvec3(L, 4);
	box.extents.set(mins, maxs);
	box.radians = luaL_optfloat(L, 7, 0.0f);
	return 6;
}


//============================================================================//
//============================================================================//

static int PushReflect(lua_State* L, const Obstacle* obs,
                       const fvec3& pos, const fvec3& vel)
{
	fvec3 normal;
	if (obs != NULL) {
		obs->get3DNormal(pos, normal);
	}
	else {
		normal[0] = 0.0f;
		normal[1] = 0.0f;
		normal[2] = 1.0f;
	}

	fvec3 dir = vel;
	ShotStrategy::reflect(dir, normal);

	lua_pushnumber(L, dir[0]);
	lua_pushnumber(L, dir[1]);
	lua_pushnumber(L, dir[2]);

	return 3;
}


int LuaSpatial::RayTrace(lua_State* L)
{
	// FIXME -- use custom routines that do not use shootThrough checks ?
	//       -- what options might be useful here?
	fvec3 pos, vel;
	pos = luaL_checkfvec3(L, 1);
	vel = luaL_checkfvec3(L, 4);
	const float minTime = luaL_optfloat(L, 7, 0.0f);
	float hitTime       = luaL_optfloat(L, 8, +1.0e30f);
	const bool reflect  = lua_isboolean(L, 9) && lua_tobool(L, 9);

	Ray ray(pos, vel);
	int teleFace = -1;

	const bool hitGround   = ShotStrategy::getGround(ray, minTime, hitTime);
	const Obstacle* obs    = ShotStrategy::getFirstBuilding(ray, minTime, hitTime);
	const Teleporter* tele = ShotStrategy::getFirstTeleporter(ray, minTime, hitTime,
	                                                          teleFace);
	fvec3 point;
	if (tele != NULL) {
		int args = 5 + 2;
		lua_pushinteger(L, 't'); // teleporter
		lua_pushnumber(L, hitTime);
		ray.getPoint(hitTime, point);
		lua_pushfvec3(L, point);
		if (reflect) {
			args += PushReflect(L, obs, point, vel);
		}
		lua_pushinteger(L, tele->getGUID());
		lua_pushinteger(L, teleFace);
		return args;
	}
	else if (obs != NULL) {
		if (obs->getType() != MeshFace::getClassName()) {
			int args = 5 + 1;
			lua_pushinteger(L, 'o'); // obstacle
			lua_pushnumber(L, hitTime);
			ray.getPoint(hitTime, point);
			lua_pushfvec3(L, point);
			if (reflect) {
				args += PushReflect(L, obs, point, vel);
			}
			lua_pushinteger(L, obs->getGUID());
			return args;
		}
		else {
			int args = 5 + 2;
			const MeshFace* face = (const MeshFace*)obs;
			const MeshObstacle* mesh = face->getMesh();
			fflush(stdout);
			lua_pushinteger(L, 'f'); // face
			lua_pushnumber(L, hitTime);
			ray.getPoint(hitTime, point);
			lua_pushfvec3(L, point);
			if (reflect) {
				args += PushReflect(L, obs, point, vel);
			}
			if (mesh != NULL) {
				lua_pushinteger(L, mesh->getGUID());
			} else {
				lua_pushboolean(L, false); // should not happen
			}
			lua_pushinteger(L, face->getID() + 1);
			return args;
		}
	}
	else if (hitGround) {
		int args = 5;
		lua_pushinteger(L, 'g'); // ground
		lua_pushnumber(L, hitTime);
		ray.getPoint(hitTime, point);
		lua_pushfvec3(L, point);
		if (reflect) {
			args += PushReflect(L, NULL, point, vel);
		}
		return args;
	}

	return 0;
}


int LuaSpatial::RayTeleport(lua_State* L) // FIXME
{
	return 0;
	const int teleID = luaL_checkint(L, 1);
	const int faceID = luaL_checkint(L, 2);
	fvec3 inPos = luaL_checkfvec3(L, 3);
	fvec3 inVel = luaL_checkfvec3(L, 6);
	const int seed = luaL_optint(L, 9, 0);

	fvec3 outPos;
	fvec3 outVel;
	int outFace = teleID + seed;
	const Teleporter* inTele = NULL;
	const Teleporter* outTele = NULL;
	inTele->getPointWRT(*outTele, faceID, outFace,
	                    inPos,  &inVel,  0.0f,
	                    outPos, &outVel, NULL);

	lua_pushinteger(L, outTele->getGUID());
	lua_pushinteger(L, outFace);
	lua_pushnumber(L, outPos[0]);
	lua_pushnumber(L, outPos[1]);
	lua_pushnumber(L, outPos[2]);
	lua_pushnumber(L, outVel[0]);
	lua_pushnumber(L, outVel[1]);
	lua_pushnumber(L, outVel[2]);
	return 8;
}


//============================================================================//
//============================================================================//

int LuaSpatial::IsPointInView(lua_State* L)
{
	const fvec3 point = luaL_checkfvec3(L, 1);

	const ViewFrustum& vf = RENDERER.getViewFrustum();
	const int planeCount = vf.getPlaneCount();
	for (int p = 0; p < planeCount; p++) {
		const fvec4& s = vf.getSide(p);
		const float d = fvec3::dot(point, s.xyz()) + s.w;
		if (d < 0.0f) {
			lua_pushboolean(L, false);
			return 1;
		}
	}
	lua_pushboolean(L, true);
	return 1;
}


int LuaSpatial::IsSphereInView(lua_State* L)
{
	const fvec3 center = luaL_checkfvec3(L, 1);
	const float radius = luaL_checkfloat(L, 4);

	const ViewFrustum& vf = RENDERER.getViewFrustum();
	const int planeCount = vf.getPlaneCount();
	for (int p = 0; p < planeCount; p++) {
		const fvec4& s = vf.getSide(p);
		const float d = fvec3::dot(center, s.xyz()) + s.w;
		if (d < -radius) {
			lua_pushboolean(L, false);
			return 1;
		}
	}
	lua_pushboolean(L, true);
	return 1;
}


int LuaSpatial::IsAABBInView(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


int LuaSpatial::IsOBBInView(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


int LuaSpatial::IsLSSInView(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


//============================================================================//
//============================================================================//

int LuaSpatial::IsPointInRadar(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


int LuaSpatial::IsSphereInRadar(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


int LuaSpatial::IsAABBInRadar(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


int LuaSpatial::IsOBBInRadar(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


int LuaSpatial::IsLSSInRadar(lua_State* L) // FIXME
{
	return 0;
	L = L;
}


//============================================================================//
//============================================================================//
//
//  Players
//

typedef bool (*PlayerCheckFunc)(const Player*, const QueryData& data);


static bool PlayerInPlanes(const Player* player, const QueryData& data)
{
	const PlanesData& planes = (const PlanesData&)data;
	for (size_t i = 0; i < planes.planes.size(); i++) {
		const fvec4& p = planes.planes[i];
		const fvec3& pos = player->getPosition();
		const float d = fvec3::dot(pos, p.xyz()) + p.w;
		if (d < 0.0f) {
			return false;
		}
	}
	return true;
}


static bool PlayerInSphere(const Player* player, const QueryData& data)
{
	const SphereData& sphere = (const SphereData&)data;
	const fvec3& pos = player->getPosition();
	const float distSqr = (sphere.pos - pos).lengthSq();
	return (distSqr <= sphere.r2);
}


static bool PlayerInCylinder(const Player* player, const QueryData& data)
{
	const CylinderData& cyl = (const CylinderData&)data;
	const fvec3& pos = player->getPosition();
	const float distSqr = (cyl.pos - pos.xy()).lengthSq();
	return (distSqr <= cyl.r2);
}


static bool PlayerInBox(const Player* player, const QueryData& data)
{
	const BoxData& box = (const BoxData&)data;
	const fvec3& pos = player->getPosition();
	return box.extents.contains(pos);
}


static void CheckPlayers(PlayerCheckFunc checkFunc, const QueryData& data,
											 vector<const Player*> hits)
{
	// FIXME - check state

	const Player* player = NULL;

	player = (const Player*)LocalPlayer::getMyTank();
	if (player && checkFunc(player, data)) {
		hits.push_back(player);
	}

#ifdef ROBOT
	for (int i = 0; i < numRobots; i++) {
		player = robots[i];
		if (player && checkFunc(player, data)) {
			hits.push_back(player);
		}
	}
#endif

	for (int i = 0; i < curMaxPlayers; i++) {
		player = remotePlayers[i];
		if (player && checkFunc(player, data)) {
			hits.push_back(player);
		}
	}
}


static void PushPlayers(lua_State* L, vector<const Player*> players)
{
	lua_createtable(L, players.size(), 0);
	for (size_t i = 0; i < players.size(); i++) {
		const Player* player = players[i];
		lua_pushinteger(L, player->getId());
		lua_rawseti(L, -2, i + 1);
	}
}


//============================================================================//

int LuaSpatial::GetPlayersInPlanes(lua_State* L)
{
	PlanesData planes;
	ParsePlanes(L, planes);
	vector<const Player*> hits;
	CheckPlayers(PlayerInPlanes, planes, hits);
	PushPlayers(L, hits);
	return 1;
}


int LuaSpatial::GetPlayersInSphere(lua_State* L)
{
	SphereData sphere;
	ParseSphere(L, sphere);
	vector<const Player*> hits;
	CheckPlayers(PlayerInSphere, sphere, hits);
	PushPlayers(L, hits);
	return 1;
}


int LuaSpatial::GetPlayersInCylinder(lua_State* L)
{
	CylinderData cyl;
	ParseCylinder(L, cyl);
	vector<const Player*> hits;
	CheckPlayers(PlayerInCylinder, cyl, hits);
	PushPlayers(L, hits);
	return 1;
}


int LuaSpatial::GetPlayersInBox(lua_State* L)
{
	BoxData box;
	ParseBox(L, box);
	vector<const Player*> hits;
	CheckPlayers(PlayerInBox, box, hits);
	PushPlayers(L, hits);
	return 1;
}


int LuaSpatial::GetVisiblePlayers(lua_State* L)
{
	PlanesData planes;
	if (!GetViewPlanes(planes)) {
		lua_newtable(L);
		return 1;
	}
	vector<const Player*> hits;
	CheckPlayers(PlayerInPlanes, planes, hits);
	PushPlayers(L, hits);
	return 1;
}


int LuaSpatial::GetRadarPlayers(lua_State* L)
{
	BoxData box;
	if (!GetRadarBox(box)) {
		lua_newtable(L);
		return 1;
	}
	vector<const Player*> hits;
	CheckPlayers(PlayerInBox, box, hits);
	PushPlayers(L, hits);
	return 1;
}


//============================================================================//
//============================================================================//
//
//  Flags
//

typedef bool (*FlagCheckFunc)(const Flag*, const QueryData& data);


static bool FlagInPlanes(const Flag* flag, const QueryData& data)
{
	const PlanesData& planes = (const PlanesData&)data;
	for (size_t i = 0; i < planes.planes.size(); i++) {
		const fvec4& p = planes.planes[i];
		const fvec3& pos = flag->position;
		const float d = fvec3::dot(pos, p.xyz()) + p.w;
		if (d < 0.0f) {
			return false;
		}
	}
	return true;
}


static bool FlagInSphere(const Flag* flag, const QueryData& data)
{
	const SphereData& sphere = (const SphereData&)data;
	const fvec3& pos = flag->position;
	const float distSqr = (sphere.pos - pos).lengthSq();
	return (distSqr <= sphere.r2);
}


static bool FlagInCylinder(const Flag* flag, const QueryData& data)
{
	const CylinderData& cyl = (const CylinderData&)data;
	const fvec3& pos = flag->position;
	const float distSqr = (cyl.pos - pos.xy()).lengthSq();
	return (distSqr <= cyl.r2);
}


static bool FlagInBox(const Flag* flag, const QueryData& data)
{
	const BoxData& box = (const BoxData&)data;
	return box.extents.contains(flag->position);
}


static void CheckFlags(FlagCheckFunc checkFunc, const QueryData& data,
											 vector<const Flag*> hits)
{
	const World* world = World::getWorld();
	if (world == NULL) {
		return;
	}
	const int maxFlags = world->getMaxFlags();
	for (int i = 0; i < maxFlags; i++) {
		const Flag* flag = &(world->getFlag(i));
		if (flag->status != FlagNoExist) {
			if (checkFunc(flag, data)) {
				hits.push_back(flag);
			}
		}
	}
}


static void PushFlags(lua_State* L, vector<const Flag*> flags)
{
	lua_createtable(L, flags.size(), 0);
	for (size_t i = 0; i < flags.size(); i++) {
		lua_pushinteger(L, flags[i]->id);
		lua_rawseti(L, -2, i + 1);
	}
}


//============================================================================//

int LuaSpatial::GetFlagsInPlanes(lua_State* L)
{
	PlanesData planes;
	ParsePlanes(L, planes);
	vector<const Flag*> hits;
	CheckFlags(FlagInPlanes, planes, hits);
	PushFlags(L, hits);
	return 1;
}


int LuaSpatial::GetFlagsInSphere(lua_State* L)
{
	SphereData sphere;
	ParseSphere(L, sphere);
	vector<const Flag*> hits;
	CheckFlags(FlagInSphere, sphere, hits);
	PushFlags(L, hits);
	return 1;
}


int LuaSpatial::GetFlagsInCylinder(lua_State* L)
{
	CylinderData cyl;
	ParseCylinder(L, cyl);
	vector<const Flag*> hits;
	CheckFlags(FlagInCylinder, cyl, hits);
	PushFlags(L, hits);
	return 1;
}


int LuaSpatial::GetFlagsInBox(lua_State* L)
{
	BoxData box;
	ParseBox(L, box);
	vector<const Flag*> hits;
	CheckFlags(FlagInBox, box, hits);
	PushFlags(L, hits);
	return 1;
}


int LuaSpatial::GetVisibleFlags(lua_State* L)
{
	PlanesData planes;
	if (!GetViewPlanes(planes)) {
		lua_newtable(L);
		return 1;
	}
	vector<const Flag*> hits;
	CheckFlags(FlagInPlanes, planes, hits);
	PushFlags(L, hits);
	return 1;
}


int LuaSpatial::GetRadarFlags(lua_State* L)
{
	BoxData box;
	if (!GetRadarBox(box)) {
		lua_newtable(L);
		return 1;
	}
	vector<const Flag*> hits;
	CheckFlags(FlagInBox, box, hits);
	PushFlags(L, hits);
	return 1;
}


//============================================================================//
//============================================================================//
//
//  Shots
//

typedef bool (*ShotCheckFunc)(const ShotPath*, const QueryData& data);


static bool ShotInPlanes(const ShotPath* shot, const QueryData& data)
{
	const PlanesData& planes = (const PlanesData&)data;
	for (size_t i = 0; i < planes.planes.size(); i++) {
		const fvec4& p = planes.planes[i];
		const fvec3& pos = shot->getPosition();
		const float d = fvec3::dot(pos, p.xyz()) + p.w;
		if (d < 0.0f) {
			return false;
		}
	}
	return true;
}


static bool ShotInSphere(const ShotPath* shot, const QueryData& data)
{
	const SphereData& sphere = (const SphereData&)data;
	const fvec3& pos = shot->getPosition();
	const float distSqr = (sphere.pos - pos).lengthSq();
	return (distSqr <= sphere.r2);
}


static bool ShotInCylinder(const ShotPath* shot, const QueryData& data)
{
	const CylinderData& cyl = (const CylinderData&)data;
	const fvec3& pos = shot->getPosition();
	const float distSqr = (cyl.pos - pos.xy()).lengthSq();
	return (distSqr <= cyl.r2);
}


static bool ShotInBox(const ShotPath* shot, const QueryData& data)
{
	const BoxData& box = (const BoxData&)data;
	const fvec3& pos = shot->getPosition();
	return box.extents.contains(pos);
}


static void CheckShots(ShotCheckFunc checkFunc, const QueryData& data,
											 vector<const ShotPath*> hits)
{
	const World* world = World::getWorld();
	if (world == NULL) {
		return;
	}
	int maxShots = world->getMaxShots();

	// draw check shots
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	if (myTank) {
		for (int i = 0; i < maxShots; i++) {
			const ShotPath* shot = myTank->getShot(i);
			if (shot && !shot->isExpired()) {
				if (checkFunc(shot, data)) {
					hits.push_back(shot);
				}
			}
		}
	}

	// check robot tanks' shots
#ifdef ROBOT
	for (int i = 0; i < numRobots; i++) {
		const RobotPlayer* player = robots[i];
		if (player != NULL) {
			for (int j = 0; j < maxShots; j++) {
				const ShotPath* shot = player->getShot(j);
				if (shot && !shot->isExpired()) {
					if (checkFunc(shot, data)) {
						hits.push_back(shot);
					}
				}
			}
		}
	}
#endif

	// check other tanks' shots
	for (int i = 0; i < curMaxPlayers; i++) {
		const RemotePlayer* player = world->getPlayer(i);
		if (player != NULL) {
			for (int j = 0; j < maxShots; j++) {
				const ShotPath* shot = player->getShot(j);
				if (shot && !shot->isExpired()) {
					if (checkFunc(shot, data)) {
						hits.push_back(shot);
					}
				}
			}
		}
	}

	// check world weapon shots
	const WorldPlayer* worldWeapons = world->getWorldWeapons();
	if (worldWeapons != NULL) {
		const int wwMaxShots = worldWeapons->getMaxShots();
		for (int i = 0; i < wwMaxShots; i++) {
			const ShotPath* shot = worldWeapons->getShot(i);
			if (shot && !shot->isExpired()) {
				if (checkFunc(shot, data)) {
					hits.push_back(shot);
				}
			}
		}
	}
}


static void PushShots(lua_State* L, vector<const ShotPath*> shots)
{
	lua_createtable(L, shots.size(), 0);
	for (size_t i = 0; i < shots.size(); i++) {
		const ShotPath* shot = shots[i];
		const uint32_t shotID = (shot->getPlayer() << 16) | shot->getShotId();
		lua_pushinteger(L, shotID);
		lua_rawseti(L, -2, i + 1);
	}
}


//============================================================================//

int LuaSpatial::GetShotsInPlanes(lua_State* L)
{
	PlanesData planes;
	ParsePlanes(L, planes);
	vector<const ShotPath*> hits;
	CheckShots(ShotInPlanes, planes, hits);
	PushShots(L, hits);
	return 1;
}


int LuaSpatial::GetShotsInSphere(lua_State* L)
{
	SphereData sphere;
	ParseSphere(L, sphere);
	vector<const ShotPath*> hits;
	CheckShots(ShotInSphere, sphere, hits);
	PushShots(L, hits);
	return 1;
}


int LuaSpatial::GetShotsInCylinder(lua_State* L)
{
	CylinderData cyl;
	ParseCylinder(L, cyl);
	vector<const ShotPath*> hits;
	CheckShots(ShotInCylinder, cyl, hits);
	PushShots(L, hits);
	return 1;
}


int LuaSpatial::GetShotsInBox(lua_State* L)
{
	BoxData box;
	ParseBox(L, box);
	vector<const ShotPath*> hits;
	CheckShots(ShotInBox, box, hits);
	PushShots(L, hits);
	return 1;
}


int LuaSpatial::GetVisibleShots(lua_State* L)
{
	PlanesData planes;
	if (!GetViewPlanes(planes)) {
		lua_newtable(L);
		return 1;
	}
	vector<const ShotPath*> hits;
	CheckShots(ShotInPlanes, planes, hits);
	PushShots(L, hits);
	return 1;
}


int LuaSpatial::GetRadarShots(lua_State* L)
{
	BoxData box;
	if (!GetRadarBox(box)) {
		lua_newtable(L);
		return 1;
	}
	vector<const ShotPath*> hits;
	CheckShots(ShotInBox, box, hits);
	PushShots(L, hits);
	return 1;
}


//============================================================================//
//============================================================================//
//
//  Obstacles
//

int LuaSpatial::GetObstaclesInPlanes(lua_State* L) // FIXME
{
	L = L;
	return 0;
}


int LuaSpatial::GetObstaclesInSphere(lua_State* L) // FIXME
{
	L = L;
	return 0;
}


int LuaSpatial::GetObstaclesInCylinder(lua_State* L) // FIXME
{
	L = L;
	return 0;
}


int LuaSpatial::GetObstaclesInBox(lua_State* L) // FIXME
{
	L = L;
	return 0;
}


int LuaSpatial::GetVisibleObstacles(lua_State* L) // FIXME
{
	L = L;
	return 0;
}


int LuaSpatial::GetRadarObstacles(lua_State* L) // FIXME
{
	L = L;
	return 0;
}


//============================================================================//
//============================================================================//
