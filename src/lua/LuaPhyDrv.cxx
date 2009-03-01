
#include "common.h"

// interface header
#include "LuaPhyDrv.h"

// system headers
#include <string>
using std::string;

// common headers
#include "PhysicsDriver.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"


//============================================================================//
//============================================================================//

bool LuaPhyDrv::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, GetPhyDrvName);
	PUSH_LUA_CFUNC(L, GetPhyDrvDeath);
	PUSH_LUA_CFUNC(L, GetPhyDrvSlideTime);
	PUSH_LUA_CFUNC(L, GetPhyDrvVelocity);
	PUSH_LUA_CFUNC(L, GetPhyDrvRadialPos);
	PUSH_LUA_CFUNC(L, GetPhyDrvRadialVel);
	PUSH_LUA_CFUNC(L, GetPhyDrvAngularPos);
	PUSH_LUA_CFUNC(L, GetPhyDrvAngularVel);

	return true;
}


//============================================================================//
//============================================================================//

static inline const PhysicsDriver* ParsePhyDrv(lua_State*L, int index)
{
	const int phydrvIndex = luaL_checkint(L, index);
	return PHYDRVMGR.getDriver(phydrvIndex);
}


//============================================================================//
//============================================================================//

int LuaPhyDrv::GetPhyDrvName(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	lua_pushstdstring(L, phydrv->getName());
	return 1;
}


int LuaPhyDrv::GetPhyDrvDeath(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	if (!phydrv->getIsDeath()) {
		return 0;
	}
	lua_pushstdstring(L, phydrv->getDeathMsg());
	return 1;
}


int LuaPhyDrv::GetPhyDrvSlideTime(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	if (!phydrv->getIsSlide()) {
		return 0;
	}
	lua_pushnumber(L, phydrv->getSlideTime());
	return 1;
}


int LuaPhyDrv::GetPhyDrvVelocity(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	const float* vec = phydrv->getLinearVel();
	lua_pushnumber(L, vec[0]);
	lua_pushnumber(L, vec[1]);
	lua_pushnumber(L, vec[2]);
	return 3;
}


int LuaPhyDrv::GetPhyDrvRadialPos(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	const float* vec = phydrv->getRadialPos();
	lua_pushnumber(L, vec[0]);
	lua_pushnumber(L, vec[1]);
	lua_pushnumber(L, vec[2]);
	return 3;
}


int LuaPhyDrv::GetPhyDrvRadialVel(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	lua_pushnumber(L, phydrv->getRadialVel());
	return 1;
}


int LuaPhyDrv::GetPhyDrvAngularPos(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	const float* vec = phydrv->getAngularPos();
	lua_pushnumber(L, vec[0]);
	lua_pushnumber(L, vec[1]);
	lua_pushnumber(L, vec[2]);
	return 3;
}


int LuaPhyDrv::GetPhyDrvAngularVel(lua_State* L)
{
	const PhysicsDriver* phydrv = ParsePhyDrv(L, 1);
	if (phydrv == NULL) {
		return 0;
	}
	lua_pushnumber(L, phydrv->getAngularVel());
	return 1;
}



//============================================================================//
//============================================================================//
