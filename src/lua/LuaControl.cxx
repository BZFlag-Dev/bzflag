
#include "common.h"

// interface header
#include "LuaControl.h"

// common headers
#include "CommandManager.h"

// bzflag headers
#include "../bzflag/LocalPlayer.h"
#include "../bzflag/playing.h"

// local headers
#include "LuaHandle.h"
#include "LuaInclude.h"
#include "LuaHashString.h"


/******************************************************************************/
/******************************************************************************/

bool LuaControl::PushEntries(lua_State* L)
{
	PUSH_LUA_CFUNC(L, Move);
	PUSH_LUA_CFUNC(L, Fire);
	PUSH_LUA_CFUNC(L, Jump);
	PUSH_LUA_CFUNC(L, Spawn);
	PUSH_LUA_CFUNC(L, Pause);
	PUSH_LUA_CFUNC(L, DropFlag);
	PUSH_LUA_CFUNC(L, SetTarget);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static inline bool ValidScript(lua_State* L)
{
	return ((L2H(L)->GetName() == "LuaBzOrg") || LuaHandle::GetDevMode());
}


/******************************************************************************/
/******************************************************************************/

int LuaControl::Move(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control movement");
	}
	const float speed  = luaL_optfloat(L, 1, 0.0f);
	const float angVel = luaL_optfloat(L, 2, 0.0f);
	forceControls(true, speed, angVel);
	return 0;
}


int LuaControl::Fire(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control firing");
	}
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	if (myTank == NULL) {
		return 0;
	}
	lua_pushboolean(L, myTank->fireShot());
	return 1;
}


int LuaControl::Jump(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control jumps");
	}
	LocalPlayer* myTank = LocalPlayer::getMyTank();
	if (myTank == NULL) {
		return 0;
	}
	myTank->doJump();
	return 0;
}


int LuaControl::Spawn(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control spawns");
	}
	CMDMGR.run("restart");
	return 0;
}


int LuaControl::Pause(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control pausing");
	}
	CMDMGR.run("pause");
	return 0;
}


int LuaControl::DropFlag(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control flag drops");
	}
	CMDMGR.run("drop");
	return 0;
}


int LuaControl::SetTarget(lua_State* L)
{
	if (!ValidScript(L)) {
		luaL_error(L, "this script can not control target locks");
	}
	CMDMGR.run("identify");
	return 0;
}


/******************************************************************************/
/******************************************************************************/
