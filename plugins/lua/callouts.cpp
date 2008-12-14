
#include "bzfsAPI.h"
#include "plugin_utils.h"
#include "plugin_files.h"

#include "mylua.h"

#include "callouts.h"

#include <assert.h>
#include <string>
#include <vector>
#include <set>
#include <map>
using std::string;
using std::vector;
using std::set;
using std::map;

extern const string& GetLuaDirectory(); // from lua.cpp

#ifndef uint32_t
#define uint32_t unsigned int 
#endif

// FIXME: TODO
// * obstacle queries, tangibility
// - plugin management
// - connections (new call-in?)
// - logging
// - polls
// - help


/******************************************************************************/
/******************************************************************************/


// FIXME -- move into "utils.cpp"
static bz_eTeamType ParseTeam(lua_State* L, int index)
{
  if (lua_israwstring(L, index)) {
    string s = lua_tostring(L, index);
    s = makelower(s);

    if (s == "auto")     { return eAutomaticTeam;  }
    if (s == "none")     { return eNoTeam;         }
    if (s == "rogue")    { return eRogueTeam;      }
    if (s == "red")      { return eRedTeam;        }
    if (s == "green")    { return eGreenTeam;      }
    if (s == "blue")     { return eBlueTeam;       }
    if (s == "purple")   { return ePurpleTeam;     }
    if (s == "rabbit")   { return eRabbitTeam;     }
    if (s == "hunter")   { return eHunterTeam;     }
    if (s == "observer") { return eObservers;      }
    if (s == "admin")    { return eAdministrators; }

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

static int GetLuaDirectory(lua_State* L);
static int GetPluginDirectory(lua_State* L); // only valid during loading

static int GetAPIVersion(lua_State* L);
static int GetProtocolVersion(lua_State* L);
static int GetServerVersion(lua_State* L);
static int GetServerPort(lua_State* L);
static int GetServerAddress(lua_State* L);
static int GetServerDescription(lua_State* L);

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

static int GetTeleLinkIDs(lua_State* L);
static int GetLinkTeleName(lua_State* L);
static int GetPhyDrvID(lua_State* L);
static int GetPhyDrvName(lua_State* L);

static int DebugMessage(lua_State* L);
static int GetDebugLevel(lua_State* L);

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

static int GetTimer(lua_State* L);
static int DiffTimers(lua_State* L);
static int DirList(lua_State* L);

static int CalcMD5(lua_State* L);

#ifdef HAVE_UNISTD_H
static int ReadStdin(lua_State* L);
#endif


/******************************************************************************/
/******************************************************************************/

bool CallOuts::PushEntries(lua_State* L)
{
  assert(sizeof(void*) >= sizeof(uint32_t));

#define PUSH_LUA_CFUNC(x)  \
  lua_pushliteral(L, #x);  \
  lua_pushcfunction(L, x); \
  lua_rawset(L, -3)

  PUSH_LUA_CFUNC(GetLuaDirectory);
  PUSH_LUA_CFUNC(GetPluginDirectory);

  PUSH_LUA_CFUNC(GetAPIVersion);
  PUSH_LUA_CFUNC(GetProtocolVersion);
  PUSH_LUA_CFUNC(GetServerVersion);
  PUSH_LUA_CFUNC(GetServerPort);
  PUSH_LUA_CFUNC(GetServerAddress);
  PUSH_LUA_CFUNC(GetServerDescription);

  PUSH_LUA_CFUNC(UpdateListServer);

  PUSH_LUA_CFUNC(AdminShutdown);
  PUSH_LUA_CFUNC(AdminRestart);
  PUSH_LUA_CFUNC(AdminSuperKill);
  PUSH_LUA_CFUNC(AdminGameOver);

  PUSH_LUA_CFUNC(GetGameType);
  PUSH_LUA_CFUNC(GetJumpingAllowed);

  PUSH_LUA_CFUNC(GetWallHeight);
  PUSH_LUA_CFUNC(GetWorldSize);
  PUSH_LUA_CFUNC(GetWorldURL);
  PUSH_LUA_CFUNC(GetWorldCache);
  PUSH_LUA_CFUNC(SetWallHeight);
  PUSH_LUA_CFUNC(SetWorldSize);
  PUSH_LUA_CFUNC(SetWorldURL);

  PUSH_LUA_CFUNC(GetTeleLinkIDs);
  PUSH_LUA_CFUNC(GetLinkTeleName);
  PUSH_LUA_CFUNC(GetPhyDrvID);
  PUSH_LUA_CFUNC(GetPhyDrvName);

  PUSH_LUA_CFUNC(DebugMessage);
  PUSH_LUA_CFUNC(GetDebugLevel);

  PUSH_LUA_CFUNC(SendMessage);
  PUSH_LUA_CFUNC(SendTeamMessage);
  PUSH_LUA_CFUNC(SendFetchResource);
  PUSH_LUA_CFUNC(SendJoinServer);
  PUSH_LUA_CFUNC(PlaySound);

  PUSH_LUA_CFUNC(GetStandardSpawn);
  PUSH_LUA_CFUNC(GetBaseAtPosition);

  // Player
  PUSH_LUA_CFUNC(GetPlayerCount);
  PUSH_LUA_CFUNC(GetPlayerIDs);
  PUSH_LUA_CFUNC(GetPlayerName);
  PUSH_LUA_CFUNC(GetPlayerTeam);
  PUSH_LUA_CFUNC(GetPlayerIPAddress);
  PUSH_LUA_CFUNC(GetPlayerReferrer);
  PUSH_LUA_CFUNC(GetPlayerFlagID);
  PUSH_LUA_CFUNC(GetPlayerClientVersion);
  PUSH_LUA_CFUNC(GetPlayerBZID);
  PUSH_LUA_CFUNC(GetPlayerStatus);
  PUSH_LUA_CFUNC(GetPlayerPaused);
  PUSH_LUA_CFUNC(GetPlayerPosition);
  PUSH_LUA_CFUNC(GetPlayerVelocity);
  PUSH_LUA_CFUNC(GetPlayerRotation);
  PUSH_LUA_CFUNC(GetPlayerAngVel);
  PUSH_LUA_CFUNC(GetPlayerFalling);
  PUSH_LUA_CFUNC(GetPlayerCrossingWall);
  PUSH_LUA_CFUNC(GetPlayerZoned);
  PUSH_LUA_CFUNC(GetPlayerPhysicsDriver);

  PUSH_LUA_CFUNC(GetPlayerSpawned);
  PUSH_LUA_CFUNC(GetPlayerCanSpawn);
  PUSH_LUA_CFUNC(GetPlayerAdmin);
  PUSH_LUA_CFUNC(GetPlayerOperator);
  PUSH_LUA_CFUNC(GetPlayerGroups);
  PUSH_LUA_CFUNC(GetPlayerVerified);
  PUSH_LUA_CFUNC(GetPlayerGlobalUser);
  PUSH_LUA_CFUNC(GetPlayerFlagHistory);
  PUSH_LUA_CFUNC(GetPlayerRank);


  PUSH_LUA_CFUNC(GetPlayerLag);
  PUSH_LUA_CFUNC(GetPlayerJitter);
  PUSH_LUA_CFUNC(GetPlayerPacketLoss);

  PUSH_LUA_CFUNC(GetPlayerWins);
  PUSH_LUA_CFUNC(GetPlayerLosses);
  PUSH_LUA_CFUNC(GetPlayerTKs);

  PUSH_LUA_CFUNC(SetPlayerWins);
  PUSH_LUA_CFUNC(SetPlayerLosses);
  PUSH_LUA_CFUNC(SetPlayerTKs);

  PUSH_LUA_CFUNC(GetPlayerCustomData);
  PUSH_LUA_CFUNC(SetPlayerCustomData);

  PUSH_LUA_CFUNC(ChangePlayerTeam);

  PUSH_LUA_CFUNC(ZapPlayer);
  PUSH_LUA_CFUNC(KillPlayer);

  PUSH_LUA_CFUNC(SetRabbit);

  PUSH_LUA_CFUNC(SetPlayerShotType);
  PUSH_LUA_CFUNC(SetPlayerOperator);
  PUSH_LUA_CFUNC(SetPlayerSpawnable);
  PUSH_LUA_CFUNC(SetPlayerLimboMessage);

  PUSH_LUA_CFUNC(GivePlayerFlag);


  // Flag
  PUSH_LUA_CFUNC(GetFlagCount);
  PUSH_LUA_CFUNC(GetFlagName);
  PUSH_LUA_CFUNC(GetFlagPosition);
  PUSH_LUA_CFUNC(GetFlagPlayer);

  PUSH_LUA_CFUNC(MoveFlag);
  PUSH_LUA_CFUNC(ResetFlag);
  PUSH_LUA_CFUNC(ResetFlags);

  // Team
  PUSH_LUA_CFUNC(GetTeamName);
  PUSH_LUA_CFUNC(GetTeamLimit);
  PUSH_LUA_CFUNC(GetTeamCount);
  PUSH_LUA_CFUNC(GetTeamScore);
  PUSH_LUA_CFUNC(GetTeamWins);
  PUSH_LUA_CFUNC(GetTeamLosses);

  PUSH_LUA_CFUNC(SetTeamWins);
  PUSH_LUA_CFUNC(SetTeamLosses);

  PUSH_LUA_CFUNC(FireWeapon);
  PUSH_LUA_CFUNC(FireMissile);

  PUSH_LUA_CFUNC(SaveRecBuf);
  PUSH_LUA_CFUNC(StartRecBuf);
  PUSH_LUA_CFUNC(StopRecBuf);

  PUSH_LUA_CFUNC(GetCountdownActive);
  PUSH_LUA_CFUNC(GetCountdownInProgress);
  PUSH_LUA_CFUNC(StartCountdown);
  PUSH_LUA_CFUNC(PauseCountdown);
  PUSH_LUA_CFUNC(ResumeCountdown);

  PUSH_LUA_CFUNC(ReloadLocalBans);
  PUSH_LUA_CFUNC(ReloadMasterBans);
  PUSH_LUA_CFUNC(ReloadUsers);
  PUSH_LUA_CFUNC(ReloadGroups);
  PUSH_LUA_CFUNC(ReloadHelp);

  PUSH_LUA_CFUNC(GetTimer);
  PUSH_LUA_CFUNC(DiffTimers);

  PUSH_LUA_CFUNC(GetGroups);
  PUSH_LUA_CFUNC(GetGroupPerms);
  PUSH_LUA_CFUNC(GetGroupHasPerm);
  PUSH_LUA_CFUNC(GetStandardPerms);

  PUSH_LUA_CFUNC(GetReportCount);
  PUSH_LUA_CFUNC(GetReportInfo);
  PUSH_LUA_CFUNC(GetReports);

  PUSH_LUA_CFUNC(FileReport);
  PUSH_LUA_CFUNC(ClearReport);
  PUSH_LUA_CFUNC(ClearReports);

  PUSH_LUA_CFUNC(GetLagWarn);
  PUSH_LUA_CFUNC(SetLagWarn);

  PUSH_LUA_CFUNC(ManualTimeLimit);
  PUSH_LUA_CFUNC(GetTimeLimit);
  PUSH_LUA_CFUNC(SetTimeLimit);

  PUSH_LUA_CFUNC(KickPlayer);
  PUSH_LUA_CFUNC(BanByIP);
  PUSH_LUA_CFUNC(BanByBZID);
  PUSH_LUA_CFUNC(BanByHost);
  PUSH_LUA_CFUNC(UnbanByIP);
  PUSH_LUA_CFUNC(UnbanByBZID);
  PUSH_LUA_CFUNC(UnbanByHost);

  PUSH_LUA_CFUNC(GetIPBanCount);
  PUSH_LUA_CFUNC(GetIPBanEntry);
  PUSH_LUA_CFUNC(GetBZIDBanCount);
  PUSH_LUA_CFUNC(GetBZIDBanEntry);
  PUSH_LUA_CFUNC(GetHostBanCount);
  PUSH_LUA_CFUNC(GetHostBanEntry);

  PUSH_LUA_CFUNC(GetMaxWaitTime);
  PUSH_LUA_CFUNC(SetMaxWaitTime);
  PUSH_LUA_CFUNC(ClearMaxWaitTime);

  PUSH_LUA_CFUNC(GetTimer);
  PUSH_LUA_CFUNC(DiffTimers);
  PUSH_LUA_CFUNC(DirList);

  PUSH_LUA_CFUNC(CalcMD5);

#ifdef HAVE_UNISTD_H
  PUSH_LUA_CFUNC(ReadStdin);
#endif

  return true;
}


/******************************************************************************/
/******************************************************************************/

static int GetLuaDirectory(lua_State* L)
{
  lua_pushstring(L, GetLuaDirectory().c_str());
  return 1;
}


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


static int GetProtocolVersion(lua_State* L)
{
  lua_pushstring(L, bz_getProtocolVersion());
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
  lua_pushstring(L, bz_getPublicAddr().c_str());
  return 1;
}


static int GetServerDescription(lua_State* L)
{
  lua_pushstring(L, bz_getPublicDescription().c_str());
  return 1;
}

/******************************************************************************/
/******************************************************************************/

static int UpdateListServer(lua_State* L)
{
  bz_updateListServer();
  return 0;
}


static int AdminShutdown(lua_State* L)
{
  bz_shutdown();
  return 0;
}


static int AdminRestart(lua_State* L)
{
  lua_pushboolean(L, bz_restart());
  return 1;
}


static int AdminSuperKill(lua_State* L)
{
  bz_superkill();
  return 0;
}


static int AdminGameOver(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  bz_eTeamType team = eNoTeam;
  if (!lua_isnone(L, 2)) {
    team = ParseTeam(L, 2);
  }
  bz_gameOver(playerID, team);
  return 0;
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
//  const char* url;
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

static int GetTeleLinkIDs(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  int frontLink, backLink;
  const int id = bz_getTeleLinkIDs(name, &frontLink, &backLink);
  if (id < 0) {
    return 0;
  }
  lua_pushinteger(L, frontLink);
  lua_pushinteger(L, backLink);
  return 2;
}


static int GetLinkTeleName(lua_State* L)
{
  const int linkID = luaL_checkint(L, 1);
  const char* name = bz_getLinkTeleName(linkID);
  if (name == NULL) {
    return 0;
  }
  lua_pushstring(L, name);
  return 1;
}


static int GetPhyDrvID(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  const int id = bz_getPhyDrvID(name);
  if (id < 0) {
    return 0;
  }
  lua_pushinteger(L, id);
  return 1;
}


static int GetPhyDrvName(lua_State* L)
{
  const int id = luaL_checkint(L, 1);
  const char* name = bz_getPhyDrvName(id);
  if (name == NULL) {
    return 0;
  }
  lua_pushstring(L, name);
  return 1;
}


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/
/******************************************************************************/

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
  const int   team     = luaL_optint(L, 4, eNoTeam);
  const char* referrer = luaL_optstring(L, 5, bz_getPublicAddr().c_str());
  const char* message  = luaL_optstring(L, 6, NULL);
  lua_pushboolean(L, bz_sendJoinServer(playerID, addr, port,
                                       team, referrer, message));
  return 1;
}


static int PlaySound(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  const char* sound  = luaL_checkstring(L, 2);
  lua_pushboolean(L, bz_sendPlayCustomLocalSound(playerID, sound));
  return 1;
}


/******************************************************************************/
/******************************************************************************/

static int GetStandardSpawn(lua_State* L)
{
  const int playerID = luaL_checkint(L, 1);
  float pos[3];
  float rot;
  if (!bz_getStandardSpawn(playerID, pos, &rot)) {
    return 0;
  }
  lua_pushnumber(L, pos[0]);
  lua_pushnumber(L, pos[1]);
  lua_pushnumber(L, pos[2]);
  lua_pushnumber(L, rot);
  return 4;
}


static int GetBaseAtPosition(lua_State* L)
{
  float pos[3];
  pos[0] = luaL_checkfloat(L, 1);
  pos[1] = luaL_checkfloat(L, 2);
  pos[2] = luaL_checkfloat(L, 3);
  const bz_eTeamType team = bz_checkBaseAtPoint(pos);
  lua_pushinteger(L, team);
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
  bz_APIIntList* playerList = bz_newIntList();
  if (!bz_getPlayerIndexList(playerList)) {
    lua_createtable(L, 0, 0);
    bz_deleteIntList(playerList);
    return 1;
  }
  lua_createtable(L, 0, playerList->size());
  for (unsigned int i = 0; i < playerList->size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushinteger(L, playerList->get(i));
    lua_rawset(L, -3);
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


/******************************************************************************/
/******************************************************************************/

static int GetPlayerStatus(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) { // FIXME -- slow
    return 0;
  }
  lua_pushinteger(L, state.status);
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
  const bool extrapolate = lua_isboolean(L, 2) && lua_tobool(L, 2);

  float pos[3];
  if (!bz_getPlayerPosition(pid, pos, extrapolate)) {
    return 0;
  }

  lua_pushnumber(L, pos[0]);
  lua_pushnumber(L, pos[1]);
  lua_pushnumber(L, pos[2]);

  return 3;
}


static int GetPlayerVelocity(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);

  float vel[3];
  if (!bz_getPlayerVelocity(pid, vel)) {
    return 0;
  }

  lua_pushnumber(L, vel[0]);
  lua_pushnumber(L, vel[1]);
  lua_pushnumber(L, vel[2]);

  return 3;
}


static int GetPlayerRotation(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  const bool extrapolate = lua_isboolean(L, 2) && lua_tobool(L, 2);

  float rot;
  if (!bz_getPlayerRotation(pid, &rot, extrapolate)) {
    return 0;
  }

  lua_pushnumber(L, rot);

  return 1;
}


static int GetPlayerAngVel(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);

  float angVel;
  if (!bz_getPlayerAngVel(pid, &angVel)) {
    return 0;
  }

  lua_pushnumber(L, angVel);

  return 1;
}


static int GetPlayerFalling(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) { // FIXME -- slow
    return 0;
  }
  lua_pushboolean(L, state.falling);
  return 1;
}


static int GetPlayerCrossingWall(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) { // FIXME -- slow
    return 0;
  }
  lua_pushboolean(L, state.crossingWall);
  return 1;
}


static int GetPlayerZoned(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_PlayerUpdateState state;
  if (!bz_getPlayerCurrentState(pid, state)) { // FIXME -- slow
    return 0;
  }
  lua_pushboolean(L, state.inPhantomZone);
  return 1;
}


static int GetPlayerPhysicsDriver(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);

  int phydrv;
  if (!bz_getPlayerPhysicsDriver(pid, &phydrv)) {
    return 0;
  }

  lua_pushinteger(L, phydrv);

  return 1;
}


static int GetPlayerSpawned(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->spawned);
  bz_freePlayerRecord(player);
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
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->op);
  bz_freePlayerRecord(player);
  return 1;
}


static int GetPlayerGroups(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_newtable(L);
  for (unsigned int i = 0; i < player->groups.size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, player->groups[i].c_str());
    lua_rawset(L, -3);
  }
  bz_freePlayerRecord(player);
  return 1;
}


static int GetPlayerRank(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_pushnumber(L, player->rank);
  bz_freePlayerRecord(player);
  return 1;
}


static int GetPlayerVerified(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->verified);
  bz_freePlayerRecord(player);
  return 1;
}


static int GetPlayerGlobalUser(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_pushboolean(L, player->globalUser);
  bz_freePlayerRecord(player);
  return 1;
}


static int GetPlayerFlagHistory(lua_State* L)
{
  const int pid = luaL_checkint(L, 1);
  bz_BasePlayerRecord* player = bz_getPlayerByIndex(pid); // FIXME -- slow
  if (player == NULL) {
    return 0;
  }
  lua_newtable(L);
  for (unsigned int i = 0; i < player->flagHistory.size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, player->flagHistory[i].c_str());
    lua_rawset(L, -3);
  }
  bz_freePlayerRecord(player);
  return 1;
}


/******************************************************************************/
/******************************************************************************/

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
  lua_pushnumber(L, bz_getPlayerPacketLoss(playerID));
  return 1;
}


/******************************************************************************/
/******************************************************************************/

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

static int GetPlayerCustomData(lua_State* L)
{
  const int    pid =    luaL_checkint(L, 1);
  const string key = luaL_checkstring(L, 2);
  const char* data = bz_getPlayerCustomData(pid, key.c_str());
  if (data == NULL) {
    return 0;
  }
  lua_pushstring(L, data);
  return 1;
}


static int SetPlayerCustomData(lua_State* L)
{
  const int    pid  =    luaL_checkint(L, 1);
  const string key  = luaL_checkstring(L, 2);
  const string data = luaL_checkstring(L, 3);
  lua_pushboolean(L, bz_setPlayerCustomData(pid, key.c_str(), data.c_str()));
  return 1;
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
  const bool spawnOnBase = !lua_isboolean(L, 2) || lua_tobool(L, 2);
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
  const bool swap = lua_isboolean(L, 2) && lua_tobool(L, 2);
  bz_newRabbit(playerID, swap);
  return 0;
}

/******************************************************************************/
/******************************************************************************/

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

  const bool force = lua_isboolean(L, 3) && lua_tobool(L, 3);
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


/******************************************************************************/
/******************************************************************************/

static int GetTeamName(lua_State* L)
{
  bz_eTeamType teamID = ParseTeam(L, 1);
  lua_pushstring(L, bzu_GetTeamName(teamID));
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


/******************************************************************************/
/******************************************************************************/

static int FireWeapon(lua_State* L)
{
  float pos[3];
  const char* flagType = luaL_checkstring(L, 1);
  pos[0]               = luaL_checkfloat(L, 2);
  pos[1]               = luaL_checkfloat(L, 3);
  pos[2]               = luaL_checkfloat(L, 4);
  const float rot      = luaL_optfloat(L, 5, 0.0f);
  const float tilt     = luaL_optfloat(L, 6, 0.0f);
  const float lifeTime = luaL_optfloat(L, 7, -1.0f);
  const float dt       = luaL_optfloat(L, 8, -1.0f);
  const int   shotID   = luaL_optint(L, 9, -1);
  
  lua_pushboolean(L, bz_fireWorldWep(flagType, lifeTime,
                                     pos, tilt, rot, shotID , dt));
  return 1;
}


static int FireMissile(lua_State* L)
{
  float pos[3];
  const int   targetID = luaL_checkint(L, 1);
  pos[0]               = luaL_checkfloat(L, 2);
  pos[1]               = luaL_checkfloat(L, 3);
  pos[2]               = luaL_checkfloat(L, 4);
  const float rot      = luaL_checkfloat(L, 5);
  const float tilt     = luaL_checkfloat(L, 6);
  const float lifeTime = luaL_checkfloat(L, 7);
  const float dt       = luaL_checkfloat(L, 8);
  
  lua_pushinteger(L, bz_fireWorldGM(targetID, lifeTime, pos, tilt, rot, dt));
  return 1;
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

static int GetGroups(lua_State* L)
{
  bz_APIStringList* list = bz_getGroupList();
  if (list == NULL) {
    lua_createtable(L, 0, 0);
    return 1;
  }
  lua_createtable(L, list->size(), 0);
  for (unsigned int i = 0; i < list->size(); i++) {
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, (*list)[i].c_str());
    lua_rawset(L, -3);
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
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, (*list)[i].c_str());
    lua_rawset(L, -3);
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
    lua_pushinteger(L, i + 1);
    lua_pushstring(L, (*list)[i].c_str());
    lua_rawset(L, -3);
  }
  bz_deleteStringList(list);
  return 1;
}


/******************************************************************************/
/******************************************************************************/

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
    lua_pushinteger(L, i + 1);    
    lua_pushstring(L, list->get(i).c_str());
    lua_rawset(L, -3);
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


/******************************************************************************/
/******************************************************************************/

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


/******************************************************************************/
/******************************************************************************/

static int ManualTimeLimit(lua_State* L)
{
  lua_pushboolean(L, bz_isTimeManualStart());
  return 1;
}


static int GetTimeLimit(lua_State* L)
{
  lua_pushnumber(L, bz_getTimeLimit());
  return 1;
}


static int SetTimeLimit(lua_State* L)
{
  const float timeLimit = luaL_checkfloat(L, 1);
  lua_pushboolean(L, bz_setTimeLimit(timeLimit));
  return 1;
}


/******************************************************************************/
/******************************************************************************/

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

/******************************************************************************/
/******************************************************************************/

static int GetBanCount(lua_State* L, bz_eBanListType listType)
{
  lua_pushnumber(L, bz_getBanListSize(listType));
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
  lua_pushnumber(L, bz_getBanItemDuration(listType, entry));
  lua_rawset(L, -3);
  lua_pushstring(L, "fromMaster");
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


/******************************************************************************/
/******************************************************************************/

static int GetMaxWaitTime(lua_State* L)
{
  const char* name = luaL_checkstring(L, 1);
  lua_pushnumber(L, bz_getMaxWaitTime(name));
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


/******************************************************************************/
/******************************************************************************/

static int GetTimer(lua_State* L)
{
  const double nowTime = bz_getCurrentTime();
  const uint32_t millisecs = (uint32_t)(nowTime * 1000.0);
  lua_pushlightuserdata(L, (void*)millisecs);
  return 1;
}


static int DiffTimers(lua_State* L)
{
  const int args = lua_gettop(L); // number of arguments
  if ((args != 2) || !lua_isuserdata(L, 1) || !lua_isuserdata(L, 2)) {
    luaL_error(L, "Incorrect arguments to DiffTimers()");
  }
  const void* p1 = lua_touserdata(L, 1);
  const void* p2 = lua_touserdata(L, 2);
  const uint32_t t1 = *((const uint32_t*)&p1);
  const uint32_t t2 = *((const uint32_t*)&p2);
  const uint32_t diffTime = (t1 - t2);
  lua_pushnumber(L, (float)diffTime * 0.001f); // return seconds
  return 1;
}


static int DirList(lua_State* L)
{
  const char* path = luaL_checkstring(L, 1);

  vector<string> dirs  = getDirsInDir(path);
  set<string> dirSet;
  for (unsigned int i = 0; i < dirs.size(); i++) {
    dirSet.insert(dirs[i]);
  }

  vector<string> files = getFilesInDir(path, NULL, false); // not recursive
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
    lua_pushinteger(L, fileCount);
    lua_pushstring(L, (*fit).c_str());
    lua_rawset(L, -3);
  }

  // dirs table
  lua_createtable(L, (int)dirSet.size(), 0);
  set<string>::const_iterator dit;
  int dirCount = 0;
  for (dit = dirSet.begin(); dit != dirSet.end(); ++dit) {
    dirCount++;
    lua_pushinteger(L, dirCount);
    lua_pushstring(L, (*dit + "/").c_str());
    lua_rawset(L, -3);
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
#ifdef HAVE_UNISTD_H
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


/******************************************************************************/
/******************************************************************************/
