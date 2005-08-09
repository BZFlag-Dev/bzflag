/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4:4786)
#endif

// interface header
#include "commands.h"

#include "WorldEventManager.h"

// system implementation headers
#include <vector>
#include <string>
#include <sstream>
#ifdef HAVE_CSTDIO
#include <cstdio>
#else
#include <stdio.h>
#endif
#ifdef HAVE_CSTRING
#include <cstring>
#else
#include <string.h>
#endif
#include <time.h>

// common implementation headers
#include "Address.h"
#include "CommandManager.h"
#include "LagInfo.h"
#include "NetHandler.h"
#include "PlayerInfo.h"
#include "TimeKeeper.h"
#include "VotingArbiter.h"
#include "global.h"
#include "version.h"

// local implementation headers
#include "CmdLineOptions.h"
#include "FlagHistory.h"
#include "FlagInfo.h"
#include "PackVars.h"
#include "Permissions.h"
#include "RecordReplay.h"
#include "MasterBanList.h"


#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif

// FIXME -- need to pull communication out of bzfs.cxx...

// extern to initialize permission groups
extern void initGroups();

// externs that poll, veto, vote, and clientquery require
extern void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message);
extern void sendPlayerMessage(GameKeeper::Player *playerData, PlayerId dstPlayer,
			      const char *message);
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;

// externs that ghost needs
extern void removePlayer(int playerIndex, const char *reason, bool notify=true);
extern void playerKilled(int victimIndex, int killerIndex, int reason, int16_t shotIndex, const FlagType* flagType, int phydrv, bool respawnOnBase = false);
// externs that shutdownserver requires
extern bool done;

// externs that superkill and gameover requires
extern bool gameOver;
extern char *getDirectMessageBuffer();
extern void broadcastMessage(uint16_t code, int len, const void *msg);

#include "PlayerInfo.h"
extern TeamInfo team[NumTeams];
extern void sendTeamUpdate(int playerIndex = -1, int teamIndex1 = -1, int teamIndex2 = -1);
extern int numFlags;
extern void sendFlagUpdate(FlagInfo &flag);
extern void sendDrop(FlagInfo &flag);

// externs that countdown requires
extern bool countdownActive;
extern int countdownDelay;
extern TimeKeeper countdownPauseStart;

// externs that identify and password requires
extern void sendIPUpdate(int targetPlayer, int playerIndex);
extern void sendPlayerInfo(void);

tmCustomSlashCommandMap	customCommands;

class NoDigit {
public:
  bool operator() (char c) {return !isdigit(c);}
};

int getSlotNumber(std::string player) {
  int slot = -1; // invalid
  if (player[0] == '#') {
    // string player is a slot number
    player[0] = '0';
    if(find_if(player.begin(), player.end(), NoDigit()) == player.end()) {
      // a valid number is in the string
      slot = atoi(player.c_str());
    }
  } else {
    // invalid number.. may be a player callsign
    slot = GameKeeper::Player::getPlayerIDByName(player);
  }
  
  GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(slot);
  if (!p)
    slot = -1; // not a player

  return slot;
}

static void handleMuteCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::mute)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unmute command");
    return;
  }
  
  
  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t, "Syntax: /mute <#slot | PlayerName | \"Player Name\">");
    return;
  }
  
  int i = getSlotNumber(argv[1]);
  
  char msg[MessageLen];
  
  // Player not found
  if (i < 0) {
    snprintf(msg, MessageLen, "player \"%s\" not found", argv[1].c_str());
    sendMessage(ServerPlayer, t, msg);
    return;
  }
  
  // mute the player
  GameKeeper::Player *muteData = GameKeeper::Player::getPlayerByIndex(i);
  if (muteData) {
    muteData->accessInfo.revokePerm(PlayerAccessInfo::talk);
    snprintf(msg, MessageLen, "You have been muted by %s.", playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, msg);
    // confirm player is muted
    snprintf(msg, MessageLen, "player id #%d \"%s\" is now muted.", i, muteData->player.getCallSign());
    sendMessage(ServerPlayer, t, msg);
  }
}

static void handleUnmuteCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::mute)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unmute command");
    return;
  }
  
  
  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t, "Syntax: /unmute <#slot | PlayerName | \"Player Name\">");
    return;
  }
  
  int i = getSlotNumber(argv[1]);
  
  char msg[MessageLen];
  
  // Player not found
  if (i < 0) {
    snprintf(msg, MessageLen, "player \"%s\" not found", argv[1].c_str());
    sendMessage(ServerPlayer, t, msg);
    return;
  }
  
  // unmute the player
  GameKeeper::Player *unmuteData = GameKeeper::Player::getPlayerByIndex(i);
  if (unmuteData) {
    unmuteData->accessInfo.grantPerm(PlayerAccessInfo::talk);
    snprintf(msg, MessageLen, "You have been unmuted by %s.", playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, msg);
    // confirm player is unmuted
    snprintf(msg, MessageLen, "player id #%d \"%s\" is now unmuted.", i, unmuteData->player.getCallSign());
    sendMessage(ServerPlayer, t, msg);
  }
}

static void handleUptimeCmd(GameKeeper::Player *playerData, const char *)
{
  float rawTime;
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};

  rawTime = float(TimeKeeper::getCurrent() - TimeKeeper::getStartTime());
  snprintf(reply, MessageLen, "%s.", TimeKeeper::printTime(rawTime).c_str());
  sendMessage(ServerPlayer, t, reply);
}

static void handleServerQueryCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  DEBUG2("Server query requested by %s [%d]\n",
	 playerData->player.getCallSign(), t);

  sendMessage(ServerPlayer, t,
	      TextUtils::format("BZFS Version: %s", getAppVersion()).c_str());
  return;
}


static void handlePartCmd(GameKeeper::Player *playerData, const char *message)
{
  std::string byeStatement = "";

  if (strlen(message) > 5) {
    if (!TextUtils::isWhitespace(*(message+5))) {
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "Unknown command [%s]", message);
      sendMessage(ServerPlayer, playerData->getIndex(), reply);
      return;
    }
    byeStatement = message + 6;
  }

  if (byeStatement[0] != '\0') {
    std::string message2;
    message2 = TextUtils::format("%s has left (\"%s\") ",
				 playerData->player.getCallSign(),  byeStatement.c_str());

    DEBUG2("%s has quit with the message \"%s\"\n", playerData->player.getCallSign(), byeStatement.c_str());
    sendMessage(ServerPlayer, AllPlayers, message2.c_str());
  }

  // now to kick the player
  int t = playerData->getIndex();
  removePlayer(t, byeStatement.c_str());
}


static void handleQuitCmd(GameKeeper::Player *playerData, const char *message)
{
  std::string byeStatement = "";

  if (strlen(message) > 5) {
    if (!TextUtils::isWhitespace(*(message+5))) {
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "Unknown command [%s]", message);
      sendMessage(ServerPlayer, playerData->getIndex(), reply);
      return;
    }
    byeStatement = message + 6;
  }

  if (byeStatement[0] != '\0') {
    std::string message2;
    message2 = TextUtils::format("%s has quit (\"%s\") ",
				 playerData->player.getCallSign(),  byeStatement.c_str());

    DEBUG2("%s has quit with the message \"%s\"\n", playerData->player.getCallSign(), byeStatement.c_str());
    sendMessage(ServerPlayer, AllPlayers, message2.c_str());
  }

  // now to kick the player
  int t = playerData->getIndex();
  removePlayer(t, byeStatement.c_str());
}


static void handleMsgCmd(GameKeeper::Player *playerData, const char *message)
{
  int from = playerData->getIndex();
  int to= -1;

  std::string message2;
  size_t callsignStart=0, callsignEnd=0, messageStart=0;

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::privateMessage)) {
    char reply[MessageLen] = {0};
    snprintf(reply, MessageLen, "%s, you are not presently authorized to /msg people privately", playerData->player.getCallSign());
    sendMessage(ServerPlayer, from, reply);
    return;
  }

  // start from after "/msg"
  std::string arguments = &message[4];
  std::string recipient = std::string("");

  // skip any leading whitespace
  callsignStart = 0;
  while ((callsignStart < arguments.size()) &&
	 (isspace(arguments[callsignStart]))) {
    callsignStart++;
  }

  // make sure there was _some_ whitespace after /msg
  if (callsignStart == 0) {
    sendMessage(ServerPlayer, from, "Usage: /msg \"some callsign\" some message");
    return;
  }

  // find the player name, optionally quoted
  if (arguments[callsignStart] == '"') {
    callsignStart++;

    // find the trailing quote
    bool foundQuote = false;
    callsignEnd = callsignStart;
    while ((callsignEnd + 1 < arguments.size()) &&
	   (!foundQuote)) {
      callsignEnd++;
      if (arguments[callsignEnd] == '"') {
	foundQuote = true;
	messageStart = callsignEnd + 1;
	callsignEnd--;
      }
    }

    // no quote means a mismatch
    if (!foundQuote) {
      sendMessage(ServerPlayer, from, "Quote mismatch?");
      sendMessage(ServerPlayer, from, "Usage: /msg \"some callsign\" some message");
      return;
    }

  } else {
    // unquoted callsign

    // find the first matching name (not the longest for sake of performance)
    bool foundCallsign = false;
    callsignEnd = callsignStart;
    while ((callsignEnd + 1 < arguments.size()) &&
	   (!foundCallsign)) {
      callsignEnd++;
      if (!isspace(arguments[callsignEnd])) {
	continue;
      }

      // we have a space
      recipient = arguments.substr(callsignStart, callsignEnd - callsignStart);
      messageStart = callsignEnd;

      to = GameKeeper::Player::getPlayerIDByName(recipient);
      if (to < curMaxPlayers) {
	callsignEnd--;
	foundCallsign = true;
      }
    }
  }

  recipient = arguments.substr(callsignStart, callsignEnd - callsignStart + 1);

  if (recipient[0] == '>') {
    // /msg >admin sends on admin channel, /msg >team on team channel
    recipient.erase(0,1);
    if (TextUtils::toupper(recipient) == "ADMIN") 
      to = AdminPlayers;				
    else if (TextUtils::toupper(recipient) == "TEAM") 
      to = 250 - (int)playerData->player.getTeam();
  } else {
    to = GameKeeper::Player::getPlayerIDByName(recipient);

    // valid callsign
    if ((to < 0) || (to >= curMaxPlayers)) {
      message2 = TextUtils::format("\"%s\" is not here.  No such callsign.", recipient.c_str());
      sendMessage(ServerPlayer, from, message2.c_str());
      return;
    }
  }

  // make sure there is something to send
  if ((messageStart >= arguments.size() - 1) || (messageStart == 0)) {
    // found player, but nothing to send
    message2 = TextUtils::format("No text to send to \"%s\".", recipient.c_str());
    sendMessage(ServerPlayer, from, message2.c_str());
    return;
  }

  // send the message
  sendPlayerMessage(playerData, to, arguments.c_str() + messageStart + 1);
  return;
}


static void handlePasswordCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (playerData->accessInfo.passwordAttemptsMax()) {
    DEBUG1("\"%s\" (%s) has attempted too many /password tries\n",
	   playerData->player.getCallSign(),
	   playerData->netHandler->getTargetIP());
    sendMessage(ServerPlayer, t, "Too many attempts");
  } else {
    if ((clOptions->password != "") && strncmp(message + 10, clOptions->password.c_str(), clOptions->password.size()) == 0 && clOptions->password.length() == strlen(message + 10)) {
      playerData->accessInfo.setAdmin();
      sendPlayerInfo();
      sendMessage(ServerPlayer, t, "You are now an administrator!");
    } else {
      sendMessage(ServerPlayer, t, "Wrong Password!");
    }
  }
  return;
}


static void handleSetCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  int setvar = playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar) ? 1 : 0;
  int setall = playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll) ? 1 : 0;
  char message2[MessageLen];
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar)
      && !playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the set command");
	DEBUG3("set failed by %s, setvar=%d, setall=%d\n",playerData->player.getCallSign(),setvar,setall);
   return;
  }
  if (Replay::enabled()) {
    sendMessage(ServerPlayer, t, "You can't /set variables in replay mode");
    return;
  }
  DEBUG3("set executed by %s, setvar=%d, setall=%d\n",playerData->player.getCallSign(),setvar,setall);
  std::string command = (message + 1);
  // we aren't case sensitive but CMDMGR is
  for (int i = 0; i < 3 /*"set"*/; ++i)
    command[i] = tolower(command[i]);
  sendMessage(ServerPlayer, t, CMDMGR.run(command).c_str());
  snprintf(message2, MessageLen, "Variable Modification Notice by %s of %s",
	   playerData->player.getCallSign(), command.c_str());
  sendMessage(ServerPlayer, AllPlayers, message2);
  return;
}


static void handleResetCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar)
      && !playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the reset command");
    return;
  }
  if (Replay::enabled()) {
    sendMessage(ServerPlayer, t, "You can't /reset variables in replay mode");
    return;
  }
  std::string command = (message + 1);
  // we aren't case sensitive but CMDMGR is
  for (int i = 0; i < 5 /*"reset"*/; ++i)
    command[i] = tolower(command[i]);
  sendMessage(ServerPlayer, t, CMDMGR.run(command).c_str());
  return;
}


static void handleShutdownserverCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::shutdownServer)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the shutdown command");
    return;
  }
  done = true;
  return;
}


static void handleSuperkillCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::superKill)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the superkill command");
    return;
  }
  for (int i = 0; i < curMaxPlayers; i++)
    removePlayer(i, "/superkill");
  gameOver = true;
  if (clOptions->timeManualStart)
    countdownActive = false;
  return;
}


static void handleGameoverCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::endGame)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the gameover command");
    return;
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, t);
  buf = nboPackUShort(buf, uint16_t(NoTeam));
  broadcastMessage(MsgScoreOver, (char*)buf - (char*)bufStart, bufStart);
  gameOver = true;
  if (clOptions->timeManualStart) {
    countdownActive = false;
    countdownPauseStart = TimeKeeper::getNullTime();
    clOptions->countdownPaused = false;
    return;
  }
}


static void handleCountdownCmd(GameKeeper::Player *playerData, const char *message)
{
  // /countdown starts timed game, if start is manual, everyone is allowed to
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::countdown)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the countdown command");
    return;
  } else if (!clOptions->timeManualStart) {
    sendMessage(ServerPlayer, t, "This server was not configured for manual clock countdowns");
    return;
  } else if (countdownDelay > 0) {
    sendMessage(ServerPlayer, t, "There is a countdown already in progress");
    return;
  }

  // if the timelimit is not set .. don't countdown
  if (clOptions->timeLimit > 1.0f) {
    std::vector<std::string> parts = TextUtils::tokenize(message, " \t",2);
    
    if (parts.size() > 1) {
      // we have an argument
      
      if (parts[1] == "pause") {
	// pause the countdown
	if (!countdownActive) {
	  sendMessage(ServerPlayer, t, "There is no active game to pause");
	  return;
	} else if (clOptions->countdownPaused) {
	  sendMessage(ServerPlayer, t, "The game is already paused");
	  return;
	}
	clOptions->countdownPaused = true;
	sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown paused by %s",playerData->player.getCallSign()).c_str());
	return;
      } else if (parts[1] == "resume") {
	// resume countdown if it was paused before
	if (!clOptions->countdownPaused) {
	  sendMessage(ServerPlayer, t, "The game is not paused");
	  return;
	}
	clOptions->countdownPaused = false;
	sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown resumed by %s",playerData->player.getCallSign()).c_str());
	return;
	      
      } else {
	// so it's the countdown delay? else tell the player how to use /countdown
	std::istringstream timespec(message+10);
	if (!(timespec >> countdownDelay)) {
	  sendMessage(ServerPlayer, t, "Usage: /countdown [<seconds>|pause|resume]");
	  return;
	}
      }
    } else {
      countdownDelay = 10;
    }
    
    // cancel here if a game is already running
    if (countdownActive) {
      sendMessage(ServerPlayer, t, "A game is already in progress");
      countdownDelay = -1;
      return;
    }

    // limit/sanity check
    const int max_delay = 120;
    if (countdownDelay > max_delay) {
      sendMessage(ServerPlayer, t, TextUtils::format("Countdown set to %d instead of %d", max_delay, countdownDelay).c_str());
      countdownDelay = max_delay;
    } else if (countdownDelay < 0) {
      sendMessage(ServerPlayer, t, TextUtils::format("Countdown set to 0 instead of %d", countdownDelay).c_str());
      countdownDelay = 0;
    }

    sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Team scores reset, countdown started by %s.",playerData->player.getCallSign()).c_str());

    // let everyone know what's going on
    long int timeArray[4];
    std::string matchBegins;
    if (countdownDelay == 0) {
      matchBegins = "Match begins now!";
    } else {
      TimeKeeper::convertTime(countdownDelay, timeArray);
      std::string countdowntime = TimeKeeper::printTime(timeArray);
      matchBegins = TextUtils::format("Match begins in about %s", countdowntime.c_str());
    }
    sendMessage(ServerPlayer, AllPlayers, matchBegins.c_str());

    TimeKeeper::convertTime(clOptions->timeLimit, timeArray);
    std::string timelimit = TimeKeeper::printTime(timeArray);
    matchBegins = TextUtils::format("Match duration is %s", timelimit.c_str());
    sendMessage(ServerPlayer, AllPlayers, matchBegins.c_str());
		
    // make sure the game always start unpaused
    clOptions->countdownPaused = false;
    countdownPauseStart = TimeKeeper::getNullTime();

  } else {
    sendMessage(ServerPlayer, AllPlayers, "Team scores reset.");
    sendMessage(ServerPlayer, t, "The server is not configured for timed matches.");
  }

  // reset team scores
  for (int i = RedTeam; i <= PurpleTeam; i++) {
    team[i].team.lost = team[i].team.won = 0;
  }
  sendTeamUpdate();

  return;
}


static void handleFlagCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::flagMod)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flag command");
    return;
  }
  if (strncasecmp(message + 6, "reset", 5) == 0) {
    bool onlyUnused = strncasecmp(message + 11, " unused", 7) == 0;
    bz_resetFlags(onlyUnused);
  } else if (strncasecmp(message + 6, "up", 2) == 0) {
    for (int i = 0; i < numFlags; i++) {
      FlagInfo &flag = *FlagInfo::get(i);
      if (flag.flag.type->flagTeam == ::NoTeam) {
	sendDrop(flag);
	flag.flag.status = FlagGoing;
	if (!flag.required)
	  flag.flag.type = Flags::Null;
	sendFlagUpdate(flag);
      }
    }

  } else if (strncasecmp(message + 6, "show", 4) == 0) {
    for (int i = 0; i < numFlags; i++) {
      char showMessage[MessageLen];
      FlagInfo::get(i)->getTextualInfo(showMessage);
      sendMessage(ServerPlayer, t, showMessage);
    }
  } else {
    sendMessage(ServerPlayer, t, "reset|show|up");
  }
  return;
}


static void handleKickCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::kick)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the kick command");
    return;
  }
  int i;
  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 3) {
    sendMessage(ServerPlayer, t, "Syntax: /kick <#slot | PlayerName | \"Player Name\"> <reason>");
    sendMessage(ServerPlayer, t, "	Please keep in mind that reason is displayed to the user.");
    return;
  }
  

  i = getSlotNumber(argv[1]);

  if (i >= 0) {
		
		// call any plugin events registered for /kick
		bz_KickEventData kickEvent;
		kickEvent.kickerID = t;
		kickEvent.kickedID = i;
		kickEvent.reason = argv[2].c_str();
		
		worldEventManager.callEvents(bz_eKickEvent,&kickEvent);
		

    char kickmessage[MessageLen];
    
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);

    // admins can override antiperms
    if (!playerData->accessInfo.isAdmin()) {
      // otherwise make sure the player is not protected with an antiperm      
      if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antikick))) {
	snprintf(kickmessage, MessageLen, "%s is protected from being kicked.", p->player.getCallSign());
	sendMessage(ServerPlayer, i, kickmessage);
	return;
      }
    }

    snprintf(kickmessage, MessageLen, "You were kicked off the server by %s",
	    playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, kickmessage);
    if (argv.size() > 2) {
      snprintf(kickmessage, MessageLen, " reason given : %s",argv[2].c_str());
      sendMessage(ServerPlayer, i, kickmessage);
    }
    snprintf(kickmessage, MessageLen, "%s kicked by %s, reason: %s", p->player.getCallSign(), playerData->player.getCallSign(),argv[2].c_str());
    sendMessage(ServerPlayer, AdminPlayers, kickmessage);
    removePlayer(i, "/kick");
  } else {
    char errormessage[MessageLen];
    snprintf(errormessage, MessageLen, "player \"%s\" not found", argv[1].c_str());
    sendMessage(ServerPlayer, t, errormessage);
  }
  return;
}

static void handleKillCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::kill)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the kill command");
    return;
  }
  int i;
  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t, "Syntax: /kill <#slot | PlayerName | \"Player Name\">  [reason]");
    sendMessage(ServerPlayer, t, "	Please keep in mind that reason is displayed to the user.");
    return;
  }

  i = getSlotNumber(argv[1]);

  if (i >= 0) {
		// call any plugin events registered for /kick
		bz_KillEventData killEvent;
		killEvent.killerID = t;
		killEvent.killedID = i;
		if (argv.size() > 2)
			killEvent.reason = argv[2].c_str();
		
		worldEventManager.callEvents(bz_eKillEvent,&killEvent);

		
    char killmessage[MessageLen];

    // admins can override antiperms
    if (!playerData->accessInfo.isAdmin()) {
      // otherwise make sure the player is not protected with an antiperm
      GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
      if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antikill))) {
	snprintf(killmessage, MessageLen, "%s is protected from being killed.", p->player.getCallSign());
	sendMessage(ServerPlayer, i, killmessage);
	return;
      }
    }

    snprintf(killmessage, MessageLen, "You were killed by %s",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, killmessage);
    if (argv.size() > 2) {
      snprintf(killmessage, MessageLen, " reason given : %s",argv[2].c_str());
      sendMessage(ServerPlayer, i, killmessage);
    }
    // kill the player
    playerKilled(i, ServerPlayer, 0, -1, Flags::Null, -1);

  } else {
    char errormessage[MessageLen];
    snprintf(errormessage, MessageLen, "player \"%s\" not found", argv[1].c_str());
    sendMessage(ServerPlayer, t, errormessage);
  }
  return;
}

static void handleBanlistCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendBans(t);
  return;
}


static void handleHostBanlistCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendHostBans(t);
  return;
}


static void handleBanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) &&
      !playerData->accessInfo.hasPerm(PlayerAccessInfo::shortBan)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }

  std::string msg = message;
  std::vector<std::string> argv = TextUtils::tokenize(msg, " \t", 4, true);

  if (argv.size() < 4) {
    sendMessage(ServerPlayer, t, "Syntax: /ban <#slot | PlayerName | \"Player Name\" | ip> <duration> <reason>");
    sendMessage(ServerPlayer, t, "	Please keep in mind that reason is displayed to the user.");
  } else {
    std::string ip = argv[1];
    std::string reason;
    int durationInt = clOptions->banTime;
    
    int victim = getSlotNumber(argv[1]);
    
    if (victim >= 0) {
      // valid slot or callsign
      GameKeeper::Player *playerBannedData  = GameKeeper::Player::getPlayerByIndex(victim);
      if (playerBannedData)
	ip = playerBannedData->netHandler->getTargetIP();
    }

    // check the ban duration
    regex_t preg;
    int res = regcomp(&preg, "^([[:digit:]]+[hwd]?)+$",
		      REG_ICASE | REG_NOSUB | REG_EXTENDED);
    res = regexec(&preg,argv[2].c_str(), 0, NULL, 0);
    regfree(&preg);
    if (res == REG_NOMATCH) {
      sendMessage(ServerPlayer, t, "Error: invalid ban duration");
      sendMessage(ServerPlayer, t, "Duration examples:  30 1h  1d  1w  and mixing: 1w2d4h 1w2d1");
      return;
    }
    

      int specifiedDuration = TextUtils::parseDuration(argv[2]);
      if ((durationInt > 0) &&
	  ((specifiedDuration > durationInt) || (specifiedDuration <= 0)) &&
	  !playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
	sendMessage (ServerPlayer, t, "You only have SHORTBAN privileges,"
				      " using default ban time");
      } else {
	durationInt = specifiedDuration;
      }

    // set the ban reason
    if (argv.size() == 4) {
      reason = argv[3];
    }
		
		// call any plugin events registered for /ban
		bz_BanEventData banEvent;
		banEvent.bannerID = t;
		banEvent.ipAddress = ip.c_str();
		banEvent.reason = reason.c_str();
		banEvent.duration = durationInt;
		// if we know for sure who is to be banned, submit it	
		if (victim >= 0)
			banEvent.banneeID = victim;
		
		worldEventManager.callEvents(bz_eBanEvent,&banEvent);

    // reload the banlist in case anyone else has added
    clOptions->acl.load();

    if (clOptions->acl.ban(ip, playerData->player.getCallSign(), durationInt,
			   reason.c_str())) {
      clOptions->acl.save();

      sendMessage(ServerPlayer, t, "IP pattern added to banlist");

      char kickmessage[MessageLen];
      GameKeeper::Player *otherPlayer;
      for (int i = 0; i < curMaxPlayers; i++) {
	otherPlayer = GameKeeper::Player::getPlayerByIndex(i);
	if (otherPlayer && !clOptions->acl.validate
	    (otherPlayer->netHandler->getIPAddress())) {

	  // admins can override antiperms
	  if (!playerData->accessInfo.isAdmin()) {
	    // make sure this player isn't protected
	    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
	    if ((p != NULL)
		&& (p->accessInfo.hasPerm(PlayerAccessInfo::antiban))) {
	      snprintf(kickmessage, MessageLen, 
		      "%s is protected from being banned (skipped).",
		      p->player.getCallSign());
	      sendMessage(ServerPlayer, t, kickmessage);
	      continue;
	    }
	  }

	  snprintf(kickmessage, MessageLen, "You were banned from this server by %s",
		  playerData->player.getCallSign());
	  sendMessage(ServerPlayer, i, kickmessage);
	  if (reason.length() > 0) {
	    snprintf(kickmessage, MessageLen, "Reason given: %s", reason.c_str());
	    sendMessage(ServerPlayer, i, kickmessage);
	  }
	  if (otherPlayer) {
	    snprintf(kickmessage, MessageLen, "%s banned by %s, reason: %s", otherPlayer->player.getCallSign(), playerData->player.getCallSign(),reason.c_str());
	    sendMessage(ServerPlayer, AdminPlayers, kickmessage);
	  }
	  removePlayer(i, "/ban");
	}
      }
    } else {
      char errormessage[MessageLen];
      snprintf(errormessage, MessageLen, "Malformed address or invalid Player/Slot: %s", argv[1].c_str());
      sendMessage(ServerPlayer, t, errormessage);
    }
  }
  return;
}


static void handleHostBanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) &&
      !playerData->accessInfo.hasPerm(PlayerAccessInfo::shortBan)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }

  std::string msg = message;
  std::vector<std::string> argv = TextUtils::tokenize( msg, " \t", 4 );

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t, "Syntax: /hostban <host pattern> [duration] [reason]");
    sendMessage(ServerPlayer, t, "	Please keep in mind that reason is displayed to the user.");
  } else {
    std::string hostpat = argv[1];
    std::string reason;
    int durationInt = clOptions->banTime;

    // set the ban time
    if (argv.size() >= 3) {
      int specifiedDuration = TextUtils::parseDuration(argv[2]);
      if ((durationInt > 0) &&
	  ((specifiedDuration > durationInt) || (specifiedDuration <= 0)) &&
	  !playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
	sendMessage (ServerPlayer, t, "You only have SHORTBAN privileges,"
				      " using default ban time");
      } else {
	durationInt = specifiedDuration;
      }
    }

    // set the ban reason
    if( argv.size() == 4 ) {
      reason = argv[3];
    }

		// call any plugin events registered for /hostban
		bz_HostBanEventData hostBanEvent;
		hostBanEvent.bannerID = t;
		hostBanEvent.hostPattern = hostpat.c_str();
		hostBanEvent.reason = reason.c_str();
		hostBanEvent.duration = durationInt;
		
		worldEventManager.callEvents(bz_eHostBanEvent,&hostBanEvent);

		
    clOptions->acl.hostBan(hostpat, playerData->player.getCallSign(),
			   durationInt,
			   reason.c_str());
    clOptions->acl.save();

    GameKeeper::Player::setAllNeedHostbanChecked(true);

    sendMessage(ServerPlayer, t, "Host pattern added to banlist");
  }
  return;
}


static void handleUnbanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unban command");
    return;
  }

  if (clOptions->acl.unban(message + 7)) {
    sendMessage(ServerPlayer, t, "Removed IP pattern from the ban list");
    clOptions->acl.save();
  } else {
    sendMessage(ServerPlayer, t, "No pattern removed");
  }
  return;
}

static void handleHostUnbanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unban command");
    return;
  }

  if (clOptions->acl.hostUnban(message + 11)) {
    sendMessage(ServerPlayer, t, "Removed host pattern from the ban list");
    clOptions->acl.save();
  } else {
    sendMessage(ServerPlayer, t, "No pattern removed");
  }
  return;
}


static void handleLagwarnCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::lagwarn)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagwarn command");
    return;
  }

  char reply[MessageLen] = {0};

  if (message[8] == ' ') {
    const char *maxlag = message + 9;
    clOptions->lagwarnthresh = (float) (atoi(maxlag) / 1000.0);
    snprintf(reply, MessageLen, "lagwarn is now %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
  } else {
    snprintf(reply, MessageLen, "lagwarn is set to %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
  }
  sendMessage(ServerPlayer, t, reply);
  return;
}


bool lagCompare(const GameKeeper::Player *a, const GameKeeper::Player *b)
{
  if (a->player.isObserver() && !b->player.isObserver())
    return true;
  if (!a->player.isObserver() && b->player.isObserver())
    return false;
  return a->lagInfo.getLag() < b->lagInfo.getLag();
}

static void handleLagstatsCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::lagStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagstats command");
    return;
  }
  // yeah, ok this is ugly but it works - curMaxPlayers is never >255
  GameKeeper::Player *sortedPlayer[256];
  int i = 0, j = 0;
  for (i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if (p != NULL) {
      sortedPlayer[j++] = p;
    }
  }
  std::sort(sortedPlayer, sortedPlayer + j, lagCompare);

  char reply[MessageLen];
  for (i = 0; i < j; i++) {
    GameKeeper::Player *p = sortedPlayer[i];
    p->lagInfo.getLagStats(reply);
    if (reply[0])
      sendMessage(ServerPlayer, t, reply);
  }
}


static void handleIdlestatsCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::idleStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the idlestats command");
    return;
  }

  GameKeeper::Player *otherData;
  std::string reply;
  for (int i = 0; i < curMaxPlayers; i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (!otherData)
      continue;
    reply = otherData->player.getIdleStat();
    if (reply != "")
      sendMessage(ServerPlayer, t, reply.c_str());
  }
  return;
}


static void handleFlaghistoryCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::flagHistory)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flaghistory command");
    return;
  }

  char reply[MessageLen];
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData != NULL && otherData->player.isPlaying()
	&& !otherData->player.isObserver()) {
      snprintf(reply, MessageLen, "%-16s : ", otherData->player.getCallSign());
      otherData->flagHistory.get(reply+strlen(reply));
      sendMessage(ServerPlayer, t, reply);
    }
  }
}


static void handlePlayerlistCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::playerList)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the playerlist command");
    return;
  }

  GameKeeper::Player *otherData;
  char reply[MessageLen] = {0};

  for (int i = 0; i < curMaxPlayers; i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->player.isPlaying()) {
      otherData->netHandler->getPlayerList(reply);
      sendMessage(ServerPlayer, t, reply);
    }
  }
  return;
}


static void handleReportCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::talk)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the report command");
    return;
  }

  if (strlen(message + 1) < 8) {
    sendMessage(ServerPlayer, t, "Nothing reported");
  } else {
    time_t now = time(NULL);
    char* timeStr = ctime(&now);
    std::string reportStr;
    reportStr = reportStr + timeStr + "Reported by " +
      playerData->player.getCallSign() + ": " + (message + 8);
    if (clOptions->reportFile.size() > 0) {
      std::ofstream ofs(clOptions->reportFile.c_str(), std::ios::out | std::ios::app);
      ofs << reportStr << std::endl << std::endl;
    }
    if (clOptions->reportPipe.size() > 0) {
      FILE* pipeWrite = popen(clOptions->reportPipe.c_str(), "w");
      if (pipeWrite != NULL) {
	fprintf(pipeWrite, "%s\n\n", reportStr.c_str());
      } else {
	DEBUG1("Couldn't write report to the pipe\n");
      }
      pclose(pipeWrite);
    }
    if (clOptions->reportFile.size() == 0 && clOptions->reportPipe.size() == 0) {
      sendMessage(ServerPlayer, t, "The report command is disabled on this server");
    } else {
      std::string temp = std::string("**\"") + playerData->player.getCallSign() + "\" reports: " +
			 (message + 8);
      if (temp.size() <= (unsigned) MessageLen) {
	sendMessage (ServerPlayer, AdminPlayers, temp.c_str());
	return;
      }
      const std::vector<std::string> words = TextUtils::tokenize(temp, " \t");
      unsigned int cur = 0;
      const unsigned int wordsize = words.size();
      while (cur != wordsize) {
	std::string temp2;
	while (temp2.size() <= (unsigned) MessageLen &&
	       cur != wordsize &&
	       (temp2.size() + words[cur].size()) <= (unsigned) MessageLen) {
	    temp2 += words[cur] + " ";
	    ++cur;
	}
	sendMessage (ServerPlayer, AdminPlayers, temp2.c_str());
      }
      sendMessage (ServerPlayer, AdminPlayers, message);
      DEBUG1("Player %s [%d] has filed a report (time: %s).\n",
	     playerData->player.getCallSign(), t, timeStr);

      sendMessage(ServerPlayer, t, "Your report has been filed. Thank you.");
    }
  }

  return;
}


static void handleHelpCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};

  if (strlen(message + 1) == 4) {
    const std::vector<std::string>& chunks = clOptions->textChunker.getChunkNames();
    sendMessage(ServerPlayer, t, "Available help pages (use /help <page>)");
    for (int i = 0; i < (int) chunks.size(); i++) {
      sendMessage(ServerPlayer, t, chunks[i].c_str());
    }
  } else {
    bool foundChunk = false;
    const std::vector<std::string>& chunks = clOptions->textChunker.getChunkNames();
    for (int i = 0; i < (int)chunks.size() && (!foundChunk); i++) {
      if (chunks[i] == (message +6)){
	const std::vector<std::string>* lines = clOptions->textChunker.getTextChunk((message + 6));
	if (lines != NULL) {
	  for (int j = 0; j < (int)lines->size(); j++) {
	    sendMessage(ServerPlayer, t, (*lines)[j].c_str());
	  }
	  foundChunk = true;
	  break;
	}
      }
    }
    if (!foundChunk) {
      snprintf(reply, MessageLen, "Help command %s not found", message + 6);
      sendMessage(ServerPlayer, t, reply);
    }
  }
  return;
}


static void handleIdentifyCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/identify command disabled");
    return;
  }
  // player is trying to send an ID
  if (playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You have already identified");
  } else if (playerData->accessInfo.gotAccessFailure()) {
    sendMessage(ServerPlayer, t, "You have attempted to identify too many times");
  } else {
    // get their info
    if (!playerData->accessInfo.isRegistered()) {
      // not in DB, tell them to reg
      sendMessage(ServerPlayer, t, "This callsign is not registered,"
		  " please register it with a /register command");
    } else {
      if (playerData->accessInfo.isPasswordMatching(message + 10)) {
	sendMessage(ServerPlayer, t, "Password Accepted, welcome back.");

	// get their real info
	playerData->accessInfo.setPermissionRights();

	// if they have the PLAYERLIST permission, send the IP list
	sendIPUpdate(t, -1);
	sendPlayerInfo();
      } else {
	playerData->accessInfo.setLoginFail();
	sendMessage(ServerPlayer, t, "Identify Failed, please make sure"
		    " your password was correct");
      }
    }
  }
  return;
}


static void handleRegisterCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/register command disabled");
    return;
  }
  if (playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You have already registered and"
		" identified this callsign");
  } else {
    if (playerData->accessInfo.isRegistered()) {
      sendMessage(ServerPlayer, t, "This callsign is already registered,"
		  " if it is yours /identify to login");
    } else {
      if (strlen(message) > 12) {
	playerData->accessInfo.storeInfo(message + 10);
	sendMessage(ServerPlayer, t, "Callsign registration confirmed,"
		    " please /identify to login");
      } else {
	sendMessage(ServerPlayer, t, "Your password must be 3 or more characters");
      }
    }
  }
  return;
}


static void handleGhostCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/ghost command disabled");
    return;
  }
  char *p1 = (char*)strchr(message + 1, '\"');
  char *p2 = 0;
  if (p1) p2 = strchr(p1 + 1, '\"');
  if (!p2) {
    sendMessage(ServerPlayer, t, "not enough parameters, usage"
		" /ghost \"CALLSIGN\" PASSWORD");
  } else {
    std::string ghostie(p1 + 1, p2 - p1 - 1);
    std::string ghostPass = p2 + 2;

    makeupper(ghostie);

    int user = GameKeeper::Player::getPlayerIDByName(ghostie);
    if (user == -1) {
      sendMessage(ServerPlayer, t, "There is no user logged in by that name");
    } else {
      if (!userExists(ghostie)) {
	sendMessage(ServerPlayer, t, "That callsign is not registered");
      } else {
	if (!verifyUserPassword(ghostie, ghostPass)) {
	  sendMessage(ServerPlayer, t, "Invalid Password");
	} else {
	  sendMessage(ServerPlayer, t, "Ghosting User");
	  char temp[MessageLen];
	  snprintf(temp, MessageLen, "Your Callsign is registered to another user,"
		  " You have been ghosted by %s",
		  playerData->player.getCallSign());
	  sendMessage(ServerPlayer, user, temp);
	  removePlayer(user, "Ghost");
	}
      }
    }
  }
  return;
}


static void handleDeregisterCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/deregister command disabled");
    return;
  }
  if (!playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the deregister command");
    return;
  }

  if (strlen(message) == 11) {
    // removing own callsign
    passwordDatabase.erase(playerData->accessInfo.getName());
    userDatabase.erase(playerData->accessInfo.getName());
    PlayerAccessInfo::updateDatabases();
    sendMessage(ServerPlayer, t, "Your callsign has been deregistered");
  } else if (strlen(message) > 12
	     && playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    char reply[MessageLen];

    // removing someone else's
    std::string name = message + 12;
    makeupper(name);
    if (userExists(name)) {

      // admins can override antiperms
      if (!playerData->accessInfo.isAdmin()) {
	// make sure this player isn't protected
	int v = GameKeeper::Player::getPlayerIDByName(name);
	GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(v);
	if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antideregister))) {
	  snprintf(reply, MessageLen, "%s is protected from being deregistered.", p->player.getCallSign());
	  sendMessage(ServerPlayer, t, reply);
	  return;
	}
      }

      passwordDatabase.erase(name);
      userDatabase.erase(name);
      PlayerAccessInfo::updateDatabases();

      snprintf(reply, MessageLen, "%s has been deregistered", name.c_str());
      sendMessage(ServerPlayer, t, reply);
    } else {
      snprintf(reply, MessageLen, "user %s does not exist", name.c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to deregister this user");
  }

  return;
}


static void handleSetpassCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/setpass command disabled");
    return;
  }
  if (!playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the setpass command");
    return;
  }

  size_t startPosition = 7;
  /* skip any leading whitespace */
  while (isspace(message[++startPosition]))
    ;
  if (startPosition == strlen(message) || !isspace(message[8])) {
    sendMessage(ServerPlayer, t, "Not enough parameters: usage /setpass PASSWORD");
    return;
  }
  std::string pass = message + startPosition;
  playerData->accessInfo.setPasswd(pass);
  char text[MessageLen];
  snprintf(text, MessageLen, "Your password is now set to \"%s\"", pass.c_str());
  sendMessage(ServerPlayer, t, text);
  return;
}


static void handleGrouplistCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  sendMessage(ServerPlayer, t, "Group List:");
  PlayerAccessMap::iterator itr = groupAccess.begin();
  while (itr != groupAccess.end()) {
    sendMessage(ServerPlayer, t, itr->first.c_str());
    itr++;
  }
  return;
}


static void handleShowgroupCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  std::string settie;

  if (strlen(message) == 10) {	 // show own groups
    if (playerData->accessInfo.isVerified()) {
      settie = playerData->accessInfo.getName();
    } else {
      sendMessage(ServerPlayer, t, "You are not identified");
    }
  } else if (playerData->accessInfo.hasPerm(PlayerAccessInfo::showOthers)) {
    // show groups for other player
    char *p1 = (char*) strchr(message + 1, '\"');
    char *p2 = 0;
    if (p1) p2 = strchr(p1 + 1, '\"');
    if (p2) {
      settie = std::string(p1 + 1, p2 - p1 - 1);
      makeupper(settie);
    } else {
      sendMessage(ServerPlayer, t, "wrong format, usage"
		  " /showgroup  or  /showgroup \"CALLSIGN\"");
    }
  } else {
    sendMessage(ServerPlayer, t, "No permission!");
  }

  // something is wrong
  if (settie != "") {
    int playerIndex = GameKeeper::Player::getPlayerIDByName(settie);
    // once for global groups
    if (playerIndex < curMaxPlayers) {
      GameKeeper::Player* target = GameKeeper::Player::getPlayerByIndex(playerIndex);
      if (target != NULL) {
	PlayerAccessInfo &info = target->accessInfo;
	// FIXME remove local groups from this list. better yet unify the two.
	std::string line = "Global Groups (only extras) for ";
	line += settie;
	line += ": ";
	std::vector<std::string>::iterator itr = info.groups.begin();
	while (itr != info.groups.end()) {
	  line += *itr;
	  line += " ";
	  itr++;
	}
	while (line.size() > (unsigned int)MessageLen) {
	  sendMessage(ServerPlayer, t, line.substr(0, MessageLen).c_str());
	  line.erase(line.begin(), line.begin() + (MessageLen - 1));
	}
	sendMessage(ServerPlayer, t, line.c_str());
      }
    }
    // once for local groups
    if (userExists(settie)) {
      PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

      std::string line = "Local groups for ";
      line += settie;
      line += ": ";
      std::vector<std::string>::iterator itr = info.groups.begin();
      while (itr != info.groups.end()) {
	line += *itr;
	line += " ";
	itr++;
      }
      while (line.size() > (unsigned int)MessageLen) {
	sendMessage(ServerPlayer, t, line.substr(0, MessageLen).c_str());
	line.erase(line.begin(), line.begin() + (MessageLen - 1));
      }
      sendMessage(ServerPlayer, t, line.c_str());
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return;
}


static void handleGrouppermsCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  sendMessage(ServerPlayer, t, "Group List:");
  PlayerAccessMap::iterator itr = groupAccess.begin();
  std::string line;
  while (itr != groupAccess.end()) {
    line = itr->first + ":   ";
    sendMessage(ServerPlayer, t, line.c_str());

    // allows first
    if (itr->second.explicitAllows.any()) {
      sendMessage(ServerPlayer, t, "  Allows");
      for (int i = 0; i < PlayerAccessInfo::lastPerm; i++) {
     	if (itr->second.explicitAllows.test(i) && !itr->second.explicitDenys.test(i) ) {
	  line = "     ";
	  line += nameFromPerm((PlayerAccessInfo::AccessPerm)i);
	  sendMessage(ServerPlayer, t, line.c_str());
      	}
      }
    }
		
    // same about denys
    if (itr->second.explicitDenys.any()) {
      sendMessage(ServerPlayer, t, "  Denys");
      for (int i = 0; i < PlayerAccessInfo::lastPerm; i++) {
     	if (itr->second.explicitDenys.test(i) ) {
	  line = "     ";
	  line += nameFromPerm((PlayerAccessInfo::AccessPerm)i);
	  sendMessage(ServerPlayer, t, line.c_str());
      	}
      }
    }
			
    itr++;
  }
  return;
}


static void handleSetgroupCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!userDatabaseFile.size()) {
    sendMessage(ServerPlayer, t, "/setgroup command disabled");
    return;
  }
  char *p1 = (char*)strchr(message + 1, '\"');
  char *p2 = 0;
  if (p1) p2 = strchr(p1 + 1, '\"');
  if (!p2) {
    sendMessage(ServerPlayer, t, "not enough parameters, usage /setgroup \"CALLSIGN\" GROUP");
  } else {
    std::string settie(p1 + 1, p2 - p1 - 1);
    std::string group = p2 + 2;

    makeupper(settie);
    makeupper(group);

    if (userExists(settie)) {
      if (!playerData->accessInfo.canSet(group)) {
	sendMessage(ServerPlayer, t, "You do not have permission to set this group");
      } else {
	PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

	if (info.addGroup(group)) {
	  sendMessage(ServerPlayer, t, "Group Add successful");
	  int getID = GameKeeper::Player::getPlayerIDByName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    snprintf(temp, MessageLen, "you have been added to the %s group, by %s",
		    group.c_str(), playerData->player.getCallSign());
	    sendMessage(ServerPlayer, getID, temp);
	    GameKeeper::Player::getPlayerByIndex(getID)->accessInfo.
	      addGroup(group);
	  }
	  PlayerAccessInfo::updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "Group Add failed (user may already be in that group)");
	}
      }
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return;
}


static void handleRemovegroupCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!userDatabaseFile.size()) {
    sendMessage(ServerPlayer, t, "/removegroup command disabled");
    return;
  }
  char *p1 = (char*)strchr(message + 1, '\"');
  char *p2 = 0;
  if (p1) p2 = strchr(p1 + 1, '\"');
  if (!p2) {
    sendMessage(ServerPlayer, t, "not enough parameters, usage /removegroup \"CALLSIGN\" GROUP");
  } else {
    std::string settie(p1 + 1, p2 - p1 - 1);
    std::string group = p2 + 2;

    makeupper(settie);
    makeupper(group);
    if (userExists(settie)) { 
      if (!playerData->accessInfo.canSet(group)) {
	sendMessage(ServerPlayer, t, "You do not have permission to remove this group");
      } else {
	PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);
	if (info.removeGroup(group)) {
	  sendMessage(ServerPlayer, t, "Group Remove successful");
	  int getID = GameKeeper::Player::getPlayerIDByName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    snprintf(temp, MessageLen, "You have been removed from the %s group, by %s",
		    group.c_str(), playerData->player.getCallSign());
	    sendMessage(ServerPlayer, getID, temp);
	    GameKeeper::Player::getPlayerByIndex(getID)->accessInfo.
	      removeGroup(group);
	  }
	  PlayerAccessInfo::updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "Group Remove failed (user may not have been in group)");
	}
      }
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return;
}


static void handleReloadCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the reload command");
    return;
  }

  // reload the banlist
  clOptions->acl.load();

  groupAccess.clear();
  userDatabase.clear();
  passwordDatabase.clear();
  initGroups();
  if (passFile.size())
    readPassFile(passFile);
  if (userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);
  GameKeeper::Player::reloadAccessDatabase();
  sendMessage(ServerPlayer, t, "Databases reloaded");

  // Validate all of the current players

  std::string reason;
  char kickmessage[MessageLen];

  // Check host bans
  GameKeeper::Player::setAllNeedHostbanChecked(true);

  // Check IP bans
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *otherPlayer = GameKeeper::Player::getPlayerByIndex(i);
      if (otherPlayer && !clOptions->acl.validate
	(otherPlayer->netHandler->getIPAddress())) {
	  // admins can override antiperms
	  if (!playerData->accessInfo.isAdmin()) {
	    // make sure this player isn't protected
	    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
	    if ((p != NULL)
		&& (p->accessInfo.hasPerm(PlayerAccessInfo::antiban))) {
	      snprintf(kickmessage, MessageLen, 
		    "%s is protected from being banned (skipped).",
		    p->player.getCallSign());
	      sendMessage(ServerPlayer, t, kickmessage);
	      continue;
	    }
	  }

	  snprintf(kickmessage, MessageLen, "You were banned from this server by %s",
		  playerData->player.getCallSign());
	  sendMessage(ServerPlayer, i, kickmessage);
	  if (reason.length() > 0) {
	    snprintf(kickmessage, MessageLen, "Reason given: %s", reason.c_str());
	    sendMessage(ServerPlayer, i, kickmessage);
	  }
	  removePlayer(i, "/ban");
	}
  }
  return;
}


static void handleVoteCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};
  std::string callsign = std::string(playerData->player.getCallSign());

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::vote)) {
    /* permission denied for /vote */
    snprintf(reply, MessageLen, "%s, you are presently not authorized to run /vote", callsign.c_str());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return;
  }

  // only need to get this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure that there is a poll to vote upon */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sendMessage(ServerPlayer, t, "A poll is not presently in progress.  There is nothing to vote on");
    return;
  }

  std::string voteCmd = &message[5];
  std::string answer;

  /* find the start of the vote answer */
  size_t startPosition = 0;
  while ((startPosition < voteCmd.size()) &&
	 (isspace(voteCmd[startPosition]))) {
    startPosition++;
  }

  /* stash the answer ('yes', 'no', etc) in lowercase to simplify comparison */
  for (size_t i = startPosition;  i < voteCmd.size() && !isspace(voteCmd[i]); i++) {
    answer += tolower(voteCmd[i]);
  }

  std::vector<std::string> yesAnswers;
  yesAnswers.push_back("y");
  yesAnswers.push_back("1");
  yesAnswers.push_back("yes");
  yesAnswers.push_back("yea");
  yesAnswers.push_back("si"); // spanish+
  yesAnswers.push_back("ja");  // german
  yesAnswers.push_back("oui"); // french
  yesAnswers.push_back("sim"); // portuguese
  yesAnswers.push_back("tak"); // polish

  std::vector<std::string> noAnswers;
  noAnswers.push_back("n");
  noAnswers.push_back("0");
  noAnswers.push_back("no");
  noAnswers.push_back("nay");
  noAnswers.push_back("nein"); // german
  noAnswers.push_back("nien"); // german misspelled
  noAnswers.push_back("non"); // french
  noAnswers.push_back("nao"); // portuguese
  noAnswers.push_back("nie"); // polish

  // see if the vote response is a valid yes or no answer
  int vote = -1;
  unsigned int maxAnswerCount = noAnswers.size() > yesAnswers.size() ? noAnswers.size() : yesAnswers.size();
  for (unsigned int v = 0; v < maxAnswerCount; v++) {
    if (v < yesAnswers.size()) {
      if (answer == yesAnswers[v]) {
	vote = 1;
	break;
      }
    }
    if (v < noAnswers.size()) {
      if (answer == noAnswers[v]) {
	vote = 0;
	break;
      }
    }
  }

  // cast the vote or complain
  bool cast = false;
  if (vote == 0) {
    if ((cast = arbiter->voteNo(callsign)) == true) {
      /* player voted no */
      snprintf(reply, MessageLen, "%s, your vote in opposition of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else if (vote == 1) {
    if ((cast = arbiter->voteYes(callsign)) == true) {
      /* player voted yes */
      snprintf(reply, MessageLen, "%s, your vote in favor of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else {
    if (answer.length() == 0) {
      snprintf(reply, MessageLen, "%s, you did not provide a vote answer", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
    } else {
      snprintf(reply, MessageLen, "%s, you did not vote in favor or in opposition", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
    }
    sendMessage(ServerPlayer, t, "Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao");
    return;
  }

  if (arbiter->hasVoted(callsign)) {
    /* player already voted */
    snprintf(reply, MessageLen, "%s, you have already voted on the poll to %s %s", callsign.c_str(), arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  if (!cast){
    /* There was an error while voting, probably could send a less generic message */
    snprintf(reply, MessageLen, "%s, there was an error while voting on the poll to %s %s", callsign.c_str(), arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  return;
}


static void handleVetoCmd(GameKeeper::Player *playerData, const char * /*message*/)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::veto)) {
    /* permission denied for /veto */
    sendMessage(ServerPlayer, t,
		TextUtils::format
		("%s, you are presently not authorized to run /veto",
		 playerData->player.getCallSign()).c_str());
    return;
  }

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return;
  }

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure there is an unexpired poll */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sendMessage(ServerPlayer, t,
		TextUtils::format
		("%s, there is presently no active poll to veto",
		 playerData->player.getCallSign()).c_str());
    return;
  }

  sendMessage(ServerPlayer, t,
	      TextUtils::format("%s, you have cancelled the poll to %s %s",
				  playerData->player.getCallSign(),
				  arbiter->getPollAction().c_str(),
				  arbiter->getPollTarget().c_str()).c_str());

  /* poof */
  arbiter->forgetPoll();

  sendMessage(ServerPlayer, AllPlayers,
	      TextUtils::format("The poll was cancelled by %s",
				  playerData->player.getCallSign()).c_str());

  return;
}


static void handlePollCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};
  std::string callsign = std::string(playerData->player.getCallSign());

  DEBUG2("\"%s\" has requested a poll: %s\n", callsign.c_str(), message);

  /* make sure player has permission to request a poll */
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::poll)) {
    snprintf(reply, MessageLen, "%s, you are presently not authorized to run /poll", callsign.c_str());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  DEBUG3("Player has permission to run /poll\n");

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return;
  }

  DEBUG3("BZDB poll value is not empty\n");

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  DEBUG3("Arbiter was acquired with address 0x%p\n", arbiter);

  /* make sure that there is not a poll active already */
  if (arbiter->knowsPoll()) {
    snprintf(reply, MessageLen, "A poll to %s %s is presently in progress", arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    sendMessage(ServerPlayer, t, "Unable to start a new poll until the current one is over");
    return;
  }

  DEBUG3("The arbiter says there is not another poll active\n");

  // get available voter count
  unsigned short int available = 0;
  for (int i = 0; i < curMaxPlayers; i++) {
    // any registered/known users on the server (including observers)
    // are eligible to vote
    GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->accessInfo.exists()) {
      available++;
    }
  }

  DEBUG3("There are %d available players for %d votes required\n", available, clOptions->votesRequired);

  /* make sure there are enough players to even make a poll that has a chance
   * of succeeding (not counting the person being acted upon)
   */
  if (available - 1 < clOptions->votesRequired) {
    sendMessage(ServerPlayer, t, "Unable to initiate a new poll.  There are not enough registered players playing.");
    snprintf(reply, MessageLen, "There needs to be at least %d other %s and only %d %s available.",
	    clOptions->votesRequired,
	    clOptions->votesRequired - 1 == 1 ? "player" : "players",
	    available - 1,
	    available - 1 == 1 ? "is" : "are");
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  std::string arguments = &message[5]; /* skip "/poll" */
  std::string cmd = "";

  DEBUG3("The arguments string is [%s]\n", arguments.c_str());

  /* find the start of the command */
  size_t startPosition = 0;
  while ((startPosition < arguments.size()) &&
	 (isspace(arguments[startPosition]))) {
    startPosition++;
  }

  DEBUG3("Start position is %d\n", (int)startPosition);

  /* find the end of the command */
  size_t endPosition = startPosition + 1;
  while ((endPosition < arguments.size()) &&
	 (!isspace(arguments[endPosition]))) {
    endPosition++;
  }

  DEBUG3("End position is %d\n", (int)endPosition);

  /* stash the command ('kick', etc) in lowercase to simplify comparison */
  if ((startPosition != arguments.size()) &&
      (endPosition > startPosition)) {
    for (size_t i = startPosition; i < endPosition; i++) {
      cmd += tolower(arguments[i]);
    }
  }

  DEBUG3("Command is %s\n", cmd.c_str());

  /* handle subcommands */

  if ((cmd == "ban") || (cmd == "kick") || (cmd == "kill") || (cmd == "set") || (cmd == "flagreset")) {
    std::string target;
    std::string targetIP = "";

    arguments = arguments.substr(endPosition);

    if (arguments.size() == 0) {
      sendMessage(ServerPlayer, t, "/poll: incorrect syntax, argument required.");
      DEBUG3("No command arguments, stopping poll.\n");
      return;
    }
    
    DEBUG3("Command arguments are [%s]\n", arguments.c_str());

    /* find the start of the target (e.g. player name) */
    startPosition = 0;
    while ((startPosition < arguments.size()) &&
	   (isspace(arguments[startPosition]))) {
      startPosition++;
    }
    // do not include a starting quote, if given
    if (arguments[startPosition] == '"') {
      startPosition++;
    }

    DEBUG3("Start position for target is %d\n", (int)startPosition);

    /* find the end of the target */
    endPosition = arguments.size() - 1;
    while ((endPosition > 0) &&
	   (isspace(arguments[endPosition]))) {
      endPosition--;
    }
    // do not include a trailing quote, if given
    if (arguments[endPosition] == '"') {
      endPosition--;
    }

    DEBUG3("End position for target is %d\n", (int)endPosition);

    target = arguments.substr(startPosition, endPosition - startPosition + 1);

    DEBUG3("Target specified to vote upon is [%s]\n", target.c_str());

    if ((target.length() == 0) && (cmd != "flagreset")) {
      snprintf(reply, MessageLen, "%s, no target was specified for the [%s] vote", callsign.c_str(), cmd.c_str());
      sendMessage(ServerPlayer, t, reply);
      snprintf(reply, MessageLen, "Usage: /poll %s target", cmd.c_str());
      sendMessage(ServerPlayer, t, reply);
      return;
    }

    // Make sure the specific poll type is allowed
    if ((cmd == "set") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollSet))) {
      snprintf(reply, MessageLen, "%s, you may not /poll set on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll set\n", callsign.c_str());
      return;
    }
    if ((cmd == "flagreset") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollFlagReset))) {
      snprintf(reply, MessageLen, "%s, you may not /poll flagreset on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll flagreset\n", callsign.c_str());
      return;
    }
    if ((cmd == "ban") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollBan))) {
      snprintf(reply, MessageLen, "%s, you may not /poll ban on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll ban\n", callsign.c_str());
      return;
    }
    if ((cmd == "kick") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKick))) {
      snprintf(reply, MessageLen, "%s, you may not /poll kick on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll kick\n", callsign.c_str());
      return;
    }

    if ((cmd == "kill") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKill))) {
      snprintf(reply, MessageLen, "%s, you may not /poll kill on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll kill\n", callsign.c_str());
      return;
    }

    if ((cmd != "set") && (cmd != "flagreset")) {
      // all polls that are not set or flagreset polls take a player name

      /* make sure the requested player is actually here */
      int v = GameKeeper::Player::getPlayerIDByName(target);
      if (v >= curMaxPlayers) {
	/* wrong name? */
	snprintf(reply, MessageLen, 
		"The player specified for a %s vote is not here", cmd.c_str());
	sendMessage(ServerPlayer, t, reply);
	return;
      }
      GameKeeper::Player* targetData = GameKeeper::Player::getPlayerByIndex(v);
      if (!targetData) {
	/* wrong name? */
	snprintf(reply, MessageLen, "The server has no information on %s.", cmd.c_str());
	sendMessage(ServerPlayer, t, reply);
	return;
      }
      targetIP = targetData->netHandler->getTargetIP();

      // admins can override antiperms
      if (!playerData->accessInfo.isAdmin()) {
	// otherwise make sure the player is not protected with an antiperm
	GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(v);
	if (p != NULL) {
	  if (p->accessInfo.hasPerm(PlayerAccessInfo::antipoll)) {
	    snprintf(reply, MessageLen, "%s is protected from being polled against.", p->player.getCallSign());
	    sendMessage(ServerPlayer, t, reply);
	    return;
	  }
	  if (cmd == "ban") {
	    if (p->accessInfo.hasPerm(PlayerAccessInfo::antipollban)) {
	      snprintf(reply, MessageLen, "%s is protected from being poll banned.", p->player.getCallSign());
	      sendMessage(ServerPlayer, t, reply);
	      return;
	    }
	  } else if (cmd == "kick") {
	    if (p->accessInfo.hasPerm(PlayerAccessInfo::antipollkick)) {
	      snprintf(reply, MessageLen, "%s is protected from being poll kicked.", p->player.getCallSign());
	      sendMessage(ServerPlayer, t, reply);
	      return;
	    }
	  } else if (cmd == "kill") {
	    if (p->accessInfo.hasPerm(PlayerAccessInfo::antipollkill)) {
	      snprintf(reply, MessageLen, "%s is protected from being poll killed.", p->player.getCallSign());
	      sendMessage(ServerPlayer, t, reply);
	      return;
	    }
	  }
	}
      } // end admin check

    }

    /* create and announce the new poll */
    bool canDo = false;
    if (cmd == "ban") {
      canDo = (arbiter->pollToBan(target, callsign, targetIP));
    } else if (cmd == "kick") {
      canDo = (arbiter->pollToKick(target, callsign, targetIP));
    } else if (cmd == "kill") {
      canDo = (arbiter->pollToKill(target, callsign, targetIP));
    } else if (cmd == "set") {
      canDo = (arbiter->pollToSet(target, callsign));
    } else if (cmd == "flagreset") {
      canDo = (arbiter->pollToResetFlags(callsign));
    }

    if (!canDo) {
      snprintf(reply, MessageLen, "You are not able to request a %s poll right now, %s", cmd.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      return;
    } else {
      snprintf(reply, MessageLen, "A poll to %s %s has been requested by %s", cmd.c_str(), target.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, AllPlayers, reply);
    }

    unsigned int necessaryToSucceed = (unsigned int)((clOptions->votePercentage / 100.0) * (double)available);
    snprintf(reply, MessageLen, "%d player%s available, %d additional affirming vote%s required to pass the poll (%f %%)", available, available==1?" is":"s are", necessaryToSucceed, necessaryToSucceed==1?"":"s", clOptions->votePercentage);
    sendMessage(ServerPlayer, AllPlayers, reply);

    // set the number of available voters
    arbiter->setAvailableVoters(available);

    // keep track of who is allowed to vote
    for (int j = 0; j < curMaxPlayers; j++) {
      // any registered/known users on the server (including
      // observers) are eligible to vote
      GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(j);
      if (otherData && otherData->accessInfo.exists()) {
	arbiter->grantSuffrage(otherData->player.getCallSign());
      }
    }

    // automatically place a vote for the player requesting the poll
    DEBUG3("Attempting to automatically place a vote for [%s]\n", callsign.c_str());

    bool voted = arbiter->voteYes(callsign);
    if (!voted) {
      sendMessage(ServerPlayer, t, "Unable to automatically place your vote for some unknown reason");
      DEBUG3("Unable to automatically place a vote for [%s]\n", callsign.c_str());
    }

  } else if (cmd == "vote") {
    std::string voteCmd = "/vote ";
    voteCmd += arguments;
    handleVoteCmd(playerData, voteCmd.c_str());
    return;

  } else if (cmd == "veto") {
    std::string vetoCmd = "/veto ";
    vetoCmd += arguments;
    handleVetoCmd(playerData, vetoCmd.c_str());
    return;

  } else {
    sendMessage(ServerPlayer, t, "Invalid option to the poll command");
    sendMessage(ServerPlayer, t, "Usage: /poll vote yes|no");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollBan))
      sendMessage(ServerPlayer, t, "    or /poll ban playername");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKick))
      sendMessage(ServerPlayer, t, "    or /poll kick playername");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKill))
      sendMessage(ServerPlayer, t, "    or /poll kill playername");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollSet))
      sendMessage(ServerPlayer, t, "    or /poll set variable value");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollFlagReset))
      sendMessage(ServerPlayer, t, "    or /poll flagreset");

  } /* end handling of poll subcommands */

  return;
}


static void handleViewReportsCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  std::string line;
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::viewReports)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the viewreports command");
    return;
  }
  if (clOptions->reportFile.size() == 0 && clOptions->reportPipe.size() == 0) {
    line = "The /report command is disabled on this server or there are no reports filed.";
    sendMessage(ServerPlayer, t, line.c_str());
  }
  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    sendMessage(ServerPlayer, t, "Error reading from report file.");
    return;
  }
  while (std::getline(ifs, line))
    sendMessage(ServerPlayer, t, line.c_str());
}


static void handleClientqueryCmd(GameKeeper::Player *playerData, const char * message)
{
  int t = playerData->getIndex();
  DEBUG2("Clientquery requested by %s [%d]\n",
	 playerData->player.getCallSign(), t);
  if (message[12] != '\0') {
    std::string name = message + 13; // assumes there is a space
    while (isspace(name[0]))
      name.erase(name.begin());
    GameKeeper::Player *target;
    int i;
    for (i = 0; i < curMaxPlayers;i++) {
      target = GameKeeper::Player::getPlayerByIndex(i);
      if (target && strcmp(target->player.getCallSign(), name.c_str()) == 0) {
	sendMessage(i, t, TextUtils::format("Version: %s",
		     target->player.getClientVersion()).c_str());
	return;
      }
    }
    sendMessage(ServerPlayer, t, "Player not found.");
    return;
  }
  sendMessage(ServerPlayer, AllPlayers, "[Sent version information per request]");
  // send server's own version string just for kicks
  sendMessage(ServerPlayer, t,
	      TextUtils::format("BZFS Version: %s", getAppVersion()).c_str());
  // send all players' version strings
  // is faking a message from the remote client rude?
  // did that so that /clientquery and CLIENTQUERY look about the same.
  GameKeeper::Player *otherData;
  for (int i = 0; i < curMaxPlayers;i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->player.isPlaying()) {
      sendMessage(i, t, TextUtils::format
		  ("Version: %s",
		   otherData->player.getClientVersion()).c_str());
    }
  }
  return;
}


/** /record command
 *
 *  /record start	       # start buffering
 *  /record stop		# stop buffering (or saving to file)
 *  /record size <Mbytes>       # set the buffer size, and truncate
 *  /record rate <secs>	 # set the state capture rate
 *  /record stats	       # display buffer time and memory information
 *  /record file [filename]     # begin capturing straight to file, flush buffer
 *  /record save [filename]     # save buffer to file (or default filename)
 */
static void handleRecordCmd(GameKeeper::Player *playerData, const char * message)
{
  int t = playerData->getIndex();
  const char *buf = message + 8;
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::record)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /record command");
    return;
  }
  while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

  if (strncasecmp (buf, "start", 5) == 0) {
    Record::start(t);
  }
  else if (strncasecmp (buf, "stop", 4) == 0) {
    Record::stop(t);
  }
  else if (strncasecmp (buf, "size", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
      return;
    }
    int size = atoi (buf);
    Record::setSize (t, size);
  }
  else if (strncasecmp (buf, "rate", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
      return;
    }
    int seconds = atoi (buf);
    Record::setRate (t, seconds);
  }
  else if (strncasecmp (buf, "stats", 5) == 0) {
    Record::sendStats(t);
  }
  else if (strncasecmp (buf, "list", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::sendFileList (t, SortNone);   // stolen from '/replay'
    } else if (strncasecmp (buf, "-t", 2) == 0) {
      Replay::sendFileList (t, SortByTime);  
    } else if (strncasecmp (buf, "-n", 2) == 0) {
      Replay::sendFileList (t, SortByName); 
    } else {
      Record::sendHelp (t);
    }
  }
  else if (strncasecmp (buf, "save", 4) == 0) {
    buf = buf + 4;
    char filename[MessageLen];

    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace
    if (*buf == '\0') {
      Record::sendHelp (t);
    }

    // get the filename
    sscanf (buf, "%s", filename);

    // FIXME - do this a little better? use quotations for strings?
    while ((*buf != '\0') && !isspace (*buf)) buf++; // eat filename
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::saveBuffer (t, filename, 0);
    }
    else {
      Record::saveBuffer (t, filename, atoi(buf));
    }
  }
  else if (strncasecmp (buf, "file", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
    }
    else {
      Record::saveFile (t, buf);
    }
  }
  else {
    Record::sendHelp (t);
  }

  return;
}


/** /replay command
 *
 *  /replay list		# list available replay files
 *  /replay load [filename]     # set the replay file (or load the default)
 *  /replay play		# began playing
 *  /replay loop		# began playing in looped mode
 *  /replay stats		# report the current replay state
 *  /replay skip <secs>	 # fast foward or rewind in time
 */
static void handleReplayCmd(GameKeeper::Player *playerData, const char * message)
{
  int t = playerData->getIndex();
  const char *buf = message + 7;
  while ((*buf != '\0') && isspace (*buf)) { // eat whitespace
    buf++;
  }
  
  // everyone can use the replay stats command
  if (strncasecmp (buf, "stats", 4) == 0) {
    Replay::sendStats (t);
    return;
  }

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::replay)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /replay command");
    return;
  }

  if (strncasecmp (buf, "list", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::sendFileList (t, SortNone); 
    } else if (strncasecmp (buf, "-t", 2) == 0) {
      Replay::sendFileList (t, SortByTime);  
    } else if (strncasecmp (buf, "-n", 2) == 0) {
      Replay::sendFileList (t, SortByName); 
    } else {
      Replay::sendHelp (t);
    }
  }
  else if (strncasecmp (buf, "load", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::sendHelp (t);
    }
    else {
      Replay::loadFile (t, buf);
    }
  }
  else if (strncasecmp (buf, "play", 4) == 0) {
    Replay::play (t);
  }
  else if (strncasecmp (buf, "loop", 4) == 0) {
    Replay::loop (t);
  }
  else if (strncasecmp (buf, "skip", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::skip (t, 0);
    }
    else {
      int skip = atoi (buf);
      Replay::skip (t, skip);
    }
  }
  else if (strncasecmp (buf, "pause", 5) == 0) {
    Replay::pause (t);
  }
  else {
    Replay::sendHelp (t);
  }

  return;
}

static void handleSayCmd(GameKeeper::Player *playerData, const char * message)
{
  size_t messageStart = 0;
  int t = playerData->getIndex();

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::say)) {
    char reply[MessageLen] = {0};
    snprintf(reply, MessageLen, "%s, you do not have permission to run the /say command", playerData->player.getCallSign());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  std::string messageText = &message[4];

  // skip any leading whitespace
  while ((messageStart < messageText.size()) &&
	 (isspace(messageText[messageStart]))) {
    messageStart++;
  }

  // make sure there was _some_ whitespace after /say
  if (messageStart == 0) {
    sendMessage(ServerPlayer, t, "Usage: /say some message");
    return;
  }

  // no anonymous messages
  messageText += " (";
  messageText += playerData->player.getCallSign();
  messageText += ")";

  // send the message
  sendMessage(ServerPlayer, AllPlayers, messageText.c_str() + messageStart );
  return;
}



static void handleDateCmd(GameKeeper::Player *playerData, const char * /*message*/)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::date)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /date command");
    return;
  }
  time_t now = time(NULL);
  char* timeStr = ctime(&now);
  timeStr[24] = '\0';
  sendMessage(ServerPlayer, t, timeStr);
}


/** /masterban command
 *
 * /masterban flush	     # remove all master ban entries from this server
 * /masterban reload	    # reread and reload all master ban entries
 * /masterban list	      # output a list of who is banned
 */
static void handleMasterBanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  std::string callsign = std::string(playerData->player.getCallSign());

  DEBUG2("\"%s\" has requested masterban: %s\n", callsign.c_str(), message);

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::masterBan)) {
    sendMessage(ServerPlayer, t, TextUtils::format("%s, you are presently not authorized to run /masterban", callsign.c_str()).c_str());
    return;
  }

  DEBUG3("Player has permission to run /masterban\n");

  if (!clOptions->publicizeServer) {
    sendMessage(ServerPlayer, t, "This is not a public server.  Private servers do not use the master ban list.");
  }
  if (clOptions->suppressMasterBanList) {
    sendMessage(ServerPlayer, t, "The master ban list is disabled on this server.");
  }

  std::string argument = &message[10]; /* skip "/masterban" */
  std::string cmd = "";

  // allow for arbitrary whitespace
  size_t start = 0;
  while ((start < argument.size()) &&
	 (isspace(argument[start]))) {
    start++;
  }

  size_t end = 0;
  while ((end < argument.size()) &&
	 (!isspace(argument[end]))) {
    end++;
  }

  // make sure the command is lower case for comparison simplicity/insensitivity
  cmd = argument.substr(start, end - start);
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);

  if (cmd == "reload") {
    if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) ||
	!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
      sendMessage(ServerPlayer, t, "You do not have permission to reload the master ban list.");
      sendMessage(ServerPlayer, t, "Permission to ban and unban is required to reload the master ban list.");
      return;
    }

    if (clOptions->publicizeServer && !clOptions->suppressMasterBanList) {
      MasterBanList	banList;
      int               banCount;

      clOptions->acl.purgeMasters();
      sendMessage(ServerPlayer, t, "Previous master ban list entries have been flushed.");

      for (std::vector<std::string>::const_iterator i = clOptions->masterBanListURL.begin(); i != clOptions->masterBanListURL.end(); i++) {
	banCount = clOptions->acl.merge(banList.get(i->c_str()));
	std::string reloadmsg = TextUtils::format("Loaded %d master bans from %s", banCount, i->c_str());
	DEBUG1("%s\n", reloadmsg.c_str());
	sendMessage(ServerPlayer, t, reloadmsg.c_str());
      }

    } else {
      sendMessage(ServerPlayer, t, "No action taken.");
    }

  } else if (cmd == "flush") {
    if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
      sendMessage(ServerPlayer, t, "You do not have permission to reload the master ban list.");
      sendMessage(ServerPlayer, t, "Permission to unban is required to flush the master ban list.");
      return;
    }

    clOptions->acl.purgeMasters();
    sendMessage(ServerPlayer, t, "The master ban list has been flushed.");

  } else if (cmd == "list") {
    std::vector<std::pair<std::string, std::string> > bans = clOptions->acl.listMasterBans();

    if (bans.size() > 20) {
      sendMessage(ServerPlayer, t, TextUtils::format("There are %d bans, only displaying the first 20", bans.size()).c_str());

    } else if (bans.size() == 0) {
      sendMessage(ServerPlayer, t, "There are no master bans loaded.");

    } else {
      // print out a list header
      std::string banmsg = TextUtils::format("Master Bans from %s:", DefaultMasterBanURL);
      sendMessage(ServerPlayer, t, banmsg.c_str());
      for (size_t i = 0; i < banmsg.size(); i++) {
	banmsg[i] = '-';
      }
      sendMessage(ServerPlayer, t, banmsg.c_str());
    }

    // print out the bans
    int counter = 0;
    for (std::vector<std::pair<std::string, std::string> >::const_iterator j = bans.begin(); j != bans.end() && counter < 20; j++, counter++) {
      sendMessage(ServerPlayer, t, TextUtils::format("%s: %s", (j->first).c_str(), (j->second).c_str()).c_str());
    }

  } else {
    if (cmd.size() > 0) {
      sendMessage(ServerPlayer, t, TextUtils::format("Unknown masterban command [%s]", cmd.c_str()).c_str());
    }
    sendMessage(ServerPlayer, t, TextUtils::format("Usage: /masterban list|reload|flush").c_str());
  }

  return;
}


// parse server comands
void parseServerCommand(const char *message, int t)
{
  if (!message) {
    std::cerr << "WARNING: parseCommand was given a null message?!" << std::endl;
    return;
  }

  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(t);
  if (!playerData)
    return;

  if (strncasecmp(message + 1, "msg", 3) == 0) {
    handleMsgCmd(playerData, message);

  } else if (strncasecmp(message + 1, "serverquery", 11) == 0) {
    handleServerQueryCmd(playerData, message);

  } else if (strncasecmp(message + 1, "part", 4) == 0) {
    handlePartCmd(playerData, message);

  } else if (strncasecmp(message + 1, "quit", 4) == 0) {
    handleQuitCmd(playerData, message);

  } else if (strncasecmp(message + 1, "uptime", 6) == 0) {
    handleUptimeCmd(playerData, message);

  } else if (strncasecmp(message + 1, "password", 8) == 0) {
    handlePasswordCmd(playerData, message);

  } else if (strncasecmp(message + 1, "set ", 4) == 0) {
    handleSetCmd(playerData, message);

  } else if (strncasecmp(message + 1, "reset", 5) == 0) {
    handleResetCmd(playerData, message);

  } else if (strncasecmp(message + 1, "shutdownserver", 8) == 0) {
    handleShutdownserverCmd(playerData, message);

  } else if (strncasecmp(message + 1, "superkill", 8) == 0) {
    handleSuperkillCmd(playerData, message);

  } else if (strncasecmp(message + 1, "gameover", 8) == 0) {
    handleGameoverCmd(playerData, message);

  } else if (strncasecmp(message + 1, "countdown", 9) == 0) {
    handleCountdownCmd(playerData, message);

  } else if (strncasecmp(message + 1, "flag ", 5) == 0) {
    handleFlagCmd(playerData,message);

  } else if (strncasecmp(message + 1, "kick", 4) == 0) {
    handleKickCmd(playerData, message);

  } else if (strncasecmp(message + 1, "kill", 4) == 0) {
    handleKillCmd(playerData, message);

  } else if (strncasecmp(message+1, "banlist", 7) == 0) {
    handleBanlistCmd(playerData, message);

  } else if (strncasecmp(message+1, "hostbanlist", 11) == 0) {
    handleHostBanlistCmd(playerData, message);

  } else if (strncasecmp(message+1, "ban", 3) == 0) {
    handleBanCmd(playerData, message);

  } else if (strncasecmp(message+1, "hostban", 7) == 0) {
    handleHostBanCmd(playerData, message);

  } else if (strncasecmp(message+1, "unban", 5) == 0) {
    handleUnbanCmd(playerData, message);

  } else if (strncasecmp(message+1, "hostunban", 9) == 0) {
    handleHostUnbanCmd(playerData, message);

  } else if (strncasecmp(message+1, "lagwarn",7) == 0) {
    handleLagwarnCmd(playerData, message);

  } else if (strncasecmp(message+1, "lagstats",8) == 0) {
    handleLagstatsCmd(playerData, message);

  } else if (strncasecmp(message+1, "idlestats",9) == 0) {
    handleIdlestatsCmd(playerData, message);

  } else if (strncasecmp(message+1, "flaghistory", 11 ) == 0) {
    handleFlaghistoryCmd(playerData, message); 

  } else if (strncasecmp(message+1, "mute", 4 ) == 0 ) {
    handleMuteCmd(playerData, message);

  } else if (strncasecmp(message+1, "unmute", 6 ) == 0 ) {
    handleUnmuteCmd(playerData, message);

  } else if (strncasecmp(message+1, "playerlist", 10) == 0) {
    handlePlayerlistCmd(playerData, message);

  } else if (strncasecmp(message+1, "report", 6) == 0) {
    handleReportCmd(playerData, message);

  } else if (strncasecmp(message+1, "help", 4) == 0) {
    handleHelpCmd(playerData, message);

  } else if (strncasecmp(message + 1, "identify", 8) == 0) {
    handleIdentifyCmd(playerData, message);

  } else if (strncasecmp(message + 1, "register", 8) == 0) {
    handleRegisterCmd(playerData, message);

  } else if (strncasecmp(message + 1, "ghost", 5) == 0) {
    handleGhostCmd(playerData, message);

  } else if (strncasecmp(message + 1, "deregister", 10) == 0) {
    handleDeregisterCmd(playerData, message);

  } else if (strncasecmp(message + 1, "setpass", 7) == 0) {
    handleSetpassCmd(playerData, message);

  } else if (strncasecmp(message + 1, "grouplist", 9) == 0) {
    handleGrouplistCmd(playerData, message);

  } else if (strncasecmp(message + 1, "showgroup", 9) == 0) {
    handleShowgroupCmd(playerData, message);

  } else if (strncasecmp(message + 1, "groupperms", 10) == 0) {
    handleGrouppermsCmd(playerData, message);

  } else if (strncasecmp(message + 1, "setgroup", 8) == 0) {
    handleSetgroupCmd(playerData, message);

  } else if (strncasecmp(message + 1, "removegroup", 11) == 0) {
    handleRemovegroupCmd(playerData, message);

  } else if (strncasecmp(message + 1, "reload", 6) == 0) {
    handleReloadCmd(playerData, message);

  } else if (strncasecmp(message + 1, "poll", 4) == 0) {
    handlePollCmd(playerData, message);

  } else if (strncasecmp(message + 1, "vote", 4) == 0) {
    handleVoteCmd(playerData, message);

  } else if (strncasecmp(message + 1, "veto", 4) == 0) {
    handleVetoCmd(playerData, message);

  } else if (strncasecmp(message + 1, "viewreports", 11) == 0) {
    handleViewReportsCmd(playerData, message);

  } else if (strncasecmp(message + 1, "clientquery", 11) == 0) {
    handleClientqueryCmd(playerData, message);

  } else if (strncasecmp(message + 1, "date", 4) == 0 || strncasecmp(message + 1, "time", 4) == 0) {
    handleDateCmd(playerData, message);

  } else if (strncasecmp(message + 1, "record", 6) == 0) {
    handleRecordCmd(playerData, message);

  } else if (strncasecmp(message + 1, "replay", 6) == 0) {
    handleReplayCmd(playerData, message);

  } else if (strncasecmp(message + 1, "say", 3) == 0) {
    handleSayCmd(playerData, message);

  } else if (strncasecmp(message + 1, "masterban", 9) == 0) {
    handleMasterBanCmd(playerData, message);

  } else {
    // lets see if it is a custom command
    std::vector<std::string> params = TextUtils::tokenize(std::string(message+1),std::string(" "));

    if (params.size() == 0)
      return;
    
    tmCustomSlashCommandMap::iterator itr = customCommands.find(TextUtils::tolower(params[0]));
  
    bzApiString	command = params[0];
    bzApiString param;

    if (params.size()>1)
      param = params[1];
    else
      param = message;

    if (itr != customCommands.end()) {  // see if we have a registerd custom command and call it
      if (itr->second->handle(t, command, param))  // if it handles it, then we are good
        return;
    }
  
    // lets see if anyone wants to handle the unhandled event
    bz_UnknownSlashCommandEventData commandData;
    commandData.from = t;
    commandData.message = message;
    commandData.time = TimeKeeper::getCurrent().getSeconds();
  
    worldEventManager.callEvents(bz_eUnknownSlashCommand, &commandData);
    if (commandData.handled) // did anyone do it?
      return;

    char reply[MessageLen];
    snprintf(reply, MessageLen, "Unknown command [%s]", message + 1);
    sendMessage(ServerPlayer, t, reply);
  }
}

void registerCustomSlashCommand(std::string command, bz_CustomSlashCommandHandler* handler)
{
  if (handler)
    customCommands[TextUtils::tolower(command)] = handler;
}

void removeCustomSlashCommand(std::string command)
{
  tmCustomSlashCommandMap::iterator itr = customCommands.find(TextUtils::tolower(command));
  if (itr != customCommands.end())
    customCommands.erase(itr);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
