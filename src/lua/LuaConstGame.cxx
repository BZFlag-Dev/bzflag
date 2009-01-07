
#include "common.h"

// implementation header
#include "LuaConstGame.h"

// common headers
#include "global.h"
#include "Flag.h"
#include "Address.h"
#include "AnsiCodes.h"
#include "Obstacle.h"
#include "GfxBlock.h"

// local headers
#include "LuaInclude.h"
#include "LuaUtils.h"


/******************************************************************************/
/******************************************************************************/

static bool PushGameTypes(lua_State* L);
static bool PushPlayerTypes(lua_State* L);
static bool PushShotTypes(lua_State* L);
static bool PushTeams(lua_State* L);
static bool PushChatTeams(lua_State* L);
static bool PushAnsiCodes(lua_State* L);
static bool PushObstacleTypes(lua_State* L);
static bool PushGfxBlockTypes(lua_State* L);
//static bool PushFlagQualities(lua_State* L);
//static bool PushPlayers(lua_State* L);
//static bool PushPlayerStates(lua_State* L);
//static bool PushObstacles(lua_State* L);
//static bool PushPermissions(lua_State* L);


/******************************************************************************/
/******************************************************************************/

bool LuaConstGame::PushEntries(lua_State* L)
{
	return
		PushGameTypes(L)     &&
		PushPlayerTypes(L)   &&
		PushTeams(L)         &&
		PushChatTeams(L)     &&
		PushShotTypes(L)     &&
		PushAnsiCodes(L)     &&
		PushObstacleTypes(L) &&
		PushGfxBlockTypes(L);
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

static bool PushGameTypes(lua_State* L)
{
  lua_pushliteral(L, "GAME");
  lua_newtable(L);

  PushDualPair(L, "ffa",     TeamFFA);
  PushDualPair(L, "ctf",     ClassicCTF);
  PushDualPair(L, "openffa", OpenFFA);
  PushDualPair(L, "rabbit",  RabbitChase);

  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushTeams(lua_State* L)
{
  lua_pushliteral(L, "TEAM");
  lua_newtable(L);

  PushDualPair(L, "auto",     AutomaticTeam);
  PushDualPair(L, "none",     NoTeam);
  PushDualPair(L, "rogue",    RogueTeam);
  PushDualPair(L, "red",      RedTeam);
  PushDualPair(L, "green",    GreenTeam);
  PushDualPair(L, "blue",     BlueTeam);
  PushDualPair(L, "purple",   PurpleTeam);
  PushDualPair(L, "rabbit",   RabbitTeam);
  PushDualPair(L, "hunter",   HunterTeam);
  PushDualPair(L, "observer", ObserverTeam);

  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushChatTeams(lua_State* L)
{
  lua_pushliteral(L, "TEAM");
  lua_newtable(L);

  PushDualPair(L, "none",     NoPlayer);
  PushDualPair(L, "all",      AllPlayers);
  PushDualPair(L, "server",   ServerPlayer);
  PushDualPair(L, "admin",    AdminPlayers);

  const int topTeam = 250;

  PushDualPair(L, "rogue",    topTeam - RogueTeam);
  PushDualPair(L, "red",      topTeam - RedTeam);
  PushDualPair(L, "green",    topTeam - GreenTeam);
  PushDualPair(L, "blue",     topTeam - BlueTeam);
  PushDualPair(L, "purple",   topTeam - PurpleTeam);
  PushDualPair(L, "rabbit",   topTeam - RabbitTeam);
  PushDualPair(L, "hunter",   topTeam - HunterTeam);
  PushDualPair(L, "observer", topTeam - ObserverTeam);

  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushShotTypes(lua_State* L)
{
  lua_pushliteral(L, "SHOT");
  lua_newtable(L);

	PushDualPair(L, "no",         NoShot);
	PushDualPair(L, "standard",   StandardShot);
	PushDualPair(L, "gm",         GMShot);
	PushDualPair(L, "laser",      LaserShot);
	PushDualPair(L, "thief",      ThiefShot);
	PushDualPair(L, "super",      SuperShot);
	PushDualPair(L, "phantom",    PhantomShot);
	PushDualPair(L, "shockWave",  ShockWaveShot);
	PushDualPair(L, "rico",       RicoShot);
	PushDualPair(L, "machineGun", MachineGunShot);
	PushDualPair(L, "invisible",  InvisibleShot);
	PushDualPair(L, "cloaked",    CloakedShot);
	PushDualPair(L, "rapidFire",  RapidFireShot);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushPlayerTypes(lua_State* L)
{
  lua_pushliteral(L, "PLAYER");
  lua_newtable(L);

	PushDualPair(L, "tank",     TankPlayer);
	PushDualPair(L, "computer", ComputerPlayer);
	PushDualPair(L, "chat",     ChatPlayer);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushAnsiCodes(lua_State* L)
{
  lua_pushliteral(L, "ANSI");
  lua_newtable(L);

	LuaPushNamedString(L, "RESET",        ANSI_STR_RESET_FINAL);
	LuaPushNamedString(L, "RESET_BRIGHT", ANSI_STR_RESET);            
	LuaPushNamedString(L, "BRIGHT",       ANSI_STR_BRIGHT);
	LuaPushNamedString(L, "DIM",          ANSI_STR_DIM);
	LuaPushNamedString(L, "UNDERLINE",    ANSI_STR_UNDERLINE);
	LuaPushNamedString(L, "NO_UNDERLINE", ANSI_STR_NO_UNDERLINE);
	LuaPushNamedString(L, "PULSATING",    ANSI_STR_PULSATING);
	LuaPushNamedString(L, "NO_PULSATE",   ANSI_STR_NO_PULSATE);
	LuaPushNamedString(L, "REVERSE",      ANSI_STR_REVERSE);
	LuaPushNamedString(L, "NO_REVERSE",   ANSI_STR_NO_REVERSE);

	LuaPushNamedString(L, "BLACK",   ANSI_STR_FG_BLACK);
	LuaPushNamedString(L, "RED",     ANSI_STR_FG_RED);
	LuaPushNamedString(L, "GREEN",   ANSI_STR_FG_GREEN);
	LuaPushNamedString(L, "YELLOW",  ANSI_STR_FG_YELLOW);
	LuaPushNamedString(L, "BLUE",    ANSI_STR_FG_BLUE);
	LuaPushNamedString(L, "MAGENTA", ANSI_STR_FG_MAGENTA);
	LuaPushNamedString(L, "CYAN",    ANSI_STR_FG_CYAN);
	LuaPushNamedString(L, "WHITE",   ANSI_STR_FG_WHITE);
	LuaPushNamedString(L, "ORANGE",  ANSI_STR_FG_ORANGE);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushObstacleTypes(lua_State* L)
{
  lua_pushliteral(L, "OBSTACLE");
  lua_newtable(L);

  PushDualPair(L, "wall",   wallType);
  PushDualPair(L, "box",    boxType);
  PushDualPair(L, "pyr",    pyrType);
  PushDualPair(L, "base",   baseType);
  PushDualPair(L, "tele",   teleType);
  PushDualPair(L, "mesh",   meshType);
  PushDualPair(L, "arc",    arcType);
  PushDualPair(L, "cone",   coneType);
  PushDualPair(L, "sphere", sphereType);
  PushDualPair(L, "tetra",  tetraType);

  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushGfxBlockTypes(lua_State* L)
{
  lua_pushliteral(L, "GFXBLOCKTYPE");
  lua_newtable(L);
  for (int i = 0; i < GfxBlock::BlockTypeCount; i++) {
    PushDualPair(L, GfxBlock::getTypeString(i), i);
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "GFXBLOCKID");
  lua_newtable(L);
  for (int i = 0; i < GfxBlockMgr::BlockIDCount; i++) {
    PushDualPair(L, GfxBlockMgr::getIDString(i), i);
  }
  lua_rawset(L, -3);

  return true;
}


/******************************************************************************/
/******************************************************************************/
