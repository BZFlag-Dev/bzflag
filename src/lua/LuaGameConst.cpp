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
#include "LuaGameConst.h"

// common headers
#include "game/global.h"
#include "game/Flag.h"
#include "net/Address.h"
#include "AnsiCodes.h"
#include "obstacle/Obstacle.h"
#include "clientbase/GfxBlock.h"
#include "game/PlayerState.h"
#include "Protocol.h"

// bzflag headers
#include "bzflag/Roaming.h"
#include "bzflag/ControlPanel.h"

// local headers
#include "LuaHeader.h"
#include "LuaUtils.h"


//============================================================================//
//============================================================================//

static bool PushGameTypes(lua_State* L);
static bool PushGameOptionBits(lua_State* L);
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

bool LuaGameConst::PushEntries(lua_State* L) {
  return
    PushGameTypes(L)       &&
    PushGameOptionBits(L)  &&
    PushPlayerTypes(L)     &&
    PushPlayerStateBits(L) &&
    PushTeams(L)     &&
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

static bool PushGameTypes(lua_State* L) {
  lua_pushliteral(L, "GAMETYPE");
  lua_newtable(L);

  LuaSetDualPair(L, "FFA",     TeamFFA);
  LuaSetDualPair(L, "CTF",     ClassicCTF);
  LuaSetDualPair(L, "OPENFFA", OpenFFA);
  LuaSetDualPair(L, "RABBIT",  RabbitChase);

  lua_rawset(L, -3);

  return true;
}


static bool PushGameOptionBits(lua_State* L) {
  lua_pushliteral(L, "GAMEBITS");
  lua_newtable(L);

  LuaSetDualPair(L, "REPLAY", ReplayServer);
  LuaSetDualPair(L, "SUPER_FLAGS",   SuperFlagGameStyle);
  LuaSetDualPair(L, "NO_TEAM_KILLS", NoTeamKills);
  LuaSetDualPair(L, "JUMPING",       JumpingGameStyle);
  LuaSetDualPair(L, "INERTIA",       InertiaGameStyle);
  LuaSetDualPair(L, "RICOCHET",      RicochetGameStyle);
  LuaSetDualPair(L, "SHAKABLE",      ShakableGameStyle);
  LuaSetDualPair(L, "ANTIDOTE",      AntidoteGameStyle);
  LuaSetDualPair(L, "HANDICAP",      HandicapGameStyle);
  LuaSetDualPair(L, "LUA_WORLD",     LuaWorldScript);
  LuaSetDualPair(L, "LUA_RULES",     LuaRulesScript);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushTeams(lua_State* L) {
  lua_pushliteral(L, "TEAM");
  lua_newtable(L);

  LuaSetDualPair(L, "AUTO",     AutomaticTeam);
  LuaSetDualPair(L, "NONE",     NoTeam);
  LuaSetDualPair(L, "ROGUE",    RogueTeam);
  LuaSetDualPair(L, "RED",      RedTeam);
  LuaSetDualPair(L, "GREEN",    GreenTeam);
  LuaSetDualPair(L, "BLUE",     BlueTeam);
  LuaSetDualPair(L, "PURPLE",   PurpleTeam);
  LuaSetDualPair(L, "RABBIT",   RabbitTeam);
  LuaSetDualPair(L, "HUNTER",   HunterTeam);
  LuaSetDualPair(L, "OBSERVER", ObserverTeam);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushChatTeams(lua_State* L) {
  lua_pushliteral(L, "CHAT_TEAM");
  lua_newtable(L);

  LuaSetDualPair(L, "NONE",     NoPlayer);
  LuaSetDualPair(L, "ALL",      AllPlayers);
  LuaSetDualPair(L, "SERVER",   ServerPlayer);
  LuaSetDualPair(L, "ADMIN",    AdminPlayers);

  const int topTeam = 250;
  LuaSetDualPair(L, "TOP", topTeam);

  LuaSetDualPair(L, "ROGUE",    topTeam - RogueTeam);
  LuaSetDualPair(L, "RED",      topTeam - RedTeam);
  LuaSetDualPair(L, "GREEN",    topTeam - GreenTeam);
  LuaSetDualPair(L, "BLUE",     topTeam - BlueTeam);
  LuaSetDualPair(L, "PURPLE",   topTeam - PurpleTeam);
  LuaSetDualPair(L, "RABBIT",   topTeam - RabbitTeam);
  LuaSetDualPair(L, "HUNTER",   topTeam - HunterTeam);
  LuaSetDualPair(L, "OBSERVER", topTeam - ObserverTeam);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushShotTypes(lua_State* L) {
  lua_pushliteral(L, "SHOT");
  lua_newtable(L);

  LuaSetDualPair(L, "NONE",       NoShot);
  LuaSetDualPair(L, "STANDARD",   StandardShot);
  LuaSetDualPair(L, "GM",   GMShot);
  LuaSetDualPair(L, "LASER",      LaserShot);
  LuaSetDualPair(L, "THIEF",      ThiefShot);
  LuaSetDualPair(L, "SUPER",      SuperShot);
  LuaSetDualPair(L, "PHANTOM",    PhantomShot);
  LuaSetDualPair(L, "SHOCKWAVE",  ShockWaveShot);
  LuaSetDualPair(L, "RICO",       RicoShot);
  LuaSetDualPair(L, "MACHINEGUN", MachineGunShot);
  LuaSetDualPair(L, "INVISIBLE",  InvisibleShot);
  LuaSetDualPair(L, "CLOAKED",    CloakedShot);
  LuaSetDualPair(L, "RAPIDFIRE",  RapidFireShot);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushPlayerTypes(lua_State* L) {
  lua_pushliteral(L, "PLAYER_TYPE");
  lua_newtable(L);

  LuaSetDualPair(L, "TANK",     TankPlayer);
  LuaSetDualPair(L, "COMPUTER", ComputerPlayer);
  LuaSetDualPair(L, "CHAT",     ChatPlayer);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushPlayerStateBits(lua_State* L) {
  lua_pushliteral(L, "PLAYER_BIT");
  lua_newtable(L);

  LuaSetDualPair(L, "ALIVE",   PlayerState::Alive);
  LuaSetDualPair(L, "EXPLODING",     PlayerState::Exploding);
  LuaSetDualPair(L, "TELEPORTING",   PlayerState::Teleporting);
  LuaSetDualPair(L, "FLAG_ACTIVE",   PlayerState::FlagActive);
  LuaSetDualPair(L, "CROSSING_WALL", PlayerState::CrossingWall);
  LuaSetDualPair(L, "FALLING",       PlayerState::Falling);
  LuaSetDualPair(L, "ON_DRIVER",     PlayerState::OnDriver);
  LuaSetDualPair(L, "USER_INPUTS",   PlayerState::UserInputs);
  LuaSetDualPair(L, "JUMP_JETS",     PlayerState::JumpJets);
  LuaSetDualPair(L, "PLAY_SOUND",    PlayerState::PlaySound);
  LuaSetDualPair(L, "ZONED",   PlayerState::PhantomZoned);
  LuaSetDualPair(L, "IN_BUILDING",   PlayerState::InBuilding);
  LuaSetDualPair(L, "BACKED_OFF",    PlayerState::BackedOff);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushAnsiCodes(lua_State* L) {
  lua_pushliteral(L, "ANSI_COLOR");
  lua_newtable(L);

  luaset_strstr(L, "RESET", ANSI_STR_RESET_FINAL);
  luaset_strstr(L, "RESET_BRIGHT", ANSI_STR_RESET);
  luaset_strstr(L, "BRIGHT",       ANSI_STR_BRIGHT);
  luaset_strstr(L, "DIM",   ANSI_STR_DIM);
  luaset_strstr(L, "UNDERLINE",    ANSI_STR_UNDERLINE);
  luaset_strstr(L, "NO_UNDERLINE", ANSI_STR_NO_UNDERLINE);
  luaset_strstr(L, "BLINK", ANSI_STR_PULSATING);
  luaset_strstr(L, "NO_BLINK",     ANSI_STR_NO_PULSATE);
  luaset_strstr(L, "REVERSE",      ANSI_STR_REVERSE);
  luaset_strstr(L, "NO_REVERSE",   ANSI_STR_NO_REVERSE);

  luaset_strstr(L, "BLACK",   ANSI_STR_FG_BLACK);
  luaset_strstr(L, "RED",     ANSI_STR_FG_RED);
  luaset_strstr(L, "GREEN",   ANSI_STR_FG_GREEN);
  luaset_strstr(L, "YELLOW",  ANSI_STR_FG_YELLOW);
  luaset_strstr(L, "BLUE",    ANSI_STR_FG_BLUE);
  luaset_strstr(L, "MAGENTA", ANSI_STR_FG_MAGENTA);
  luaset_strstr(L, "CYAN",    ANSI_STR_FG_CYAN);
  luaset_strstr(L, "WHITE",   ANSI_STR_FG_WHITE);
  luaset_strstr(L, "ORANGE",  ANSI_STR_FG_ORANGE);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushObstacleTypes(lua_State* L) {
  lua_pushliteral(L, "OBSTACLE");
  lua_newtable(L);

  LuaSetDualPair(L, "WALL",   wallType);
  LuaSetDualPair(L, "BOX",    boxType);
  LuaSetDualPair(L, "PYR",    pyrType);
  LuaSetDualPair(L, "BASE",   baseType);
  LuaSetDualPair(L, "TELE",   teleType);
  LuaSetDualPair(L, "MESH",   meshType);
  LuaSetDualPair(L, "ARC",    arcType);
  LuaSetDualPair(L, "CONE",   coneType);
  LuaSetDualPair(L, "SPHERE", sphereType);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushGfxBlockTypes(lua_State* L) { // FIXME -- uppercase?
  lua_pushliteral(L, "GFXBLOCK_TYPE");
  lua_newtable(L);
  for (int i = 0; i < GfxBlock::BlockTypeCount; i++) {
    LuaSetDualPair(L, GfxBlock::getTypeString(i), i);
  }
  lua_rawset(L, -3);

  lua_pushliteral(L, "GFXBLOCK_ID");
  lua_newtable(L);
  for (int i = 0; i < GfxBlockMgr::BlockIDCount; i++) {
    LuaSetDualPair(L, GfxBlockMgr::getIDString(i), i);
  }
  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushRoamModes(lua_State* L) {
  lua_pushliteral(L, "ROAM");
  lua_newtable(L);

  LuaSetDualPair(L, "NONE",   Roaming::roamViewDisabled);
  LuaSetDualPair(L, "FREE",   Roaming::roamViewFree);
  LuaSetDualPair(L, "TRACK",  Roaming::roamViewTrack);
  LuaSetDualPair(L, "FOLLOW", Roaming::roamViewFollow);
  LuaSetDualPair(L, "FPS",    Roaming::roamViewFP);
  LuaSetDualPair(L, "FLAG",   Roaming::roamViewFlag);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushConsoleTypes(lua_State* L) {
  lua_pushliteral(L, "CONSOLE");
  lua_newtable(L);

  LuaSetDualPair(L, "ALL",    ControlPanel::MessageAll);
  LuaSetDualPair(L, "CHAT",   ControlPanel::MessageChat);
  LuaSetDualPair(L, "SERVER", ControlPanel::MessageServer);
  LuaSetDualPair(L, "MISC",   ControlPanel::MessageMisc);
  LuaSetDualPair(L, "DEBUG",  ControlPanel::MessageDebug);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushMouseButtons(lua_State* L) {
  lua_pushliteral(L, "MOUSE");
  lua_newtable(L);

  LuaSetDualPair(L, "LEFT",   0);
  LuaSetDualPair(L, "MIDDLE", 1);
  LuaSetDualPair(L, "RIGHT",  2);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushFlagStates(lua_State* L) {
  lua_pushliteral(L, "FLAG_STATE");
  lua_newtable(L);

  LuaSetDualPair(L, "NONE",   FlagNoExist);
  LuaSetDualPair(L, "GROUND", FlagOnGround);
  LuaSetDualPair(L, "TANK",   FlagOnTank);
  LuaSetDualPair(L, "AIR",    FlagInAir);
  LuaSetDualPair(L, "COMING", FlagComing);
  LuaSetDualPair(L, "GOING",  FlagGoing);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushFlagQualities(lua_State* L) {
  lua_pushliteral(L, "FLAG_QUALITY");
  lua_newtable(L);

  LuaSetDualPair(L, "GOOD", FlagGood);
  LuaSetDualPair(L, "BAD",  FlagBad);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushFlagEndurance(lua_State* L) {
  lua_pushliteral(L, "FLAG_ENDURANCE");
  lua_newtable(L);

  LuaSetDualPair(L, "NORMAL",   FlagNormal);
  LuaSetDualPair(L, "UNSTABLE", FlagUnstable);
  LuaSetDualPair(L, "STICKY",   FlagSticky);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushKilledReasons(lua_State* L) {
  lua_pushliteral(L, "KILL_REASON");
  lua_newtable(L);

  LuaSetDualPair(L, "MESSAGE",       GotKilledMsg);
  LuaSetDualPair(L, "SHOT",   GotShot);
  LuaSetDualPair(L, "RUN_OVER",      GotRunOver);
  LuaSetDualPair(L, "CAPTURED",      GotCaptured);
  LuaSetDualPair(L, "GENOCIDE",      GenocideEffect);
  LuaSetDualPair(L, "SELF_DESTRUCT", SelfDestruct);
  LuaSetDualPair(L, "WATER",   WaterDeath);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
//============================================================================//


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
