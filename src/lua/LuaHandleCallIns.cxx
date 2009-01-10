
#include "common.h"

// implementation header
#include "LuaHandle.h"

// system headers
#include <string>
#include <set>
using std::string;
using std::set;

// common headers
#include "Flag.h"
#include "ShotUpdate.h"
#include "BzfDisplay.h"

// local headers
#include "LuaInclude.h"
#include "LuaHashString.h"
#include "LuaUtils.h"
#include "LuaCallInDB.h"
#include "LuaCallInCheck.h"

// bzflag headers
#include "../bzflag/playing.h"
#include "../bzflag/MainWindow.h"
#include "../bzflag/Player.h"
#include "../bzflag/ShotPath.h"

/******************************************************************************/
/******************************************************************************/

void LuaHandle::Shutdown()
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 2);
	if (!PushCallIn(LUA_CI_Shutdown)) {
		return; // the call is not defined
	}

	// call the routine
	RunCallIn(LUA_CI_Shutdown, 0, 0);
	return;
}


void LuaHandle::Update()
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 2);
	if (!PushCallIn(LUA_CI_Update)) {
		return; // the call is not defined
	}

	// call the routine
	RunCallIn(LUA_CI_Update, 0, 0);
	return;
}


void LuaHandle::BZDBChange(const string& name)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_BZDBChange)) {
		return; // the call is not defined
	}

	lua_pushlstring(L, name.data(), name.size());

	// call the routine
	RunCallIn(LUA_CI_BZDBChange, 1, 0);
	return;
}


void LuaHandle::ServerJoined()
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 2);
	if (!PushCallIn(LUA_CI_ServerJoined)) {
		return; // the call is not defined
	}

	// call the routine
	RunCallIn(LUA_CI_ServerJoined, 0, 0);
	return;
}


void LuaHandle::ServerParted()
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 2);
	if (!PushCallIn(LUA_CI_ServerParted)) {
		return; // the call is not defined
	}

	// call the routine
	RunCallIn(LUA_CI_ServerParted, 0, 0);
	return;
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::RecvCommand(const std::string& msg)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_RecvCommand)) {
		return; // the call is not defined
	}

	lua_pushlstring(L, msg.data(), msg.size());
	
	// call the routine
	RunCallIn(LUA_CI_RecvCommand, 1, 0);
	return;
}


void LuaHandle::RecvLuaData(int srcPlayerID, int srcScriptID,
                            int dstPlayerID, int dstScriptID,
                            int status, const std::string& data)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 8);
	if (!PushCallIn(LUA_CI_RecvLuaData)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, srcPlayerID);
	lua_pushinteger(L, srcScriptID);
	lua_pushinteger(L, dstPlayerID);
	lua_pushinteger(L, dstScriptID);
	lua_pushinteger(L, status);
	lua_pushstdstring(L, data);
	
	// call the routine
	RunCallIn(LUA_CI_RecvLuaData, 6, 0);
	return;
}


void LuaHandle::RecvChatMsg(const std::string& msg, int srcID, int dstID)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_RecvChatMsg)) {
		return; // the call is not defined
	}

	lua_pushlstring(L, msg.data(), msg.size());
	lua_pushinteger(L, srcID);
	lua_pushinteger(L, dstID);
	
	// call the routine
	RunCallIn(LUA_CI_RecvChatMsg, 3, 0);
	return;
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::PlayerAdded(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerAdded)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerAdded, 1, 0);
	return;
}


void LuaHandle::PlayerRemoved(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerRemoved)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerRemoved, 1, 0);
	return;
}


void LuaHandle::PlayerSpawned(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerSpawned)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerSpawned, 1, 0);
	return;
}


void LuaHandle::PlayerKilled(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerKilled)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerKilled, 1, 0);
	return;
}


void LuaHandle::PlayerJumped(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerJumped)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerJumped, 1, 0);
	return;
}


void LuaHandle::PlayerLanded(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerLanded)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerLanded, 1, 0);
	return;
}


void LuaHandle::PlayerTeleported(const Player& player, int srcLink, int dstLink)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_PlayerTeleported)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());
	lua_pushinteger(L, srcLink);
	lua_pushinteger(L, dstLink);

	// call the routine
	RunCallIn(LUA_CI_PlayerTeleported, 3, 0);
	return;
}


void LuaHandle::PlayerTeamChange(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerTeamChange)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerTeamChange, 1, 0);
	return;
}


void LuaHandle::PlayerScoreChange(const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_PlayerScoreChange)) {
		return; // the call is not defined
	}

	lua_pushnumber(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_PlayerScoreChange, 1, 0);
	return;
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::ShotAdded(const FiringInfo& info)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_ShotAdded)) {
		return; // the call is not defined
	}

	const uint8_t playerID = (uint8_t)info.shot.player;
	const uint16_t infoID = info.shot.id;
	const uint32_t shotID = (playerID << 16) | infoID;

	lua_pushinteger(L, shotID);
	lua_pushinteger(L, playerID);
	lua_pushinteger(L, info.shotType);

	// call the routine
	RunCallIn(LUA_CI_ShotAdded, 3, 0);
	return;
}


void LuaHandle::ShotRemoved(const FiringInfo& info)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_ShotRemoved)) {
		return; // the call is not defined
	}

	const uint8_t playerID = (uint8_t)info.shot.player;
	const uint16_t infoID = info.shot.id;
	const uint32_t shotID = (playerID << 16) | infoID;

	lua_pushinteger(L, shotID);

	// call the routine
	RunCallIn(LUA_CI_ShotRemoved, 1, 0);
	return;
}


void LuaHandle::ShotTeleported(const ShotPath& path, int srcLink, int dstLink)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_ShotTeleported)) {
		return; // the call is not defined
	}

	const FiringInfo& info = path.getFiringInfo();
	const uint8_t playerID = (uint8_t)info.shot.player;
	const uint16_t infoID = info.shot.id;
	const uint32_t shotID = (playerID << 16) | infoID;

	lua_pushinteger(L, shotID);
	lua_pushinteger(L, srcLink);
	lua_pushinteger(L, dstLink);

	// call the routine
	RunCallIn(LUA_CI_ShotTeleported, 1, 0);
	return;
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::FlagAdded(const Flag& flag)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_FlagAdded)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, flag.id);

	// call the routine
	RunCallIn(LUA_CI_FlagAdded, 1, 0);
	return;
}


void LuaHandle::FlagRemoved(const Flag& flag)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_FlagRemoved)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, flag.id);

	// call the routine
	RunCallIn(LUA_CI_FlagRemoved, 1, 0);
	return;
}


void LuaHandle::FlagGrabbed(const Flag& flag, const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_FlagGrabbed)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, flag.id);
	lua_pushinteger(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_FlagGrabbed, 2, 0);
	return;
}


void LuaHandle::FlagDropped(const Flag& flag, const Player& player)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_FlagDropped)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, flag.id);
	lua_pushinteger(L, player.getId());

	// call the routine
	RunCallIn(LUA_CI_FlagDropped, 2, 0);
	return;
}


void LuaHandle::FlagCaptured(const Flag& flag, const Player&)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_FlagCaptured)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, flag.id);

	// call the routine
	RunCallIn(LUA_CI_FlagCaptured, 1, 0);
	return;
}


void LuaHandle::FlagTransferred(const Flag& flag,
                                const Player& src, const Player& dst)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_FlagTransferred)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, flag.id);
	lua_pushinteger(L, src.getId());
	lua_pushinteger(L, dst.getId());

	// call the routine
	RunCallIn(LUA_CI_FlagTransferred, 3, 0);
	return;
}


/******************************************************************************/
/******************************************************************************/

void LuaHandle::ViewResize()
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 2);
	if (!PushCallIn(LUA_CI_ViewResize)) {
		return; // the call is not defined
	}

	// call the routine
	RunCallIn(LUA_CI_ViewResize, 0, 0);
	return;
}


// NOTE:  GLContextInit  vs.  GLReload
void LuaHandle::GLContextInit()
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 2);
	if (!PushCallIn(LUA_CI_GLReload)) {
		return; // the call is not defined
	}

	// call the routine
	RunCallIn(LUA_CI_GLReload, 0, 0);
	return;
}


void LuaHandle::GLContextFree()
{
	return; // do nothing, lua scripts should not need it
}


/******************************************************************************/
/******************************************************************************/

#define DRAW_CALLIN(name)                \
	void LuaHandle:: name () {             \
		LUA_CALL_IN_CHECK(L);	               \
		lua_checkstack(L, 2);                \
		if (!PushCallIn(LUA_CI_ ## name )) { \
			return;                            \
		}                                    \
		RunCallIn(LUA_CI_ ## name, 0, 0);    \
	}

DRAW_CALLIN(DrawGenesis)
DRAW_CALLIN(DrawWorldStart)
DRAW_CALLIN(DrawWorld)
DRAW_CALLIN(DrawWorldAlpha)
DRAW_CALLIN(DrawWorldShadow)
DRAW_CALLIN(DrawScreenStart)
DRAW_CALLIN(DrawScreen)
DRAW_CALLIN(DrawRadar)


/******************************************************************************/
/******************************************************************************/

void LuaHandle::GotGfxBlock(int type, int id)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_GotGfxBlock)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, type);
	lua_pushinteger(L, id);

	// call the routine
	RunCallIn(LUA_CI_GotGfxBlock, 2, 0);
	return;
}


void LuaHandle::LostGfxBlock(int type, int id)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_LostGfxBlock)) {
		return; // the call is not defined
	}

	lua_pushinteger(L, type);
	lua_pushinteger(L, id);

	// call the routine
	RunCallIn(LUA_CI_LostGfxBlock, 2, 0);
}


/******************************************************************************/
/******************************************************************************/

bool LuaHandle::KeyPress(int key, bool isRepeat)
{
	BzfDisplay* dpy = getDisplay();
	if (dpy == NULL) {
		return false;
	}

	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 6);
	if (!PushCallIn(LUA_CI_KeyPress)) {
		return false; // the call is not defined, do not take the event
	}

	lua_pushinteger(L, key);

	bool alt, ctrl, shift;
	dpy->getModState(shift, ctrl, alt);

	int mods = 0;
	if (alt)   { mods |= (1 << 0); }
	if (ctrl)  { mods |= (1 << 1); }
	if (shift) { mods |= (1 << 2); }
	lua_pushinteger(L, mods);

	lua_pushboolean(L, isRepeat);

	// call the function
	if (!RunCallIn(LUA_CI_KeyPress, 3, 1)) {
		return false;
	}

	// const int args = lua_gettop(L); unused
	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


bool LuaHandle::KeyRelease(int key)
{
	BzfDisplay* dpy = getDisplay();
	if (dpy == NULL) {
		return false;
	}

	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_KeyRelease)) {
		return false; // the call is not defined, do not take the event
	}

	lua_pushinteger(L, key);

	bool alt, ctrl, shift;
	dpy->getModState(shift, ctrl, alt);

	int mods = 0;
	if (alt)   { mods |= (1 << 0); }
	if (ctrl)  { mods |= (1 << 1); }
	if (shift) { mods |= (1 << 2); }
	lua_pushinteger(L, mods);

	// call the function
	if (!RunCallIn(LUA_CI_KeyRelease, 2, 1)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


bool LuaHandle::MousePress(int x, int y, int button)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_MousePress)) {
		return false; // the call is not defined, do not take the event
	}

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, button);

	// call the function
	if (!RunCallIn(LUA_CI_MousePress, 3, 1)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


bool LuaHandle::MouseRelease(int x, int y, int button)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 5);
	if (!PushCallIn(LUA_CI_MouseRelease)) {
		return false; // the call is not defined, do not take the event
	}

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, button);

	// call the function
	if (!RunCallIn(LUA_CI_MouseRelease, 3, 1)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


bool LuaHandle::MouseMove(int x, int y)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_MouseMove)) {
		return false; // the call is not defined, do not take the event
	}

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	// call the function
	if (!RunCallIn(LUA_CI_MouseMove, 2, 1)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


bool LuaHandle::MouseWheel(float value)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_MouseWheel)) {
		return false; // the call is not defined, do not take the event
	}

	lua_pushnumber(L, value);

	// call the function
	if (!RunCallIn(LUA_CI_MouseWheel, 1, 1)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


bool LuaHandle::IsAbove(int x, int y)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_IsAbove)) {
		return false; // the call is not defined
	}

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	// call the function
	if (!RunCallIn(LUA_CI_IsAbove, 2, 1)) {
		return false;
	}

	if (!lua_isboolean(L, -1)) {
		lua_pop(L, 1);
		return false;
	}
	const bool retval = lua_tobool(L, -1);
	lua_pop(L, 1);
	return retval;
}


string LuaHandle::GetTooltip(int x, int y)
{
	LUA_CALL_IN_CHECK(L);
	lua_checkstack(L, 4);
	if (!PushCallIn(LUA_CI_GetTooltip)) {
		return ""; // the call is not defined
	}

	lua_pushinteger(L, x);
	lua_pushinteger(L, y);

	// call the function
	if (!RunCallIn(LUA_CI_GetTooltip, 2, 1)) {
		return "";
	}

	if (!lua_israwstring(L, -1)) {
		lua_pop(L, 1);
		return "";
	}
	const string retval = lua_tostring(L, -1);
	lua_pop(L, 1);
	return retval;
}


void LuaHandle::WordComplete(const string& line,
                             set<string>& partials)
{
	LUA_CALL_IN_CHECK(L);	
	lua_checkstack(L, 3);
	if (!PushCallIn(LUA_CI_WordComplete)) {
		return;
	}

	lua_pushstring(L, line.c_str());

	// call the routine
	if (!RunCallIn(LUA_CI_WordComplete, 1, 1)) {
		return;
	}

	const int table = 1;
	if (lua_istable(L, table)) {
		for (int i = 1; lua_checkgeti(L, table, i) != 0; lua_pop(L, 1), i++) {
			if (lua_israwstring(L, -1)) {
				partials.insert(lua_tostring(L, -1));
			}
		}		
	}
	lua_pop(L, 1);

	return;
}


/******************************************************************************/
/******************************************************************************/
