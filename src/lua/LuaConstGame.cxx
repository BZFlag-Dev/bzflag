
#include "common.h"

// interface header
#include "LuaConstGame.h"

// common headers
#include "global.h"
#include "Flag.h"
#include "Address.h"
#include "AnsiCodes.h"
#include "Obstacle.h"
#include "GfxBlock.h"
#include "PlayerState.h"
#include "Protocol.h"

// bzflag headers
#include "../bzflag/Roaming.h"
#include "../bzflag/ControlPanel.h"

// local headers
#include "LuaInclude.h"
#include "LuaUtils.h"


/******************************************************************************/
/******************************************************************************/

static bool PushGameTypes(lua_State* L);
static bool PushPlayerTypes(lua_State* L);
static bool PushPlayerStateBits(lua_State* L);
static bool PushShotTypes(lua_State* L);
static bool PushTeams(lua_State* L);
static bool PushChatTeams(lua_State* L);
static bool PushAnsiCodes(lua_State* L);
static bool PushObstacleTypes(lua_State* L);
static bool PushGfxBlockTypes(lua_State* L);
static bool PushRoamModes(lua_State* L);
static bool PushConsoleTypes(lua_State* L);
static bool PushMouseButtons(lua_State* L);
static bool PushFlagStates(lua_State* L);
static bool PushFlagQualities(lua_State* L);
static bool PushFlagEndurance(lua_State* L);
static bool PushKilledReasons(lua_State* L);
//static bool PushPlayers(lua_State* L);
//static bool PushPermissions(lua_State* L);


/******************************************************************************/
/******************************************************************************/

bool LuaConstGame::PushEntries(lua_State* L)
{
	return
		PushGameTypes(L)       &&
		PushPlayerTypes(L)     &&
		PushPlayerStateBits(L) &&
		PushTeams(L)           &&
		PushChatTeams(L)       &&
		PushShotTypes(L)       &&
		PushAnsiCodes(L)       &&
		PushObstacleTypes(L)   &&
		PushGfxBlockTypes(L)   &&
		PushRoamModes(L)       &&
		PushConsoleTypes(L)    &&
		PushMouseButtons(L)    &&
		PushFlagStates(L)      &&
		PushFlagQualities(L)   &&
		PushFlagEndurance(L)   &&
		PushKilledReasons(L);
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

	PushDualPair(L, "FFA",     TeamFFA);
	PushDualPair(L, "CTF",     ClassicCTF);
	PushDualPair(L, "OPENFFA", OpenFFA);
	PushDualPair(L, "RABBIT",  RabbitChase);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushTeams(lua_State* L)
{
	lua_pushliteral(L, "TEAM");
	lua_newtable(L);

	PushDualPair(L, "AUTO",     AutomaticTeam);
	PushDualPair(L, "NONE",     NoTeam);
	PushDualPair(L, "ROGUE",    RogueTeam);
	PushDualPair(L, "RED",      RedTeam);
	PushDualPair(L, "GREEN",    GreenTeam);
	PushDualPair(L, "BLUE",     BlueTeam);
	PushDualPair(L, "PURPLE",   PurpleTeam);
	PushDualPair(L, "RABBIT",   RabbitTeam);
	PushDualPair(L, "HUNTER",   HunterTeam);
	PushDualPair(L, "OBSERVER", ObserverTeam);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushChatTeams(lua_State* L)
{
	lua_pushliteral(L, "CHAT_TEAM");
	lua_newtable(L);

	PushDualPair(L, "NONE",     NoPlayer);
	PushDualPair(L, "ALL",      AllPlayers);
	PushDualPair(L, "SERVER",   ServerPlayer);
	PushDualPair(L, "ADMIN",    AdminPlayers);

	const int topTeam = 250;
	PushDualPair(L, "TOP", topTeam);

	PushDualPair(L, "ROGUE",    topTeam - RogueTeam);
	PushDualPair(L, "RED",      topTeam - RedTeam);
	PushDualPair(L, "GREEN",    topTeam - GreenTeam);
	PushDualPair(L, "BLUE",     topTeam - BlueTeam);
	PushDualPair(L, "PURPLE",   topTeam - PurpleTeam);
	PushDualPair(L, "RABBIT",   topTeam - RabbitTeam);
	PushDualPair(L, "HUNTER",   topTeam - HunterTeam);
	PushDualPair(L, "OBSERVER", topTeam - ObserverTeam);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushShotTypes(lua_State* L)
{
	lua_pushliteral(L, "SHOT");
	lua_newtable(L);

	PushDualPair(L, "NONE",       NoShot);
	PushDualPair(L, "STANDARD",   StandardShot);
	PushDualPair(L, "GM",         GMShot);
	PushDualPair(L, "LASER",      LaserShot);
	PushDualPair(L, "THIEF",      ThiefShot);
	PushDualPair(L, "SUPER",      SuperShot);
	PushDualPair(L, "PHANTOM",    PhantomShot);
	PushDualPair(L, "SHOCKWAVE",  ShockWaveShot);
	PushDualPair(L, "RICO",       RicoShot);
	PushDualPair(L, "MACHINEGUN", MachineGunShot);
	PushDualPair(L, "INVISIBLE",  InvisibleShot);
	PushDualPair(L, "CLOAKED",    CloakedShot);
	PushDualPair(L, "RAPIDFIRE",  RapidFireShot);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushPlayerTypes(lua_State* L)
{
	lua_pushliteral(L, "PLAYER_TYPE");
	lua_newtable(L);

	PushDualPair(L, "TANK",     TankPlayer);
	PushDualPair(L, "COMPUTER", ComputerPlayer);
	PushDualPair(L, "CHAT",     ChatPlayer);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushPlayerStateBits(lua_State* L)
{
	lua_pushliteral(L, "PLAYER_BIT");
	lua_newtable(L);

	PushDualPair(L, "ALIVE",         PlayerState::Alive);
	PushDualPair(L, "PAUSED",        PlayerState::Paused);
	PushDualPair(L, "EXPLODING",     PlayerState::Exploding);
	PushDualPair(L, "TELEPORTING",   PlayerState::Teleporting);
	PushDualPair(L, "FLAG_ACTIVE",   PlayerState::FlagActive);
	PushDualPair(L, "CROSSING_WALL", PlayerState::CrossingWall);
	PushDualPair(L, "FALLING",       PlayerState::Falling);
	PushDualPair(L, "ON_DRIVER",     PlayerState::OnDriver);
	PushDualPair(L, "USER_INPUTS",   PlayerState::UserInputs);
	PushDualPair(L, "JUMP_JETS",     PlayerState::JumpJets);
	PushDualPair(L, "PLAY_SOUND",    PlayerState::PlaySound);
	PushDualPair(L, "ZONED",         PlayerState::PhantomZoned);
	PushDualPair(L, "IN_BUILDING",   PlayerState::InBuilding);
	PushDualPair(L, "BACKED_OFF",    PlayerState::BackedOff);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushAnsiCodes(lua_State* L)
{
	lua_pushliteral(L, "ANSI_COLOR");
	lua_newtable(L);

	LuaPushNamedString(L, "RESET",        ANSI_STR_RESET_FINAL);
	LuaPushNamedString(L, "RESET_BRIGHT", ANSI_STR_RESET);            
	LuaPushNamedString(L, "BRIGHT",       ANSI_STR_BRIGHT);
	LuaPushNamedString(L, "DIM",          ANSI_STR_DIM);
	LuaPushNamedString(L, "UNDERLINE",    ANSI_STR_UNDERLINE);
	LuaPushNamedString(L, "NO_UNDERLINE", ANSI_STR_NO_UNDERLINE);
	LuaPushNamedString(L, "BLINK",        ANSI_STR_PULSATING);
	LuaPushNamedString(L, "NO_BLINK",     ANSI_STR_NO_PULSATE);
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

	PushDualPair(L, "WALL",   wallType);
	PushDualPair(L, "BOX",    boxType);
	PushDualPair(L, "PYR",    pyrType);
	PushDualPair(L, "BASE",   baseType);
	PushDualPair(L, "TELE",   teleType);
	PushDualPair(L, "MESH",   meshType);
	PushDualPair(L, "ARC",    arcType);
	PushDualPair(L, "CONE",   coneType);
	PushDualPair(L, "SPHERE", sphereType);
	PushDualPair(L, "TETRA",  tetraType);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushGfxBlockTypes(lua_State* L) // FIXME -- uppercase?
{
	lua_pushliteral(L, "GFXBLOCK_TYPE");
	lua_newtable(L);
	for (int i = 0; i < GfxBlock::BlockTypeCount; i++) {
		PushDualPair(L, GfxBlock::getTypeString(i), i);
	}
	lua_rawset(L, -3);

	lua_pushliteral(L, "GFXBLOCK_ID");
	lua_newtable(L);
	for (int i = 0; i < GfxBlockMgr::BlockIDCount; i++) {
		PushDualPair(L, GfxBlockMgr::getIDString(i), i);
	}
	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushRoamModes(lua_State* L)
{
	lua_pushliteral(L, "ROAM");
	lua_newtable(L);

	PushDualPair(L, "NONE",   Roaming::roamViewDisabled);
	PushDualPair(L, "FREE",   Roaming::roamViewFree);
	PushDualPair(L, "TRACK",  Roaming::roamViewTrack);
	PushDualPair(L, "FOLLOW", Roaming::roamViewFollow);
	PushDualPair(L, "FPS",    Roaming::roamViewFP);
	PushDualPair(L, "FLAG",   Roaming::roamViewFlag);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushConsoleTypes(lua_State* L)
{
	lua_pushliteral(L, "CONSOLE");
	lua_newtable(L);

	PushDualPair(L, "ALL",    ControlPanel::MessageAll);
	PushDualPair(L, "CHAT",   ControlPanel::MessageChat);
	PushDualPair(L, "SERVER", ControlPanel::MessageServer);
	PushDualPair(L, "MISC",   ControlPanel::MessageMisc);
	PushDualPair(L, "DEBUG",  ControlPanel::MessageDebug);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushMouseButtons(lua_State* L)
{
	lua_pushliteral(L, "MOUSE");
	lua_newtable(L);

	PushDualPair(L, "LEFT",   0);
	PushDualPair(L, "MIDDLE", 1);
	PushDualPair(L, "RIGHT",  2);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushFlagStates(lua_State* L)
{
	lua_pushliteral(L, "FLAG_STATE");
	lua_newtable(L);

	PushDualPair(L, "NONE",   FlagNoExist);
	PushDualPair(L, "GROUND", FlagOnGround);
	PushDualPair(L, "TANK",   FlagOnTank);
	PushDualPair(L, "AIR",    FlagInAir);
	PushDualPair(L, "COMING", FlagComing);
	PushDualPair(L, "GOING",  FlagGoing);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushFlagQualities(lua_State* L)
{
	lua_pushliteral(L, "FLAG_QUALITY");
	lua_newtable(L);

	PushDualPair(L, "GOOD", FlagGood);
	PushDualPair(L, "BAD",  FlagBad);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushFlagEndurance(lua_State* L)
{
	lua_pushliteral(L, "FLAG_ENDURANCE");
	lua_newtable(L);

	PushDualPair(L, "NORMAL",   FlagNormal);
	PushDualPair(L, "UNSTABLE", FlagUnstable);
	PushDualPair(L, "STICKY",   FlagSticky);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/

static bool PushKilledReasons(lua_State* L)
{
	lua_pushliteral(L, "KILL_REASON");
	lua_newtable(L);

	PushDualPair(L, "MESSAGE",       GotKilledMsg);
	PushDualPair(L, "SHOT",          GotShot);
	PushDualPair(L, "RUN_OVER",      GotRunOver);
	PushDualPair(L, "CAPTURED",      GotCaptured);
	PushDualPair(L, "GENOCIDE",      GenocideEffect);
	PushDualPair(L, "SELF_DESTRUCT", SelfDestruct);
	PushDualPair(L, "WATER",         WaterDeath);

	lua_rawset(L, -3);

	return true;
}


/******************************************************************************/
/******************************************************************************/
