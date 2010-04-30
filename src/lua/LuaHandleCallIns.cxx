/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "common.h"

// interface header
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
#include "LuaHeader.h"
#include "LuaUtils.h"
#include "LuaCallInDB.h"
#include "LuaCallInCheck.h"
#include "LuaGLPointers.h"

// bzflag headers
#include "bzflag/guiplaying.h"
#include "bzflag/MainWindow.h"
#include "clientbase/Player.h"
#include "clientbase/ShotPath.h"


//============================================================================//
//============================================================================//

static bool PopBool(lua_State* L, bool def)
{
  if (!lua_isboolean(L, -1)) {
    lua_pop(L, 1);
    return def;
  }
  const bool retval = lua_tobool(L, -1);
  lua_pop(L, 1);
  return retval;
}


//============================================================================//
//============================================================================//

void LuaHandle::Shutdown()
{
  LUA_CALL_IN_CHECK(L, 0)

  // NOTE: we don't use PushCallIn() here because Shutdown() is not managed by
  //       EventHandler, so no warning should be given if it is not found
  lua_rawgeti(L, LUA_CALLINSINDEX, LUA_CI_Shutdown);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return;
  }

  RunCallIn(LUA_CI_Shutdown, 0, 0);
}


void LuaHandle::Update()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_Update)) {
    return; // the call is not defined
  }

  RunCallIn(LUA_CI_Update, 0, 0);
}


void LuaHandle::BZDBChange(const string& name)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_BZDBChange)) {
    return; // the call is not defined
  }

  lua_pushlstring(L, name.data(), name.size());

  RunCallIn(LUA_CI_BZDBChange, 1, 0);
}


void LuaHandle::ServerJoined()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_ServerJoined)) {
    return; // the call is not defined
  }

  RunCallIn(LUA_CI_ServerJoined, 0, 0);
}


void LuaHandle::ServerParted()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_ServerParted)) {
    return; // the call is not defined
  }

  RunCallIn(LUA_CI_ServerParted, 0, 0);
}


//============================================================================//
//============================================================================//

bool LuaHandle::CommandFallback(const std::string& cmd)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_CommandFallback)) {
    return false; // the call is not defined
  }

  lua_pushlstring(L, cmd.data(), cmd.size());

  if (!RunCallIn(LUA_CI_CommandFallback, 1, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::RecvCommand(const std::string& cmd)
{
  LUA_CALL_IN_CHECK(L, 1)

  // NOTE: we don't use PushCallIn() here because RecvCommand() is not managed
  //       by EventHandler, so no warning should be given if it is not found
  lua_rawgeti(L, LUA_CALLINSINDEX, LUA_CI_RecvCommand);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return false;
  }

  lua_pushlstring(L, cmd.data(), cmd.size());

  if (!RunCallIn(LUA_CI_RecvCommand, 1, 1)) {
    return false;
  }

  return PopBool(L, false);
}


void LuaHandle::RecvLuaData(int srcPlayerID, int srcScriptID,
			    int dstPlayerID, int dstScriptID,
			    int status, const std::string& data)
{
  LUA_CALL_IN_CHECK(L, 6)
  if (!PushCallIn(LUA_CI_RecvLuaData)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, srcPlayerID);
  lua_pushinteger(L, srcScriptID);
  lua_pushinteger(L, dstPlayerID);
  lua_pushinteger(L, dstScriptID);
  lua_pushinteger(L, status);
  lua_pushstdstring(L, data);

  RunCallIn(LUA_CI_RecvLuaData, 6, 0);
}


void LuaHandle::RecvChatMsg(const std::string& msg,
			    int srcID, int dstID, bool action)
{
  LUA_CALL_IN_CHECK(L, 4)
  if (!PushCallIn(LUA_CI_RecvChatMsg)) {
    return; // the call is not defined
  }

  lua_pushlstring(L, msg.data(), msg.size());
  lua_pushinteger(L, srcID);
  lua_pushinteger(L, dstID);
  lua_pushboolean(L, action);

  RunCallIn(LUA_CI_RecvChatMsg, 4, 0);
}


//============================================================================//
//============================================================================//

void LuaHandle::PlayerAdded(const Player& player)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_PlayerAdded)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_PlayerAdded, 1, 0);
}


void LuaHandle::PlayerRemoved(const Player& player)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_PlayerRemoved)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_PlayerRemoved, 1, 0);
}


void LuaHandle::PlayerSpawned(const Player& player)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_PlayerSpawned)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_PlayerSpawned, 1, 0);
}


void LuaHandle::PlayerKilled(const Player& victim, const Player* killer,
			     int reason, const FlagType* flagType, int phyDrv)
{
  LUA_CALL_IN_CHECK(L, 5)
  if (!PushCallIn(LUA_CI_PlayerKilled)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, victim.getId()); // 1
  if (killer) {
    lua_pushinteger(L, killer->getId()); // 2
  } else {
    lua_pushnil(L); // 2
  }
  lua_pushinteger(L, reason); // 3
  if (flagType) {
    lua_pushstring(L, flagType->flagAbbv.c_str()); // 4
  } else {
    lua_pushnil(L); // 4
  }
  lua_pushinteger(L, phyDrv); // 5

  RunCallIn(LUA_CI_PlayerKilled, 5, 0);
}


void LuaHandle::PlayerJumped(const Player& player)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_PlayerJumped)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_PlayerJumped, 1, 0);
}


void LuaHandle::PlayerLanded(const Player& player, float vel)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_PlayerLanded)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());
  lua_pushfloat(L, vel);

  RunCallIn(LUA_CI_PlayerLanded, 2, 0);
}


void LuaHandle::PlayerTeleported(const Player& player, int srcLink, int dstLink)
{
  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_PlayerTeleported)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());
  lua_pushinteger(L, srcLink);
  lua_pushinteger(L, dstLink);

  RunCallIn(LUA_CI_PlayerTeleported, 3, 0);
}


void LuaHandle::PlayerTeamChange(const Player& player, int oldTeam)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_PlayerTeamChange)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());
  lua_pushinteger(L, oldTeam);

  RunCallIn(LUA_CI_PlayerTeamChange, 2, 0);
}


void LuaHandle::PlayerScoreChange(const Player& player)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_PlayerScoreChange)) {
    return; // the call is not defined
  }

  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_PlayerScoreChange, 1, 0);
}


//============================================================================//
//============================================================================//

void LuaHandle::ShotAdded(const FiringInfo& info)
{
  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_ShotAdded)) {
    return; // the call is not defined
  }

  const uint8_t playerID = (uint8_t)info.shot.player;
  const uint16_t infoID = info.shot.id;
  const uint32_t shotID = (playerID << 16) | infoID;

  lua_pushinteger(L, shotID);
  lua_pushinteger(L, playerID);
  lua_pushinteger(L, info.shotType);

  RunCallIn(LUA_CI_ShotAdded, 3, 0);
}


void LuaHandle::ShotRemoved(const FiringInfo& info)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_ShotRemoved)) {
    return; // the call is not defined
  }

  const uint8_t playerID = (uint8_t)info.shot.player;
  const uint16_t infoID = info.shot.id;
  const uint32_t shotID = (playerID << 16) | infoID;

  lua_pushinteger(L, shotID);

  RunCallIn(LUA_CI_ShotRemoved, 1, 0);
}


void LuaHandle::ShotRicochet(const ShotPath& path,
			     const float* pos, const float* normal)
{
  LUA_CALL_IN_CHECK(L, 7)
  if (!PushCallIn(LUA_CI_ShotRicochet)) {
    return; // the call is not defined
  }

  const FiringInfo& info = path.getFiringInfo();
  const uint8_t playerID = (uint8_t)info.shot.player;
  const uint16_t infoID = info.shot.id;
  const uint32_t shotID = (playerID << 16) | infoID;

  lua_pushinteger(L, shotID);
  lua_pushfloat(L, pos[0]);
  lua_pushfloat(L, pos[1]);
  lua_pushfloat(L, pos[2]);
  lua_pushfloat(L, normal[0]);
  lua_pushfloat(L, normal[1]);
  lua_pushfloat(L, normal[2]);

  RunCallIn(LUA_CI_ShotRicochet, 7, 0);
}


void LuaHandle::ShotTeleported(const ShotPath& path, int srcLink, int dstLink)
{
  LUA_CALL_IN_CHECK(L, 3)
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

  RunCallIn(LUA_CI_ShotTeleported, 3, 0);
}


//============================================================================//
//============================================================================//

void LuaHandle::FlagAdded(const Flag& flag)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_FlagAdded)) {
    return; // the call is not defined
  }

  if (L2H(L)->HasFullRead()) {
    lua_pushinteger(L, flag.id);
  } else {
    lua_pushnil(L);
  }
  lua_pushstdstring(L, flag.type->flagAbbv);

  RunCallIn(LUA_CI_FlagAdded, 2, 0);
}


void LuaHandle::FlagRemoved(const Flag& flag)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_FlagRemoved)) {
    return; // the call is not defined
  }

  if (L2H(L)->HasFullRead()) {
    lua_pushinteger(L, flag.id);
  } else {
    lua_pushnil(L);
  }
  lua_pushstdstring(L, flag.type->flagAbbv);

  RunCallIn(LUA_CI_FlagRemoved, 2, 0);
}


void LuaHandle::FlagGrabbed(const Flag& flag, const Player& player)
{
  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_FlagGrabbed)) {
    return; // the call is not defined
  }

  if (L2H(L)->HasFullRead()) {
    lua_pushinteger(L, flag.id);
  } else {
    lua_pushnil(L);
  }
  lua_pushstdstring(L, flag.type->flagAbbv);
  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_FlagGrabbed, 3, 0);
}


void LuaHandle::FlagDropped(const Flag& flag, const Player& player)
{
  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_FlagDropped)) {
    return; // the call is not defined
  }

  if (L2H(L)->HasFullRead()) {
    lua_pushinteger(L, flag.id);
  } else {
    lua_pushnil(L);
  }
  lua_pushstdstring(L, flag.type->flagAbbv);
  lua_pushinteger(L, player.getId());

  RunCallIn(LUA_CI_FlagDropped, 3, 0);
}


void LuaHandle::FlagCaptured(const Flag& flag, const Player* player)
{
  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_FlagCaptured)) {
    return; // the call is not defined
  }

  if (L2H(L)->HasFullRead()) {
    lua_pushinteger(L, flag.id);
  } else {
    lua_pushnil(L);
  }

  lua_pushstdstring(L, flag.type->flagAbbv);

  if (player == NULL) {
    lua_pushnil(L);
  } else {
    lua_pushinteger(L, player->getId());
  }

  RunCallIn(LUA_CI_FlagCaptured, 3, 0);
}


void LuaHandle::FlagTransferred(const Flag& flag,
				const Player& src, const Player& dst)
{
  LUA_CALL_IN_CHECK(L, 4)
  if (!PushCallIn(LUA_CI_FlagTransferred)) {
    return; // the call is not defined
  }

  if (L2H(L)->HasFullRead()) {
    lua_pushinteger(L, flag.id);
  } else {
    lua_pushnil(L);
  }
  lua_pushstdstring(L, flag.type->flagAbbv);
  lua_pushinteger(L, src.getId());
  lua_pushinteger(L, dst.getId());

  RunCallIn(LUA_CI_FlagTransferred, 4, 0);
}


//============================================================================//
//============================================================================//

void LuaHandle::GLResize()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_GLResize)) {
    return; // the call is not defined
  }

  RunCallIn(LUA_CI_GLResize, 0, 0);
}


//
// NOTE:  'GLContextInit' vs. 'GLReload'  name change
//
void LuaHandle::GLContextInit()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_GLReload)) {
    return; // the call is not defined
  }

  RunCallIn(LUA_CI_GLReload, 0, 0);
}


void LuaHandle::GLUnmapped()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_GLUnmapped)) {
    return; // the call is not defined
  }

  RunCallIn(LUA_CI_GLUnmapped, 0, 0);
}


void LuaHandle::GLContextFree()
{
  return; // do nothing, lua scripts should not need it
}


//============================================================================//
//============================================================================//

#define DRAW_CALLIN(name) \
  void LuaHandle:: name () { \
    LUA_CALL_IN_CHECK(L, 0) \
    lua_checkstack(L, 2); \
    if (!PushCallIn(LUA_CI_ ## name )) { \
      return; \
    } \
    LuaGLPointers::Enable(L); \
    RunCallIn(LUA_CI_ ## name, 0, 0); \
    LuaGLPointers::Reset(L); \
  }

DRAW_CALLIN(DrawGenesis)
DRAW_CALLIN(DrawWorldStart)
DRAW_CALLIN(DrawWorld)
DRAW_CALLIN(DrawWorldAlpha)
DRAW_CALLIN(DrawWorldShadow)
DRAW_CALLIN(DrawScreenStart)
DRAW_CALLIN(DrawScreen)
DRAW_CALLIN(DrawRadar)


//============================================================================//
//============================================================================//

void LuaHandle::GotGfxBlock(int type, int id)
{
  LUA_CALL_IN_CHECK(L, 2)

  // NOTE: we don't use PushCallIn() here because this is not managed by
  //       EventHandler, so no warning should be given if it is not found
  lua_rawgeti(L, LUA_CALLINSINDEX, LUA_CI_GotGfxBlock);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return;
  }

  lua_pushinteger(L, type);
  lua_pushinteger(L, id);

  RunCallIn(LUA_CI_GotGfxBlock, 2, 0);
}


void LuaHandle::LostGfxBlock(int type, int id)
{
  LUA_CALL_IN_CHECK(L, 2)

  // NOTE: we don't use PushCallIn() here because this is not managed by
  //       EventHandler, so no warning should be given if it is not found
  lua_rawgeti(L, LUA_CALLINSINDEX, LUA_CI_LostGfxBlock);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1);
    return;
  }

  lua_pushinteger(L, type);
  lua_pushinteger(L, id);

  RunCallIn(LUA_CI_LostGfxBlock, 2, 0);
}


//============================================================================//
//============================================================================//

bool LuaHandle::KeyPress(bool taken, int key, int mods)
{
  BzfDisplay* dpy = getDisplay();
  if (dpy == NULL) {
    return false;
  }

  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_KeyPress)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);
  lua_pushinteger(L, key);
  lua_pushinteger(L, mods);

  if (!RunCallIn(LUA_CI_KeyPress, 3, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::KeyRelease(bool taken, int key, int mods)
{
  BzfDisplay* dpy = getDisplay();
  if (dpy == NULL) {
    return false;
  }

  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_KeyRelease)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);
  lua_pushinteger(L, key);
  lua_pushinteger(L, mods);

  if (!RunCallIn(LUA_CI_KeyRelease, 3, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::UnicodeText(bool taken, uint32_t unicode)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_UnicodeText)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);

  lua_pushinteger(L, unicode);

  if (!RunCallIn(LUA_CI_UnicodeText, 2, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::MousePress(bool taken, int x, int y, int button)
{
  LUA_CALL_IN_CHECK(L, 4)
  if (!PushCallIn(LUA_CI_MousePress)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, button);

  if (!RunCallIn(LUA_CI_MousePress, 4, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::MouseRelease(bool taken, int x, int y, int button)
{
  LUA_CALL_IN_CHECK(L, 4)
  if (!PushCallIn(LUA_CI_MouseRelease)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, button);

  if (!RunCallIn(LUA_CI_MouseRelease, 4, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::MouseMove(bool taken, int x, int y)
{
  LUA_CALL_IN_CHECK(L, 3)
  if (!PushCallIn(LUA_CI_MouseMove)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);

  if (!RunCallIn(LUA_CI_MouseMove, 3, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::MouseWheel(bool taken, float value)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_MouseWheel)) {
    return false; // the call is not defined, do not take the event
  }

  lua_pushboolean(L, taken);
  lua_pushfloat(L, value);

  if (!RunCallIn(LUA_CI_MouseWheel, 2, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::IsAbove(int x, int y)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_IsAbove)) {
    return false; // the call is not defined
  }

  lua_pushinteger(L, x);
  lua_pushinteger(L, y);

  if (!RunCallIn(LUA_CI_IsAbove, 2, 1)) {
    return false;
  }

  return PopBool(L, false);
}


string LuaHandle::GetTooltip(int x, int y)
{
  LUA_CALL_IN_CHECK(L, 2)
  if (!PushCallIn(LUA_CI_GetTooltip)) {
    return ""; // the call is not defined
  }

  lua_pushinteger(L, x);
  lua_pushinteger(L, y);

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
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_WordComplete)) {
    return;
  }

  lua_pushstring(L, line.c_str());

  if (!RunCallIn(LUA_CI_WordComplete, 1, 1)) {
    return;
  }

  const int table = lua_gettop(L);
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


//============================================================================//
//============================================================================//

bool LuaHandle::ForbidSpawn()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_ForbidSpawn)) {
    return false;
  }

  if (!RunCallIn(LUA_CI_ForbidSpawn, 0, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::ForbidJump()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_ForbidJump)) {
    return false;
  }

  if (!RunCallIn(LUA_CI_ForbidJump, 0, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::ForbidFlagDrop()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_ForbidFlagDrop)) {
    return false;
  }

  if (!RunCallIn(LUA_CI_ForbidFlagDrop, 0, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::ForbidShot()
{
  LUA_CALL_IN_CHECK(L, 0)
  if (!PushCallIn(LUA_CI_ForbidShot)) {
    return false;
  }

  if (!RunCallIn(LUA_CI_ForbidShot, 0, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::ForbidShotLock(const Player& player)
{
  LUA_CALL_IN_CHECK(L, 1)
  if (!PushCallIn(LUA_CI_ForbidShotLock)) {
    return false;
  }

  lua_pushinteger(L, player.getId());

  if (!RunCallIn(LUA_CI_ForbidShotLock, 1, 1)) {
    return false;
  }

  return PopBool(L, false);
}


bool LuaHandle::ForbidShotHit(const Player& player,
                              const ShotPath& shot, const fvec3& pos)
{
  LUA_CALL_IN_CHECK(L, 5)
  if (!PushCallIn(LUA_CI_ForbidShotHit)) {
    return false;
  }

  const FiringInfo& info = shot.getFiringInfo();
  const uint8_t playerID = (uint8_t)info.shot.player;
  const uint16_t infoID = info.shot.id;
  const uint32_t shotID = (playerID << 16) | infoID;

  lua_pushinteger(L, player.getId());
  lua_pushinteger(L, shotID);
  lua_pushfloat(L, pos.x);
  lua_pushfloat(L, pos.y);
  lua_pushfloat(L, pos.z);

  if (!RunCallIn(LUA_CI_ForbidShotHit, 5, 1)) {
    return false;
  }

  return PopBool(L, false);
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
