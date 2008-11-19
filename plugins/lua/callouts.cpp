
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "callouts.h"

#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;


// TODO
// - connections (new call-in?)
// - groups
// - timers
// - logging
// - reports
// - admin
// - lagwarn
// - timelimit
// - countdown
// - polls
// - help
// * obstacle queries, tangibility
// - plugin management
// - more player data (global auth, etc...)

/******************************************************************************/
/******************************************************************************/


// FIXME -- move into "utils.cpp"
static bz_eTeamType ParseTeam(lua_State* L, int index)
{
  if (lua_israwstring(L, index)) {
    const string s = lua_tostring(L, index);
    if (s == "auto")     { return eAutomaticTeam;    }
    if (s == "none")     { return eNoTeam;           }
    if (s == "rogue")    { return eRogueTeam;        }
    if (s == "red")      { return eRedTeam;          }
    if (s == "green")    { return eGreenTeam;        }
    if (s == "blue")     { return eBlueTeam;         }
    if (s == "purple")   { return ePurpleTeam;       }
    if (s == "rabbit")   { return eRabbitTeam;       }
    if (s == "hunter")   { return eHunterTeam;       }
    if (s == "observer") { return eObservers;        }
    if (s == "admin")    { return eAdministrators;   }
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



/******************************************************************************/
/******************************************************************************/

// only valid during loading, otherwise empty
static int GetPluginDirectory(lua_State* L);

static int GetAPIVersion(lua_State* L);
static int GetServerVersion(lua_State* L);
static int GetServerPort(lua_State* L);
static int GetServerAddress(lua_State* L);
static int GetServerDescription(lua_State* L);

static int GetGameType(lua_State* L);
static int GetJumpingAllowed(lua_State* L);

static int GetWallHeight(lua_State* L);
static int SetWallHeight(lua_State* L);
static int GetWorldSize(lua_State* L);
static int SetWorldSize(lua_State* L);
static int GetWorldURL(lua_State* L);
static int SetWorldURL(lua_State* L);
static int GetWorldCache(lua_State* L);

static int SendMessage(lua_State* L);
static int SendTeamMessage(lua_State* L);

static int PlaySound(lua_State* L); // FIXME

static int GetStandardSpawn(lua_State* L); // FIXME
static int GetBaseAtPosition(lua_State* L); // FIXME

static int GetPlayerCount(lua_State* L);
static int GetPlayerIDs(lua_State* L);
static int GetPlayerName(lua_State* L);
static int GetPlayerTeam(lua_State* L);
static int GetPlayerIPAddress(lua_State* L);
static int GetPlayerFlagID(lua_State* L);
static int GetPlayerClientVersion(lua_State* L);
static int GetPlayerBZID(lua_State* L);
static int GetPlayerCustomData(lua_State* L);
static int GetPlayerPaused(lua_State* L); // FIXME
static int GetPlayerState(lua_State* L); // FIXME
static int GetPlayerPosition(lua_State* L);
static int GetPlayerVelocity(lua_State* L);
static int GetPlayerRotation(lua_State* L);
static int GetPlayerAngVel(lua_State* L);
static int GetPlayerStatus(lua_State* L);
static int GetPlayerFalling(lua_State* L);
static int GetPlayerCrossingWall(lua_State* L);
static int GetPlayerZoned(lua_State* L);
static int GetPlayerPhysicsDriver(lua_State* L);

static int GetPlayerWins(lua_State* L);
static int GetPlayerLosses(lua_State* L);
static int GetPlayerTKs(lua_State* L);

static int SetPlayerWins(lua_State* L);
static int SetPlayerLosses(lua_State* L);
static int SetPlayerTKs(lua_State* L);

static int ChangePlayerTeam(lua_State* L);

static int ZapPlayer(lua_State* L);
static int KillPlayer(lua_State* L);

static int SetRabbit(lua_State* L);

static int GivePlayerFlag(lua_State* L);

static int GetFlagCount(lua_State* L);
static int GetFlagName(lua_State* L);
static int GetFlagPosition(lua_State* L);
static int GetFlagPlayer(lua_State* L);

static int MoveFlag(lua_State* L);
static int ResetFlag(lua_State* L);

static int GetTeamLimit(lua_State* L);
static int GetTeamCount(lua_State* L);
static int GetTeamScore(lua_State* L);
static int GetTeamWins(lua_State* L);
static int GetTeamLosses(lua_State* L);
static int SetTeamWins(lua_State* L);
static int SetTeamLosses(lua_State* L);

static int SaveRecBuf(lua_State* L);
static int StartRecBuf(lua_State* L);
static int StopRecBuf(lua_State* L);

static int PauseCountdown(lua_State* L);
static int StartCountdown(lua_State* L);
static int ResumeCountdown(lua_State* L);

static int ReloadLocalBans(lua_State* L);
static int ReloadMasterBans(lua_State* L);
static int ReloadUsers(lua_State* L);
static int ReloadGroups(lua_State* L);
static int ReloadHelp(lua_State* L);



/******************************************************************************/
/******************************************************************************/

bool CallOuts::PushEntries(lua_State* L)
{
#define REGISTER_LUA_CFUNC(x) \
  lua_pushliteral(L, #x);     \
  lua_pushcfunction(L, x);    \
  lua_rawset(L, -3)

  // FIXME - not implemented
/*
  REGISTER_LUA_CFUNC(GameShutdown);
  REGISTER_LUA_CFUNC(GameRestart);
  REGISTER_LUA_CFUNC(GameOver);
  REGISTER_LUA_CFUNC(GameSuperKill);

  REGISTER_LUA_CFUNC(PlaySound);

  REGISTER_LUA_CFUNC(SetMaxWaitTime);
  REGISTER_LUA_CFUNC(ClearMaxWaitTime);

  REGISTER_LUA_CFUNC(FireWeapon);
  REGISTER_LUA_CFUNC(FireWeaponGM);

  // BZDB

  // Message
*/
  REGISTER_LUA_CFUNC(GetPluginDirectory);

  REGISTER_LUA_CFUNC(GetAPIVersion);
  REGISTER_LUA_CFUNC(GetServerVersion);
  REGISTER_LUA_CFUNC(GetServerPort);
  REGISTER_LUA_CFUNC(GetServerAddress);
  REGISTER_LUA_CFUNC(GetServerDescription);

  REGISTER_LUA_CFUNC(GetGameType);
  REGISTER_LUA_CFUNC(GetJumpingAllowed);

  REGISTER_LUA_CFUNC(GetWallHeight);
  REGISTER_LUA_CFUNC(GetWorldSize);
  REGISTER_LUA_CFUNC(GetWorldURL);
  REGISTER_LUA_CFUNC(GetWorldCache);
  REGISTER_LUA_CFUNC(SetWallHeight);
  REGISTER_LUA_CFUNC(SetWorldSize);
  REGISTER_LUA_CFUNC(SetWorldURL);

  REGISTER_LUA_CFUNC(SendMessage);
  REGISTER_LUA_CFUNC(SendTeamMessage);

  // Player
  REGISTER_LUA_CFUNC(GetPlayerCount);
  REGISTER_LUA_CFUNC(GetPlayerIDs);
  REGISTER_LUA_CFUNC(GetPlayerName);
  REGISTER_LUA_CFUNC(GetPlayerTeam);
  REGISTER_LUA_CFUNC(GetPlayerIPAddress);
  REGISTER_LUA_CFUNC(GetPlayerFlagID);
  REGISTER_LUA_CFUNC(GetPlayerClientVersion);
  REGISTER_LUA_CFUNC(GetPlayerBZID);
  REGISTER_LUA_CFUNC(GetPlayerCustomData);
  REGISTER_LUA_CFUNC(GetPlayerStatus);
  REGISTER_LUA_CFUNC(GetPlayerPosition);
  REGISTER_LUA_CFUNC(GetPlayerVelocity);
  REGISTER_LUA_CFUNC(GetPlayerRotation);
  REGISTER_LUA_CFUNC(GetPlayerAngVel);
  REGISTER_LUA_CFUNC(GetPlayerFalling);
  REGISTER_LUA_CFUNC(GetPlayerCrossingWall);
  REGISTER_LUA_CFUNC(GetPlayerZoned);
  REGISTER_LUA_CFUNC(GetPlayerPhysicsDriver);
  REGISTER_LUA_CFUNC(GetPlayerWins);
  REGISTER_LUA_CFUNC(GetPlayerLosses);
  REGISTER_LUA_CFUNC(GetPlayerTKs);

  REGISTER_LUA_CFUNC(SetPlayerWins);
  REGISTER_LUA_CFUNC(SetPlayerLosses);
  REGISTER_LUA_CFUNC(SetPlayerTKs);

  REGISTER_LUA_CFUNC(ChangePlayerTeam);

  REGISTER_LUA_CFUNC(ZapPlayer);
  REGISTER_LUA_CFUNC(KillPlayer);

  REGISTER_LUA_CFUNC(SetRabbit);

/*
  REGISTER_LUA_CFUNC(SetPlayerOperator);
  REGISTER_LUA_CFUNC(SetPlayerShotType);
*/

  REGISTER_LUA_CFUNC(GivePlayerFlag);


  // Flag
  REGISTER_LUA_CFUNC(GetFlagCount);
  REGISTER_LUA_CFUNC(GetFlagName);
  REGISTER_LUA_CFUNC(GetFlagPosition);
  REGISTER_LUA_CFUNC(GetFlagPlayer);

  REGISTER_LUA_CFUNC(MoveFlag);
  REGISTER_LUA_CFUNC(ResetFlag);

  // Team
  REGISTER_LUA_CFUNC(GetTeamLimit);
  REGISTER_LUA_CFUNC(GetTeamCount);
  REGISTER_LUA_CFUNC(GetTeamScore);
  REGISTER_LUA_CFUNC(GetTeamWins);
  REGISTER_LUA_CFUNC(GetTeamLosses);

  REGISTER_LUA_CFUNC(SetTeamWins);
  REGISTER_LUA_CFUNC(SetTeamLosses);

  REGISTER_LUA_CFUNC(SaveRecBuf);
  REGISTER_LUA_CFUNC(StartRecBuf);
  REGISTER_LUA_CFUNC(StopRecBuf);

  REGISTER_LUA_CFUNC(PauseCountdown);
  REGISTER_LUA_CFUNC(StartCountdown);
  REGISTER_LUA_CFUNC(ResumeCountdown);

  REGISTER_LUA_CFUNC(ReloadLocalBans);
  REGISTER_LUA_CFUNC(ReloadMasterBans);
  REGISTER_LUA_CFUNC(ReloadUsers);
  REGISTER_LUA_CFUNC(ReloadGroups);
  REGISTER_LUA_CFUNC(ReloadHelp);

  return true;
}


/******************************************************************************/
/******************************************************************************/

static int GetPluginDirectory(lua_State* L)
{
  lua_pushstring(L, bz_pluginBinPath());
  return 1;
}


static int GetAPIVersion(lua_State* L)
{
  lua_pushinteger(L, bz_APIVersion());
  return 1;
}


static int GetServerVersion(lua_State* L)
{
  lua_pushstring(L, bz_getServerVersion());
  return 1;
}


static int GetServerPort(lua_State* L)
{
  lua_pushinteger(L, bz_getPublicPort());
  return 1;
}


static int GetServerAddress(lua_State* L)
{
  // FIXME - free bz_ApiString?
  lua_pushstring(L, bz_getPublicAddr().c_str());
  return 1;
}


static int GetServerDescription(lua_State* L)
{
  // FIXME - free bz_ApiString?
  lua_pushstring(L, bz_getPublicDescription().c_str());
  return 1;
}


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/
/******************************************************************************/

static int GetWallHeight(lua_State* L)
{
  float size, height;
  bz_getWorldSize(&size, &height);
  lua_pushnumber(L, height);
  return 1;
}


static int GetWorldSize(lua_State* L)
{
  float size, height;
  bz_getWorldSize(&size, &height);
  lua_pushnumber(L, size);
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
  const char* url;
  return 0;
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
  const char* url = luaL_checkstring(L, 1);
  bz_setClientWorldDownloadURL(url);
  return 0;
}


/******************************************************************************/
/******************************************************************************/

static int SendMessage(lua_State* L)
{
  const int   from  = luaL_checkint(L, 1);
  const int   to    = luaL_checkint(L, 2);
  const char* msg   = luaL_checkstring(L, 3);
  lua_pushboolean(L, bz_sendTextMessage(from, to, msg));
  return 1;
}


static int SendTeamMessage(lua_State* L)
{
  const int          from = luaL_checkint(L, 1);
  const bz_eTeamType to   = ParseTeam(L, 2);
  const char*        msg  = luaL_checkstring(L, 3);
  lua_pushboolean(L, bz_sendTextMessage(from, to, msg));
  return 1;
}


/******************************************************************************/
/******************************************************************************/

static int GetPlayerCount(lua_State* L)
{
  lua_pushinteger(L, bz_getPlayerCount());
  return 1;
}


static int GetPlayerIDs(lua_State* L)
{
  bz_APIIntList playerList;
  if (!bz_getPlayerIndexList(&playerList)) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  lua_createtable(L, 0, playerList.size());
  for (int i = 0; i < playerList.size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushinteger(L, playerList[i]);
    lua_rawset(L, -3);
  }
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


static int GetPlayerFlagID(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushinteger(L, player->currentFlagID);
  lua_pushstring(L,  player->currentFlag.c_str());

  bz_freePlayerRecord(player);

  return 2;
}


static int GetPlayerClientVersion(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushstring(L, player->clientVersion.c_str());

  bz_freePlayerRecord(player);

  return 1;
}


static int GetPlayerBZID(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushstring(L, player->bzID.c_str());

  bz_freePlayerRecord(player);

  return 1;
}


static int GetPlayerCustomData(lua_State* L)
{
  const int    pid =    luaL_checkint(L, 1);
  const string key = luaL_checkstring(L, 2);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid);
  if (player == NULL) {
    return 0;
  }
  lua_pushstring(L, player->getCustomData(key.c_str()));

  bz_freePlayerRecord(player);

  return 1;
}


static int GetPlayerStatus(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushinteger(L, state.status);
  return 1;
}


static int GetPlayerPosition(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushnumber(L, state.pos[0]);
  lua_pushnumber(L, state.pos[1]);
  lua_pushnumber(L, state.pos[2]);
  return 3;
}


static int GetPlayerVelocity(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushnumber(L, state.velocity[0]);
  lua_pushnumber(L, state.velocity[1]);
  lua_pushnumber(L, state.velocity[2]);
  return 3;
}


static int GetPlayerRotation(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushnumber(L, state.rotation);
  return 1;
}


static int GetPlayerAngVel(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushnumber(L, state.angVel);
  return 1;
}


static int GetPlayerFalling(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushboolean(L, state.falling);
  return 1;
}


static int GetPlayerCrossingWall(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushboolean(L, state.crossingWall);
  return 1;
}


static int GetPlayerZoned(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushboolean(L, state.inPhantomZone);
  return 1;
}


static int GetPlayerPhysicsDriver(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) {
    return 0;
  }
  lua_pushinteger(L, state.phydrv);
  return 1;
}


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



/******************************************************************************/
/******************************************************************************/

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
  const bool spawnOnBase = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : true;
  const int killerID = luaL_optint(L, 3, -1);
  const char* flagID = luaL_optstring(L, 4, NULL);
  bz_killPlayer(playerID, spawnOnBase, killerID, flagID);
  return 0;  
}


static int SetRabbit(lua_State* L)
{
  // FIXME ?
  const int playerID = luaL_checkint(L, 1);
  if (lua_isnil(L, 2)) {
    bz_removeRabbit(playerID);
    return 0;
  }
  const bool swap = lua_isboolean(L, 2) && lua_toboolean(L, 2);
  bz_newRabbit(playerID, swap);
  return 0;
}


/******************************************************************************/
/******************************************************************************/

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

  const bool force = lua_isboolean(L, 3) && lua_toboolean(L, 3);
  lua_pushboolean(L, bz_givePlayerFlag(playerID, flagType, force));

  return 1;
}


/******************************************************************************/
/******************************************************************************/

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
  float pos[3];
  if (!bz_getFlagPosition(flagID, pos)) {
    return 0;
  }
  lua_pushnumber(L, pos[0]);
  lua_pushnumber(L, pos[1]);
  lua_pushnumber(L, pos[2]);
  return 3;
}


static int GetFlagPlayer(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  float pos[3] = { 0.0f, 0.0f, 0.0f };
  lua_pushinteger(L, bz_flagPlayer(flagID));
  return 1;
}


static int MoveFlag(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  float pos[3] = { 0.0f, 0.0f, 0.0f };
  pos[0] = luaL_checkfloat(L, 2);
  pos[1] = luaL_checkfloat(L, 3);
  pos[2] = luaL_checkfloat(L, 4);
  const bool reset = lua_isboolean(L, 5) ? lua_toboolean(L, 5) : true;
  bz_moveFlag(flagID, pos, reset);
  return 0;
}


static int ResetFlag(lua_State* L)
{
  const int flagID = luaL_checkint(L, 1);
  bz_resetFlag(flagID);
  return 0;
}


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/
/******************************************************************************/

static int PauseCountdown(lua_State* L)
{
  const char* playerName = luaL_checkstring(L, 1);
  bz_pauseCountdown(playerName);
  return 0;
}


static int StartCountdown(lua_State* L)
{
  const char* playerName = luaL_checkstring(L, 1);
  const int   delay = luaL_checkint(L, 2);
  const float limit = luaL_checkfloat(L, 3);
  bz_startCountdown(delay, limit, playerName);
  return 0;
}


static int ResumeCountdown(lua_State* L)
{
  const char* playerName = luaL_checkstring(L, 1);
  bz_resumeCountdown(playerName);
  return 0;
}


/******************************************************************************/
/******************************************************************************/

static int ReloadLocalBans(lua_State* L)
{
  bz_reloadLocalBans();
  return 0;
}


static int ReloadMasterBans(lua_State* L)
{
  bz_reloadMasterBans();
  return 0;
}


static int ReloadUsers(lua_State* L)
{
  bz_reloadUsers();
  return 0;
}


static int ReloadGroups(lua_State* L)
{
  bz_reloadGroups();
  return 0;
}


static int ReloadHelp(lua_State* L)
{
  bz_reloadHelp();
  return 0;
}


/******************************************************************************/
/******************************************************************************/
