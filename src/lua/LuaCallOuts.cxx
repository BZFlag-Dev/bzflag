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
#include "LuaCallOuts.h"

// system headers
#include <sstream> // for ReadImage
#include <string.h>
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;

// common headers
#include "AnsiCodes.h"
#include "Bundle.h"
#include "BundleMgr.h"
#include "BzVFS.h"
#include "BzfDisplay.h"
#include "BzfWindow.h"
#include "CacheManager.h"
#include "CollisionManager.h"
#include "CommandManager.h"
#include "GameTime.h"
#include "GfxBlock.h"
#include "KeyManager.h"
#include "MapInfo.h"
#include "MediaFile.h"
#include "OpenGLLight.h"
#include "SceneRenderer.h"
#include "Team.h"
#include "TextUtils.h"
#include "BzTime.h"
#include "bzfio.h"
#include "bz_md5.h"
#include "version.h"

// bzflag headers
#include "bzflag/guiplaying.h"
#include "bzflag/sound.h"
#include "bzflag/ControlPanel.h"
#include "bzflag/HUDDialogStack.h"
#include "bzflag/LocalCommand.h"
#include "bzflag/MainWindow.h"
#include "bzflag/RadarRenderer.h"
#include "bzflag/Roaming.h"
#include "bzflag/ScoreboardRenderer.h"
#include "clientbase/playing.h"
#include "clientbase/ClientFlag.h"
#include "clientbase/LocalPlayer.h"
#include "clientbase/Roster.h"
#include "clientbase/ServerLink.h"
#include "clientbase/World.h"
#include "clientbase/WorldPlayer.h"

// mediafile headers
#include "mediafile/PNGImageFile.h"

// local headers
#include "LuaDouble.h"
#include "LuaHeader.h"
#include "LuaHandle.h"

// LuaHandle headers
#include "LuaUser.h"


//============================================================================//
//============================================================================//

bool LuaCallOuts::PushEntries(lua_State* L)
{
  const bool fullRead  = L2H(L)->HasFullRead();
  const bool gameCtrl  = L2H(L)->HasGameCtrl();
  const bool inputCtrl = L2H(L)->HasInputCtrl();
  const bool isBzOrg   = L2H(L)->GetName() == "LuaBzOrg";

  PUSH_LUA_CFUNC(L, GetBzLuaVersion);
  PUSH_LUA_CFUNC(L, GetClientVersion);
  PUSH_LUA_CFUNC(L, GetProtocolVersion);

  if (isBzOrg) {
    PUSH_LUA_CFUNC(L, JoinGame);
    PUSH_LUA_CFUNC(L, LeaveGame);
    PUSH_LUA_CFUNC(L, OpenMenu);
    PUSH_LUA_CFUNC(L, CloseMenu);
  }

  PUSH_LUA_CFUNC(L, Print);
  PUSH_LUA_CFUNC(L, Debug);
  PUSH_LUA_CFUNC(L, GetDebugLevel);

  PUSH_LUA_CFUNC(L, CalcMD5);
  PUSH_LUA_CFUNC(L, StripAnsiCodes);
  PUSH_LUA_CFUNC(L, ExpandColorString);
  PUSH_LUA_CFUNC(L, LocalizeString);
  PUSH_LUA_CFUNC(L, GetCacheFilePath);

  PUSH_LUA_CFUNC(L, InGame);
  PUSH_LUA_CFUNC(L, GetGameInfo);
  PUSH_LUA_CFUNC(L, GetWorldInfo);

  PUSH_LUA_CFUNC(L, GetServerAddress);
  PUSH_LUA_CFUNC(L, GetServerIP);
  PUSH_LUA_CFUNC(L, GetServerPort);
  PUSH_LUA_CFUNC(L, GetServerCallsign);
  PUSH_LUA_CFUNC(L, GetServerDescription);

  PUSH_LUA_CFUNC(L, GetWind);
  if (fullRead) {
    PUSH_LUA_CFUNC(L, GetLights);
  }

  PUSH_LUA_CFUNC(L, GetWorldHash);

  PUSH_LUA_CFUNC(L, SendLuaData);

  PUSH_LUA_CFUNC(L, SendCommand);

  if (inputCtrl) {
    PUSH_LUA_CFUNC(L, BlockControls);
    PUSH_LUA_CFUNC(L, WarpMouse);
    PUSH_LUA_CFUNC(L, SetMouseBox);
  }

  PUSH_LUA_CFUNC(L, PlaySound);

  PUSH_LUA_CFUNC(L, ReadImageData);
  PUSH_LUA_CFUNC(L, ReadImageFile);

  PUSH_LUA_CFUNC(L, GetViewType);

  PUSH_LUA_CFUNC(L, GetKeyToCmds);
  PUSH_LUA_CFUNC(L, GetCmdToKeys);

  PUSH_LUA_CFUNC(L, GetRoamInfo);
  PUSH_LUA_CFUNC(L, SetRoamInfo);

  PUSH_LUA_CFUNC(L, GetDrawingMirror);

  PUSH_LUA_CFUNC(L, GetScreenGeometry);
  PUSH_LUA_CFUNC(L, GetWindowGeometry);
  PUSH_LUA_CFUNC(L, GetViewGeometry);
  PUSH_LUA_CFUNC(L, GetRadarGeometry);
  PUSH_LUA_CFUNC(L, GetRadarRange);

  PUSH_LUA_CFUNC(L, GetVisualExtents);
  PUSH_LUA_CFUNC(L, GetLengthPerPixel);

  PUSH_LUA_CFUNC(L, SetCameraView);
  PUSH_LUA_CFUNC(L, SetCameraProjection);
  PUSH_LUA_CFUNC(L, GetCameraPosition);
  PUSH_LUA_CFUNC(L, GetCameraDirection);
  PUSH_LUA_CFUNC(L, GetCameraUp);
  PUSH_LUA_CFUNC(L, GetCameraRight);
  PUSH_LUA_CFUNC(L, GetCameraMatrix);
  PUSH_LUA_CFUNC(L, GetFrustumPlane);

  PUSH_LUA_CFUNC(L, NotifyStyleChange);

  PUSH_LUA_CFUNC(L, GetSun);

  PUSH_LUA_CFUNC(L, GetTime);
  PUSH_LUA_CFUNC(L, GetGameTime);
  PUSH_LUA_CFUNC(L, GetTimer);
  PUSH_LUA_CFUNC(L, DiffTimers);

  PUSH_LUA_CFUNC(L, GetTeamList);
  PUSH_LUA_CFUNC(L, GetTeamPlayers);
  PUSH_LUA_CFUNC(L, GetTeamName);
  PUSH_LUA_CFUNC(L, GetTeamLongName);
  PUSH_LUA_CFUNC(L, GetTeamCount);
  PUSH_LUA_CFUNC(L, GetTeamScore);
  PUSH_LUA_CFUNC(L, GetTeamColor);
  PUSH_LUA_CFUNC(L, GetTeamRadarColor);
  PUSH_LUA_CFUNC(L, GetTeamsAreEnemies);
  PUSH_LUA_CFUNC(L, GetPlayersAreEnemies);

  PUSH_LUA_CFUNC(L, GetMousePosition);
  PUSH_LUA_CFUNC(L, GetMouseButtons);
  PUSH_LUA_CFUNC(L, GetJoyPosition);
  PUSH_LUA_CFUNC(L, GetKeyModifiers);

  PUSH_LUA_CFUNC(L, GetLocalPlayer);
  PUSH_LUA_CFUNC(L, GetLocalPlayerTarget);
  PUSH_LUA_CFUNC(L, GetLocalTeam);
  PUSH_LUA_CFUNC(L, GetRabbitPlayer);
  PUSH_LUA_CFUNC(L, GetAntidotePosition);

  PUSH_LUA_CFUNC(L, GetGfxBlock);
  PUSH_LUA_CFUNC(L, SetGfxBlock);
  PUSH_LUA_CFUNC(L, GetPlayerGfxBlock);
  PUSH_LUA_CFUNC(L, SetPlayerGfxBlock);
  PUSH_LUA_CFUNC(L, GetPlayerRadarGfxBlock);
  PUSH_LUA_CFUNC(L, SetPlayerRadarGfxBlock);
  PUSH_LUA_CFUNC(L, GetFlagGfxBlock);
  PUSH_LUA_CFUNC(L, SetFlagGfxBlock);
  PUSH_LUA_CFUNC(L, GetFlagRadarGfxBlock);
  PUSH_LUA_CFUNC(L, SetFlagRadarGfxBlock);
  PUSH_LUA_CFUNC(L, GetShotGfxBlock);
  PUSH_LUA_CFUNC(L, SetShotGfxBlock);
  PUSH_LUA_CFUNC(L, GetShotRadarGfxBlock);
  PUSH_LUA_CFUNC(L, SetShotRadarGfxBlock);

  PUSH_LUA_CFUNC(L, GetPlayerList);
  if (!gameCtrl) {
    PUSH_LUA_CFUNC(L, GetPlayerName);
    PUSH_LUA_CFUNC(L, GetPlayerMotto);
  }
  PUSH_LUA_CFUNC(L, GetPlayerType);
  PUSH_LUA_CFUNC(L, GetPlayerTeam);
  PUSH_LUA_CFUNC(L, GetPlayerFlagType);
  PUSH_LUA_CFUNC(L, GetPlayerScore);
  PUSH_LUA_CFUNC(L, GetPlayerAutoPilot);
  PUSH_LUA_CFUNC(L, GetPlayerLagInfo);
  if (fullRead) {
    if (!gameCtrl) {
      PUSH_LUA_CFUNC(L, GetPlayerCustomData);
    }
    PUSH_LUA_CFUNC(L, GetPlayerFlag);
    PUSH_LUA_CFUNC(L, GetPlayerShots);
    PUSH_LUA_CFUNC(L, GetPlayerState);
    PUSH_LUA_CFUNC(L, GetPlayerStateBits);
    PUSH_LUA_CFUNC(L, GetPlayerPosition);
    PUSH_LUA_CFUNC(L, GetPlayerRotation);
    PUSH_LUA_CFUNC(L, GetPlayerDirection);
    PUSH_LUA_CFUNC(L, GetPlayerVelocity);
    PUSH_LUA_CFUNC(L, GetPlayerAngVel);
    PUSH_LUA_CFUNC(L, GetPlayerDimensions);
    PUSH_LUA_CFUNC(L, GetPlayerPhysicsDriver);
    PUSH_LUA_CFUNC(L, GetPlayerDesiredSpeed);
    PUSH_LUA_CFUNC(L, GetPlayerDesiredAngVel);
    PUSH_LUA_CFUNC(L, GetPlayerExplodeTime);
    PUSH_LUA_CFUNC(L, GetPlayerOpacity);
  }
  if (!gameCtrl) {
    PUSH_LUA_CFUNC(L, IsPlayerAdmin);
    PUSH_LUA_CFUNC(L, IsPlayerVerified);
    PUSH_LUA_CFUNC(L, IsPlayerRegistered);
    PUSH_LUA_CFUNC(L, IsPlayerHunted);
  }
  PUSH_LUA_CFUNC(L, SetPlayerCustomData);

  if (fullRead) {
    PUSH_LUA_CFUNC(L, GetFlagList);
    PUSH_LUA_CFUNC(L, GetFlagName);
    PUSH_LUA_CFUNC(L, GetFlagType);
    PUSH_LUA_CFUNC(L, GetFlagShotType);
    PUSH_LUA_CFUNC(L, GetFlagQuality);
    PUSH_LUA_CFUNC(L, GetFlagEndurance);
    PUSH_LUA_CFUNC(L, GetFlagTeam);
    PUSH_LUA_CFUNC(L, GetFlagOwner);
    PUSH_LUA_CFUNC(L, GetFlagState);
    PUSH_LUA_CFUNC(L, GetFlagPosition);
  }

  if (fullRead) {
    PUSH_LUA_CFUNC(L, GetShotList);
    PUSH_LUA_CFUNC(L, GetShotType);
    PUSH_LUA_CFUNC(L, GetShotFlagType);
    PUSH_LUA_CFUNC(L, GetShotPlayer);
    PUSH_LUA_CFUNC(L, GetShotTeam);
    PUSH_LUA_CFUNC(L, GetShotPosition);
    PUSH_LUA_CFUNC(L, GetShotVelocity);
    PUSH_LUA_CFUNC(L, GetShotLeftTime);
    PUSH_LUA_CFUNC(L, GetShotLifeTime);
    PUSH_LUA_CFUNC(L, GetShotReloadTime);
  }
  if (gameCtrl) {
    PUSH_LUA_CFUNC(L, RemoveShot);
  }


#ifdef HAVE_UNISTD_H
  PUSH_LUA_CFUNC(L, ReadStdin);
#endif

  return true;
}


//============================================================================//
//============================================================================//
//
//  PlayerMap utility
//

typedef map<int, Player*> PlayerMap;


static void MakePlayerMap(PlayerMap& playerMap)
{
  playerMap.clear();

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    playerMap[myTank->getId()] = myTank;
  }

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    if (robots[i] != NULL) {
      playerMap[robots[i]->getId()] = robots[i];
    }
  }
#endif

  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i] != NULL) {
      playerMap[remotePlayers[i]->getId()] = remotePlayers[i];
    }
  }
}


//============================================================================//
//============================================================================//
//
//  Parsing utilities
//

static inline const Player* ParsePlayer(lua_State* L, int index)
{
  const int playerID = luaL_checkint(L, index);
  return lookupPlayer((PlayerId)playerID);
}


static inline const ClientFlag* ParseFlag(lua_State* L, int index)
{
  World* world = World::getWorld();
  if (world == NULL) {
    return NULL;
  }
  const int flagID = luaL_checkint(L, index);
  if ((flagID < 0) || (flagID >= world->getMaxFlags())) {
    return NULL;
  }
  return (const ClientFlag*)&(world->getFlag(flagID));
}


static inline const ShotPath* ParseShot(lua_State* L, int index)
{
  const uint32_t fullID   = luaL_checkint(L, index);
  const PlayerId playerID = (fullID >> 16);
  const uint16_t shotID   = fullID & 0xffff;

  const Player* player = lookupPlayer(playerID);
  if (player == NULL) {
    return NULL;
  }

  const ShotPath* shot = player->getShot(shotID);
  if ((shot == NULL) || shot->isExpired()) {
    return NULL;
  }

  return shot;
}


static const map<string, TeamColor>& GetTeamNameMap()
{
  static map<string, TeamColor> teamNames;
  if (teamNames.empty()) {
    teamNames["automatic"] = AutomaticTeam;
    teamNames["rogue"]     = RogueTeam;
    teamNames["red"]       = RedTeam;
    teamNames["green"]     = GreenTeam;
    teamNames["blue"]      = BlueTeam;
    teamNames["purple"]    = PurpleTeam;
    teamNames["observer"]  = ObserverTeam;
    teamNames["rabbit"]    = RabbitTeam;
    teamNames["hunter"]    = HunterTeam;
  }
  return teamNames;
}


static inline TeamColor ParseTeam(lua_State* L, int index)
{
  if (lua_israwstring(L, index)) {
    const string teamName = lua_tostring(L, index);
    const map<string, TeamColor>& teamNames = GetTeamNameMap();
    map<string, TeamColor>::const_iterator it = teamNames.find(teamName);
    if (it == teamNames.end()) {
      return NoTeam;
    }
    return it->second;
  }

  TeamColor teamNum = (TeamColor)luaL_checkint(L, index);
  if ((teamNum < AutomaticTeam) || (teamNum >= NumTeams)) {
    return NoTeam;
  }

  return teamNum;
}


//============================================================================//
//============================================================================//

static inline int PushShot(lua_State* L, const ShotPath* shot, int count)
{
  count++;
  lua_pushinteger(L, count);
  const uint32_t shotID = (shot->getPlayer() << 16) | shot->getShotId();
  lua_pushinteger(L, shotID);
  lua_rawset(L, -3);
  return count;
}


//============================================================================//
//============================================================================//
//
//  Versions
//

int LuaCallOuts::GetBzLuaVersion(lua_State* L)
{
  lua_pushliteral(L, "0.1");
  return 1;
}


int LuaCallOuts::GetClientVersion(lua_State* L)
{
  lua_pushstring(L, getAppVersion());
  return 1;
}


int LuaCallOuts::GetProtocolVersion(lua_State* L)
{
  lua_pushstring(L, getProtocolVersion());
  return 1;
}


//============================================================================//
//
//  Game control
//

int LuaCallOuts::JoinGame(lua_State* L)
{
  const int table = 1;
  luaL_checktype(L, table, LUA_TTABLE);

  std::string address;
  std::string callsign;
  std::string password;
  std::string motto;
  int port = ServerPort;
  int team = AutomaticTeam;

  for (lua_pushnil(L); lua_next(L, table); lua_pop(L, 1)) {
    if (!lua_israwstring(L, -2)) {
      luaL_error(L, "invalid key");
    }
    const std::string key = lua_tostring(L, -2);
    if (key == "address") {
      address = luaL_checkstring(L, -1);
    } else if (key == "callsign") {
      callsign = luaL_checkstring(L, -1);
    } else if (key == "password") {
      password = luaL_checkstring(L, -1);
    } else if (key == "motto") {
      motto = luaL_checkstring(L, -1);
    } else if (key == "port") {
      port = luaL_checkint(L, -1);
    } else if (key == "team") {
      team = luaL_checkint(L, -1);
    }
  }

  const TeamColor teamColor = (TeamColor) team;

  if (address.empty()) {
    lua_pushnil(L); lua_pushliteral(L, "empty address"); return 2;
  }
  if (callsign.empty()) {
    lua_pushnil(L); lua_pushliteral(L, "empty callsign"); return 2;
  }
  if ((teamColor != AutomaticTeam) &&
      ((teamColor < RogueTeam) || (teamColor > ObserverTeam))) {
    lua_pushnil(L); lua_pushliteral(L, "invalid team"); return 2;
  }

  if (LuaHandle::GetDevMode()) {
    address = "127.0.0.1";
    callsign = "devmode";
  }

  StartupInfo* info = getStartupInfo();

  strncpy(info->serverName, address.c_str(),  ServerNameLen - 1);
  strncpy(info->callsign,   callsign.c_str(), CallSignLen - 1);
  strncpy(info->password,   password.c_str(), PasswordLen - 1);
  info->motto = motto;
  info->serverPort = port;
  info->team = teamColor;
  info->referrer[0] = '\0';

  joinGame();

  lua_pushboolean(L, true);
  return 1;
}


int LuaCallOuts::LeaveGame(lua_State* /*L*/)
{
  leaveGame();
  return 1;
}


//============================================================================//
//
//  Printing
//

static std::string ArgsToString(lua_State* L, int start, int end)
{
  std::string msg;

  // copied from lua/src/lib/lbaselib.c
  lua_getglobal(L, "tostring");

  for (int i = start; i <= end; i++) {
    const char *s;
    lua_pushvalue(L, -1); // copy the tostring() function
    lua_pushvalue(L, i);  // copy the value
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1); // get the result
    if (s == NULL) {
      luaL_error(L, "`tostring' must return a string to `print'");
    }
    if (i > start) {
      msg += ", ";
    }
    msg += s;
    lua_pop(L, 1); // pop the result
  }

  lua_pop(L, 1); // pop the tostring() function

  return msg;
}


int LuaCallOuts::Print(lua_State* L)
{
  if (controlPanel == NULL) {
    return 0;
  }
  ControlPanel::MessageModes mode = ControlPanel::MessageMisc;
  int end = lua_gettop(L);
  int start = 1;
  if (lua_israwnumber(L, start)) {
    mode = (ControlPanel::MessageModes) lua_toint(L, start);
    start++;
  }
  controlPanel->addMessage(ArgsToString(L, start, end), mode);
  return 0;
}


int LuaCallOuts::Debug(lua_State* L)
{
  int level = 0;
  int end = lua_gettop(L);
  int start = 1;
  if (lua_israwnumber(L, start)) {
    level = lua_toint(L, start);
    start++;
  }
  logDebugMessage(level, ArgsToString(L, start, end));
  return 0;
}


int LuaCallOuts::GetDebugLevel(lua_State* L)
{
  lua_pushinteger(L, debugLevel);
  return 1;
}


int LuaCallOuts::LocalizeString(lua_State* L)
{
  const char* text = luaL_checkstring(L, 1);
  Bundle* bundle = BundleMgr::getCurrentBundle();
  if (bundle == NULL) {
    lua_settop(L, 1);
    lua_pushboolean(L, false);
    return 2;
  }
  lua_pushstdstring(L, bundle->getLocalString(text));
  lua_pushboolean(L, true);
  return 2;
}


//============================================================================//
//
//  Text manipulation
//

int LuaCallOuts::CalcMD5(lua_State* L)
{
  MD5 md5;
  for (int arg = 1; lua_israwstring(L, arg); arg++) {
    size_t len;
    const char* text = luaL_checklstring(L, arg, &len);
    md5.update((const unsigned char*)text, len);
  }
  md5.finalize();
  lua_pushstdstring(L, md5.hexdigest());
  return 1;
}


int LuaCallOuts::StripAnsiCodes(lua_State* L)
{
  size_t len;
  const char* text = luaL_checklstring(L, 1, &len);
  lua_pushstring(L, stripAnsiCodes(text));
  return 1;
}


int LuaCallOuts::ExpandColorString(lua_State* L)
{
  size_t len;
  const char* text = luaL_checklstring(L, 1, &len);
  lua_pushstdstring(L, TextUtils::unescape_colors(text));
  return 1;
}


int LuaCallOuts::GetCacheFilePath(lua_State* L)
{
  const char* text = luaL_checkstring(L, 1);
  lua_pushstdstring(L, CACHEMGR.getLocalName(text));
  return 1;
}


//============================================================================//
//
//  Game / server information
//

int LuaCallOuts::InGame(lua_State* L)
{
  lua_pushboolean(L, entered && serverLink && LocalPlayer::getMyTank());
  return 1;
}


int LuaCallOuts::GetGameInfo(lua_State* L)
{
  lua_newtable(L);

  World* world = World::getWorld();
  if (world == NULL) {
    return 1;
  }

  luaset_strint(L,  "gameOptions", world->getGameOptions());
  luaset_strint(L,  "type",	world->getGameType());
  luaset_strbool(L, "teams",       world->allowTeams());
  luaset_strbool(L, "teamKills",   world->allowTeamKills());
  luaset_strbool(L, "teamFlags",   world->allowTeamFlags());
  luaset_strbool(L, "superFlags",  world->allowSuperFlags());
  luaset_strbool(L, "ricochet",    world->allShotsRicochet());
  luaset_strbool(L, "rabbit",      world->allowRabbit());
  luaset_strbool(L, "handicap",    world->allowHandicap());
  luaset_strbool(L, "antidote",    world->allowAntidote());
  luaset_strint(L,  "maxFlags",    world->getMaxFlags());
  luaset_strint(L,  "maxShots",    world->getMaxShots());
  luaset_strint(L,  "maxPlayers",  world->getMaxPlayers());
  if (world->allowShakeWins()) {
    luaset_strint(L, "shakeWins", world->getFlagShakeWins());
  }
  if (world->allowShakeTimeout()) {
    luaset_strnum(L, "shakeTime", world->getFlagShakeTimeout());
  }

  return 1;
}


int LuaCallOuts::GetWorldInfo(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  const vector<string>* entries = NULL;

  if (!lua_israwstring(L, 1)) {
    entries = &world->getMapInfo().getVec();
  }
  else {
    const char* key = lua_tostring(L, 1);
    const MapInfo::InfoMap& infoMap = world->getMapInfo().getMap();
    MapInfo::InfoMap::const_iterator it = infoMap.find(key);
    if (it != infoMap.end()) {
      entries = &it->second;
    }
  }

  if (entries == NULL) {
    lua_pushboolean(L, false);
    return 1;
  }

  lua_createtable(L, entries->size(), 0);
  for (size_t i = 0; i < entries->size(); i++) {
    lua_pushstdstring(L, (*entries)[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


int LuaCallOuts::GetServerAddress(lua_State* L) // FIXME
{
  if (!serverLink) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, serverLink->getJoinServer());
  return 1;
}


int LuaCallOuts::GetServerPort(lua_State* L)
{
  if (!serverLink) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, serverLink->getJoinPort());
  return 1;
}


int LuaCallOuts::GetServerCallsign(lua_State* L)
{
  if (!serverLink) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, serverLink->getJoinCallsign());
  return 1;
}


int LuaCallOuts::GetServerIP(lua_State* L) // FIXME
{
  if (!serverLink) {
    return luaL_pushnil(L);
  }
  return 0;
}


int LuaCallOuts::GetServerDescription(lua_State* L) // FIXME
{
  if (!serverLink) {
    return luaL_pushnil(L);
  }
  return 0;
}


//============================================================================//
//
//  More world information
//

int LuaCallOuts::GetWind(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }

  fvec3 pos = luaL_optfvec3(L, 1, fvec3(0.0f, 0.0f, 0.0f));

  fvec3 wind;
  world->getWind(wind, pos);
  lua_pushfvec3(L, wind);

  return 3;
}


int LuaCallOuts::GetLights(lua_State* L)
{
  lua_newtable(L);

  const int numAllLights = RENDERER.getNumAllLights();
  for (int li = 0; li < numAllLights; li++) {
    const OpenGLLight& light = RENDERER.getLight(li);
    lua_newtable(L); {
      if (light.getOnlyReal())   { luaset_strbool(L, "onlyReal",   true); }
      if (light.getOnlyGround()) { luaset_strbool(L, "onlyGround", true); }

      luaset_strnum(L, "maxDist",    light.getMaxDist());
      luaset_strnum(L, "importance", light.getImportance());

      const fvec4& pos = light.getPosition();
      lua_pushliteral(L, "pos");
      lua_createtable(L, 4, 0); {
	lua_pushfloat(L, pos.x); lua_rawseti(L, -2, 1);
	lua_pushfloat(L, pos.y); lua_rawseti(L, -2, 2);
	lua_pushfloat(L, pos.z); lua_rawseti(L, -2, 3);
	lua_pushfloat(L, pos.w); lua_rawseti(L, -2, 4);
      }
      lua_rawset(L, -3);

      const fvec4& color = light.getColor();
      lua_pushliteral(L, "color");
      lua_createtable(L, 4, 0); {
	lua_pushfloat(L, color.r); lua_rawseti(L, -2, 1);
	lua_pushfloat(L, color.g); lua_rawseti(L, -2, 2);
	lua_pushfloat(L, color.b); lua_rawseti(L, -2, 3);
	lua_pushfloat(L, color.a); lua_rawseti(L, -2, 4);
      }
      lua_rawset(L, -3);

      const fvec3& atten = light.getAttenuation();
      lua_pushliteral(L, "atten");
      lua_createtable(L, 3, 0); {
	lua_pushfloat(L, atten[0]); lua_rawseti(L, -2, 1);
	lua_pushfloat(L, atten[1]); lua_rawseti(L, -2, 2);
	lua_pushfloat(L, atten[2]); lua_rawseti(L, -2, 3);
      }
      lua_rawset(L, -3);
    }
    lua_rawseti(L, -2, li + 1);
  }

  return 1;
}


int LuaCallOuts::GetWorldHash(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, world->getMapHash());
  return 1;
}


//============================================================================//

int LuaCallOuts::SendLuaData(lua_State* L)
{
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return luaL_pushnil(L);
  }
  if (serverLink == NULL) {
    return luaL_pushnil(L);
  }

  size_t len;
  const char* ptr = luaL_checklstring(L, 1, &len);
  const string data(ptr, len);

  const PlayerId dstPlayerID = (PlayerId)luaL_optint(L, 2, AllPlayers);
  const int16_t  dstScriptID =  (int16_t)luaL_optint(L, 3, 0);
  const uint8_t  statusBits  =  (uint8_t)luaL_optint(L, 4, 0);

  lua_pushboolean(L, serverLink->sendLuaData(myTank->getId(),
					     L2H(L)->GetScriptID(),
					     dstPlayerID, dstScriptID,
					     statusBits, data));
  return 1;
}


int LuaCallOuts::SendCommand(lua_State* L) // FIXME -- removed for safety
{
  L = L;
/*
  const char* command = luaL_checkstring(L, 1);
  const bool local = lua_tobool(L, 2);
  if (local) {
    lua_pushboolean(L, LocalCommand::execute(command));
  } else {
    lua_pushstdstring(L, CMDMGR.run(command));
  }
  return 1;
*/
  return 0;
}


//============================================================================//

int LuaCallOuts::BlockControls(lua_State* /*L*/)
{
  forceControls(true, 0.0f, 0.0f);
  return 0;
}


//============================================================================//

int LuaCallOuts::OpenMenu(lua_State* L)
{
  HUDDialogStack* stack = HUDDialogStack::get();
  while (stack->isActive()) {
    stack->pop();
  }
  lua_pushboolean(L, 1);
  return 1;
}


int LuaCallOuts::CloseMenu(lua_State* L)
{
  HUDDialogStack* stack = HUDDialogStack::get();
  while (stack->isActive()) {
    stack->pop();
  }
  lua_pushboolean(L, 1);
  return 1;
}


//============================================================================//

int LuaCallOuts::PlaySound(lua_State* L)
{
  int soundID = -1;
  if (lua_israwnumber(L, 1)) {
    soundID = lua_toint(L, 1);
  }
  else if (lua_israwstring(L, 1)) {
    soundID = SOUNDSYSTEM.getID(lua_tostring(L, 1));
  }
  if (soundID < 0) {
    lua_pushboolean(L, false);
    return 1;
  }

  bool local = true;
  bool repeated = false;
  bool important = false;
  float volume = 1.0f; // currently unused
  fvec3 pos(0.0f, 0.0f, 0.0f);

  if (lua_istable(L, 2)) {
    lua_getfield(L, 2, "volume"); // NOTE -- not used
    if (lua_israwnumber(L, -1)) {
      volume = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "important");
    important = lua_isboolean(L, -1) && lua_tobool(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, 2, "repeated");
    repeated = lua_isboolean(L, -1) && lua_tobool(L, -1);
    lua_pop(L, 1);

    const char* posNames[3] = { "px", "py", "pz" };
    for (int a = 0; a < 3; a++) {
      lua_getfield(L, 2, posNames[a]);
      if (lua_israwnumber(L, -1)) {
	pos[a] = lua_tonumber(L, -1);
	local = false;
      }
      lua_pop(L, 1);
    }
  }

  SOUNDSYSTEM.play(soundID, local ? NULL : (const float*)pos,
		   important, local, repeated);

  return 0;
}


//============================================================================//
//
//  Image manipulation
//

static int PushImageInfo(lua_State* L, PNGImageFile& image)
{
  if (!image.isOpen()) {
    return luaL_pushnil(L);
  }

  lua_pushinteger(L, image.getWidth());
  lua_pushinteger(L, image.getHeight());
  lua_pushinteger(L, image.getNumChannels());

  return 3;
}


static int PushImageData(lua_State* L, PNGImageFile& image)
{
  if (!image.isOpen()) {
    return luaL_pushnil(L);
  }

  const int width = image.getWidth();
  const int height = image.getHeight();
  const int channels = image.getNumChannels();

  const int bufSize = width * height * channels;
  char* buf = new char[bufSize];
  if (!image.read(buf)) {
    delete[] buf;
    return luaL_pushnil(L);
  }

  lua_pushinteger(L, width);
  lua_pushinteger(L, height);
  lua_pushinteger(L, channels);
  lua_pushlstring(L, buf, bufSize);

  delete[] buf;

  return 4;
}


int LuaCallOuts::ReadImageData(lua_State* L)
{
  size_t inLen;
  const char* inData = luaL_checklstring(L, 1, &inLen);
  const bool infoOnly = lua_isboolean(L, 2) && lua_tobool(L, 2);

  std::istringstream iss(string(inData, inLen));
  PNGImageFile image(&iss);

  if (infoOnly) {
    return PushImageInfo(L, image);
  }
  return PushImageData(L, image);
}


int LuaCallOuts::ReadImageFile(lua_State* L)
{
  const char* path = luaL_checkstring(L, 1);

  string modes = L2ES(L)->vfsModes->readDefault;
  bool infoOnly = false;
  if (!lua_israwstring(L, 2)) {
    infoOnly = lua_isboolean(L, 2) && lua_tobool(L, 2);
  }
  else {
    modes = lua_tostring(L, 2); // wanted
    modes = BzVFS::allowModes(modes, L2ES(L)->vfsModes->readAllowed);
    infoOnly = lua_isboolean(L, 3) && lua_tobool(L, 3);
  }

  string data;
  if (!bzVFS.readFile(path, modes, data)) {
    return luaL_pushnil(L);
  }

  std::istringstream iss(data);
  PNGImageFile image(&iss);

  if (infoOnly) {
    return PushImageInfo(L, image);
  }
  return PushImageData(L, image);
}


//============================================================================//

int LuaCallOuts::GetViewType(lua_State* L)
{
  switch (RENDERER.getViewType()) {
    case SceneRenderer::Normal:       { lua_pushliteral(L, "normal");       break; }
    case SceneRenderer::Stereo:       { lua_pushliteral(L, "stereo");       break; }
    case SceneRenderer::Stacked:      { lua_pushliteral(L, "stacked");      break; }
    case SceneRenderer::ThreeChannel: { lua_pushliteral(L, "threeChannel"); break; }
    case SceneRenderer::Anaglyph:     { lua_pushliteral(L, "anaglyph");     break; }
    case SceneRenderer::Interlaced:   { lua_pushliteral(L, "interlaced");   break; }
    default: {
      lua_pushliteral(L, "unknown");
    }
  }
  return 1;
}


int LuaCallOuts::GetKeyToCmds(lua_State* L)
{
  const string keyString = luaL_checkstring(L, 1);
  BzfKeyEvent keyEvent;
  if (!KEYMGR.stringToKeyEvent(keyString, keyEvent)) {
    return luaL_pushnil(L);
  }
  const bool press = !lua_isboolean(L, 2) || lua_tobool(L, 2);
  const string command = KEYMGR.get(keyEvent, press);
  lua_pushstdstring(L, command);
  return 1;
}


int LuaCallOuts::GetCmdToKeys(lua_State* L)
{
  const string command = luaL_checkstring(L, 1);
  const bool press = !lua_isboolean(L, 2) || lua_tobool(L, 2);
  const vector<string> keys = KEYMGR.getKeysFromCommand(command, press);
  lua_createtable(L, keys.size(), 0);
  for (size_t i = 0; i < keys.size(); i++) {
    lua_pushstdstring(L, keys[i]);
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}


int LuaCallOuts::GetRoamInfo(lua_State* L)
{
  if (!ROAM.isRoaming()) {
    return luaL_pushnil(L);
  }

  lua_pushinteger(L, ROAM.getMode());
  const Roaming::RoamingCamera* cam = ROAM.getCamera();
  lua_pushfvec3(L, cam->pos);
  lua_pushfloat(L, cam->theta);
  lua_pushfloat(L, cam->phi);
  lua_pushfloat(L, cam->zoom);

  // push the target
  switch (ROAM.getMode()) {
    case Roaming::roamViewTrack:
    case Roaming::roamViewFollow:
    case Roaming::roamViewFP: {
      const Player* player = ROAM.getTargetTank();
      if (player != NULL) {
	lua_pushinteger(L, player->getId());
      }
      break;
    }
    case Roaming::roamViewFlag: {
      const ClientFlag* flag = (const ClientFlag*)ROAM.getTargetFlag();
      if (flag != NULL) {
	lua_pushinteger(L, flag->id);
      }
      break;
    }
    default: {
      break;
    }
  }

  lua_settop(L, 8);
  return 8;
}


int LuaCallOuts::SetRoamInfo(lua_State* L)
{
  if (!ROAM.isRoaming()) {
    return 0;
  }

  const Roaming::RoamingView mode = (Roaming::RoamingView)luaL_checkint(L, 1);
  if ((mode <= Roaming::roamViewDisabled) || (mode >= Roaming::roamViewCount)) {
    return 0;
  }

  ROAM.setMode(mode);

  switch (mode) {
    case Roaming::roamViewTrack:
    case Roaming::roamViewFollow:
    case Roaming::roamViewFP: {
      const int type = lua_type(L, 2);
      if ((type == LUA_TNUMBER) || (type == LUA_TBOOLEAN)) {
	const Player* player = ParsePlayer(L, 2);
	if (player == NULL) {
	  lua_pushboolean(L, false);
	  return 1;
	}

	if (type == LUA_TNUMBER) {
	  ROAM.changeTarget(Roaming::explicitSet, player->getId());
	}
	else {
	  const Roaming::RoamingTarget target =
	    lua_tobool(L, 2) ? Roaming::next : Roaming::previous;
	  ROAM.changeTarget(target, 0);
	}
      }
      lua_pushboolean(L, true);
      return 1;
    }
    case Roaming::roamViewFlag: {
      const int type = lua_type(L, 2);
      if ((type == LUA_TNUMBER) || (type == LUA_TBOOLEAN)) {
	const ClientFlag* flag = ParseFlag(L, 2);
	if (flag == NULL) {
	  lua_pushboolean(L, false);
	  return 1;
	}

	if (type == LUA_TNUMBER) {
	  ROAM.changeTarget(Roaming::explicitSet, flag->id);
	}
	else {
	  const Roaming::RoamingTarget target =
	    lua_tobool(L, 2) ? Roaming::next : Roaming::previous;
	  ROAM.changeTarget(target, 0);
	}
      }
      lua_pushboolean(L, true);
      return 1;
    }
    case Roaming::roamViewFree: {
      Roaming::RoamingCamera cam;
      memcpy(&cam, ROAM.getCamera(), sizeof(Roaming::RoamingCamera));
      cam.pos    = luaL_optfvec3(L, 2, cam.pos);
      cam.theta  = luaL_optfloat(L, 5, cam.theta);
      cam.phi    = luaL_optfloat(L, 6, cam.phi);
      cam.zoom   = luaL_optfloat(L, 7, cam.zoom);
      ROAM.setCamera(&cam);
      lua_pushboolean(L, true);
      return 1;
    }
    case Roaming::roamViewDisabled:
    case Roaming::roamViewCount: {
      return 0;
    }
  }

  return 0;
}


int LuaCallOuts::GetDrawingMirror(lua_State* L)
{
  lua_pushboolean(L, RENDERER.getDrawingMirror());
  return 1;
}


//============================================================================//

int LuaCallOuts::GetScreenGeometry(lua_State* L)
{
  BzfDisplay* dpy = getDisplay();
  if (dpy == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, dpy->getWidth());
  lua_pushinteger(L, dpy->getHeight());
  lua_pushinteger(L, 0);
  lua_pushinteger(L, 0);
  return 4;
}


int LuaCallOuts::GetWindowGeometry(lua_State* L)
{
  MainWindow* window = getMainWindow();
  if (window == NULL) {
    return luaL_pushnil(L);
  }
  BzfDisplay* dpy = getDisplay();
  if (dpy == NULL) {
    return luaL_pushnil(L);
  }

  const int wsx = window->getWidth();
  const int wsy = window->getHeight();

  int wpx, wpy;
  BzfWindow* bzWindow = window->getWindow();
  bzWindow->getPosition(wpx, wpy);
  wpy = (dpy->getHeight() - (wpy + wsy));

  lua_pushinteger(L, wsx);
  lua_pushinteger(L, wsy);
  lua_pushinteger(L, wpx);
  lua_pushinteger(L, wpy);
  return 4;
}


int LuaCallOuts::GetViewGeometry(lua_State* L)
{
  MainWindow* window = getMainWindow();
  if (window == NULL) {
    return luaL_pushnil(L);
  }
  const int wh = window->getHeight();
  const int vh = window->getViewHeight();
  const int yOffset = (wh - vh);

  lua_pushinteger(L, window->getWidth());
  lua_pushinteger(L, window->getViewHeight());
  lua_pushinteger(L, window->getOriginX());
  lua_pushinteger(L, window->getOriginY() + yOffset);
  return 4;
}


int LuaCallOuts::GetRadarGeometry(lua_State* L)
{
  const RadarRenderer* radar = getRadarRenderer();
  if (radar == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, radar->getWidth());
  lua_pushinteger(L, radar->getHeight());
  lua_pushinteger(L, radar->getX());
  lua_pushinteger(L, radar->getY());
  return 4;
}


int LuaCallOuts::GetRadarRange(lua_State* L)
{
  World* world = World::getWorld();
  const RadarRenderer* radar = getRadarRenderer();
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if ((world == NULL) || (radar == NULL) || (myTank == NULL)) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, radar->getRange());
  return 1;
}


//============================================================================//

int LuaCallOuts::GetWorldExtents(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  const Extents& exts = COLLISIONMGR.getWorldExtents();
  lua_pushfvec3(L, exts.mins);
  lua_pushfvec3(L, exts.maxs);
  return 6;
}


int LuaCallOuts::GetVisualExtents(lua_State* L)
{
  const Extents* exts = RENDERER.getVisualExtents();
  if (exts == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, exts->mins);
  lua_pushfvec3(L, exts->maxs);
  return 6;
}


int LuaCallOuts::GetLengthPerPixel(lua_State* L)
{
  lua_pushfloat(L, RENDERER.getLengthPerPixel());
  return 1;
}


//============================================================================//

int LuaCallOuts::SetCameraView(lua_State* L)
{
  if (!L2H(L)->HasFullRead()) {
    return 0;
  }

  // get the defaults
  ViewFrustum& vf = RENDERER.getViewFrustum();
  const fvec3& currPos = vf.getEye();

  const fvec3& currDir = vf.getDirection();
  fvec3 pos = currPos;
  fvec3 dir = currDir;

  const int table = 1;
  if (!lua_istable(L, table)) {
    pos = luaL_optfvec3(L, 1, pos);
    dir = luaL_optfvec3(L, 4, dir);
  }
  else {
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      if (lua_israwstring(L, -2) && // key
	  lua_israwnumber(L, -1)) { // value
	const string key = lua_tostring(L, -2);
	     if (key == "px") { pos.x = lua_tofloat(L, -1); }
	else if (key == "py") { pos.y = lua_tofloat(L, -1); }
	else if (key == "pz") { pos.z = lua_tofloat(L, -1); }
	else if (key == "dx") { dir.x = lua_tofloat(L, -1); }
	else if (key == "dy") { dir.y = lua_tofloat(L, -1); }
	else if (key == "dz") { dir.z = lua_tofloat(L, -1); }
      }
    }
  }

  fvec3 target = pos + dir;

  vf.setView(pos, target);

  return 0;
}


int LuaCallOuts::SetCameraProjection(lua_State* L)
{
  if (!L2H(L)->HasFullRead()) {
    return 0;
  }

  MainWindow* window = getMainWindow();
  if (window == NULL) {
    return 0;
  }

  // get the defaults
  ViewFrustum& vf = RENDERER.getViewFrustum();
  float fov      = vf.getFOVy();
  float nearZ    = vf.getNear();
  float farZ     = vf.getFar();
  float deepFarZ = vf.getDeepFar();
  int ww  = window->getWidth();
  int wh  = window->getHeight();
  int wvh = window->getViewHeight();

  const int table = 1;
  if (!lua_istable(L, table)) {
    fov      = luaL_optfloat(L, 1,  fov);
    nearZ    = luaL_optfloat(L, 2,  nearZ);
    farZ     = luaL_optfloat(L, 3,  farZ);
    deepFarZ = luaL_optfloat(L, 4, deepFarZ);
    ww       = luaL_optint(L, 5, ww);
    wh       = luaL_optint(L, 6, wh);
    wvh      = luaL_optint(L, 7, wvh);
  }
  else {
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      if (lua_israwstring(L, -2) && // key
	  lua_israwnumber(L, -1)) { // value
	const string key = lua_tostring(L, -2);
	     if (key == "fov")	{ fov      = lua_tofloat(L, -1); }
	else if (key == "nearZ")      { nearZ    = lua_tofloat(L, -1); }
	else if (key == "farZ")       { farZ     = lua_tofloat(L, -1); }
	else if (key == "deepFarZ")   { deepFarZ = lua_tofloat(L, -1); }
	else if (key == "width")      { ww  = lua_toint(L, -1); }
	else if (key == "height")     { wh  = lua_toint(L, -1); }
	else if (key == "viewHeight") { wvh = lua_toint(L, -1); }
      }
    }
  }

  vf.setProjection(fov, nearZ, farZ, deepFarZ, ww, wh, wvh);

  return 0;
}


int LuaCallOuts::GetCameraPosition(lua_State* L)
{
  const ViewFrustum& vf = RENDERER.getViewFrustum();
  const fvec3& pos = vf.getEye();
  lua_pushfvec3(L, pos);
  return 3;
}


int LuaCallOuts::GetCameraDirection(lua_State* L)
{
  const ViewFrustum& vf = RENDERER.getViewFrustum();
  const fvec3& dir = vf.getDirection();
  lua_pushfvec3(L, dir);
  return 3;
}


int LuaCallOuts::GetCameraUp(lua_State* L)
{
  const ViewFrustum& vf = RENDERER.getViewFrustum();
  const fvec3& up = vf.getUp();
  lua_pushfvec3(L, up);
  return 3;
}


int LuaCallOuts::GetCameraRight(lua_State* L)
{
  const ViewFrustum& vf = RENDERER.getViewFrustum();
  const fvec3& right = vf.getRight();
  lua_pushfvec3(L, right);
  return 3;
}


int LuaCallOuts::GetCameraMatrix(lua_State* L)
{
  const ViewFrustum& vf = RENDERER.getViewFrustum();
  const float* m;
  if (!lua_tobool(L, 1)) {
     m = vf.getViewMatrix();
  } else {
     m = vf.getProjectionMatrix();
  }
  for (int i = 0; i < 16; i++) {
    lua_pushfloat(L, m[i]);
  }
  return 16;
}


int LuaCallOuts::GetFrustumPlane(lua_State* L)
{
  const ViewFrustum& vf = RENDERER.getViewFrustum();
  int plane = -1;
  if (lua_israwnumber(L, 1)) {
    plane = lua_toint(L, 1);
  }
  else if (lua_israwstring(L, 1)) {
    const string key = lua_tostring(L, 1);
    if (key == "near")   { plane = 0; }
    if (key == "left")   { plane = 1; }
    if (key == "right")  { plane = 2; }
    if (key == "bottom") { plane = 3; }
    if (key == "top")    { plane = 4; }
    if (key == "far")    { plane = 5; }
  }
  else {
    return luaL_pushnil(L);
  }

  if ((plane < 0) || (plane >= 6)) {
    return luaL_pushnil(L);
  }

  lua_pushfvec4(L, vf.getSide(plane));
  return 4;
}


//============================================================================//

int LuaCallOuts::NotifyStyleChange(lua_State* /*L*/)
{
  RENDERER.notifyStyleChange();
  return 0;
}


//============================================================================//

int LuaCallOuts::GetSun(lua_State* L)
{
  const string param = luaL_checkstring(L, 1);

  if (param == "brightness") {
    lua_pushfloat(L, RENDERER.getSunBrightness());
    return 1;
  }
  else if (param == "dir") {
    const fvec3* ptr = RENDERER.getSunDirection();
    if (ptr) {
      lua_pushfvec3(L, *ptr);
      return 3;
    }
  }
  else if (param == "ambient") {
    lua_pushfvec3(L, RENDERER.getAmbientColor().rgb());
    return 3;
  }
  else if (param == "diffuse") {
    lua_pushfvec3(L, RENDERER.getSunColor().rgb());
    return 3;
  }
  else if (param == "specular") {
    lua_pushfvec3(L, RENDERER.getSunColor().rgb());
    return 3;
  }

  return 0;
}


//============================================================================//
//
//  Time calls
//

int LuaCallOuts::GetTime(lua_State* L)
{
  const double nowTime = BzTime::getCurrent().getSeconds();
  LuaDouble::PushDouble(L, nowTime);
  return 1;
}


int LuaCallOuts::GetGameTime(lua_State* L)
{
  const double gameTime = GameTime::getStepTime();
  LuaDouble::PushDouble(L, gameTime);
  return 1;
}


int LuaCallOuts::GetTimer(lua_State* L)
{
  const double nowTime = BzTime::getCurrent().getSeconds();
  const uint32_t millisecs = (uint32_t)(nowTime * 1000.0);
  lua_pushlightuserdata(L, (void*)millisecs);
  return 1;
}


int LuaCallOuts::DiffTimers(lua_State* L)
{
  const int args = lua_gettop(L); // number of arguments
  if ((args != 2) || !lua_isuserdata(L, 1) || !lua_isuserdata(L, 2)) {
    luaL_error(L, "Incorrect arguments to DiffTimers()");
  }
  const char* p1 = (char*) lua_touserdata(L, 1);
  const char* p2 = (char*) lua_touserdata(L, 2);
  const uint32_t t1 = uint32_t(p1 - (char*)0);
  const uint32_t t2 = uint32_t(p2 - (char*)0);
  const uint32_t diffTime = (t1 - t2);
  lua_pushfloat(L, float(diffTime) * 0.001f); // return seconds
  return 1;
}


//============================================================================//
//============================================================================//
//
//  Mouse and keyboard
//

int LuaCallOuts::GetMousePosition(lua_State* L)
{
  MainWindow* wnd = getMainWindow();
  if (wnd == NULL) {
    return luaL_pushnil(L);
  }

  int mx, my;
  wnd->getWindow()->getMouse(mx, my);
  lua_pushinteger(L, mx);
  lua_pushinteger(L, wnd->getHeight() - my - 1);
  return 2;
}


int LuaCallOuts::GetMouseButtons(lua_State* L)
{
  MainWindow* wnd = getMainWindow();
  if (wnd == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, leftMouseButton);
  lua_pushboolean(L, middleMouseButton);
  lua_pushboolean(L, rightMouseButton);
  return 3;
}


int LuaCallOuts::GetJoyPosition(lua_State* L)
{
  MainWindow* wnd = getMainWindow();
  if (wnd == NULL) {
    return luaL_pushnil(L);
  }
  int jx, jy;
  wnd->getJoyPosition(jx, jy);
  lua_pushinteger(L, jx);
  lua_pushinteger(L, jy);
  return 2;
}


int LuaCallOuts::GetKeyModifiers(lua_State* L)
{
  BzfDisplay* dpy = getDisplay();
  if (dpy == NULL) {
    return luaL_pushnil(L);
  }
  bool alt, ctrl, shift;
  dpy->getModState(shift, ctrl, alt);
  lua_pushboolean(L, alt);
  lua_pushboolean(L, ctrl);
  lua_pushboolean(L, shift);
  return 3;
}


//============================================================================//
//============================================================================//

int LuaCallOuts::WarpMouse(lua_State* L)
{
  const int mx = luaL_checkint(L, 1);
  const int my = luaL_checkint(L, 2);

  MainWindow* wnd = getMainWindow();
  if (wnd == NULL) {
    return 0;
  }

  BzfWindow* bzWnd = wnd->getWindow();
  bzWnd->warpMouse(mx, wnd->getHeight() - my - 1);

  return 0;
}


int LuaCallOuts::SetMouseBox(lua_State* L)
{
  const int size = luaL_checkint(L, 1);
  RENDERER.setMaxMotionFactor(size);
  return 0;
}


//============================================================================//
//============================================================================//
//
//  Team info
//

int LuaCallOuts::GetTeamList(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    lua_newtable(L);
    return 1;
  }

  lua_newtable(L);
  int index = 0;
  for (int i = 0; i < NumTeams; i++) {
    if (world->getTeam((TeamColor)i).size > 0) {
      index++;
      lua_pushint(L, i);
      lua_rawseti(L, -2, index);
    }
  }

  return 1;
}


int LuaCallOuts::GetTeamPlayers(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    lua_newtable(L);
    return 1;
  }

  PlayerMap playerMap;
  MakePlayerMap(playerMap);


  map<TeamColor, vector<int> > teamPlayers;

  PlayerMap::const_iterator it;
  for (it = playerMap.begin(); it != playerMap.end(); ++it) {
    const TeamColor team = it->second->getTeam();
    teamPlayers[team].push_back(it->first);
  }

  lua_newtable(L);

  map<TeamColor, vector<int> >::const_iterator teamIt;
  for (teamIt = teamPlayers.begin(); teamIt != teamPlayers.end(); ++teamIt) {
    const vector<int>& ids = teamIt->second;
    lua_pushint(L, int(teamIt->first));
    lua_createtable(L, teamIt->second.size(), 0);
    for (size_t i = 0; i < ids.size(); i++) {
      lua_pushint(L, ids[i]);
      lua_rawseti(L, -2, i + 1);
    }
    lua_rawset(L, -3);
  }

  return 1;
}


int LuaCallOuts::GetTeamName(lua_State* L)
{
  const TeamColor teamID = ParseTeam(L, 1);
  if (teamID == NoTeam) {
    return luaL_pushnil(L);
  }
  const char* name = Team::getShortName(teamID);
  if (name == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, name);
  return 1;
}


int LuaCallOuts::GetTeamLongName(lua_State* L)
{
  const TeamColor teamID = ParseTeam(L, 1);
  if (teamID == NoTeam) {
    return luaL_pushnil(L);
  }
  const char* name = Team::getName(teamID);
  if (name == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, name);
  return 1;
}


int LuaCallOuts::GetTeamCount(lua_State* L)
{
  const TeamColor teamID = ParseTeam(L, 1);
  if (teamID == NoTeam) {
    return luaL_pushnil(L);
  }
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  if ((teamID < 0) || (teamID >= NumTeams)) {
    return luaL_pushnil(L); // not safe for world->getTeam()
  }
  const Team& team = world->getTeam(teamID);
  lua_pushinteger(L, team.size);
  return 1;
}


int LuaCallOuts::GetTeamScore(lua_State* L)
{
  const TeamColor teamID = ParseTeam(L, 1);
  if (teamID == NoTeam) {
    return luaL_pushnil(L);
  }
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  if ((teamID < 0) || (teamID > NumTeams)) {
    return luaL_pushnil(L); // not safe for world->getTeam()
  }
  const Team& team = world->getTeam(teamID);
  lua_pushinteger(L, team.won);
  lua_pushinteger(L, team.lost);
  return 2;
}


int LuaCallOuts::GetTeamColor(lua_State* L)
{
  const TeamColor teamID = ParseTeam(L, 1);
  if (teamID == NoTeam) {
    return luaL_pushnil(L);
  }
  const fvec4& color = Team::getTankColor(teamID);
  if (color == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, color.rgb());
  return 3;
}


int LuaCallOuts::GetTeamRadarColor(lua_State* L)
{
  const TeamColor teamID = ParseTeam(L, 1);
  if (teamID == NoTeam) {
    return luaL_pushnil(L);
  }
  const fvec4& color = Team::getRadarColor(teamID);
  if (color == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, color.rgb());
  return 3;
}


int LuaCallOuts::GetTeamsAreEnemies(lua_State* L)
{
  const TeamColor t1 = ParseTeam(L, 1);
  const TeamColor t2 = ParseTeam(L, 2);
  if ((t1 == NoTeam) || (t2 == NoTeam)) {
    return luaL_pushnil(L);
  }
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, Team::areFoes(t1, t2, world->getGameType()));
  return 1;
}


int LuaCallOuts::GetPlayersAreEnemies(lua_State* L)
{
  const Player* p1 = ParsePlayer(L, 1);
  if (p1 == NULL) {
    return luaL_pushnil(L);
  }
  const Player* p2 = ParsePlayer(L, 2);
  if (p2 == NULL) {
    return luaL_pushnil(L);
  }
  World* world = World::getWorld();
  if (world == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, Team::areFoes(p1->getTeam(), p2->getTeam(),
				   world->getGameType()));
  return 1;
}


//============================================================================//
//============================================================================//

int LuaCallOuts::GetLocalPlayer(lua_State* L)
{
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, myTank->getId());
  return 1;
}


int LuaCallOuts::GetLocalPlayerTarget(lua_State* L)
{
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return luaL_pushnil(L);
  }
  const Player* target = myTank->getTarget();
  if (target == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, target->getId());
  return 1;
}


int LuaCallOuts::GetLocalTeam(lua_State* L)
{
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, myTank->getTeam());
  return 1;
}


int LuaCallOuts::GetRabbitPlayer(lua_State* L)
{
  // FIXME -- is this valid?
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    if (myTank->getTeam() == RabbitTeam) {
      lua_pushinteger(L, myTank->getId());
      return 1;
    }
  }

  World* world = World::getWorld();
  if (world != NULL) {
    const RemotePlayer* rabbit = world->getCurrentRabbit();
    if (rabbit != NULL) {
      lua_pushinteger(L, rabbit->getId());
      return 1;
    }
  }

  return 0;
}


int LuaCallOuts::GetAntidotePosition(lua_State* L)
{
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank == NULL) {
    return luaL_pushnil(L);
  }
  const fvec3* pos = myTank->getAntidoteLocation();
  if (pos == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, *pos);
  return 3;
}


//============================================================================//
//============================================================================//
//
//  GfxBlock routines
//

static GfxBlock* ParseGfxBlock(lua_State* L, int index)
{
  int id = -1;

  if (lua_israwnumber(L, index)) {
    id = luaL_checkint(L, index);
  } else {
    id = GfxBlockMgr::getStringID(luaL_checkstring(L, index));
  }

  if ((id < 0) || (id >= GfxBlockMgr::BlockIDCount)) {
    luaL_error(L, "invalid GfxBlock id");
  }

  GfxBlock* gfxBlock = GfxBlockMgr::get(id);
  if (gfxBlock == NULL) {
    return NULL;
  }

  return gfxBlock;
}


static int Get_GfxBlock(lua_State* L, const GfxBlock& gfxBlock)
{
  EventClient* ec = L2H(L);
  lua_pushboolean(L, gfxBlock.blocked());
  lua_pushboolean(L, gfxBlock.test(ec));
  return 2;
}


static int Set_GfxBlock(lua_State* L, GfxBlock& gfxBlock)
{
  EventClient* ec = L2H(L);
  if ((ec == luaUser) && gfxBlock.worldBlock()) {
    return luaL_pushnil(L);
  }
  luaL_checktype(L, 2, LUA_TBOOLEAN);
  const bool block = lua_tobool(L, 2);
  if (!block) {
    gfxBlock.remove(ec);
    return luaL_pushnil(L);
  }
  const bool queue = lua_isboolean(L, 3) && lua_tobool(L, 3);
  lua_pushboolean(L, gfxBlock.set(ec, queue));
  return 1;
}


int LuaCallOuts::SetGfxBlock(lua_State* L)
{
  GfxBlock* gfxBlock = ParseGfxBlock(L, 1);
  if (gfxBlock == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, *gfxBlock);
}


int LuaCallOuts::GetGfxBlock(lua_State* L)
{
  const GfxBlock* gfxBlock = ParseGfxBlock(L, 1);
  if (gfxBlock == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, *gfxBlock);
}


int LuaCallOuts::SetPlayerGfxBlock(lua_State* L)
{
  // FIXME - remove these 3 const_cast<>s for now const Parse routines ?
  Player* player = const_cast<Player*>(ParsePlayer(L, 1));
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, player->getGfxBlock());
}


int LuaCallOuts::GetPlayerGfxBlock(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, player->getGfxBlock());
}


int LuaCallOuts::SetPlayerRadarGfxBlock(lua_State* L)
{
  // FIXME - remove these 3 const_cast<>s for now const Parse routines ?
  Player* player = const_cast<Player*>(ParsePlayer(L, 1));
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, player->getRadarGfxBlock());
}


int LuaCallOuts::GetPlayerRadarGfxBlock(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, player->getRadarGfxBlock());
}


int LuaCallOuts::SetFlagGfxBlock(lua_State* L)
{
  // FIXME - remove these 3 const_cast<>s for now const Parse routines ?
  ClientFlag* flag = const_cast<ClientFlag*>(ParseFlag(L, 1));
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, flag->gfxBlock);
}


int LuaCallOuts::GetFlagGfxBlock(lua_State* L)
{
  const ClientFlag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, flag->gfxBlock);
}


int LuaCallOuts::SetFlagRadarGfxBlock(lua_State* L)
{
  ClientFlag* flag = const_cast<ClientFlag*>(ParseFlag(L, 1));
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, flag->radarGfxBlock);
}


int LuaCallOuts::GetFlagRadarGfxBlock(lua_State* L)
{
  const ClientFlag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, flag->radarGfxBlock);
}


int LuaCallOuts::SetShotGfxBlock(lua_State* L)
{
  ShotPath* shot = const_cast<ShotPath*>(ParseShot(L, 1));
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, shot->getGfxBlock());
}


int LuaCallOuts::GetShotGfxBlock(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, shot->getGfxBlock());
}


int LuaCallOuts::SetShotRadarGfxBlock(lua_State* L)
{
  ShotPath* shot = const_cast<ShotPath*>(ParseShot(L, 1));
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  return Set_GfxBlock(L, shot->getRadarGfxBlock());
}


int LuaCallOuts::GetShotRadarGfxBlock(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  return Get_GfxBlock(L, shot->getRadarGfxBlock());
}


//============================================================================//
//============================================================================//
//
//  Player routines
//

int LuaCallOuts::GetPlayerList(lua_State* L)
{
  lua_newtable(L);
  int count = 0;

  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    count++;
    lua_pushinteger(L, myTank->getId());
    lua_rawseti(L, -2, count);
  }

#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    if (robots[i] != NULL) {
      count++;
      lua_pushinteger(L, robots[i]->getId());
      lua_rawseti(L, -2, count);
    }
  }
#endif

  for (int i = 0; i < curMaxPlayers; i++) {
    if (remotePlayers[i] != NULL) {
      count++;
      lua_pushinteger(L, remotePlayers[i]->getId());
      lua_rawseti(L, -2, count);
    }
  }

  return 1;
}


int LuaCallOuts::GetPlayerName(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, player->getCallSign());
  return 1;
}


int LuaCallOuts::GetPlayerType(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, player->getPlayerType());
  return 1;
}


int LuaCallOuts::GetPlayerTeam(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, player->getTeam());
  return 1;
}


int LuaCallOuts::GetPlayerFlag(lua_State* L) // FIXME -- linear search, ew
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  const int flagID = player->getFlagID();
  if (flagID < 0) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flagID);
  return 1;
}


int LuaCallOuts::GetPlayerFlagType(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  const FlagType* ft = player->getFlagType();
  if (ft == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, ft->flagAbbv.c_str());
  return 1;
}


int LuaCallOuts::GetPlayerScore(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, player->getWins());
  lua_pushinteger(L, player->getLosses());
  lua_pushinteger(L, player->getTeamKills());
  return 3;
}


int LuaCallOuts::GetPlayerMotto(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  map<string, string>::const_iterator it = player->customData.find("motto");
  if (it == player->customData.end()) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, it->second);
  return 1;
}


int LuaCallOuts::GetPlayerAutoPilot(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, player->isAutoPilot());
  return 1;
}


int LuaCallOuts::GetPlayerLagInfo(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, player->getLag());
  lua_pushfloat(L, player->getJitter());
  lua_pushfloat(L, player->getPacketLoss());
  return 3;
}


int LuaCallOuts::GetPlayerCustomData(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  const string key = luaL_checkstring(L, 2);
  map<string, string>::const_iterator it = player->customData.find(key);
  if (it == player->customData.end()) {
    return luaL_pushnil(L);
  }
  lua_pushstdstring(L, it->second);
  return 1;
}


int LuaCallOuts::GetPlayerShots(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }

  lua_newtable(L);

  World* world = World::getWorld();
  if (world == NULL) {
    return 1;
  }

  int maxShots = 0;
  const WorldPlayer* worldWeapons = world->getWorldWeapons();
  if (player != (const Player*)worldWeapons) {
    maxShots = world->getMaxShots();
  } else {
    maxShots = worldWeapons->getMaxShots();
  }

  int count = 0;
  for (int i = 0; i < maxShots; i++) {
    const ShotPath* shot = player->getShot(i);
    if ((shot != NULL) && !shot->isExpired()) {
      count = PushShot(L, shot, count);
    }
  }

  return 1;
}


int LuaCallOuts::GetPlayerState(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }

  lua_newtable(L);
  if (player->isAlive())	{ luaset_strbool(L, "alive",	true); }
  if (player->isPaused())       { luaset_strbool(L, "paused",       true); }
  if (player->isFalling())      { luaset_strbool(L, "falling",      true); }
  if (player->isPhantomZoned()) { luaset_strbool(L, "zoned",	true); }
  if (player->isTeleporting())  { luaset_strbool(L, "teleporting",  true); }
  if (player->isCrossingWall()) { luaset_strbool(L, "crossingWall", true); }
  if (player->isExploding())    { luaset_strbool(L, "exploding",    true); }

  return 1;
}


int LuaCallOuts::GetPlayerStateBits(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, player->getStatus());
  return 1;
}


int LuaCallOuts::GetPlayerPosition(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, player->getPosition());
  return 3;
}


int LuaCallOuts::GetPlayerRotation(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, player->getAngle());
  return 1;
}


int LuaCallOuts::GetPlayerDirection(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, player->getForward());
  return 3;
}


int LuaCallOuts::GetPlayerVelocity(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, player->getVelocity());
  return 3;
}


int LuaCallOuts::GetPlayerAngVel(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, player->getAngularVelocity());
  return 1;
}


int LuaCallOuts::GetPlayerDimensions(lua_State* L)
{

  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, player->getDimensions());
  return 3;
}


int LuaCallOuts::GetPlayerPhysicsDriver(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }

  const int phydrv = player->getPhysicsDriver();
  if (phydrv < 0) {
    lua_pushboolean(L, false);
  } else {
    lua_pushinteger(L, phydrv);
  }
  return 1;
}


int LuaCallOuts::GetPlayerDesiredSpeed(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, player->getUserSpeed());
  return 1;
}


int LuaCallOuts::GetPlayerDesiredAngVel(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, player->getUserAngVel());
  return 1;
}


int LuaCallOuts::GetPlayerExplodeTime(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  if (!player->isExploding()) {
    return luaL_pushnil(L);
  }
  const BzTime current = BzTime::getTick();
  const BzTime explode = player->getExplodeTime();
  const float explodeTime = (float)(current - explode);
  if (explodeTime < 0.0f) {
    return luaL_pushnil(L); // should not happen
  }
  lua_pushfloat(L, explodeTime);
  return 1;
}


int LuaCallOuts::GetPlayerOpacity(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, player->getAlpha());
  return 1;
}


int LuaCallOuts::IsPlayerAdmin(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, player->isAdmin());
  return 1;
}


int LuaCallOuts::IsPlayerVerified(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, player->isVerified());
  return 1;
}


int LuaCallOuts::IsPlayerRegistered(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, player->isRegistered());
  return 1;
}


int LuaCallOuts::IsPlayerHunted(lua_State* L)
{
  const Player* player = ParsePlayer(L, 1);
  if (player == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushboolean(L, player->isHunted());
  return 1;
}


int LuaCallOuts::SetPlayerCustomData(lua_State* L)
{
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (!myTank || !serverLink || !entered) {
    return luaL_pushnil(L);
  }
  const std::string key   = luaL_checkstdstring(L, 1);
  const std::string value = luaL_checkstdstring(L, 2);
  if (key.empty()) {
    return luaL_pushnil(L);
  }

  serverLink->sendCustomData(key, value);

  lua_pushboolean(L, true);
  return 1;
}


//============================================================================//
//============================================================================//
//
//  Flag routines
//

int LuaCallOuts::GetFlagList(lua_State* L)
{
  World* world = World::getWorld();
  if (world == NULL) {
    lua_newtable(L);
    return 1;
  }

  // arg1 means <show all flags>, defaults to false
  const bool onlyActive = !lua_isboolean(L, 1) || !lua_tobool(L, 1);

  lua_newtable(L);
  int index = 0;
  for (int fi = 0; fi < world->getMaxFlags(); fi++) {
    const Flag& flag = world->getFlag(fi);
    if (!onlyActive || (flag.status != FlagNoExist)) {
      index++;
      luaset_intint(L, index, fi);
    }
  }

  return 1;
}


int LuaCallOuts::GetFlagName(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, flag->type->flagName.c_str());
  return 1;
}


int LuaCallOuts::GetFlagType(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, flag->type->flagAbbv.c_str());
  return 1;
}


int LuaCallOuts::GetFlagShotType(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flag->type->flagShot);
  return 1;
}


int LuaCallOuts::GetFlagQuality(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flag->type->flagQuality);
  return 1;
}


int LuaCallOuts::GetFlagEndurance(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flag->endurance);
  return 1;
}


int LuaCallOuts::GetFlagTeam(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flag->type->flagTeam);
  return 1;
}


int LuaCallOuts::GetFlagOwner(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  if (flag->owner == (PlayerId) -1) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flag->owner);
  return 1;
}


int LuaCallOuts::GetFlagState(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, flag->status);
  return 1;
}


int LuaCallOuts::GetFlagPosition(lua_State* L)
{
  const Flag* flag = ParseFlag(L, 1);
  if (flag == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, flag->position);
  return 3;
}


//============================================================================//
//============================================================================//
//
//  Shot routines
//

int LuaCallOuts::GetShotList(lua_State* L)
{
  lua_newtable(L);
  int count = 0;

  World* world = World::getWorld();
  if (world == NULL) {
    return 1;
  }

  const int maxShots = world->getMaxShots();

  // my shots
  const LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (myTank != NULL) {
    for (int i = 0; i < maxShots; i++) {
      const ShotPath* shot = myTank->getShot(i);
      if ((shot != NULL) && !shot->isExpired()) {
	count = PushShot(L, shot, count);
      }
    }
  }

  // robot player shots
#ifdef ROBOT
  for (int i = 0; i < numRobots; i++) {
    const RobotPlayer* player = robots[i];
    if (player != NULL) {
      for (int j = 0; j < maxShots; j++) {
	const ShotPath* shot = player->getShot(i);
	if ((shot != NULL) && !shot->isExpired()) {
	  count = PushShot(L, shot, count);
	}
      }
    }
  }
#endif

  // remote player shots
  for (int i = 0; i < curMaxPlayers; i++) {
    const RemotePlayer* player = world->getPlayer(i);
    if (player != NULL) {
      for (int j = 0; j < maxShots; j++) {
	const ShotPath* shot = player->getShot(i);
	if ((shot != NULL) && !shot->isExpired()) {
	  count = PushShot(L, shot, count);
	}
      }
    }
  }

  // world weapon shots
  const WorldPlayer* worldWeapons = world->getWorldWeapons();
  if (worldWeapons != NULL) {
    const int wwMaxShots = worldWeapons->getMaxShots();
    for (int i = 0; i < wwMaxShots; i++) {
      const ShotPath* shot = worldWeapons->getShot(i);
      if ((shot != NULL) && !shot->isExpired()) {
	count = PushShot(L, shot, count);
      }
    }
  }

  return 1;
}


int LuaCallOuts::GetShotType(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, shot->getShotType());
  return 1;
}


int LuaCallOuts::GetShotFlagType(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushstring(L, shot->getFlagType()->flagAbbv.c_str());
  return 1;
}


int LuaCallOuts::GetShotPlayer(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, shot->getPlayer());
  return 1;
}


int LuaCallOuts::GetShotTeam(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushinteger(L, shot->getTeam());
  return 1;
}


int LuaCallOuts::GetShotPosition(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, shot->getPosition());
  return 3;
}


int LuaCallOuts::GetShotVelocity(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfvec3(L, shot->getVelocity());
  return 3;
}


int LuaCallOuts::GetShotLifeTime(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, shot->getLifetime());
  return 1;
}


int LuaCallOuts::GetShotLeftTime(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  const double timeLeft = shot->getCurrentTime() - shot->getStartTime();
  lua_pushfloat(L, (float)timeLeft);
  return 1;
}


int LuaCallOuts::GetShotReloadTime(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  lua_pushfloat(L, shot->getReloadTime());
  return 1;
}


int LuaCallOuts::RemoveShot(lua_State* L)
{
  const ShotPath* shot = ParseShot(L, 1);
  if (shot == NULL) {
    return luaL_pushnil(L);
  }
  const bool explode = lua_isboolean(L, 2) && lua_tobool(L, 2);

  Player* player = lookupPlayer(shot->getPlayer());
  if (!player) {
    lua_pushboolean(L, false);
  }
  else {
    player->endShot(shot->getShotId(), false, explode);
    lua_pushboolean(L, true);
  }
  return 1;
}


//============================================================================//
//============================================================================//

// whacky bit of dev'ing fun
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
  #include <unistd.h>
  #include <fcntl.h>
  int LuaCallOuts::ReadStdin(lua_State* L)
  {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    char buf[4096];
    const int r = read(STDIN_FILENO, buf, sizeof(buf));
    if (r <= 0) {
      return luaL_pushnil(L);
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
