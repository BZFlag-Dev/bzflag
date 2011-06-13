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
#include "Constants.h"

// system headers
#include <string>

// common headers
#include "global.h"
#include "bzfs/bzfsAPI.h"
#include "obstacle/Obstacle.h"

// local headers
#include "LuaHeader.h"
#include "LuaUtils.h"


static bool PushGameTypes(lua_State* L);
static bool PushGameOptionBits(lua_State* L);
static bool PushTeams(lua_State* L);
static bool PushShots(lua_State* L);
static bool PushFlagQualities(lua_State* L);
static bool PushPlayers(lua_State* L);
static bool PushPlayerStates(lua_State* L);
static bool PushObstacles(lua_State* L);
static bool PushCapabilities(lua_State* L);
static bool PushPermissions(lua_State* L);


//============================================================================//
//============================================================================//

bool Constants::PushEntries(lua_State* L) {
  PushGameTypes(L);
  PushGameOptionBits(L);
  PushTeams(L);
  PushShots(L);
  PushFlagQualities(L);
  PushPlayers(L);
  PushPlayerStates(L);
  PushObstacles(L);
  PushCapabilities(L);
  PushPermissions(L);

  return true;
}


//============================================================================//
//============================================================================//

static bool PushGameTypes(lua_State* L) {
  lua_pushliteral(L, "GAMETYPE");
  lua_newtable(L);

  LuaSetDualPair(L, "FFA",    eTeamFFAGame);
  LuaSetDualPair(L, "CTF",    eClassicCTFGame);
  LuaSetDualPair(L, "RABBIT", eRabbitGame);
  LuaSetDualPair(L, "OPEN",   eOpenFFAGame);

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


static bool PushTeams(lua_State* L) {
  lua_pushliteral(L, "TEAM");
  lua_newtable(L);

  LuaSetDualPair(L, "AUTO",     eAutomaticTeam);
  LuaSetDualPair(L, "NONE",     eNoTeam);
  LuaSetDualPair(L, "ROGUE",    eRogueTeam);
  LuaSetDualPair(L, "RED",      eRedTeam);
  LuaSetDualPair(L, "GREEN",    eGreenTeam);
  LuaSetDualPair(L, "BLUE",     eBlueTeam);
  LuaSetDualPair(L, "PURPLE",   ePurpleTeam);
  LuaSetDualPair(L, "RABBIT",   eRabbitTeam);
  LuaSetDualPair(L, "HUNTER",   eHunterTeam);
  LuaSetDualPair(L, "OBSERVER", eObservers);
  LuaSetDualPair(L, "ADMIN",    eAdministrators);

  lua_rawset(L, -3);

  return true;
}


static bool PushShots(lua_State* L) {
  lua_pushliteral(L, "SHOT");
  lua_newtable(L);

  LuaSetDualPair(L, "NO",  eNoShot);
  LuaSetDualPair(L, "STD", eStandardShot);
  LuaSetDualPair(L, "GM",  eGMShot);
  LuaSetDualPair(L, "L",   eLaserShot);
  LuaSetDualPair(L, "TH",  eThiefShot);
  LuaSetDualPair(L, "SH",  eSuperShot);
  LuaSetDualPair(L, "PZ",  ePhantomShot);
  LuaSetDualPair(L, "SW",  eShockWaveShot);
  LuaSetDualPair(L, "R",   eRicoShot);
  LuaSetDualPair(L, "MG",  eMachineGunShot);
  LuaSetDualPair(L, "IB",  eInvisibleShot);
  LuaSetDualPair(L, "CL",  eCloakedShot);
  LuaSetDualPair(L, "F",   eRapidFireShot);

  lua_rawset(L, -3);

  return true;
}


static bool PushObstacles(lua_State* L) {
  lua_pushliteral(L, "OBSTACLE");
  lua_newtable(L);

  LuaSetDualPair(L, "WALL",    wallType);
  LuaSetDualPair(L, "BOX",     boxType);
  LuaSetDualPair(L, "PYRAMID", pyrType);
  LuaSetDualPair(L, "BASE",    baseType);
  LuaSetDualPair(L, "TELE",    teleType);
  LuaSetDualPair(L, "MESH",    meshType);
  LuaSetDualPair(L, "ARC",     arcType);
  LuaSetDualPair(L, "CONE",    coneType);
  LuaSetDualPair(L, "SPHERE",  sphereType);
  LuaSetDualPair(L, "FACE",    faceType);

  lua_rawset(L, -3);

  return true;
}


static bool PushFlagQualities(lua_State* L) {
  lua_pushliteral(L, "FLAGQUAL");
  lua_newtable(L);

  LuaSetDualPair(L, "GOOD", eGoodFlag);
  LuaSetDualPair(L, "BAD",  eBadFlag);

  lua_rawset(L, -3);

  return true;
}


static bool PushPlayers(lua_State* L) {
  lua_pushliteral(L, "PLAYER");
  lua_newtable(L);

  LuaSetDualPair(L, "SERVER", BZ_SERVER);
  LuaSetDualPair(L, "ALL",    BZ_ALLUSERS);
  LuaSetDualPair(L, "NULL",   BZ_NULLUSER);

  lua_rawset(L, -3);

  return true;
}


static bool PushPlayerStates(lua_State* L) {
  lua_pushliteral(L, "STATUS");
  lua_newtable(L);

  LuaSetDualPair(L, "DEAD", eDead);
  LuaSetDualPair(L, "ALIVE",       eAlive);
  LuaSetDualPair(L, "PAUSED",      ePaused);
  LuaSetDualPair(L, "EXPLODING",   eExploding);
  LuaSetDualPair(L, "TELEPORTING", eTeleporting);
  LuaSetDualPair(L, "INBUILDING",  eInBuilding);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//

static bool PushCapabilities(lua_State* L) {
  lua_pushliteral(L, "CAPABILITY");
  lua_newtable(L);

  LuaSetDualPair(L, "JUMP",   AllowJump);
  LuaSetDualPair(L, "FIRE",   AllowShoot);
  LuaSetDualPair(L, "TURN",   AllowTurnLeft | AllowTurnRight);
  LuaSetDualPair(L, "TURN_LEFT",     AllowTurnLeft);
  LuaSetDualPair(L, "TURN_RIGHT",    AllowTurnRight);
  LuaSetDualPair(L, "MOVE",   AllowMoveForward | AllowMoveBackward);
  LuaSetDualPair(L, "MOVE_FORWARD",  AllowMoveForward);
  LuaSetDualPair(L, "MOVE_BACKWARD", AllowMoveBackward);
  LuaSetDualPair(L, "ALL",   0xFF);

  lua_rawset(L, -3);

  return true;
}


//============================================================================//
static bool PushPermissions(lua_State* L) {
  lua_pushliteral(L, "PERM");
  lua_newtable(L);

#define ADD_PERM(x)     \
  lua_pushliteral(L, #x);     \
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
  ADD_PERM(luaServer);
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


//============================================================================//
//============================================================================//

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
