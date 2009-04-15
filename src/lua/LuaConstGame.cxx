
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


//============================================================================//
//============================================================================//

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


//============================================================================//
//============================================================================//

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


//============================================================================//
//============================================================================//

static bool PushGameTypes(lua_State* L)
{
	lua_pushliteral(L, "GAME");
	lua_newtable(L);

	LuaPushDualPair(L, "FFA",     TeamFFA);
	LuaPushDualPair(L, "CTF",     ClassicCTF);
	LuaPushDualPair(L, "OPENFFA", OpenFFA);
	LuaPushDualPair(L, "RABBIT",  RabbitChase);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushTeams(lua_State* L)
{
	lua_pushliteral(L, "TEAM");
	lua_newtable(L);

	LuaPushDualPair(L, "AUTO",     AutomaticTeam);
	LuaPushDualPair(L, "NONE",     NoTeam);
	LuaPushDualPair(L, "ROGUE",    RogueTeam);
	LuaPushDualPair(L, "RED",      RedTeam);
	LuaPushDualPair(L, "GREEN",    GreenTeam);
	LuaPushDualPair(L, "BLUE",     BlueTeam);
	LuaPushDualPair(L, "PURPLE",   PurpleTeam);
	LuaPushDualPair(L, "RABBIT",   RabbitTeam);
	LuaPushDualPair(L, "HUNTER",   HunterTeam);
	LuaPushDualPair(L, "OBSERVER", ObserverTeam);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushChatTeams(lua_State* L)
{
	lua_pushliteral(L, "CHAT_TEAM");
	lua_newtable(L);

	LuaPushDualPair(L, "NONE",     NoPlayer);
	LuaPushDualPair(L, "ALL",      AllPlayers);
	LuaPushDualPair(L, "SERVER",   ServerPlayer);
	LuaPushDualPair(L, "ADMIN",    AdminPlayers);

	const int topTeam = 250;
	LuaPushDualPair(L, "TOP", topTeam);

	LuaPushDualPair(L, "ROGUE",    topTeam - RogueTeam);
	LuaPushDualPair(L, "RED",      topTeam - RedTeam);
	LuaPushDualPair(L, "GREEN",    topTeam - GreenTeam);
	LuaPushDualPair(L, "BLUE",     topTeam - BlueTeam);
	LuaPushDualPair(L, "PURPLE",   topTeam - PurpleTeam);
	LuaPushDualPair(L, "RABBIT",   topTeam - RabbitTeam);
	LuaPushDualPair(L, "HUNTER",   topTeam - HunterTeam);
	LuaPushDualPair(L, "OBSERVER", topTeam - ObserverTeam);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushShotTypes(lua_State* L)
{
	lua_pushliteral(L, "SHOT");
	lua_newtable(L);

	LuaPushDualPair(L, "NONE",       NoShot);
	LuaPushDualPair(L, "STANDARD",   StandardShot);
	LuaPushDualPair(L, "GM",         GMShot);
	LuaPushDualPair(L, "LASER",      LaserShot);
	LuaPushDualPair(L, "THIEF",      ThiefShot);
	LuaPushDualPair(L, "SUPER",      SuperShot);
	LuaPushDualPair(L, "PHANTOM",    PhantomShot);
	LuaPushDualPair(L, "SHOCKWAVE",  ShockWaveShot);
	LuaPushDualPair(L, "RICO",       RicoShot);
	LuaPushDualPair(L, "MACHINEGUN", MachineGunShot);
	LuaPushDualPair(L, "INVISIBLE",  InvisibleShot);
	LuaPushDualPair(L, "CLOAKED",    CloakedShot);
	LuaPushDualPair(L, "RAPIDFIRE",  RapidFireShot);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushPlayerTypes(lua_State* L)
{
	lua_pushliteral(L, "PLAYER_TYPE");
	lua_newtable(L);

	LuaPushDualPair(L, "TANK",     TankPlayer);
	LuaPushDualPair(L, "COMPUTER", ComputerPlayer);
	LuaPushDualPair(L, "CHAT",     ChatPlayer);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushPlayerStateBits(lua_State* L)
{
	lua_pushliteral(L, "PLAYER_BIT");
	lua_newtable(L);

	LuaPushDualPair(L, "ALIVE",         PlayerState::Alive);
	LuaPushDualPair(L, "PAUSED",        PlayerState::Paused);
	LuaPushDualPair(L, "EXPLODING",     PlayerState::Exploding);
	LuaPushDualPair(L, "TELEPORTING",   PlayerState::Teleporting);
	LuaPushDualPair(L, "FLAG_ACTIVE",   PlayerState::FlagActive);
	LuaPushDualPair(L, "CROSSING_WALL", PlayerState::CrossingWall);
	LuaPushDualPair(L, "FALLING",       PlayerState::Falling);
	LuaPushDualPair(L, "ON_DRIVER",     PlayerState::OnDriver);
	LuaPushDualPair(L, "USER_INPUTS",   PlayerState::UserInputs);
	LuaPushDualPair(L, "JUMP_JETS",     PlayerState::JumpJets);
	LuaPushDualPair(L, "PLAY_SOUND",    PlayerState::PlaySound);
	LuaPushDualPair(L, "ZONED",         PlayerState::PhantomZoned);
	LuaPushDualPair(L, "IN_BUILDING",   PlayerState::InBuilding);
	LuaPushDualPair(L, "BACKED_OFF",    PlayerState::BackedOff);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

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


//============================================================================//
//============================================================================//

static bool PushObstacleTypes(lua_State* L)
{
	lua_pushliteral(L, "OBSTACLE");
	lua_newtable(L);

	LuaPushDualPair(L, "WALL",   wallType);
	LuaPushDualPair(L, "BOX",    boxType);
	LuaPushDualPair(L, "PYR",    pyrType);
	LuaPushDualPair(L, "BASE",   baseType);
	LuaPushDualPair(L, "TELE",   teleType);
	LuaPushDualPair(L, "MESH",   meshType);
	LuaPushDualPair(L, "ARC",    arcType);
	LuaPushDualPair(L, "CONE",   coneType);
	LuaPushDualPair(L, "SPHERE", sphereType);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushGfxBlockTypes(lua_State* L) // FIXME -- uppercase?
{
	lua_pushliteral(L, "GFXBLOCK_TYPE");
	lua_newtable(L);
	for (int i = 0; i < GfxBlock::BlockTypeCount; i++) {
		LuaPushDualPair(L, GfxBlock::getTypeString(i), i);
	}
	lua_rawset(L, -3);

	lua_pushliteral(L, "GFXBLOCK_ID");
	lua_newtable(L);
	for (int i = 0; i < GfxBlockMgr::BlockIDCount; i++) {
		LuaPushDualPair(L, GfxBlockMgr::getIDString(i), i);
	}
	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushRoamModes(lua_State* L)
{
	lua_pushliteral(L, "ROAM");
	lua_newtable(L);

	LuaPushDualPair(L, "NONE",   Roaming::roamViewDisabled);
	LuaPushDualPair(L, "FREE",   Roaming::roamViewFree);
	LuaPushDualPair(L, "TRACK",  Roaming::roamViewTrack);
	LuaPushDualPair(L, "FOLLOW", Roaming::roamViewFollow);
	LuaPushDualPair(L, "FPS",    Roaming::roamViewFP);
	LuaPushDualPair(L, "FLAG",   Roaming::roamViewFlag);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushConsoleTypes(lua_State* L)
{
	lua_pushliteral(L, "CONSOLE");
	lua_newtable(L);

	LuaPushDualPair(L, "ALL",    ControlPanel::MessageAll);
	LuaPushDualPair(L, "CHAT",   ControlPanel::MessageChat);
	LuaPushDualPair(L, "SERVER", ControlPanel::MessageServer);
	LuaPushDualPair(L, "MISC",   ControlPanel::MessageMisc);
	LuaPushDualPair(L, "DEBUG",  ControlPanel::MessageDebug);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushMouseButtons(lua_State* L)
{
	lua_pushliteral(L, "MOUSE");
	lua_newtable(L);

	LuaPushDualPair(L, "LEFT",   0);
	LuaPushDualPair(L, "MIDDLE", 1);
	LuaPushDualPair(L, "RIGHT",  2);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushFlagStates(lua_State* L)
{
	lua_pushliteral(L, "FLAG_STATE");
	lua_newtable(L);

	LuaPushDualPair(L, "NONE",   FlagNoExist);
	LuaPushDualPair(L, "GROUND", FlagOnGround);
	LuaPushDualPair(L, "TANK",   FlagOnTank);
	LuaPushDualPair(L, "AIR",    FlagInAir);
	LuaPushDualPair(L, "COMING", FlagComing);
	LuaPushDualPair(L, "GOING",  FlagGoing);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushFlagQualities(lua_State* L)
{
	lua_pushliteral(L, "FLAG_QUALITY");
	lua_newtable(L);

	LuaPushDualPair(L, "GOOD", FlagGood);
	LuaPushDualPair(L, "BAD",  FlagBad);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushFlagEndurance(lua_State* L)
{
	lua_pushliteral(L, "FLAG_ENDURANCE");
	lua_newtable(L);

	LuaPushDualPair(L, "NORMAL",   FlagNormal);
	LuaPushDualPair(L, "UNSTABLE", FlagUnstable);
	LuaPushDualPair(L, "STICKY",   FlagSticky);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//

static bool PushKilledReasons(lua_State* L)
{
	lua_pushliteral(L, "KILL_REASON");
	lua_newtable(L);

	LuaPushDualPair(L, "MESSAGE",       GotKilledMsg);
	LuaPushDualPair(L, "SHOT",          GotShot);
	LuaPushDualPair(L, "RUN_OVER",      GotRunOver);
	LuaPushDualPair(L, "CAPTURED",      GotCaptured);
	LuaPushDualPair(L, "GENOCIDE",      GenocideEffect);
	LuaPushDualPair(L, "SELF_DESTRUCT", SelfDestruct);
	LuaPushDualPair(L, "WATER",         WaterDeath);

	lua_rawset(L, -3);

	return true;
}


//============================================================================//
//============================================================================//
