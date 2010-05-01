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

#ifndef LUA_CALLOUTS_H
#define LUA_CALLOUTS_H

#include "common.h"

struct lua_State;


class LuaCallOuts {
  public:
    static bool PushEntries(lua_State* L);

  public:
    // call-outs
    static int GetBzLuaVersion(lua_State* L);
    static int GetClientVersion(lua_State* L);
    static int GetProtocolVersion(lua_State* L);

    static int JoinGame(lua_State* L);
    static int LeaveGame(lua_State* L);

    static int Print(lua_State* L);
    static int Debug(lua_State* L);
    static int GetDebugLevel(lua_State* L);

    static int CalcMD5(lua_State* L);
    static int StripAnsiCodes(lua_State* L);
    static int ExpandColorString(lua_State* L);
    static int LocalizeString(lua_State* L);
    static int GetCacheFilePath(lua_State* L);

    static int GetGameInfo(lua_State* L);
    static int GetWorldInfo(lua_State* L);

    static int GetServerAddress(lua_State* L);
    static int GetServerPort(lua_State* L);
    static int GetServerCallsign(lua_State* L);
    static int GetServerIP(lua_State* L);
    static int GetServerDescription(lua_State* L);

    static int GetWind(lua_State* L);
    static int GetLights(lua_State* L);

    static int GetWorldHash(lua_State* L);

    static int SendLuaData(lua_State* L);
    static int SendCommand(lua_State* L);

    static int BlockControls(lua_State* L);

    static int OpenMenu(lua_State* L);
    static int CloseMenu(lua_State* L);

    static int PlaySound(lua_State* L);

    static int ReadImageData(lua_State* L);
    static int ReadImageFile(lua_State* L);

    static int GetViewType(lua_State* L);

    static int GetKeyToCmds(lua_State* L);
    static int GetCmdToKeys(lua_State* L);

    static int GetTime(lua_State* L);
    static int GetGameTime(lua_State* L);
    static int GetTimer(lua_State* L);
    static int DiffTimers(lua_State* L);

    static int GetRoamInfo(lua_State* L);
    static int SetRoamInfo(lua_State* L);

    static int GetDrawingMirror(lua_State* L);

    static int GetScreenGeometry(lua_State* L);
    static int GetWindowGeometry(lua_State* L);
    static int GetViewGeometry(lua_State* L);
    static int GetRadarGeometry(lua_State* L);
    static int GetRadarRange(lua_State* L);

    static int GetWorldExtents(lua_State* L);
    static int GetVisualExtents(lua_State* L);
    static int GetLengthPerPixel(lua_State* L);

    static int SetCameraView(lua_State* L);
    static int SetCameraProjection(lua_State* L);
    static int GetCameraPosition(lua_State* L);
    static int GetCameraDirection(lua_State* L);
    static int GetCameraUp(lua_State* L);
    static int GetCameraRight(lua_State* L);
    static int GetCameraMatrix(lua_State* L);
    static int GetFrustumPlane(lua_State* L);

    static int NotifyStyleChange(lua_State* L);

    static int GetSun(lua_State* L);

    static int GetTeamList(lua_State* L);
    static int GetTeamPlayers(lua_State* L);
    static int GetTeamName(lua_State* L);
    static int GetTeamLongName(lua_State* L);
    static int GetTeamCount(lua_State* L);
    static int GetTeamScore(lua_State* L);
    static int GetTeamColor(lua_State* L);
    static int GetTeamRadarColor(lua_State* L);
    static int GetTeamsAreEnemies(lua_State* L);
    static int GetPlayersAreEnemies(lua_State* L);

    static int GetMousePosition(lua_State* L);
    static int GetMouseButtons(lua_State* L);
    static int GetJoyPosition(lua_State* L);
    static int GetKeyModifiers(lua_State* L);

    static int WarpMouse(lua_State* L);
    static int SetMouseBox(lua_State* L);

    static int GetLocalPlayer(lua_State* L);
    static int GetLocalPlayerTarget(lua_State* L);
    static int GetLocalTeam(lua_State* L);
    static int GetRabbitPlayer(lua_State* L);
    static int GetAntidotePosition(lua_State* L);

    static int GetJoyButtons(lua_State* L); // FIXME -- not implemented
    static int GetJoyHatswitch(lua_State* L);

    static int SetGfxBlock(lua_State* L);
    static int GetGfxBlock(lua_State* L);
    static int SetPlayerGfxBlock(lua_State* L);
    static int GetPlayerGfxBlock(lua_State* L);
    static int SetPlayerRadarGfxBlock(lua_State* L);
    static int GetPlayerRadarGfxBlock(lua_State* L);
    static int SetFlagGfxBlock(lua_State* L);
    static int GetFlagGfxBlock(lua_State* L);
    static int SetFlagRadarGfxBlock(lua_State* L);
    static int GetFlagRadarGfxBlock(lua_State* L);
    static int SetShotGfxBlock(lua_State* L);
    static int GetShotGfxBlock(lua_State* L);
    static int SetShotRadarGfxBlock(lua_State* L);
    static int GetShotRadarGfxBlock(lua_State* L);

    // players
    static int GetPlayerList(lua_State* L);
    static int GetPlayerName(lua_State* L);
    static int GetPlayerType(lua_State* L);
    static int GetPlayerTeam(lua_State* L);
    static int GetPlayerFlag(lua_State* L);
    static int GetPlayerFlagType(lua_State* L);
    static int GetPlayerScore(lua_State* L);
    static int GetPlayerMotto(lua_State* L);
    static int GetPlayerAutoPilot(lua_State* L);
    static int GetPlayerCustomData(lua_State* L);
    static int GetPlayerShots(lua_State* L);
    static int GetPlayerState(lua_State* L);
    static int GetPlayerStateBits(lua_State* L);
    static int GetPlayerPosition(lua_State* L);
    static int GetPlayerRotation(lua_State* L);
    static int GetPlayerDirection(lua_State* L);
    static int GetPlayerVelocity(lua_State* L);
    static int GetPlayerAngVel(lua_State* L);
    static int GetPlayerDimensions(lua_State* L);
    static int GetPlayerPhysicsDriver(lua_State* L);
    static int GetPlayerDesiredSpeed(lua_State* L);
    static int GetPlayerDesiredAngVel(lua_State* L);
    static int GetPlayerExplodeTime(lua_State* L);
    static int IsPlayerAdmin(lua_State* L);
    static int IsPlayerVerified(lua_State* L);
    static int IsPlayerRegistered(lua_State* L);
    static int IsPlayerHunted(lua_State* L);

    // flags
    static int GetFlagList(lua_State* L); // FIXME?
    static int GetFlagName(lua_State* L);
    static int GetFlagType(lua_State* L);
    static int GetFlagShotType(lua_State* L);
    static int GetFlagQuality(lua_State* L);
    static int GetFlagEndurance(lua_State* L);
    static int GetFlagTeam(lua_State* L);
    static int GetFlagOwner(lua_State* L);
    static int GetFlagState(lua_State* L);
    static int GetFlagPosition(lua_State* L);

    // shots
    static int GetShotList(lua_State* L);
    static int GetShotType(lua_State* L);
    static int GetShotFlagType(lua_State* L);
    static int GetShotPlayer(lua_State* L);
    static int GetShotTeam(lua_State* L);
    static int GetShotPosition(lua_State* L);
    static int GetShotVelocity(lua_State* L);
    static int GetShotLifeTime(lua_State* L);
    static int GetShotLeftTime(lua_State* L);
    static int GetShotReloadTime(lua_State* L);

    // unix utility
#if defined(HAVE_UNISTD_H) && defined(HAVE_FCNTL_H)
    static int ReadStdin(lua_State* L);
#endif
};


#endif // LUA_CALLOUTS_H


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
