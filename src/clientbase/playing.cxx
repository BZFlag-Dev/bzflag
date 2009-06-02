/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "playing.h"

// system includes
#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#  include <shlobj.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#  include <utime.h>
#endif

// common headers
#include "AnsiCodes.h"
#include "TextUtils.h"

// FIXME: The following are referenced in some parts of the client base
//        note: many of them are from clientCommands.cxx
StartupInfo startupInfo;
BzfDisplay* display = NULL;
ControlPanel *controlPanel = NULL;
HUDRenderer *hud = NULL;
MainWindow *mainWindow = NULL;
CommandCompleter completer;
float roamDZoom = 0.0f;
int savedVolume = -1;
bool fireButton = false;
bool roamButton = false;
// END FIXME

ServerLink *serverLink = NULL;
WordFilter *wordFilter = NULL;
PlayerId msgDestination;

bool canSpawn = true;
bool gameOver = false;

int numFlags = 0;

float clockAdjust = 0.0f;
bool pausedByUnmap = false;
float pauseCountdown = 0.0f;
float destructCountdown = 0.0f;

std::string customLimboMessage;


void addMessage(const Player *player, const std::string &msg,
		ControlPanel::MessageModes mode, bool highlight,
		const char *oldColor)
{
  std::string prefix;
  const char *message;

  if (BZDB.isTrue("colorful")) {
    if (player) {
      if (highlight) {
	if (BZDB.get("killerhighlight") == "1")
	  prefix += ColorStrings[PulsatingColor];
	else if (BZDB.get("killerhighlight") == "2")
	  prefix += ColorStrings[UnderlineColor];
      }
      const PlayerId pid = player->getId();
      if (pid < 200) {
	int color = player->getTeam();
	if (color < 0 || (color > 4 && color != HunterTeam))
	  // non-teamed, rabbit are white (same as observer)
	  color = WhiteColor;

	prefix += ColorStrings[color];
      } else if (pid == ServerPlayer) {
	prefix += ColorStrings[YellowColor];
      } else {
	prefix += ColorStrings[CyanColor]; //replay observers
      }
      prefix += player->getCallSign();

      if (highlight)
	prefix += ColorStrings[ResetColor];
#ifdef BWSUPPORT
      prefix += " (";
      prefix += Team::getName(player->getTeam());
      prefix += ")";
#endif
      prefix += std::string(ColorStrings[DefaultColor]) + ": ";
    }
    message = msg.c_str();
  } else {
    if (oldColor != NULL)
      prefix = oldColor;

    if (player) {
      prefix += player->getCallSign();

#ifdef BWSUPPORT
      prefix += " (";
      prefix += Team::getName(player->getTeam());
      prefix += ")";
#endif
      prefix += ": ";
    }
    message = stripAnsiCodes(msg.c_str());
  }
  const std::string msgf = TextUtils::format("%s%s", prefix.c_str(), message);
  showMessage(msgf, mode);
}


bool handleServerMessage(bool /*human*/, BufferedNetworkMessage *msg)
{
  switch (msg->getCode()) {
default:
  return false;

case MsgSetShot:
  handleSetShotType(msg);
  break;
  }
  return true;
}


void handleServerMessage(bool human, uint16_t code, uint16_t len, void *msg)
{
  std::vector<std::string> args;
  bool checkScores = false;

  switch (code) {
    case MsgWhatTimeIsIt: {
      handleWhatTimeIsIt(msg);
      break;
    }
    case MsgNearFlag: {
      handleNearFlag(msg);
      break;
    }
    case MsgSetTeam: {
      handleSetTeam(msg, len);
      break;
    }
    case MsgFetchResources: {
      handleResourceFetch(msg);
      break;
    }
    case MsgCustomSound: {
      handleCustomSound(msg);
      break;
    }
    case MsgUDPLinkEstablished: {
      serverLink->enableOutboundUDP();  // server got our initial UDP packet
      break;
    }
    case MsgUDPLinkRequest: {
      serverLink->confirmIncomingUDP();  // we got server's initial UDP packet
      break;
    }
    case MsgJoinServer: {
      handleJoinServer(msg);
      break;
    }
    case MsgSuperKill: {
      handleSuperKill(msg);
      break;
    }
    case MsgAccept: {
      break;
    }
    case MsgReject: {
      handleRejectMessage(msg);
      break;
    }
    case MsgNegotiateFlags: {
      handleFlagNegotiation(msg, len);
      break;
    }
    case MsgFlagType: {
      handleFlagType(msg);
      break;
    }
    case MsgGameSettings: {
      handleGameSettings(msg);
      break;
    }
    case MsgCacheURL: {
      handleCacheURL(msg, len);
      break;
    }
    case MsgWantWHash: {
      handleWantHash(msg, len);
      break;
    }
    case MsgGetWorld: {
      handleGetWorld(msg, len);
      break;
    }
    case MsgGameTime: {
      handleGameTime(msg);
      break;
    }
    case MsgTimeUpdate: {
      handleTimeUpdate(msg);
      break;
    }
    case MsgScoreOver: {
      handleScoreOver(msg);
      break;
    }
    case MsgAddPlayer: {
      handleAddPlayer(msg, checkScores);
      break;
    }
    case MsgRemovePlayer: {
      handleRemovePlayer(msg, checkScores);
      break;
    }
    case MsgFlagUpdate: {
      handleFlagUpdate(msg, len);
      break;
    }
    case MsgTeamUpdate: {
      handleTeamUpdate(msg, checkScores);
      break;
    }
    case MsgAlive: {
      handleAliveMessage(msg);
      break;
    }
    case MsgAutoPilot: {
      handleAutoPilot(msg);
      break;
    }
    case MsgAllow: {
      handleAllow(msg);
      break;
    }
    case MsgKilled: {
      handleKilledMessage(msg, human, checkScores);
      break;
    }
    case MsgGrabFlag: {
      handleGrabFlag(msg);
      break;
    }
    case MsgDropFlag: {
      handleDropFlag(msg);
      break;
    }
    case MsgCaptureFlag: {
      handleCaptureFlag(msg, checkScores);
      break;
    }
    case MsgNewRabbit: {
      handleNewRabbit(msg);
      break;
    }
    case MsgShotBegin: {
      handleShotBegin(human, msg);
      break;
    }
    case MsgWShotBegin: {
      handleWShotBegin(msg);
      break;
    }
    case MsgShotEnd: {
      handleShotEnd(msg);
      break;
    }
    case MsgHandicap: {
      handleHandicap(msg);
      break;
    }
    case MsgScore: {
      handleScore(msg);
      break;
    }
    case MsgSetVar: {
      handleMsgSetVars(msg);
      break;
    }
    case MsgTeleport: {
      handleTeleport(msg);
      break;
    }
    case MsgTransferFlag: {
      handleTransferFlag(msg);
      break;
    }
    case MsgMessage: {
      handleMessage(msg);
      break;
    }
    case MsgReplayReset: {
      handleReplayReset(msg, checkScores);
      break;
    }
    case MsgAdminInfo: {
      handleAdminInfo(msg);
      break;
    }
    case MsgPlayerInfo: {
      handlePlayerInfo(msg);
      break;
    }
    case MsgNewPlayer: {
      handleNewPlayer(msg);
      break;
    }
    // inter-player relayed message
    case MsgPlayerUpdate:
    case MsgPlayerUpdateSmall: {
      handleMovementUpdate(code, msg);
      break;
    }
    case MsgGMUpdate: {
      handleGMUpdate(msg);
      break;
    }
    case MsgLagPing: {
      serverLink->sendLagPing((char *)msg);
      break;
    }
    case MsgTangibilityUpdate: {
      handleTangUpdate(len, msg);
      break;
    }
    case MsgTangibilityReset: {
      handleTangReset();
      break;
    }
    case MsgAllowSpawn: {
      handleAllowSpawn(len, msg);
      break;
    }
    case MsgLimboMessage: {
      handleLimboMessage(msg);
      break;
    }
    case MsgPlayerData: {
      handlePlayerData(msg);
      break;
    }
    case MsgPause: {
      printf("MsgPause(FIXME) %s:%i\n", __FILE__, __LINE__);
      break;
    }
  }

  if (checkScores) {
    updateHighScores();
  }
}



// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
