/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// FIXME -- rework all of the constants

#include "common.h"

// interface header
#include "CallOuts.h"

// system headers
#include <assert.h>
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;

// common headers
#include "bzfio.h"
#include "TextUtils.h"
#include "PlayerState.h"
#include "LinkManager.h"
#include "MeshFace.h"
#include "bzfsAPI.h"
#include "vectors.h"
#include "version.h"

// bzfs headers
#include "../GameKeeper.h"
#include "../CmdLineOptions.h"
#include "../bzfsMessages.h"
#include "../commands.h"

// local headers
#include "LuaHeader.h"


extern const string& GetLuaDir(); // from lua.cpp

// FIXME: TODO
// - plugin management  (not part of the plan...)
// - logging
// - polls
// - help


//============================================================================//
//============================================================================//

static inline GameKeeper::Player* getPlayerByIndex(int playerID)
{
  return GameKeeper::Player::getPlayerByIndex(playerID);
}


//============================================================================//
//============================================================================//


// FIXME -- move into "utils.cpp"
static bz_eTeamType ParseTeam(lua_State* L, int index)
{
  if (lua_israwstring(L, index)) {
    static map<string, bz_eTeamType> nameMap;
    if (nameMap.empty()) {
      nameMap["auto"]     = eAutomaticTeam;
      nameMap["none"]     = eNoTeam;
      nameMap["rogue"]    = eRogueTeam;
      nameMap["red"]      = eRedTeam;
      nameMap["green"]    = eGreenTeam;
      nameMap["blue"]     = eBlueTeam;
      nameMap["purple"]   = ePurpleTeam;
      nameMap["rabbit"]   = eRabbitTeam;
      nameMap["hunter"]   = eHunterTeam;
      nameMap["observer"] = eObservers;
      nameMap["admin"]    = eAdministrators;
    }
    string s = lua_tostring(L, index);
    s = TextUtils::tolower(s);
    map<string, bz_eTeamType>::const_iterator it = nameMap.find(s);
    if (it != nameMap.end()) {
      return it->second;
    }
    luaL_error(L, "invalid team: %s", s.c_str());
  }
  else if (lua_israwnumber(L, index)) {
    return (bz_eTeamType)lua_toint(L, index);
  }
  else {
    luaL_error(L, "invalid team argument");
  }
  return eRogueTeam;
}



//============================================================================//
//============================================================================//

static int GetLuaDirectory(lua_State* L);

static int GetProtocolVersion(lua_State* L);
static int GetServerVersion(lua_State* L);
static int GetServerPort(lua_State* L);
static int GetServerAddress(lua_State* L);
static int GetServerDescription(lua_State* L);
static int GetServerOwner(lua_State* L);

static int UpdateListServer(lua_State* L);

static int AdminShutdown(lua_State* L);
static int AdminRestart(lua_State* L);
static int AdminSuperKill(lua_State* L);
static int AdminGameOver(lua_State* L);

static int GetGameType(lua_State* L);
static int GetJumpingAllowed(lua_State* L);

static int GetWallHeight(lua_State* L);
static int SetWallHeight(lua_State* L);
static int GetWorldSize(lua_State* L);
static int SetWorldSize(lua_State* L);
static int GetWorldURL(lua_State* L);
static int SetWorldURL(lua_State* L);
static int GetWorldCache(lua_State* L);

static int DebugMessage(lua_State* L);
static int GetDebugLevel(lua_State* L);
static int SetDebugLevel(lua_State* L);

static int SendMessage(lua_State* L);
static int SendTeamMessage(lua_State* L);
static int SendFetchResource(lua_State* L);
static int SendJoinServer(lua_State* L);
static int PlaySound(lua_State* L);

static int GetStandardSpawn(lua_State* L);
static int GetBaseAtPosition(lua_State* L);

static int GetPlayerCount(lua_State* L);
static int GetPlayerIDs(lua_State* L);
static int GetPlayerName(lua_State* L);
static int GetPlayerTeam(lua_State* L);
static int GetPlayerIPAddress(lua_State* L);
static int GetPlayerReferrer(lua_State* L);
static int GetPlayerFlagID(lua_State* L);
static int GetPlayerClientVersion(lua_State* L);
static int GetPlayerBZID(lua_State* L);
static int GetPlayerPaused(lua_State* L);
static int GetPlayerPosition(lua_State* L);
static int GetPlayerVelocity(lua_State* L);
static int GetPlayerRotation(lua_State* L);
static int GetPlayerAngVel(lua_State* L);
static int GetPlayerStatus(lua_State* L);
static int GetPlayerFalling(lua_State* L);
static int GetPlayerCrossingWall(lua_State* L);
static int GetPlayerZoned(lua_State* L);
static int GetPlayerPhysicsDriver(lua_State* L);

static int GetPlayerSpawned(lua_State* L);
static int GetPlayerCanSpawn(lua_State* L);
static int GetPlayerAdmin(lua_State* L);
static int GetPlayerOperator(lua_State* L);
static int GetPlayerGroups(lua_State* L);
static int GetPlayerRank(lua_State* L);
static int GetPlayerVerified(lua_State* L);
static int GetPlayerGlobalUser(lua_State* L);
static int GetPlayerFlagHistory(lua_State* L);

static int GetPlayerLag(lua_State* L);
static int GetPlayerJitter(lua_State* L);
static int GetPlayerPacketLoss(lua_State* L);

static int GetPlayerWins(lua_State* L);
static int GetPlayerLosses(lua_State* L);
static int GetPlayerTKs(lua_State* L);

static int SetPlayerWins(lua_State* L);
static int SetPlayerLosses(lua_State* L);
static int SetPlayerTKs(lua_State* L);

static int GetPlayerCustomData(lua_State* L);
static int SetPlayerCustomData(lua_State* L);

static int GetPlayerAutoPilot(lua_State* L);
static int SetPlayerAutoPilot(lua_State* L);

static int ChangePlayerTeam(lua_State* L);

static int ZapPlayer(lua_State* L);
static int KillPlayer(lua_State* L);

static int SetRabbit(lua_State* L);

static int SetPlayerShotType(lua_State* L);
static int SetPlayerOperator(lua_State* L);
static int SetPlayerSpawnable(lua_State* L);
static int SetPlayerLimboMessage(lua_State* L);

static int GivePlayerFlag(lua_State* L);

static int GetFlagCount(lua_State* L);
static int GetFlagName(lua_State* L);
static int GetFlagPosition(lua_State* L);
static int GetFlagPlayer(lua_State* L);

static int ZapFlag(lua_State* L);
static int MoveFlag(lua_State* L);
static int ResetFlag(lua_State* L);
static int ResetFlags(lua_State* L);

static int GetTeamName(lua_State* L);
static int GetTeamLimit(lua_State* L);
static int GetTeamCount(lua_State* L);
static int GetTeamScore(lua_State* L);
static int GetTeamWins(lua_State* L);
static int GetTeamLosses(lua_State* L);
static int SetTeamWins(lua_State* L);
static int SetTeamLosses(lua_State* L);

static int FireWeapon(lua_State* L);
static int FireMissile(lua_State* L);

static int SaveRecBuf(lua_State* L);
static int StartRecBuf(lua_State* L);
static int StopRecBuf(lua_State* L);

static int GetCountdownActive(lua_State* L);
static int GetCountdownInProgress(lua_State* L);
static int StartCountdown(lua_State* L);
static int PauseCountdown(lua_State* L);
static int ResumeCountdown(lua_State* L);

static int ReloadLocalBans(lua_State* L);
static int ReloadMasterBans(lua_State* L);
static int ReloadUsers(lua_State* L);
static int ReloadGroups(lua_State* L);
static int ReloadHelp(lua_State* L);

static int GetGroups(lua_State* L);
static int GetGroupPerms(lua_State* L);
static int GetGroupHasPerm(lua_State* L);
static int GetStandardPerms(lua_State* L);

static int GetReportCount(lua_State* L);
static int GetReportInfo(lua_State* L);
static int GetReports(lua_State* L);

static int FileReport(lua_State* L);
static int ClearReport(lua_State* L);
static int ClearReports(lua_State* L);

static int GetLagWarn(lua_State* L);
static int SetLagWarn(lua_State* L);

static int ManualTimeLimit(lua_State* L);
static int GetTimeLimit(lua_State* L);
static int SetTimeLimit(lua_State* L);

static int KickPlayer(lua_State* L);
static int BanByIP(lua_State* L);
static int BanByBZID(lua_State* L);
static int BanByHost(lua_State* L);
static int UnbanByIP(lua_State* L);
static int UnbanByBZID(lua_State* L);
static int UnbanByHost(lua_State* L);

static int GetIPBanCount(lua_State* L);
static int GetIPBanEntry(lua_State* L);
static int GetBZIDBanCount(lua_State* L);
static int GetBZIDBanEntry(lua_State* L);
static int GetHostBanCount(lua_State* L);
static int GetHostBanEntry(lua_State* L);

static int GetMaxWaitTime(lua_State* L);
static int SetMaxWaitTime(lua_State* L);
static int ClearMaxWaitTime(lua_State* L);

static int GetTime(lua_State* L);

static int DirList(lua_State* L);
static int CalcMD5(lua_State* L);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
static int ReadStdin(lua_State* L);
#endif


//============================================================================//
//============================================================================//

bool CallOuts::PushEntries(lua_State* L)
{
  assert(sizeof(void*) >= sizeof(uint32_t));

  PUSH_LUA_CFUNC(L, GetLuaDirectory);

  PUSH_LUA_CFUNC(L, GetProtocolVersion);
  PUSH_LUA_CFUNC(L, GetServerVersion);
  PUSH_LUA_CFUNC(L, GetServerPort);
  PUSH_LUA_CFUNC(L, GetServerAddress);
  PUSH_LUA_CFUNC(L, GetServerDescription);
  PUSH_LUA_CFUNC(L, GetServerOwner);

  PUSH_LUA_CFUNC(L, UpdateListServer);

  PUSH_LUA_CFUNC(L, AdminShutdown);
  PUSH_LUA_CFUNC(L, AdminRestart);
  PUSH_LUA_CFUNC(L, AdminSuperKill);
  PUSH_LUA_CFUNC(L, AdminGameOver);

  PUSH_LUA_CFUNC(L, GetGameType);
  PUSH_LUA_CFUNC(L, GetJumpingAllowed);

  PUSH_LUA_CFUNC(L, GetWallHeight);
  PUSH_LUA_CFUNC(L, GetWorldSize);
  PUSH_LUA_CFUNC(L, GetWorldURL);
  PUSH_LUA_CFUNC(L, GetWorldCache);
  PUSH_LUA_CFUNC(L, SetWallHeight);
  PUSH_LUA_CFUNC(L, SetWorldSize);
  PUSH_LUA_CFUNC(L, SetWorldURL);

  PUSH_LUA_CFUNC(L, DebugMessage);
  PUSH_LUA_CFUNC(L, GetDebugLevel);
  PUSH_LUA_CFUNC(L, SetDebugLevel);

  PUSH_LUA_CFUNC(L, SendMessage);
  PUSH_LUA_CFUNC(L, SendTeamMessage);
  PUSH_LUA_CFUNC(L, SendFetchResource);
  PUSH_LUA_CFUNC(L, SendJoinServer);
  PUSH_LUA_CFUNC(L, PlaySound);

  PUSH_LUA_CFUNC(L, GetStandardSpawn);
  PUSH_LUA_CFUNC(L, GetBaseAtPosition);

  // Player
  PUSH_LUA_CFUNC(L, GetPlayerCount);
  PUSH_LUA_CFUNC(L, GetPlayerIDs);
  PUSH_LUA_CFUNC(L, GetPlayerName);
  PUSH_LUA_CFUNC(L, GetPlayerTeam);
  PUSH_LUA_CFUNC(L, GetPlayerIPAddress);
  PUSH_LUA_CFUNC(L, GetPlayerReferrer);
  PUSH_LUA_CFUNC(L, GetPlayerFlagID);
  PUSH_LUA_CFUNC(L, GetPlayerClientVersion);
  PUSH_LUA_CFUNC(L, GetPlayerBZID);
  PUSH_LUA_CFUNC(L, GetPlayerStatus);
  PUSH_LUA_CFUNC(L, GetPlayerPaused);
  PUSH_LUA_CFUNC(L, GetPlayerPosition);
  PUSH_LUA_CFUNC(L, GetPlayerVelocity);
  PUSH_LUA_CFUNC(L, GetPlayerRotation);
  PUSH_LUA_CFUNC(L, GetPlayerAngVel);
  PUSH_LUA_CFUNC(L, GetPlayerFalling);
  PUSH_LUA_CFUNC(L, GetPlayerCrossingWall);
  PUSH_LUA_CFUNC(L, GetPlayerZoned);
  PUSH_LUA_CFUNC(L, GetPlayerPhysicsDriver);

  PUSH_LUA_CFUNC(L, GetPlayerSpawned);
  PUSH_LUA_CFUNC(L, GetPlayerCanSpawn);
  PUSH_LUA_CFUNC(L, GetPlayerAdmin);
  PUSH_LUA_CFUNC(L, GetPlayerOperator);
  PUSH_LUA_CFUNC(L, GetPlayerGroups);
  PUSH_LUA_CFUNC(L, GetPlayerVerified);
  PUSH_LUA_CFUNC(L, GetPlayerGlobalUser);
  PUSH_LUA_CFUNC(L, GetPlayerFlagHistory);
  PUSH_LUA_CFUNC(L, GetPlayerRank);

  PUSH_LUA_CFUNC(L, GetPlayerLag);
  PUSH_LUA_CFUNC(L, GetPlayerJitter);
  PUSH_LUA_CFUNC(L, GetPlayerPacketLoss);

  PUSH_LUA_CFUNC(L, GetPlayerWins);
  PUSH_LUA_CFUNC(L, GetPlayerLosses);
  PUSH_LUA_CFUNC(L, GetPlayerTKs);

  PUSH_LUA_CFUNC(L, SetPlayerWins);
  PUSH_LUA_CFUNC(L, SetPlayerLosses);
  PUSH_LUA_CFUNC(L, SetPlayerTKs);

  PUSH_LUA_CFUNC(L, GetPlayerCustomData);
  PUSH_LUA_CFUNC(L, SetPlayerCustomData);

  PUSH_LUA_CFUNC(L, GetPlayerAutoPilot);
  PUSH_LUA_CFUNC(L, SetPlayerAutoPilot);

  PUSH_LUA_CFUNC(L, ChangePlayerTeam);

  PUSH_LUA_CFUNC(L, ZapPlayer);
  PUSH_LUA_CFUNC(L, KillPlayer);

  PUSH_LUA_CFUNC(L, SetRabbit);

  PUSH_LUA_CFUNC(L, SetPlayerShotType);
  PUSH_LUA_CFUNC(L, SetPlayerOperator);
  PUSH_LUA_CFUNC(L, SetPlayerSpawnable);
  PUSH_LUA_CFUNC(L, SetPlayerLimboMessage);

  PUSH_LUA_CFUNC(L, GivePlayerFlag);

  // Flag
  PUSH_LUA_CFUNC(L, GetFlagCount);
  PUSH_LUA_CFUNC(L, GetFlagName);
  PUSH_LUA_CFUNC(L, GetFlagPosition);
  PUSH_LUA_CFUNC(L, GetFlagPlayer);

  PUSH_LUA_CFUNC(L, ZapFlag);
  PUSH_LUA_CFUNC(L, MoveFlag);
  PUSH_LUA_CFUNC(L, ResetFlag);
  PUSH_LUA_CFUNC(L, ResetFlags);

  // Team
  PUSH_LUA_CFUNC(L, GetTeamName);
  PUSH_LUA_CFUNC(L, GetTeamLimit);
  PUSH_LUA_CFUNC(L, GetTeamCount);
  PUSH_LUA_CFUNC(L, GetTeamScore);
  PUSH_LUA_CFUNC(L, GetTeamWins);
  PUSH_LUA_CFUNC(L, GetTeamLosses);

  PUSH_LUA_CFUNC(L, SetTeamWins);
  PUSH_LUA_CFUNC(L, SetTeamLosses);

  PUSH_LUA_CFUNC(L, FireWeapon);
  PUSH_LUA_CFUNC(L, FireMissile);

  PUSH_LUA_CFUNC(L, SaveRecBuf);
  PUSH_LUA_CFUNC(L, StartRecBuf);
  PUSH_LUA_CFUNC(L, StopRecBuf);

  PUSH_LUA_CFUNC(L, GetCountdownActive);
  PUSH_LUA_CFUNC(L, GetCountdownInProgress);
  PUSH_LUA_CFUNC(L, StartCountdown);
  PUSH_LUA_CFUNC(L, PauseCountdown);
  PUSH_LUA_CFUNC(L, ResumeCountdown);

  PUSH_LUA_CFUNC(L, ReloadLocalBans);
  PUSH_LUA_CFUNC(L, ReloadMasterBans);
  PUSH_LUA_CFUNC(L, ReloadUsers);
  PUSH_LUA_CFUNC(L, ReloadGroups);
  PUSH_LUA_CFUNC(L, ReloadHelp);

  PUSH_LUA_CFUNC(L, GetGroups);
  PUSH_LUA_CFUNC(L, GetGroupPerms);
  PUSH_LUA_CFUNC(L, GetGroupHasPerm);
  PUSH_LUA_CFUNC(L, GetStandardPerms);

  PUSH_LUA_CFUNC(L, GetReportCount);
  PUSH_LUA_CFUNC(L, GetReportInfo);
  PUSH_LUA_CFUNC(L, GetReports);

  PUSH_LUA_CFUNC(L, FileReport);
  PUSH_LUA_CFUNC(L, ClearReport);
  PUSH_LUA_CFUNC(L, ClearReports);

  PUSH_LUA_CFUNC(L, GetLagWarn);
  PUSH_LUA_CFUNC(L, SetLagWarn);

  PUSH_LUA_CFUNC(L, ManualTimeLimit);
  PUSH_LUA_CFUNC(L, GetTimeLimit);
  PUSH_LUA_CFUNC(L, SetTimeLimit);

  PUSH_LUA_CFUNC(L, KickPlayer);
  PUSH_LUA_CFUNC(L, BanByIP);
  PUSH_LUA_CFUNC(L, BanByBZID);
  PUSH_LUA_CFUNC(L, BanByHost);
  PUSH_LUA_CFUNC(L, UnbanByIP);
  PUSH_LUA_CFUNC(L, UnbanByBZID);
  PUSH_LUA_CFUNC(L, UnbanByHost);

  PUSH_LUA_CFUNC(L, GetIPBanCount);
  PUSH_LUA_CFUNC(L, GetIPBanEntry);
  PUSH_LUA_CFUNC(L, GetBZIDBanCount);
  PUSH_LUA_CFUNC(L, GetBZIDBanEntry);
  PUSH_LUA_CFUNC(L, GetHostBanCount);
  PUSH_LUA_CFUNC(L, GetHostBanEntry);

  PUSH_LUA_CFUNC(L, GetMaxWaitTime);
  PUSH_LUA_CFUNC(L, SetMaxWaitTime);
  PUSH_LUA_CFUNC(L, ClearMaxWaitTime);

  PUSH_LUA_CFUNC(L, GetTime);

  PUSH_LUA_CFUNC(L, DirList);
  PUSH_LUA_CFUNC(L, CalcMD5);

#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  PUSH_LUA_CFUNC(L, ReadStdin);
#endif

  return true;
}


//============================================================================//
//============================================================================//

static int GetLuaDirectory(lua_State* L)
{
  lua_pushstdstring(L, GetLuaDir());
  return 1;
}


static int GetProtocolVersion(lua_State* L)
{
  lua_pushstring(L, getProtocolVersion());
  return 1;
}


static int GetServerVersion(lua_State* L)
{
  lua_pushstring(L, getAppVersion());
  return 1;
}


static int GetServerPort(lua_State* L)
{
  if (!clOptions->useGivenPort) {
    lua_pushinteger(L, ServerPort);
  } else {
    lua_pushinteger(L, clOptions->wksPort);
  }
  return 1;
}


static int GetServerAddress(lua_State* L)
{
  if (!clOptions->publicizeServer) {
    return 0;
  }
  lua_pushstdstring(L, clOptions->publicizedAddress);
  return 1;
}


static int GetServerDescription(lua_State* L)
{
  if (!clOptions->publicizeServer) {
    return 0;
  }
  lua_pushstdstring(L, clOptions->publicizedTitle);
  return 1;
}


static int GetServerOwner(lua_State* L)
{
  if (!clOptions->publicizeServer) {
    return 0;
  }
  lua_pushstdstring(L, clOptions->publicizedUsername);
  return 1;
}


//============================================================================//
//============================================================================//

static int UpdateListServer(lua_State* /*L*/)
{
  publicize();
  return 0;
}


static int AdminShutdown(lua_State* /*L*/)
{
  shutdownCommand(NULL, NULL);
  return 0;
}


static int AdminRestart(lua_State* L)
{
  lua_pushboolean(L, bz_restart());
  return 1;
}


static int AdminSuperKill(lua_State* /*L*/)
{
  superkillCommand(NULL, NULL);
  return 0;
}


static int AdminGameOver(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  bz_eTeamType teamID = eNoTeam;
  if (!lua_isnone(L, 2)) {
    teamID = ParseTeam(L, 2);
  }
  bz_gameOver(playerID, teamID);
  return 0;
}


//============================================================================//
//============================================================================//

static int GetGameType(lua_State* L)
{
  lua_pushinteger(L, bz_getGameType());
  return 1;
}


static int GetJumpingAllowed(lua_State* L)
{
  lua_pushboolean(L, bz_allowJumping());
  return 1;
}


//============================================================================//
//============================================================================//

static int GetWallHeight(lua_State* L)
{
  float size, height;
  bz_getWorldSize(&size, &height);
  lua_pushfloat(L, height);
  return 1;
}


static int GetWorldSize(lua_State* L)
{
  float size, height;
  bz_getWorldSize(&size, &height);
  lua_pushfloat(L, size);
  return 1;
}


static int GetWorldCache(lua_State* L)
{
  unsigned int   size = bz_getWorldCacheSize();
  unsigned char* data = new unsigned char[size];
  bz_getWorldCacheData(data);
  lua_pushlstring(L, (char*)data, size);
  delete[] data;
  return 1;
}


static int GetWorldURL(lua_State* L)
{
  const bz_ApiString url =  bz_getClientWorldDownloadURL();
  lua_pushstring(L, url.c_str());
  return 1;
}


static int SetWallHeight(lua_State* L)
{
  const float value = luaL_checkfloat(L, 1);
  float size, height;
  bz_getWorldSize(&size, &height);
  height = value;
  lua_pushboolean(L, bz_setWorldSize(size, height));
  return 1;
}


static int SetWorldSize(lua_State* L)
{
  const float value = luaL_checkfloat(L, 1);
  float size, height;
  bz_getWorldSize(&size, &height);
  size = value;
  lua_pushboolean(L, bz_setWorldSize(size, height));
  return 1;
}


static int SetWorldURL(lua_State* L)
{
  clOptions->cacheURL = luaL_checkstring(L, 1);
  return 0;
}


//============================================================================//
//============================================================================//

static int DebugMessage(lua_State* L)
{
  const int level = luaL_checkint(L, 1);
  const char* msg = luaL_checkstring(L, 2);
  bz_debugMessage(level, msg);
  return 1;
}


static int GetDebugLevel(lua_State* L)
{
  lua_pushinteger(L, bz_getDebugLevel());
  return 1;
}


static int SetDebugLevel(lua_State* L)
{
  debugLevel = luaL_checkint(L, 1);
  return 0;
}


//============================================================================//
//============================================================================//

static int SendMessage(lua_State* L)
{
  const int   src = luaL_checkint(L, 1);
  const int   dst = luaL_checkint(L, 2);
  const char* msg = luaL_checkstring(L, 3);
  lua_pushboolean(L, bz_sendTextMessage(src, dst, msg));
  return 1;
}


static int SendTeamMessage(lua_State* L)
{
  const int          src = luaL_checkint(L, 1);
  const bz_eTeamType dst = ParseTeam(L, 2);
  const char*        msg = luaL_checkstring(L, 3);
  lua_pushboolean(L, bz_sendTextMessage(src, dst, msg));
  return 1;
}


static int SendFetchResource(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const char* url    = luaL_checkstring(L, 2);
  lua_pushboolean(L, bz_sendFetchResMessage(playerID, url));
  return 1;
}


static int SendJoinServer(lua_State* L)
{
  const int   playerID = luaL_checkint(L, 1);
  const char* addr     = luaL_checkstring(L, 2);
  const int   port     = luaL_checkint(L, 3);
  const int   teamID   = luaL_optint(L, 4, eNoTeam);
  const char* referrer = luaL_optstring(L, 5, bz_getPublicAddr().c_str());
  const char* message  = luaL_optstring(L, 6, NULL);
  lua_pushboolean(L, bz_sendJoinServer(playerID, addr, port,
                                       teamID, referrer, message));
  return 1;
}


static int PlaySound(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const char* sound  = luaL_checkstring(L, 2);
  fvec3 pos;
  const float* posPtr = NULL;
  if (lua_gettop(L) >= 5) {
    posPtr = pos;
    pos.x = luaL_checkfloat(L, 3);
    pos.y = luaL_checkfloat(L, 4);
    pos.z = luaL_checkfloat(L, 5);
  }
  lua_pushboolean(L, bz_sendPlayCustomLocalSound(playerID, sound, posPtr));
  return 1;
}


//============================================================================//
//============================================================================//

static int GetStandardSpawn(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  fvec3 pos;
  float rot;
  if (!bz_getStandardSpawn(playerID, pos, &rot)) {
    return 0;
  }
  lua_pushfloat(L, pos.x);
  lua_pushfloat(L, pos.y);
  lua_pushfloat(L, pos.z);
  lua_pushfloat(L, rot);
  return 4;
}


static int GetBaseAtPosition(lua_State* L)
{
  fvec3 pos;
  pos.x = luaL_checkfloat(L, 1);
  pos.y = luaL_checkfloat(L, 2);
  pos.z = luaL_checkfloat(L, 3);
  const bz_eTeamType teamID = bz_checkBaseAtPoint(pos);
  lua_pushinteger(L, teamID);
  return 1;
}


//============================================================================//
//============================================================================//

static int GetPlayerCount(lua_State* L)
{
  lua_pushinteger(L, bz_getPlayerCount());
  return 1;
}


static int GetPlayerIDs(lua_State* L)
{
  bz_APIIntList* playerList = bz_newIntList();
  if (!bz_getPlayerIndexList(playerList)) {
    lua_createtable(L, 0, 0);
    bz_deleteIntList(playerList);
    return 1;
  }
  lua_createtable(L, 0, playerList->size());
  for (unsigned int i = 0; i < playerList->size(); i++) {
    lua_pushinteger(L, playerList->get(i));
    lua_rawseti(L, -2, i + 1);
  }
  bz_deleteIntList(playerList);
  return 1;
}


static int GetPlayerName(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  const char* name = bz_getPlayerCallsign(pid);
  if (name == NULL) {
    return 0;
  }
  lua_pushstring(L, name);
  return 1;
}


static int GetPlayerTeam(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_getPlayerTeam(pid));
  return 1;
}


static int GetPlayerIPAddress(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  const char* addr = bz_getPlayerIPAddress(pid);
  if (addr == NULL) {
    return 0;
  }
  lua_pushstring(L, addr);
  return 1;
}


static int GetPlayerReferrer(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  const char* referrer = bz_getPlayerReferrer(pid);
  if (referrer == NULL) {
    return 0;
  }
  lua_pushstring(L, referrer);
  return 1;
}


static int GetPlayerFlagID(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const int flagID = player->player.getFlag();
  const FlagInfo* flagInfo = FlagInfo::get(flagID);

  lua_pushinteger(L, flagID);
  if (flagInfo && flagInfo->flag.type) {
    lua_pushstdstring(L, flagInfo->flag.type->label());
  } else {
    lua_pushliteral(L, "");
  }
  return 2;
}


static int GetPlayerClientVersion(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  lua_pushstring(L, player->player.getClientVersion());
  return 1;
}


static int GetPlayerBZID(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  lua_pushstdstring(L, player->getBzIdentifier());
  return 1;
}


//============================================================================//
//============================================================================//

static int GetPlayerStatus(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  short status;
  if (useLastState) {
    status = player->lastState.status;
  } else {
    status = player->getCurrentStateAsState().status;
  }

  lua_pushinteger(L, status);
  return 1;
}


static int GetPlayerPaused(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushboolean(L, bz_isPlayerPaused(pid));
  return 1;
}


static int GetPlayerPosition(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  fvec3 pos;
  if (useLastState) {
    pos = player->lastState.pos;
  } else {
    float rotation;
    player->getPlayerCurrentPosRot(pos, rotation);
  }

  lua_pushfloat(L, pos.x);
  lua_pushfloat(L, pos.y);
  lua_pushfloat(L, pos.z);
  return 3;
}


static int GetPlayerVelocity(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  PlayerState state;
  const PlayerState* statePtr = &state;
  if (useLastState) {
    statePtr = &player->lastState;
  } else {
    state = player->getCurrentStateAsState();
  }

  const fvec3& vel = statePtr->velocity;
  lua_pushfloat(L, vel.x);
  lua_pushfloat(L, vel.y);
  lua_pushfloat(L, vel.z);
  return 3;
}


static int GetPlayerRotation(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  float rotation = 0.0f;
  if (useLastState) {
    rotation = player->lastState.azimuth;
  } else {
    fvec3 pos;
    player->getPlayerCurrentPosRot(pos, rotation);
  }

  lua_pushfloat(L, rotation);
  return 1;
}


static int GetPlayerAngVel(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  PlayerState state;
  const PlayerState* statePtr = &state;
  if (useLastState) {
    statePtr = &player->lastState;
  } else {
    state = player->getCurrentStateAsState();
  }

  lua_pushfloat(L, statePtr->angVel);
  return 1;
}


static int GetPlayerFalling(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  short status;
  if (useLastState) {
    status = player->lastState.status;
  } else {
    status = player->getCurrentStateAsState().status;
  }

  lua_pushboolean(L, (status & PlayerState::Falling) != 0);
  return 1;
}


static int GetPlayerCrossingWall(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  short status;
  if (useLastState) {
    status = player->lastState.status;
  } else {
    status = player->getCurrentStateAsState().status;
  }

  lua_pushboolean(L, (status & PlayerState::CrossingWall) != 0);
  return 1;
}


static int GetPlayerZoned(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  short status;
  if (useLastState) {
    status = player->lastState.status;
  } else {
    status = player->getCurrentStateAsState().status;
  }

  lua_pushboolean(L, (status & PlayerState::PhantomZoned) != 0);
  return 1;
}


static int GetPlayerPhysicsDriver(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }

  const bool useLastState = lua_tobool(L, 2);
  PlayerState state;
  const PlayerState* statePtr = &state;
  if (useLastState) {
    statePtr = &player->lastState;
  } else {
    state = player->getCurrentStateAsState();
  }

  if (statePtr->phydrv < 0) {
    lua_pushboolean(L, false);
  } else {
    lua_pushinteger(L, statePtr->phydrv);
  }
  return 1;
}


static int GetPlayerSpawned(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->player.isAlive());
  return 1;
}


static int GetPlayerCanSpawn(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushboolean(L, bz_canPlayerSpawn(pid));
  return 1;
}


static int GetPlayerAdmin(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushboolean(L, bz_getAdmin(pid));
  return 1;
}


static int GetPlayerOperator(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->accessInfo.isOperator());
  return 1;
}


static int GetPlayerGroups(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_newtable(L);
  const vector<string>& groups = player->accessInfo.groups;
  for (unsigned int i = 0; i < groups.size(); i++) {
    lua_pushstdstring(L, groups[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


static int GetPlayerRank(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushfloat(L, player->score.ranking());
  return 1;
}


static int GetPlayerVerified(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->accessInfo.isVerified());
  return 1;
}


static int GetPlayerGlobalUser(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->authentication.isGlobal());
  return 1;
}


static int GetPlayerFlagHistory(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  const vector<FlagType*>& flagHistory = player->flagHistory.get();
  lua_newtable(L);
  for (size_t i = 0; i < flagHistory.size(); i++) {
    lua_pushstdstring(L, flagHistory[i]->flagAbbv);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


//============================================================================//
//============================================================================//

static int GetPlayerLag(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_getPlayerLag(playerID));
  return 1;
}


static int GetPlayerJitter(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_getPlayerJitter(playerID));
  return 1;
}


static int GetPlayerPacketLoss(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  lua_pushfloat(L, bz_getPlayerPacketLoss(playerID));
  return 1;
}


//============================================================================//
//============================================================================//

static int GetPlayerWins(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_getPlayerWins(pid));
  return 1;
}


static int GetPlayerLosses(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_getPlayerLosses(pid));
  return 1;
}


static int GetPlayerTKs(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_getPlayerTKs(pid));
  return 1;
}


static int SetPlayerWins(lua_State* L)
{
  const int pid   = luaL_checkint(L, 1);
  const int value = luaL_checkint(L, 2);
  bz_setPlayerWins(pid, value);
  return 0;
}


static int SetPlayerLosses(lua_State* L)
{
  const int pid   = luaL_checkint(L, 1);
  const int value = luaL_checkint(L, 2);
  bz_setPlayerLosses(pid, value);
  return 0;
}


static int SetPlayerTKs(lua_State* L)
{
  const int pid   = luaL_checkint(L, 1);
  const int value = luaL_checkint(L, 2);
  bz_setPlayerTKs(pid, value);
  return 0;
}


//============================================================================//
//============================================================================//

static int GetPlayerCustomData(lua_State* L)
{
  const int   pid  =    luaL_checkint(L, 1);
  const char* key  = luaL_checkstring(L, 2);
  const char* data = bz_getPlayerCustomData(pid, key);
  if (data == NULL) {
    return 0;
  }
  lua_pushstring(L, data);
  return 1;
}


static int SetPlayerCustomData(lua_State* L)
{
  const int   pid  =    luaL_checkint(L, 1);
  const char* key  = luaL_checkstring(L, 2);
  const char* data = luaL_checkstring(L, 3);
  lua_pushboolean(L, bz_setPlayerCustomData(pid, key, data));
  return 1;
}


//============================================================================//
//============================================================================//

static int GetPlayerAutoPilot(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->player.isAutoPilot());
  return 1;
}


static int SetPlayerAutoPilot(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  GameKeeper::Player* player = getPlayerByIndex(pid);
  player->setAutoPilot(lua_tobool(L, 2));
  return 0;
}


//============================================================================//
//============================================================================//

static int ChangePlayerTeam(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const bz_eTeamType newTeamID = ParseTeam(L, 2);
  bz_changeTeam(playerID, newTeamID);
  return 0;
}


static int ZapPlayer(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  bz_zapPlayer(playerID);
  return 0;
}


static int KillPlayer(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const bool spawnOnBase = !lua_isboolean(L, 2) || lua_tobool(L, 2);
  const int killerID = luaL_optint(L, 3, -1);
  const char* flagID = luaL_optstring(L, 4, NULL);
  bz_killPlayer(playerID, spawnOnBase, killerID, flagID);
  return 0;
}


static int SetRabbit(lua_State* L) // FIXME ?
{
  const int playerID = luaL_checkint(L, 1);
  if (lua_isnil(L, 2)) {
    bz_removeRabbit(playerID);
    return 0;
  }
  const bool swap = lua_isboolean(L, 2) && lua_tobool(L, 2);
  bz_newRabbit(playerID, swap);
  return 0;
}

//============================================================================//
//============================================================================//

static int SetPlayerShotType(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const int shotType = luaL_checkint(L, 2);
  lua_pushboolean(L, bz_setPlayerShotType(playerID, (bz_eShotType)shotType));
  return 1;
}


static int SetPlayerOperator(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  lua_pushboolean(L, bz_setPlayerOperator(playerID));
  return 1;
}


static int SetPlayerSpawnable(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const bool spawn = !lua_isboolean(L, 2) || lua_tobool(L, 2);
  lua_pushboolean(L, bz_setPlayerSpawnable(playerID, spawn));
  return 1;
}


static int SetPlayerLimboMessage(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const char* text   = luaL_checkstring(L, 2);
  lua_pushboolean(L, bz_setPlayerLimboMessage(playerID, text));
  return 1;
}


//============================================================================//
//============================================================================//

static int GivePlayerFlag(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);

  const char* flagType = NULL;
  if (!lua_isnil(L, 2)) {
    flagType = luaL_checkstring(L, 2);
  }

  if (flagType == NULL) {
    lua_pushboolean(L, bz_removePlayerFlag(playerID));
    return 1;
  }

  const bool force = lua_isboolean(L, 3) && lua_tobool(L, 3);
  lua_pushboolean(L, bz_givePlayerFlag(playerID, flagType, force));

  return 1;
}


//============================================================================//
//============================================================================//

static int GetFlagCount(lua_State* L)
{
  lua_pushinteger(L, bz_getNumFlags());
  return 1;
}


static int GetFlagName(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  lua_pushstring(L, bz_getFlagName(flagID).c_str());
  return 1;
}


static int GetFlagPosition(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  fvec3 pos;
  if (!bz_getFlagPosition(flagID, pos)) {
    return 0;
  }
  lua_pushfloat(L, pos.x);
  lua_pushfloat(L, pos.y);
  lua_pushfloat(L, pos.z);
  return 3;
}


static int GetFlagPlayer(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  lua_pushinteger(L, bz_flagPlayer(flagID));
  return 1;
}


static int ZapFlag(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  FlagInfo* flag = FlagInfo::get(flagID);
  if (flag == NULL) {
    return 0;
  }
  zapFlag(*flag);
  lua_pushboolean(L, true);
  return 1;
}


static int MoveFlag(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  fvec3 pos;
  pos.x = luaL_checkfloat(L, 2);
  pos.y = luaL_checkfloat(L, 3);
  pos.z = luaL_checkfloat(L, 4);
  const bool reset = !lua_isboolean(L, 5) || lua_tobool(L, 5);
  bz_moveFlag(flagID, pos, reset);
  return 0;
}


static int ResetFlag(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  bz_resetFlag(flagID);
  return 0;
}


static int ResetFlags(lua_State* L)
{
  const bool onlyUnused = !lua_isboolean(L, 1) || lua_tobool(L, 1);
  bz_resetFlags(onlyUnused);
  return 0;
}


//============================================================================//
//============================================================================//

static int GetTeamName(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushliteral(L, "FIXME"); teamID = teamID; // FIXME bzu_GetTeamName(teamID));
  return 1;
}


static int GetTeamLimit(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushinteger(L, bz_getTeamPlayerLimit(teamID));
  return 1;
}


static int GetTeamCount(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushinteger(L, bz_getTeamCount(teamID));
  return 1;
}


static int GetTeamScore(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushinteger(L, bz_getTeamScore(teamID));
  return 1;
}


static int GetTeamWins(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushinteger(L, bz_getTeamWins(teamID));
  return 1;
}


static int GetTeamLosses(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushinteger(L, bz_getTeamLosses(teamID));
  return 1;
}


static int SetTeamWins(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  const int value = luaL_checkint(L, 2);
  bz_setTeamWins(teamID, value);
  return 0;
}


static int SetTeamLosses(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  const int value = luaL_checkint(L, 2);
  bz_setTeamLosses(teamID, value);
  return 0;
}


//============================================================================//
//============================================================================//

static int FireWeapon(lua_State* L)
{
  fvec3 pos;
  const char* flagType = luaL_checkstring(L, 1);
  pos.x                = luaL_checkfloat(L, 2);
  pos.y                = luaL_checkfloat(L, 3);
  pos.z                = luaL_checkfloat(L, 4);
  const float rot      = luaL_optfloat(L, 5,  0.0f);
  const float tilt     = luaL_optfloat(L, 6,  0.0f);
  const float lifeTime = luaL_optfloat(L, 7, -1.0f);
  const float dt       = luaL_optfloat(L, 8, -1.0f);
  const int   shotID   = luaL_optint(L, 9, -1);

  lua_pushboolean(L, bz_fireWorldWep(flagType, lifeTime,
                                     pos, tilt, rot, shotID , dt));
  return 1;
}


static int FireMissile(lua_State* L)
{
  fvec3 pos;
  int targetID   = luaL_checkint(L, 1);
  pos.x          = luaL_checkfloat(L, 2);
  pos.y          = luaL_checkfloat(L, 3);
  pos.z          = luaL_checkfloat(L, 4);
  float rot      = luaL_checkfloat(L, 5);
  float tilt     = luaL_optfloat(L, 6,  0.0f);
  float lifeTime = luaL_optfloat(L, 7, -1.0f);
  float dt       = luaL_optfloat(L, 8, -1.0f);
  
  if (lifeTime <= 0.0f) {
    lifeTime = BZDB.eval(BZDBNAMES.RELOADTIME);
  }
  if (dt < 0.0f) {
    dt = 0.0f;
  }

  lua_pushinteger(L, bz_fireWorldGM(targetID, lifeTime, pos, tilt, rot, dt));
  return 1;
}


//============================================================================//
//============================================================================//

static int SaveRecBuf(lua_State* L)
{
  const char* fileName = luaL_checkstring(L, 1);
  const int   seconds  = luaL_checkint(L, 2);
  lua_pushboolean(L, bz_saveRecBuf(fileName, seconds));
  return 1;
}


static int StartRecBuf(lua_State* L)
{
  lua_pushboolean(L, bz_startRecBuf());
  return 1;
}


static int StopRecBuf(lua_State* L)
{
  lua_pushboolean(L, bz_stopRecBuf());
  return 1;
}


//============================================================================//
//============================================================================//

static int GetCountdownActive(lua_State* L)
{
  lua_pushboolean(L, bz_isCountDownActive());
  return 1;
}


static int GetCountdownInProgress(lua_State* L)
{
  lua_pushboolean(L, bz_isCountDownInProgress());
  return 1;
}


static int StartCountdown(lua_State* L)
{
  const char* playerName = luaL_checkstring(L, 1);
  const int   delay = luaL_checkint(L, 2);
  const float limit = luaL_checkfloat(L, 3);
  bz_startCountdown(delay, limit, playerName);
  return 0;
}


static int PauseCountdown(lua_State* L)
{
  const char* playerName = luaL_checkstring(L, 1);
  bz_pauseCountdown(playerName);
  return 0;
}


static int ResumeCountdown(lua_State* L)
{
  const char* playerName = luaL_checkstring(L, 1);
  bz_resumeCountdown(playerName);
  return 0;
}


//============================================================================//
//============================================================================//

static int ReloadLocalBans(lua_State* /*L*/)
{
  bz_reloadLocalBans();
  return 0;
}


static int ReloadMasterBans(lua_State* /*L*/)
{
  bz_reloadMasterBans();
  return 0;
}


static int ReloadUsers(lua_State* /*L*/)
{
  bz_reloadUsers();
  return 0;
}


static int ReloadGroups(lua_State* /*L*/)
{
  bz_reloadGroups();
  return 0;
}


static int ReloadHelp(lua_State* /*L*/)
{
  bz_reloadHelp();
  return 0;
}


//============================================================================//
//============================================================================//

static int GetGroups(lua_State* L)
{
  bz_APIStringList* list = bz_getGroupList();
  if (list == NULL) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  lua_createtable(L, list->size(), 0);
  for (unsigned int i = 0; i < list->size(); i++) {
    lua_pushstring(L, (*list)[i].c_str());
    lua_rawseti(L, -2, i + 1);
  }
  bz_deleteStringList(list);
  return 1;
}


static int GetGroupPerms(lua_State* L)
{
  const char* group = luaL_checkstring(L, 1);
  bz_APIStringList* list = bz_getGroupPerms(group);
  if (list == NULL) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  lua_createtable(L, list->size(), 0);
  for (unsigned int i = 0; i < list->size(); i++) {
    lua_pushstring(L, (*list)[i].c_str());
    lua_rawseti(L, -2, i + 1);
  }
  bz_deleteStringList(list);
  return 1;
}


static int GetGroupHasPerm(lua_State* L)
{
  const char* group = luaL_checkstring(L, 1);
  const char* perm  = luaL_checkstring(L, 2);
  lua_pushboolean(L, bz_groupAllowPerm(group, perm));
  return 1;
}


static int GetStandardPerms(lua_State* L)
{
  bz_APIStringList* list = bz_getStandardPermList();
  if (list == NULL) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  lua_createtable(L, list->size(), 0);
  for (unsigned int i = 0; i < list->size(); i++) {
    lua_pushstring(L, (*list)[i].c_str());
    lua_rawseti(L, -2, i + 1);
  }
  bz_deleteStringList(list);
  return 1;
}


//============================================================================//
//============================================================================//

static int GetReportCount(lua_State* L)
{
  lua_pushinteger(L, bz_getReportCount());
  return 1;
}


static int GetReportInfo(lua_State* L)
{
  const unsigned int reportID = (unsigned int)luaL_checkint(L, 1);
  lua_pushstring(L, bz_getReportSource(reportID));
  lua_pushstring(L, bz_getReportBody(reportID));
  lua_pushstring(L, bz_getReportTime(reportID));
  return 3;
}


static int GetReports(lua_State* L)
{
  bz_APIStringList* list = bz_getReports();
  if (list == NULL) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  lua_createtable(L, list->size(), 0);
  for (unsigned int i = 0; i < list->size(); i++) {
    lua_pushstring(L, list->get(i).c_str());
    lua_rawseti(L, -2, i + 1);
  }
  bz_deleteStringList(list);
  return 1;
}


static int FileReport(lua_State* L)
{
  const char* msg  = luaL_checkstring(L, 1);
  const char* from = luaL_checkstring(L, 2);
  lua_pushboolean(L, bz_fileReport(msg, from));
  return 1;
}


static int ClearReport(lua_State* L)
{
  const unsigned int reportID = luaL_checkint(L, 1);
  lua_pushboolean(L, bz_clearReport(reportID));
  return 1;
}


static int ClearReports(lua_State* L)
{
  lua_pushboolean(L, bz_clearAllReports());
  return 1;
}


//============================================================================//
//============================================================================//

static int GetLagWarn(lua_State* L)
{
  lua_pushinteger(L, bz_getLagWarn());
  return 1;
}


static int SetLagWarn(lua_State* L)
{
  const int lagWarn = luaL_checkint(L, 1);
  lua_pushboolean(L, bz_setLagWarn(lagWarn));
  return 1;
}


//============================================================================//
//============================================================================//

static int ManualTimeLimit(lua_State* L)
{
  lua_pushboolean(L, bz_isTimeManualStart());
  return 1;
}


static int GetTimeLimit(lua_State* L)
{
  lua_pushfloat(L, bz_getTimeLimit());
  return 1;
}


static int SetTimeLimit(lua_State* L)
{
  const float timeLimit = luaL_checkfloat(L, 1);
  lua_pushboolean(L, bz_setTimeLimit(timeLimit));
  return 1;
}


//============================================================================//
//============================================================================//

static int KickPlayer(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const char* reason = luaL_checkstring(L, 2);
  const bool  notify = lua_isboolean(L, 3) && lua_tobool(L, 3);
  lua_pushboolean(L, bz_kickUser(playerID, reason, notify));
  return 1;
}


static int BanByIP(lua_State* L)
{
  const char* ip     = luaL_checkstring(L, 1);
  const char* source = luaL_checkstring(L, 2);
  const int duration = luaL_checkint(L, 3);
  const char* reason = luaL_checkstring(L, 4);
  lua_pushboolean(L, bz_IPBanUser(ip, source, duration, reason));
  return 1;
}


static int BanByBZID(lua_State* L)
{
  const char* bzID   = luaL_checkstring(L, 1);
  const char* source = luaL_checkstring(L, 2);
  const int duration = luaL_checkint(L, 3);
  const char* reason = luaL_checkstring(L, 4);
  lua_pushboolean(L, bz_IPBanUser(bzID, source, duration, reason));
  return 1;
}


static int BanByHost(lua_State* L)
{
  const char* host   = luaL_checkstring(L, 1);
  const char* source = luaL_checkstring(L, 2);
  const int duration = luaL_checkint(L, 3);
  const char* reason = luaL_checkstring(L, 4);
  lua_pushboolean(L, bz_IPBanUser(host, source, duration, reason));
  return 1;
}


static int UnbanByIP(lua_State* L)
{
  const char* ip = luaL_checkstring(L, 1);
  lua_pushboolean(L, bz_IPUnbanUser(ip));
  return 1;
}


static int UnbanByBZID(lua_State* L)
{
  const char* bzID = luaL_checkstring(L, 1);
  lua_pushboolean(L, bz_IDUnbanUser(bzID));
  return 1;
}


static int UnbanByHost(lua_State* L)
{
  const char* host = luaL_checkstring(L, 1);
  lua_pushboolean(L, bz_HostUnbanUser(host));
  return 1;
}

//============================================================================//
//============================================================================//

static int GetBanCount(lua_State* L, bz_eBanListType listType)
{
  lua_pushinteger(L, bz_getBanListSize(listType));
  return 1;
}


static int GetBanEntry(lua_State* L, bz_eBanListType listType)
{
  const unsigned int entry = luaL_checkint(L, 1);
  if (entry >= bz_getBanListSize(listType)) {
    return 0;
  }

  lua_newtable(L);

  lua_pushliteral(L, "data");
  lua_pushstring(L, bz_getBanItem(listType, entry));
  lua_rawset(L, -3);
  lua_pushliteral(L, "reason");
  lua_pushstring(L, bz_getBanItemReason(listType, entry));
  lua_rawset(L, -3);
  lua_pushliteral(L, "source");
  lua_pushstring(L, bz_getBanItemSource(listType, entry));
  lua_rawset(L, -3);
  lua_pushliteral(L, "duration");
  lua_pushdouble(L, bz_getBanItemDuration(listType, entry));
  lua_rawset(L, -3);
  lua_pushliteral(L, "fromMaster");
  lua_pushboolean(L, bz_getBanItemIsFromMaster(listType, entry));
  lua_rawset(L, -3);

  return 1;
}

static int GetIPBanCount(lua_State* L)   { return GetBanCount(L, eIPList);   }
static int GetIPBanEntry(lua_State* L)   { return GetBanEntry(L, eIPList);   }
static int GetBZIDBanCount(lua_State* L) { return GetBanCount(L, eIDList);   }
static int GetBZIDBanEntry(lua_State* L) { return GetBanEntry(L, eIDList);   }
static int GetHostBanCount(lua_State* L) { return GetBanCount(L, eHostList); }
static int GetHostBanEntry(lua_State* L) { return GetBanEntry(L, eHostList); }


//============================================================================//
//============================================================================//

static int GetMaxWaitTime(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  lua_pushfloat(L, bz_getMaxWaitTime(name));
  return 1;
}


static int SetMaxWaitTime(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  const float maxTime = luaL_checkfloat(L, 2);
  bz_setMaxWaitTime(maxTime, name);
  return 0;
}


static int ClearMaxWaitTime(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  bz_clearMaxWaitTime(name);
  return 0;
}


//============================================================================//
//============================================================================//

static int GetTime(lua_State* L)
{
  lua_pushdouble(L, bz_getCurrentTime());
  return 1;
}


//============================================================================//
//============================================================================//


static std::string backSlashToFrontSlash(const std::string& path)
{
  std::string p = path;
  std::replace(p.begin(), p.end(), '\\', '/');
  return p;
}


static std::string cleanDirPath(const std::string& path)
{
  if (path.empty()) {
    return path;
  }
  std::string p = backSlashToFrontSlash(path);
  if (p[p.size() - 1] != '/') {
    p += '/';
  }
  return p;
}



static bool rawDirList(const std::string& root, const std::string& path,
                       bool recursive,
                       std::vector<std::string>& dirs,
                       std::vector<std::string>& files)
{
#ifndef WIN32

  const std::string fullPath = root + path;
  DIR* dir = opendir(fullPath.c_str());
  if (dir == NULL) {
    return false;
  }
  for (dirent* de = readdir(dir); de != NULL; de = readdir(dir)) {
    const std::string name = de->d_name;
    if (name.empty() || (name == ".") || (name == "..")) {
      continue;
    }
    struct stat stbuf;
    if (stat((fullPath + name).c_str(), &stbuf) == 0) {
      const std::string filePath = path + name;
      if (!S_ISDIR(stbuf.st_mode)) {
        files.push_back(filePath);
      }
      else {
        const std::string dirPath = filePath + "/";
        dirs.push_back(dirPath);
        if (recursive) {
          rawDirList(root, dirPath, recursive, dirs, files);
        }
      }
    }
  }

  closedir(dir);

  std::sort(dirs.begin(),  dirs.end());
  std::sort(files.begin(), files.end());

  return true;

#else // WIN32

  const std::string fullPath = root + path;
  struct _finddata_t fileInfo;
  long handle = _findfirst(std::string(fullPath + "*").c_str(), &fileInfo);
  if (handle == -1L) {
    return false;
  }
  do {
    const std::string& name = fileInfo.name;
    if (name.empty() || (name == ".") || (name == "..")) {
      continue;
    }
    const std::string filePath = path + name;
    if ((fileInfo.attrib & _A_SUBDIR) == 0) {
      files.push_back(filePath);
    }
    else {
      const std::string dirPath = filePath + "/";
      dirs.push_back(dirPath);
      if (recursive) {
        rawDirList(root, dirPath, recursive, dirs, files);
      }
    }
  }
  while (_findnext(handle, &fileInfo) == 0);

  _findclose(handle);

  std::sort(dirs.begin(),  dirs.end());
  std::sort(files.begin(), files.end());

  return true;

#endif
}


//============================================================================//

static int DirList(lua_State* L)
{
  const char* path = luaL_checkstring(L, 1);
  const bool recursize = lua_tobool(L, 2);

  vector<string> dirs;
  vector<string> files;

  if (path[0] == 0) {
    path = "./";
  }

  const string cleanPath = cleanDirPath(path);
  if (!rawDirList("", cleanPath, recursize, dirs, files)) {
    return 0;
  }


  set<string> dirSet;
  for (unsigned int i = 0; i < dirs.size(); i++) {
    dirSet.insert(dirs[i]);
  }

  set<string> fileSet;
  for (unsigned int i = 0; i < files.size(); i++) {
    // do not include directories
    if (dirSet.find(files[i]) == dirSet.end()) {
      fileSet.insert(files[i]);
    }
  }

  // files table
  lua_createtable(L, (int)fileSet.size(), 0);
  set<string>::const_iterator fit;
  int fileCount = 0;
  for (fit = fileSet.begin(); fit != fileSet.end(); ++fit) {
    fileCount++;
    lua_pushstdstring(L, *fit);
    lua_rawseti(L, -2, fileCount);
  }

  // dirs table
  lua_createtable(L, (int)dirSet.size(), 0);
  set<string>::const_iterator dit;
  int dirCount = 0;
  for (dit = dirSet.begin(); dit != dirSet.end(); ++dit) {
    dirCount++;
    lua_pushstdstring(L, *dit);
    lua_rawseti(L, -2, dirCount);
  }

  return 2;
}


static int CalcMD5(lua_State* L)
{
  size_t size;
  const char* data = luaL_checklstring(L, 1, &size);
  lua_pushstring(L, bz_MD5(data, size));
  return 1;
}


// whacky bit of dev'ing fun
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  #include <unistd.h>
  #include <fcntl.h>
  static int ReadStdin(lua_State* L)
  {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    char buf[4096];
    const int r = read(STDIN_FILENO, buf, sizeof(buf));
    if (r <= 0) {
      return 0;
    }
    lua_pushlstring(L, buf, r);
    fcntl(STDIN_FILENO, F_SETFL, 0);
    return 1;
  }
#endif


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
