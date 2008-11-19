
#include "bzfsAPI.h"
#include "plugin_utils.h"

#include "mylua.h"

#include "constants.h"

#include <string>


static bool PushGames(lua_State* L);
static bool PushTeams(lua_State* L);
static bool PushShots(lua_State* L);
static bool PushFlagQualities(lua_State* L);
static bool PushPlayerStates(lua_State* L);
static bool PushObstacles(lua_State* L);
static bool PushPermissions(lua_State* L);

// FIXME -- use userdata/metatables to make these values truly constant

/******************************************************************************/
/******************************************************************************/

bool Constants::PushEntries(lua_State* L)
{
  PushGames(L);
  PushTeams(L);
  PushShots(L);
  PushFlagQualities(L);
  PushPlayerStates(L);
  PushObstacles(L);
  PushPermissions(L);

  return true;
}


/******************************************************************************/
/******************************************************************************/

static void PushDualPair(lua_State* L, const char* name, int code)
{
  lua_pushstring(L, name);
  lua_pushinteger(L, code);
  lua_rawset(L, -3);
  lua_pushinteger(L, code);
  lua_pushstring(L, name);
  lua_rawset(L, -3);
}


/******************************************************************************/
/******************************************************************************/

static bool PushGames(lua_State* L)
{
  lua_pushliteral(L, "GAME");
  lua_newtable(L);

  PushDualPair(L, "ffa",    eTeamFFAGame);
  PushDualPair(L, "ctf",    eClassicCTFGame);
  PushDualPair(L, "rabbit", eRabbitGame);
  PushDualPair(L, "open",   eOpenFFAGame);

  lua_rawset(L, -3);

  return true;
}


static bool PushTeams(lua_State* L)
{
  lua_pushliteral(L, "TEAM");
  lua_newtable(L);

  PushDualPair(L, "automatic", eAutomaticTeam);
  PushDualPair(L, "none",      eNoTeam);
  PushDualPair(L, "rogue",     eRogueTeam);
  PushDualPair(L, "red",       eRedTeam);
  PushDualPair(L, "green",     eGreenTeam);
  PushDualPair(L, "blue",      eBlueTeam);
  PushDualPair(L, "purple",    ePurpleTeam);
  PushDualPair(L, "rabbit",    eRabbitTeam);
  PushDualPair(L, "hunter",    eHunterTeam);
  PushDualPair(L, "observer",  eObservers);
  PushDualPair(L, "admin",     eAdministrators);

  lua_rawset(L, -3);

  return true;
}


static bool PushShots(lua_State* L)
{
  lua_pushliteral(L, "SHOT");
  lua_newtable(L);

  PushDualPair(L, "no",  eNoShot);
  PushDualPair(L, "std", eStandardShot);
  PushDualPair(L, "gm",  eGMShot);
  PushDualPair(L, "l",   eLaserShot);
  PushDualPair(L, "th",  eThiefShot);
  PushDualPair(L, "sb",  eSuperShot);
  PushDualPair(L, "pz",  ePhantomShot);
  PushDualPair(L, "sw",  eShockWaveShot);
  PushDualPair(L, "r",   eRicoShot);
  PushDualPair(L, "mg",  eMachineGunShot);
  PushDualPair(L, "ib",  eInvisibleShot);
  PushDualPair(L, "cl",  eCloakedShot);
  PushDualPair(L, "r",   eRapidFireShot);

  lua_rawset(L, -3);

  return true;
}


static bool PushObstacles(lua_State* L)
{
  lua_pushliteral(L, "OBSTACLE");
  lua_newtable(L);

  PushDualPair(L, "wall",    eWallObject);
  PushDualPair(L, "box",     eBoxObject);
  PushDualPair(L, "base",    eBaseObject);
  PushDualPair(L, "pyramid", ePyramidObject);
  PushDualPair(L, "mesh",    eMeshObject);
  PushDualPair(L, "arc",     eArcObject);
  PushDualPair(L, "cone",    eConeObject);
  PushDualPair(L, "sphere",  eSphereObject);
  PushDualPair(L, "tetra",   eTetraObject);
  PushDualPair(L, "unknown", eUnknownObject);

  lua_rawset(L, -3);

  return true;
}


static bool PushFlagQualities(lua_State* L)
{
  lua_pushliteral(L, "QUALITY");
  lua_newtable(L);

  PushDualPair(L, "good", eGoodFlag);
  PushDualPair(L, "bad",  eBadFlag);

  lua_rawset(L, -3);

  return true;
}


static bool PushPlayerStates(lua_State* L)
{
  lua_pushliteral(L, "STATUS");
  lua_newtable(L);

  PushDualPair(L, "dead",        eDead);
  PushDualPair(L, "alive",       eAlive);
  PushDualPair(L, "paused",      ePaused);
  PushDualPair(L, "exploding",   eExploding);
  PushDualPair(L, "teleporting", eTeleporting);
  PushDualPair(L, "inbuilding",  eInBuilding);

  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/

static bool PushPermissions(lua_State* L)
{
  lua_pushliteral(L, "PERM");
  lua_newtable(L);

#define ADD_PERM(x)                  \
  lua_pushliteral(L, #x);            \
  lua_pushliteral(L, bz_perm_ ## x); \
  lua_rawset(L, -3);

  ADD_PERM(actionMessage);
  ADD_PERM(adminMessageReceive);
  ADD_PERM(adminMessageSend);
  ADD_PERM(antiban);
  ADD_PERM(antikick);
  ADD_PERM(antikill);
  ADD_PERM(antipoll);
  ADD_PERM(antipollban);
  ADD_PERM(antipollkick);
  ADD_PERM(antipollkill);
  ADD_PERM(ban);
  ADD_PERM(banlist);
  ADD_PERM(countdown);
  ADD_PERM(date);
  ADD_PERM(endGame);
  ADD_PERM(flagHistory);
  ADD_PERM(flagMod);
  ADD_PERM(hideAdmin);
  ADD_PERM(idleStats);
  ADD_PERM(info);
  ADD_PERM(kick);
  ADD_PERM(kill);
  ADD_PERM(lagStats);
  ADD_PERM(lagwarn);
  ADD_PERM(listPlugins);
  ADD_PERM(listPerms);
  ADD_PERM(masterBan);
  ADD_PERM(mute);
  ADD_PERM(playerList);
  ADD_PERM(poll);
  ADD_PERM(pollBan);
  ADD_PERM(pollKick);
  ADD_PERM(pollKill);
  ADD_PERM(pollSet);
  ADD_PERM(pollFlagReset);
  ADD_PERM(privateMessage);
  ADD_PERM(record);
  ADD_PERM(rejoin);
  ADD_PERM(removePerms);
  ADD_PERM(replay);
  ADD_PERM(say);
  ADD_PERM(sendHelp);
  ADD_PERM(setAll);
  ADD_PERM(setPerms);
  ADD_PERM(setVar);
  ADD_PERM(showOthers);
  ADD_PERM(shortBan);
  ADD_PERM(shutdownServer);
  ADD_PERM(spawn);
  ADD_PERM(superKill);
  ADD_PERM(talk);
  ADD_PERM(unban);
  ADD_PERM(unmute);
  ADD_PERM(veto);
  ADD_PERM(viewReports);
  ADD_PERM(vote);

  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/
/******************************************************************************/
