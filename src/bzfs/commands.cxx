/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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

// implementation-specific system headers
#include <string>
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

// implementation-specific bzflag headers
#include "global.h"
#include "Address.h"
#include "version.h"
#include "CommandManager.h"

// implementation-specific bzfs-specific headers
#include "VotingArbiter.h"
#include "Permissions.h"
#include "CmdLineOptions.h"
#include "PlayerInfo.h"
#include "NetHandler.h"
#include "RecordReplay.h"
#include "LagInfo.h"
#include "FlagHistory.h"
#include "PackVars.h"
#include "FlagInfo.h"

#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif


// FIXME -- need to pull communication out of bzfs.cxx...

// externs that poll, veto, vote, and clientquery require
extern void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message);
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;
extern int NotConnected;

// externs that ghost needs
extern void removePlayer(int playerIndex, const char *reason, bool notify=true);

// externs that shutdownserver requires
extern bool done;

// externs that superkill and gameover requires
extern bool gameOver;
extern char *getDirectMessageBuffer();
extern void broadcastMessage(uint16_t code, int len, const void *msg);

// externs needed by the countdown command
#include "TimeKeeper.h"
extern TimeKeeper gameStartTime;
#include "PlayerInfo.h"
extern TeamInfo team[NumTeams];
extern void sendTeamUpdate(int playerIndex = -1, int teamIndex1 = -1, int teamIndex2 = -1);
extern int numFlags;
extern void zapFlag(int flagIndex);
extern void sendFlagUpdate(int flagIndex = -1, int playerIndex = -1);
extern void resetFlag(int flagIndex);

// externs that countdown requires
extern bool countdownActive;

// externs that identify and password requires
extern void sendIPUpdate(int targetPlayer = -1, int playerIndex = -1);

void handlePasswordCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (playerData->accessInfo.passwordAttemptsMax()) {
    DEBUG1("\"%s\" (%s) has attempted too many /password tries\n",
	   playerData->player.getCallSign(),
	   playerData->netHandler->getTargetIP());
    sendMessage(ServerPlayer, t, "Too many attempts");
  } else {
    if ((clOptions->password != "") && strncmp(message + 10, clOptions->password.c_str(), clOptions->password.size()) == 0){
      playerData->accessInfo.setAdmin();
      sendIPUpdate(t, -1);
      sendMessage(ServerPlayer, t, "You are now an administrator!");
    } else {
      sendMessage(ServerPlayer, t, "Wrong Password!");
    }
  }
  return;
}


void handleSetCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char message2[MessageLen];
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar)
      && !playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the set command");
    return;
  }
  if (Replay::enabled()) {
    sendMessage(ServerPlayer, t, "You can't /set variables in replay mode");
    return;
  }
  sendMessage(ServerPlayer, t, CMDMGR.run(message+1).c_str());
  snprintf(message2, MessageLen, "Variable Modification Notice by %s of %s",
	   playerData->player.getCallSign(), message+1); 
  sendMessage(ServerPlayer, AllPlayers, message2);
  return;
}


void handleResetCmd(GameKeeper::Player *playerData, const char *message)
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
  sendMessage(ServerPlayer, t, CMDMGR.run(message+1).c_str());
  return;
}


void handleShutdownserverCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::shutdownServer)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the shutdown command");
    return;
  }
  done = true;
  return;
}


void handleSuperkillCmd(GameKeeper::Player *playerData, const char *)
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


void handleGameoverCmd(GameKeeper::Player *playerData, const char *)
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
  if (clOptions->timeManualStart)
    countdownActive = false;
  return;
}


void handleCountdownCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::countdown)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the countdown command");
    return;
  } else if (!clOptions->timeManualStart) {
    sendMessage(ServerPlayer, t, "This server was not configured for manual clock countdowns");
    return;
  }
#ifdef TIMELIMIT
  int i, j;
  // /countdown starts timed game, if start is manual, everyone is allowed to
  char reply[MessageLen] = {0};
  if (clOptions->timeLimit > 0.0f) {
    gameStartTime = TimeKeeper::getCurrent();
    clOptions->timeElapsed = 0.0f;
    countdownActive = true;

    char msg[2];
    void *buf = msg;
    nboPackUShort(buf, (uint16_t)(int)clOptions->timeLimit);
    broadcastMessage(MsgTimeUpdate, sizeof(msg), msg);
  }
  // reset team scores
  for (i = RedTeam; i <= PurpleTeam; i++) {
    team[i].team.lost = team[i].team.won = 0;
  }
  sendTeamUpdate();

  sprintf(reply, "Countdown started.");
  sendMessage(ServerPlayer, t, reply);

  // CTF game -> simulate flag captures to return ppl to base
  if (clOptions->gameStyle & int(TeamFlagGameStyle)) {
    GameKeeper::Player *otherData;
    // get someone to can do virtual capture
    for (j = 0; j < curMaxPlayers; j++) {
      otherData = GameKeeper::Player::getPlayerByIndex(j);
      if (otherData && otherData->player.isPlaying())
	break;
    }
    if (j < curMaxPlayers) {
      for (int i = 0; i < curMaxPlayers; i++) {
	otherData = GameKeeper::Player::getPlayerByIndex(i);
	if (otherData && otherData->player.hasPlayedEarly()) {
	  void *buf, *bufStart = getDirectMessageBuffer();
	  buf = nboPackUByte(bufStart, j);
	  buf = otherData->player.packVirtualFlagCapture(buf);
	  directMessage(i, MsgCaptureFlag, (char*)buf - (char*)bufStart, bufStart);
	}
      }
    }
  }
  // reset all flags
  for (i = 0; i < numFlags; i++)
    zapFlag(i);

#endif // end TIMELIMIT

  return;
}


void handleFlagCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::flagMod)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flag command");
    return;
  }
  GameKeeper::Player *otherData;
  if (strncmp(message + 6, "reset", 5) == 0) {
    bool onlyUnused = strncmp(message + 11, " unused", 7) == 0;
    for (int i = 0; i < numFlags; i++) {
      // see if someone had grabbed flag,
      const int playerIndex = flag[i].player;
      otherData	= GameKeeper::Player::getPlayerByIndex(playerIndex);
      if (otherData && (!onlyUnused)) {
	// tell 'em to drop it.
	flag[i].player = -1;
	flag[i].flag.status = FlagNoExist;
	otherData->player.resetFlag();

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerIndex);
	buf = nboPackUShort(buf, uint16_t(i));
	buf = flag[i].flag.pack(buf);
	broadcastMessage(MsgDropFlag, (char*)buf - (char*)bufStart, bufStart);

      }
      if ((playerIndex == -1) || (!onlyUnused))
	resetFlag(i);
    }

  } else if (strncmp(message + 6, "up", 2) == 0) {
    for (int i = 0; i < numFlags; i++) {
      if (flag[i].flag.type->flagTeam != ::NoTeam) {
	// see if someone had grabbed flag.  tell 'em to drop it.
	const int playerIndex = flag[i].player;
	otherData	= GameKeeper::Player::getPlayerByIndex(playerIndex);
	if (otherData) {
	  flag[i].player = -1;
	  flag[i].flag.status = FlagNoExist;
	  otherData->player.resetFlag();

	  void *buf, *bufStart = getDirectMessageBuffer();
	  buf = nboPackUByte(bufStart, playerIndex);
	  buf = nboPackUShort(buf, uint16_t(i));
	  buf = flag[i].flag.pack(buf);
	  broadcastMessage(MsgDropFlag, (char*)buf - (char*)bufStart, bufStart);
	}
	flag[i].flag.status = FlagGoing;
	if (!flag[i].required)
	  flag[i].flag.type = Flags::Null;
	sendFlagUpdate(i);
      }
    }

  } else if (strncmp(message + 6, "show", 4) == 0) {
    for (int i = 0; i < numFlags; i++) {
      char message[MessageLen];
      sprintf(message, "%d p:%d r:%d g:%d i:%s s:%d p:%3.1fx%3.1fx%3.1f", i, flag[i].player,
	      flag[i].required, flag[i].grabs, flag[i].flag.type->flagAbbv,
	      flag[i].flag.status,
	      flag[i].flag.position[0],
	      flag[i].flag.position[1],
	      flag[i].flag.position[2]);
      sendMessage(ServerPlayer, t, message);
    }
  } else {
    sendMessage(ServerPlayer, t, "reset|show|up");
  }
  return;
}

int getTarget(const char *victimname) {
  GameKeeper::Player *targetData;
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    targetData = GameKeeper::Player::getPlayerByIndex(i);
    if (targetData && strncasecmp(targetData->player.getCallSign(),
				  victimname, 256) == 0) {
      break;
    }
  }
  return i;
}

void handleKickCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::kick)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the kick command");
    return;
  }
  int i;
  std::vector<std::string> argv = string_util::tokenize(message, " \t", 3, true);
  
  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t, "Syntax: /kick <PlayerName/\"Player Name\"> [reason]");
    sendMessage(ServerPlayer, t, "        Please keep in mind that reason is displayed to the user.");
    return;
  }

  const char *victimname = argv[1].c_str();

  i = getTarget(victimname);
  
  if (i < curMaxPlayers) {
    char kickmessage[MessageLen];
    sprintf(kickmessage, "You were kicked off the server by %s",
	    playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, kickmessage);
    if (argv.size() > 2) {
      sprintf(kickmessage, " reason given : %s",argv[2].c_str());
      sendMessage(ServerPlayer, i, kickmessage);
    }
    removePlayer(i, "/kick");
  } else {
    char errormessage[MessageLen];
    sprintf(errormessage, "player \"%s\" not found", victimname);
    sendMessage(ServerPlayer, t, errormessage);
  }
  return;
}


void handleBanlistCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendBans(t);
  return;
}


void handleHostBanlistCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendHostBans(t);
  return;
}


void handleBanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }
  char reply[MessageLen] = {0};

  std::string msg = message;
  std::vector<std::string> argv = string_util::tokenize(msg, " \t", 4);

  if (argv.size() < 2) {
    strcpy(reply, "Syntax: /ban <ip> [duration] [reason]");
    sendMessage(ServerPlayer, t, reply);
    strcpy(reply, "        Please keep in mind that reason is displayed to the user.");
    sendMessage(ServerPlayer, t, reply);
  } else {
    int durationInt = 0;
    std::string ip = argv[1];
    std::string reason;

    if (argv.size() >= 3)
      durationInt = string_util::parseDuration(argv[2]);

    if (argv.size() == 4)
      reason = argv[3];

    if (clOptions->acl.ban(ip, playerData->player.getCallSign(), durationInt,
			   reason.c_str())) {
      clOptions->acl.save();
      strcpy(reply, "IP pattern added to banlist");
      char kickmessage[MessageLen];
      for (int i = 0; i < curMaxPlayers; i++) {
	NetHandler *handler = NetHandler::getHandler(i);
	if (handler && !clOptions->acl.validate(handler->getIPAddress())) {
	  sprintf(kickmessage,"You were banned from this server by %s",
		  playerData->player.getCallSign());
	  sendMessage(ServerPlayer, i, kickmessage);
	  if (reason.length() > 0) {
	    sprintf(kickmessage,"Reason given: %s", reason.c_str());
	    sendMessage(ServerPlayer, i, kickmessage);
	  }
	  removePlayer(i, "/ban");
	}
      }
    } else {
      strcpy(reply, "Malformed address");
    }
    sendMessage(ServerPlayer, t, reply);
  }
  return;
}


void handleHostBanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }
  char reply[MessageLen] = {0};

  std::string msg = message;
  std::vector<std::string> argv = string_util::tokenize( msg, " \t", 4 );

  if( argv.size() < 2 ){
    strcpy(reply, "Syntax: /hostban <host pattern> [duration] [reason]");
    sendMessage(ServerPlayer, t, reply);
    strcpy(reply, "        Please keep in mind that reason is displayed to the user.");
    sendMessage(ServerPlayer, t, reply);
  }
  else {
    int durationInt = 0;
    std::string hostpat = argv[1];
    std::string reason;

    if( argv.size() >= 3 )
      durationInt = string_util::parseDuration(argv[2]);

    if( argv.size() == 4 )
      reason = argv[3];

    clOptions->acl.hostBan(hostpat, playerData->player.getCallSign(),
			   durationInt,
			   reason.c_str());
    clOptions->acl.save();
#ifdef HAVE_ADNS_H
    strcpy(reply, "Host pattern added to banlist");
    char kickmessage[MessageLen];
    for (int i = 0; i < curMaxPlayers; i++) {
      NetHandler *netHandler = NetHandler::getHandler(i);
      if (netHandler && netHandler->getHostname()
	  && (!clOptions->acl.hostValidate(netHandler->getHostname()))) {
	sprintf(kickmessage,"You were banned from this server by %s",
		playerData->player.getCallSign());
	sendMessage(ServerPlayer, i, kickmessage);
	if( reason.length() > 0 ){
	  sprintf(kickmessage,"Reason given: %s", reason.c_str());
	  sendMessage(ServerPlayer, i, kickmessage);
	}
	removePlayer(i, "/hostban");
      }
    }
#else
    strcpy(reply, "Host pattern added to banlist. WARNING: host patterns not supported in this compilation.");
#endif
    sendMessage(ServerPlayer, t, reply);
  }
  return;
}


void handleUnbanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unban command");
    return;
  }
  char reply[MessageLen] = {0};

  if (clOptions->acl.unban(message + 7)) {
    strcpy(reply, "removed IP pattern");
    clOptions->acl.save();
  } else {
    strcpy(reply, "no pattern removed");
  }
  sendMessage(ServerPlayer, t, reply);
  return;
}

void handleHostUnbanCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unban command");
    return;
  }
  char reply[MessageLen] = {0};

  if (clOptions->acl.hostUnban(message + 11)) {
    strcpy(reply, "removed host pattern");
    clOptions->acl.save();
  }
  else
    strcpy(reply, "no pattern removed");
  sendMessage(ServerPlayer, t, reply);
  return;
}


void handleLagwarnCmd(GameKeeper::Player *playerData, const char *message)
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
    sprintf(reply,"lagwarn is now %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
    sendMessage(ServerPlayer, t, reply);
  } else {
    sprintf(reply,"lagwarn is set to %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
    sendMessage(ServerPlayer, t, reply);
  }
  return;
}


void handleLagstatsCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::lagStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagstats command");
    return;
  }

  char reply[MessageLen];
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if (p != NULL) {
      p->lagInfo->getLagStats(reply);
      if (strlen(reply)) {
	if (p->accessInfo.isAccessVerified())
	  strcat(reply, " (R)");
	sendMessage(ServerPlayer, t, reply);
      }
    }
  }
}


void handleIdlestatsCmd(GameKeeper::Player *playerData, const char *)
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


void handleFlaghistoryCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::flagHistory)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flaghistory command");
    return;
  }

  char reply[MessageLen];
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (playerData->player.isPlaying() && !playerData->player.isObserver()) {
      sprintf(reply,"%-16s : ", playerData->player.getCallSign());
      playerData->flagHistory.get(reply+strlen(reply));
      sendMessage(ServerPlayer, t, reply);
    }
  }
}


void handlePlayerlistCmd(GameKeeper::Player *playerData, const char *)
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


void handleReportCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};

  if (strlen(message + 1) < 8) {
    sprintf(reply, "Nothing reported");
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
      sprintf(reply, "The /report command is disabled on this server.");
    } else {
      sprintf(reply, "Your report has been filed. Thank you.");
      DEBUG1("Player %s [%d] has filed a report (time: %s).\n",
	     playerData->player.getCallSign(), t, timeStr);
    }
  }
  sendMessage(ServerPlayer, t, reply);
  return;
}


void handleHelpCmd(GameKeeper::Player *playerData, const char *message)
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


void handleIdentifyCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  // player is trying to send an ID
  if (playerData->accessInfo.isAccessVerified()) {
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
      } else {
	playerData->accessInfo.setLoginFail();
	sendMessage(ServerPlayer, t, "Identify Failed, please make sure"
		    " your password was correct");
      }
    }
  }
  return;
}


void handleRegisterCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (playerData->accessInfo.isAccessVerified()) {
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


void handleGhostCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char *p1 = strchr(message + 1, '\"');
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
	  sprintf(temp, "Your Callsign is registered to another user,"
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


void handleDeregisterCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.isAccessVerified()) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the deregister command");
    return;
  }

  if (strlen(message) == 11) {
    // removing own callsign
    PasswordMap::iterator itr1
      = passwordDatabase.find(playerData->accessInfo.getName());
    PlayerAccessMap::iterator itr2
      = userDatabase.find(playerData->accessInfo.getName());
    passwordDatabase.erase(itr1);
    userDatabase.erase(itr2);
    PlayerAccessInfo::updateDatabases();
    sendMessage(ServerPlayer, t, "Your callsign has been deregistered");
  } else if (strlen(message) > 12
	     && playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    // removing someone else's
    std::string name = message + 12;
    makeupper(name);
    if (userExists(name)) {
      PasswordMap::iterator itr1 = passwordDatabase.find(name);
      PlayerAccessMap::iterator itr2 = userDatabase.find(name);
      passwordDatabase.erase(itr1);
      userDatabase.erase(itr2);
      PlayerAccessInfo::updateDatabases();
      char text[MessageLen];
      sprintf(text, "%s has been deregistered", name.c_str());
      sendMessage(ServerPlayer, t, text);
    } else {
      char text[MessageLen];
      sprintf(text, "user %s does not exist", name.c_str());
      sendMessage(ServerPlayer, t, text);
    }
  } else if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to deregister this user");
  }

  return;
}


void handleSetpassCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.isAccessVerified()) {
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


void handleGrouplistCmd(GameKeeper::Player *playerData, const char *)
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


void handleShowgroupCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  std::string settie;

  if (strlen(message) == 10) {	 // show own groups
    if (playerData->accessInfo.isAccessVerified()) {
      settie = playerData->accessInfo.getName();
    } else {
      sendMessage(ServerPlayer, t, "You are not identified");
    }
  } else if (playerData->accessInfo.hasPerm(PlayerAccessInfo::showOthers)) {
    // show groups for other player
    char *p1 = strchr(message + 1, '\"');
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
    if (userExists(settie)) {
      PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

      std::string line = "Groups for ";
      line += settie;
      line += ", ";
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


void handleGrouppermsCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  sendMessage(ServerPlayer, t, "Group List:");
  PlayerAccessMap::iterator itr = groupAccess.begin();
  std::string line;
  while (itr != groupAccess.end()) {
    line = itr->first + ":   ";
    sendMessage(ServerPlayer, t, line.c_str());

    for (int i = 0; i < PlayerAccessInfo::lastPerm; i++) {
      if (itr->second.explicitAllows.test(i)) {
	line = "     ";
	line += nameFromPerm((PlayerAccessInfo::AccessPerm)i);
	sendMessage(ServerPlayer, t, line.c_str());
      }
    }
    itr++;
  }
  return;
}


void handleSetgroupCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char *p1 = strchr(message + 1, '\"');
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
	    sprintf(temp, "you have been added to the %s group, by %s",
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


void handleRemovegroupCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char *p1 = strchr(message + 1, '\"');
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
	    sprintf(temp, "You have been removed from the %s group, by %s",
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

void handleReloadCmd(GameKeeper::Player *playerData, const char *)
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
  // reload the databases
  if (groupsFile.size())
    PlayerAccessInfo::readGroupsFile(groupsFile);
  // make sure that the 'admin' & 'default' groups exist
  PlayerAccessMap::iterator itr = groupAccess.find("DEFAULT");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    info.explicitAllows[PlayerAccessInfo::idleStats] = true;
    info.explicitAllows[PlayerAccessInfo::lagStats] = true;
    info.explicitAllows[PlayerAccessInfo::flagHistory] = true;
    groupAccess["DEFAULT"] = info;
  }
  itr = groupAccess.find("REGISTERED");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    info.explicitAllows[PlayerAccessInfo::vote] = true;
    info.explicitAllows[PlayerAccessInfo::poll] = true;
    groupAccess["REGISTERED"] = info;
  }
  itr = groupAccess.find("ADMIN");
  if (itr == groupAccess.end()) {
    PlayerAccessInfo info;
    for (int i = 0; i < PlayerAccessInfo::lastPerm; i++)
      info.explicitAllows[i] = true;
    groupAccess["ADMIN"] = info;
  }
  if (passFile.size())
    readPassFile(passFile);
  if (userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);
  GameKeeper::Player::reloadAccessDatabase();
  sendMessage(ServerPlayer, t, "Databases reloaded");

  return;
}


void handlePollCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};
  std::string callsign = std::string(playerData->player.getCallSign());

  DEBUG2("Entered poll command handler (MessageLen is %d)\n", MessageLen);

  /* make sure player has permission to request a poll */
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::poll)) {
    sprintf(reply,"%s, you are presently not authorized to run /poll", callsign.c_str());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  DEBUG2("Player has permission\n");

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return;
  }

  DEBUG2("BZDB poll value is not empty\n");

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  DEBUG2("Arbiter was acquired with address 0x%x\n", (unsigned int)arbiter);

  /* make sure that there is not a poll active already */
  if (arbiter->knowsPoll()) {
    sprintf(reply,"A poll to %s %s is presently in progress", arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    sendMessage(ServerPlayer, t, "Unable to start a new poll until the current one is over");
    return;
  }

  DEBUG2("The arbiter says there is not another poll active\n");

  // get available voter count
  unsigned short int available = 0;
  for (int i = 0; i < curMaxPlayers; i++) {
    // any registered/known users on the server (including observers)
    // are eligible to vote
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (playerData && playerData->accessInfo.exists()) {
      available++;
    }
  }

  DEBUG2("There are %d available players for %d votes required\n", available, clOptions->votesRequired);

  /* make sure there are enough players to even make a poll that has a chance
   * of succeeding (not counting the person being acted upon)
   */
  if (available - 1 < clOptions->votesRequired) {
    sendMessage(ServerPlayer, t, "Unable to initiate a new poll.  There are not enough registered players playing.");
    sprintf(reply,"There needs to be at least %d other %s and only %d %s available.",
	    clOptions->votesRequired,
	    clOptions->votesRequired - 1 == 1 ? "player" : "players",
	    available - 1,
	    available - 1 == 1 ? "is" : "are");
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  std::string arguments = &message[5];
  std::string cmd = "";

  DEBUG2("The arguments string is [%s]\n", arguments.c_str());

  /* find the start of the command */
  size_t startPosition = 0;
  while ((startPosition < arguments.size()) &&
	 (isspace(arguments[startPosition]))) {
    startPosition++;
  }

  DEBUG2("Start position is %d\n", (int)startPosition);

  /* find the end of the command */
  size_t endPosition = startPosition + 1;
  while ((endPosition < arguments.size()) &&
	 (!isspace(arguments[endPosition]))) {
    endPosition++;
  }

  DEBUG2("End position is %d\n", (int)endPosition);

  /* stash the command ('kick', etc) in lowercase to simplify comparison */
  if ((startPosition != arguments.size()) &&
      (endPosition > startPosition)) {
    for (size_t i = startPosition; i < endPosition; i++) {
      cmd += tolower(arguments[i]);
    }
  }

  DEBUG2("Command is %s\n", cmd.c_str());

  /* handle subcommands */

  if ((cmd == "ban") || (cmd == "kick") || (cmd == "set") || (cmd == "flagreset")) {
    std::string target;
    std::string targetIP = "";

    arguments = arguments.substr(endPosition);

    DEBUG2("Command arguments are [%s]\n", arguments.c_str());

    /* find the start of the target (e.g. player name) */
    startPosition = 0;
    while ((startPosition < arguments.size()) &&
	   (isspace(arguments[startPosition]))) {
      startPosition++;
    }
    // do not include a starting quote, if given
    if ( arguments[startPosition] == '"' ) {
      startPosition++;
    }

    DEBUG2("Start position for target is %d\n", (int)startPosition);

    /* find the end of the target */
    endPosition = arguments.size() - 1;
    while ((endPosition > 0) &&
	   (isspace(arguments[endPosition]))) {
      endPosition--;
    }
    // do not include a trailing quote, if given
    if ( arguments[endPosition] == '"' ) {
      endPosition--;
    }

    DEBUG2("End position for target is %d\n", (int)endPosition);

    target = arguments.substr(startPosition, endPosition - startPosition + 1);

    DEBUG2("Target specified to vote upon is [%s]\n", target.c_str());

    if ((target.length() == 0) && (cmd != "flagreset")) {
      sprintf(reply,"%s, no target was specified for the [%s] vote", callsign.c_str(), cmd.c_str());
      sendMessage(ServerPlayer, t, reply);
      sprintf(reply,"Usage: /poll %s target", cmd.c_str());
      sendMessage(ServerPlayer, t, reply);
      return;
    }

    if ((cmd != "set") && (cmd != "flagreset")) {
      // all polls that are not set or flagreset polls take a player name

      /* make sure the requested player is actually here */
      int v = getTarget(target.c_str());
      if (v >= curMaxPlayers) {
	/* wrong name? */
	sprintf(reply,
		"The player specified for a %s vote is not here", cmd.c_str());
	sendMessage(ServerPlayer, t, reply);
	return;
      }
      targetIP = NetHandler::getHandler(v)->getTargetIP();
    }

    /* create and announce the new poll */
    bool canDo = false;
    if (cmd == "ban") {
      canDo = (arbiter->pollToBan(target, callsign, targetIP));
    } else if (cmd == "kick") {
      canDo = (arbiter->pollToKick(target, callsign, targetIP));
    } else if (cmd == "set") {
      canDo = (arbiter->pollToSet(target, callsign));
    } else if (cmd == "flagreset") {
      canDo = (arbiter->pollToResetFlags(callsign));
    }

    if (!canDo) {
      sprintf(reply,"You are not able to request a %s poll right now, %s", cmd.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      return;
    } else {
      sprintf(reply,"A poll to %s %s has been requested by %s", cmd.c_str(), target.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, AllPlayers, reply);
    }

    unsigned int necessaryToSucceed = (unsigned int)((clOptions->votePercentage / 100.0) * (double)available);
    sprintf(reply, "%d player%s available, %d additional affirming vote%s required to pass the poll (%f %%)", available, available==1?" is":"s are", necessaryToSucceed, necessaryToSucceed==1?"":"s", clOptions->votePercentage);
    sendMessage(ServerPlayer, AllPlayers, reply);

    // set the number of available voters
    arbiter->setAvailableVoters(available);

    // keep track of who is allowed to vote
    for (int j = 0; j < curMaxPlayers; j++) {
      // any registered/known users on the server (including
      // observers) are eligible to vote
      GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(j);
      if (playerData && playerData->accessInfo.exists()) {
	arbiter->grantSuffrage(playerData->player.getCallSign());
      }
    }

    // automatically place a vote for the player requesting the poll
    DEBUG2("Attempting to automatically place a vote for [%s]\n", callsign.c_str());

    bool voted = arbiter->voteYes(callsign);
    if (!voted) {
      sendMessage(ServerPlayer, t, "Unable to automatically place your vote for some unknown reason");
      DEBUG2("Unable to automatically place a vote for [%s]\n", callsign.c_str());
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
    sendMessage(ServerPlayer, t, "Usage: /poll ban|kick playername");
    sendMessage(ServerPlayer, t, "    or /poll set variable value");
    sendMessage(ServerPlayer, t, "    or /poll flagreset");
    sendMessage(ServerPlayer, t, "    or /poll vote yes|no");
    sendMessage(ServerPlayer, t, "    or /poll veto");

  } /* end handling of poll subcommands */

  return;
}


void handleVoteCmd(GameKeeper::Player *playerData, const char *message)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};
  std::string callsign = std::string(playerData->player.getCallSign());

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::vote)) {
    /* permission denied for /vote */
    sprintf(reply,"%s, you are presently not authorized to run /vote", callsign.c_str());
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
      sprintf(reply,"%s, your vote in opposition of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else if (vote == 1) {
    if ((cast = arbiter->voteYes(callsign)) == true) {
      /* player voted yes */
      sprintf(reply,"%s, your vote in favor of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else {
    if (answer.length() == 0) {
      sprintf(reply,"%s, you did not provide a vote answer", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
    } else {
      sprintf(reply,"%s, you did not vote in favor or in opposition", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
    }
    sendMessage(ServerPlayer, t, "Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao");
    return;
  }

  if (!cast) {
    /* player was unable to cast their vote; probably already voted */
    sprintf(reply,"%s, you have already voted on the poll to %s %s", callsign.c_str(), arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    return;
  }

  return;
}


void handleVetoCmd(GameKeeper::Player *playerData, const char * /*message*/)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::veto)) {
    /* permission denied for /veto */
    sendMessage(ServerPlayer, t,
		string_util::format
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
		string_util::format
		("%s, there is presently no active poll to veto",
		 playerData->player.getCallSign()).c_str());
    return;
  }

  sendMessage(ServerPlayer, t,
	      string_util::format("%s, you have cancelled the poll to %s %s",
				  playerData->player.getCallSign(),
				  arbiter->getPollAction().c_str(),
				  arbiter->getPollTarget().c_str()).c_str());

  /* poof */
  arbiter->forgetPoll();

  sendMessage(ServerPlayer, AllPlayers,
	      string_util::format("The poll was cancelled by %s",
				  playerData->player.getCallSign()).c_str());

  return;
}

void handleViewReportsCmd(GameKeeper::Player *playerData, const char *)
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
 

void handleClientqueryCmd(GameKeeper::Player *playerData, const char *)
{
  int t = playerData->getIndex();
  DEBUG2("Clientquery requested by %s [%d]\n",
	 playerData->player.getCallSign(), t);
  sendMessage(ServerPlayer, AllPlayers, "[Sent version information per request]");
  // send server's own version string just for kicks
  sendMessage(ServerPlayer, t, 
              string_util::format("BZFS Version: %s", getAppVersion()).c_str());
  // send all players' version strings
  // is faking a message from the remote client rude?
  // did that so that /clientquery and CLIENTQUERY look about the same.
  GameKeeper::Player *otherData;
  for (int i = 0; i < curMaxPlayers;i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->player.isPlaying()) {
      sendMessage(i, t, string_util::format
		  ("Version: %s",
		   otherData->player.getClientVersion()).c_str());
    }
  }
  return;
}


void handleRecordCmd(GameKeeper::Player *playerData, const char * message)
{
  int t = playerData->getIndex();
  const char *buf = message + 8;
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::record)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /record command");
    return;
  }
  while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

  if (strncmp (buf, "start", 5) == 0) {
    Record::start(t);
  }
  else if (strncmp (buf, "stop", 4) == 0) {
    Record::stop(t);
  }
  else if (strncmp (buf, "size", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace
    
    if (*buf == '\0') {
      Record::sendHelp (t);
      return;
    }
    int size = atoi (buf);
    Record::setSize (t, size);
  }
  else if (strncmp (buf, "rate", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
      return;
    }
    int seconds = atoi (buf);
    Record::setRate (t, seconds);
  }
  else if (strncmp (buf, "stats", 5) == 0) {
    Record::sendStats(t);
  }
  else if (strncmp (buf, "list", 4) == 0) {
    Replay::sendFileList (t); // stolen from '/replay'
  }
  else if (strncmp (buf, "save", 4) == 0) {
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
  else if (strncmp (buf, "file", 4) == 0) {
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


void handleReplayCmd(GameKeeper::Player *playerData, const char * message)
{
  int t = playerData->getIndex();
  const char *buf = message + 7;
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::replay)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /replay command");
    return;
  }
  while ((*buf != '\0') && isspace (*buf)) { // eat whitespace
    buf++;
  }

  if (strncmp (buf, "list", 4) == 0) {
    Replay::sendFileList (t);
  }
  else if (strncmp (buf, "load", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace
    
    if (*buf == '\0') {
      Replay::sendHelp (t);
    }
    else {
      Replay::loadFile (t, buf);
    }
  }
  else if (strncmp (buf, "play", 4) == 0) {
    Replay::play (t);
  }
  else if (strncmp (buf, "skip", 4) == 0) {
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
  else if (strncmp (buf, "pause", 5) == 0) {
    Replay::pause (t);
  }
  else {
    Replay::sendHelp (t);
  }
  
  return;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
