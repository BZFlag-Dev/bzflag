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

// implementation-specific bzfs-specific headers
#include "VotingArbiter.h"
#include "Permissions.h"
#include "CmdLineOptions.h"
#include "PlayerInfo.h"
#include "NetHandler.h"
#include "RecordReplay.h"
#include "LagInfo.h"
#include "FlagHistory.h"

// FIXME -- need to pull communication out of bzfs.cxx...

// externs that poll, veto, vote, and clientquery require
extern void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer=false);
extern PlayerInfo player[MaxPlayers + ReplayObservers];
extern LagInfo *lagInfo[MaxPlayers + ReplayObservers];
extern PlayerAccessInfo accessInfo[MaxPlayers + ReplayObservers];
extern FlagHistory flagHistory[MaxPlayers  + ReplayObservers];
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;
extern int NotConnected;

// util functions
int getPlayerIDByRegName(const std::string &regName)
{
  for (int i = 0; i < curMaxPlayers; i++) {
    if (accessInfo[i].getName() == regName)
      return i;
  }
  return -1;
}

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

void handlePasswordCmd(int t, const char *message)
{
  if (accessInfo[t].passwordAttemptsMax()) {
    DEBUG1("%s has attempted too many /password tries\n",
	   player[t].getCallSign());
    sendMessage(ServerPlayer, t, "Too many attempts");
  } else {
    if ((clOptions->password != "") && strncmp(message + 10, clOptions->password.c_str(), clOptions->password.size()) == 0){
      accessInfo[t].setAdmin();
      sendIPUpdate(t, -1);
      sendMessage(ServerPlayer, t, "You are now an administrator!");
    } else {
      sendMessage(ServerPlayer, t, "Wrong Password!");
    }
  }
  return;
}


void handleSetCmd(int t, const char *message)
{
  char message2[MessageLen];
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::setVar)
      && !accessInfo[t].hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the set command");
    return;
  }
  if (Replay::enabled()) {
    sendMessage(ServerPlayer, t, "You can't /set variables in replay mode");
    return;
  }
  sendMessage(ServerPlayer, t, CMDMGR.run(message+1).c_str());
  snprintf(message2, MessageLen, "Variable Modification Notice by %s of %s", player[t].getCallSign(), message+1); 
  sendMessage(ServerPlayer, AllPlayers, message2, true);
  return;
}


void handleResetCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::setVar)
      && !accessInfo[t].hasPerm(PlayerAccessInfo::setAll)) {
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


void handleShutdownserverCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::shutdownServer)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the shutdown command");
    return;
  }
  done = true;
  return;
}


void handleSuperkillCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::superKill)) {
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


void handleGameoverCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::endGame)) {
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


void handleCountdownCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::countdown)) {
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
  sendMessage(ServerPlayer, t, reply, true);

  // CTF game -> simulate flag captures to return ppl to base
  if (clOptions->gameStyle & int(TeamFlagGameStyle)) {
    // get someone to can do virtual capture
    for (j = 0; j < curMaxPlayers; j++) {
      if (player[j].isPlaying())
	break;
    }
    if (j < curMaxPlayers) {
      for (int i = 0; i < curMaxPlayers; i++) {
	if (player[i].hasPlayedEarly()) {
	  void *buf, *bufStart = getDirectMessageBuffer();
	  buf = nboPackUByte(bufStart, j);
	  buf = player[i].packVirtualFlagCapture(buf);
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


void handleFlagCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::flagMod)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flag command");
    return;
  }
  if (strncmp(message + 6, "reset", 5) == 0) {
    bool onlyUnused = strncmp(message + 11, " unused", 7) == 0;
    for (int i = 0; i < numFlags; i++) {
      // see if someone had grabbed flag,
      const int playerIndex = flag[i].player;
      if ((playerIndex != -1) && (!onlyUnused)) {
	// tell 'em to drop it.
	flag[i].player = -1;
	flag[i].flag.status = FlagNoExist;
	player[playerIndex].resetFlag();

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
	if (playerIndex != -1) {
	  flag[i].player = -1;
	  flag[i].flag.status = FlagNoExist;
	  player[playerIndex].resetFlag();

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
      sendMessage(ServerPlayer, t, message, true);
    }
  } else {
    sendMessage(ServerPlayer, t, "reset|show|up", true);
  }
  return;
}

int getTarget(const char *victimname) {
  int i;
  for (i = 0; i < curMaxPlayers; i++) {
    if (NetHandler::exists(i)
	&& strncasecmp(player[i].getCallSign(), victimname, 256) == 0) {
      break;
    }
  }
  return i;
}

void handleKickCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::kick)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the kick command");
    return;
  }
  int i;
  std::vector<std::string> argv = string_util::tokenize(message, " \t", 3, true);
  
  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t, "Syntax: /kick <PlayerName/\"Player Name\"> [reason]", true);
    sendMessage(ServerPlayer, t, "        Please keep in mind that reason is displayed to the user.", true);
    return;
  }

  const char *victimname = argv[1].c_str();

  i = getTarget(victimname);
  
  if (i < curMaxPlayers) {
    char kickmessage[MessageLen];
    sprintf(kickmessage, "You were kicked off the server by %s",
	    player[t].getCallSign());
    sendMessage(ServerPlayer, i, kickmessage, true);
    if (argv.size() > 2) {
      sprintf(kickmessage, " reason given : %s",argv[2].c_str());
      sendMessage(ServerPlayer, i, kickmessage, true);
    }
    removePlayer(i, "/kick");
  } else {
    char errormessage[MessageLen];
    sprintf(errormessage, "player \"%s\" not found", victimname);
    sendMessage(ServerPlayer, t, errormessage, true);
  }
  return;
}


void handleBanlistCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendBans(t);
  return;
}


void handleHostBanlistCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendHostBans(t);
  return;
}


void handleBanCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::ban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }
  char reply[MessageLen] = {0};

  std::string msg = message;
  std::vector<std::string> argv = string_util::tokenize(msg, " \t", 4);

  if (argv.size() < 2) {
    strcpy(reply, "Syntax: /ban <ip> [duration] [reason]");
    sendMessage(ServerPlayer, t, reply, true);
    strcpy(reply, "        Please keep in mind that reason is displayed to the user.");
    sendMessage(ServerPlayer, t, reply, true);
  } else {
    int durationInt = 0;
    std::string ip = argv[1];
    std::string reason;

    if (argv.size() >= 3)
      durationInt = string_util::parseDuration(argv[2]);

    if (argv.size() == 4)
      reason = argv[3];

    if (clOptions->acl.ban(ip, player[t].getCallSign(), durationInt,
			   reason.c_str())) {
      clOptions->acl.save();
      strcpy(reply, "IP pattern added to banlist");
      char kickmessage[MessageLen];
      for (int i = 0; i < curMaxPlayers; i++) {
	NetHandler *handler = NetHandler::getHandler(i);
	if (handler && !clOptions->acl.validate(handler->getIPAddress())) {
	  sprintf(kickmessage,"You were banned from this server by %s",
		  player[t].getCallSign());
	  sendMessage(ServerPlayer, i, kickmessage, true);
	  if (reason.length() > 0) {
	    sprintf(kickmessage,"Reason given: %s", reason.c_str());
	    sendMessage(ServerPlayer, i, kickmessage, true);
	  }
	  removePlayer(i, "/ban");
	}
      }
    } else {
      strcpy(reply, "Malformed address");
    }
    sendMessage(ServerPlayer, t, reply, true);
  }
  return;
}


void handleHostBanCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::ban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }
  char reply[MessageLen] = {0};

  std::string msg = message;
  std::vector<std::string> argv = string_util::tokenize( msg, " \t", 4 );

  if( argv.size() < 2 ){
    strcpy(reply, "Syntax: /hostban <host pattern> [duration] [reason]");
    sendMessage(ServerPlayer, t, reply, true);
    strcpy(reply, "        Please keep in mind that reason is displayed to the user.");
    sendMessage(ServerPlayer, t, reply, true);
  }
  else {
    int durationInt = 0;
    std::string hostpat = argv[1];
    std::string reason;

    if( argv.size() >= 3 )
      durationInt = string_util::parseDuration(argv[2]);

    if( argv.size() == 4 )
      reason = argv[3];

    clOptions->acl.hostBan(hostpat, player[t].getCallSign(), durationInt,
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
		player[t].getCallSign());
	sendMessage(ServerPlayer, i, kickmessage, true);
	if( reason.length() > 0 ){
	  sprintf(kickmessage,"Reason given: %s", reason.c_str());
	  sendMessage(ServerPlayer, i, kickmessage, true);
	}
	removePlayer(i, "/hostban");
      }
    }
#else
    strcpy(reply, "Host pattern added to banlist. WARNING: host patterns not supported in this compilation.");
#endif
    sendMessage(ServerPlayer, t, reply, true);
  }
  return;
}


void handleUnbanCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::unban)) {
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
  sendMessage(ServerPlayer, t, reply, true);
  return;
}

void handleHostUnbanCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::unban)) {
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
  sendMessage(ServerPlayer, t, reply, true);
  return;
}


void handleLagwarnCmd(int t, const char *message)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::lagwarn)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagwarn command");
    return;
  }

  char reply[MessageLen] = {0};

  if (message[8] == ' ') {
    const char *maxlag = message + 9;
    clOptions->lagwarnthresh = (float) (atoi(maxlag) / 1000.0);
    sprintf(reply,"lagwarn is now %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
    sendMessage(ServerPlayer, t, reply, true);
  } else {
    sprintf(reply,"lagwarn is set to %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
    sendMessage(ServerPlayer, t, reply, true);
  }
  return;
}


void handleLagstatsCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::lagStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagstats command");
    return;
  }

  char reply[MessageLen] = {0};
  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].isPlaying() && player[i].isHuman()) {
      lagInfo[i]->getLagStats(reply);
      if (strlen(reply)) {
	if (accessInfo[i].isAccessVerified())
	  strcat(reply, " (R)");
	sendMessage(ServerPlayer, t, reply, true);
      }
    }
  return;
}


void handleIdlestatsCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::idleStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the idlestats command");
    return;
  }

  std::string reply;
  for (int i = 0; i < curMaxPlayers; i++) {
    reply = player[i].getIdleStat();
    if (reply != "")
      sendMessage(ServerPlayer, t, reply.c_str(), true);
  }
  return;
}


void handleFlaghistoryCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::flagHistory)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flaghistory command");
    return;
  }

  char reply[MessageLen];
  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].isPlaying() && !player[i].isObserver()) {
      sprintf(reply,"%-16s : ", player[i].getCallSign());
      flagHistory[i].get(reply+strlen(reply));
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
}


void handlePlayerlistCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::playerList)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the playerlist command");
    return;
  }

  char reply[MessageLen] = {0};

  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].isPlaying()) {
      NetHandler::getHandler(i)->getPlayerList(reply);
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
  return;
}


void handleReportCmd(int t, const char *message)
{
  char reply[MessageLen] = {0};

  if (strlen(message + 1) < 8) {
    sprintf(reply, "Nothing reported");
  } else {
    time_t now = time(NULL);
    char* timeStr = ctime(&now);
    std::string reportStr;
    reportStr = reportStr + timeStr + "Reported by " +
      player[t].getCallSign() + ": " + (message + 8);
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
	     player[t].getCallSign(), t, timeStr);
    }
  }
  sendMessage(ServerPlayer, t, reply, true);
  return;
}


void handleHelpCmd(int t, const char *message)
{
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
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
  return;
}


void handleIdentifyCmd(int t, const char *message)
{
  // player is trying to send an ID
  if (accessInfo[t].isAccessVerified()) {
    sendMessage(ServerPlayer, t, "You have already identified");
  } else if (accessInfo[t].gotAccessFailure()) {
    sendMessage(ServerPlayer, t, "You have attempted to identify too many times");
  } else {
    // get their info
    if (!accessInfo[t].isRegistered()) {
      // not in DB, tell them to reg
      sendMessage(ServerPlayer, t, "This callsign is not registered,"
		  " please register it with a /register command");
    } else {
      if (accessInfo[t].isPasswordMatching(message + 10)) {
	sendMessage(ServerPlayer, t, "Password Accepted, welcome back.");
	
	// get their real info
	accessInfo[t].setPermissionRights();
	
	// if they have the PLAYERLIST permission, send the IP list
	sendIPUpdate(t, -1);
      } else {
	accessInfo[t].setLoginFail();
	sendMessage(ServerPlayer, t, "Identify Failed, please make sure"
		    " your password was correct");
      }
    }
  }
  return;
}


void handleRegisterCmd(int t, const char *message)
{
  if (accessInfo[t].isAccessVerified()) {
    sendMessage(ServerPlayer, t, "You have already registered and"
		" identified this callsign");
  } else {
    if (accessInfo[t].isRegistered()) {
      sendMessage(ServerPlayer, t, "This callsign is already registered,"
		  " if it is yours /identify to login");
    } else {
      if (strlen(message) > 12) {
	accessInfo[t].storeInfo(message + 10);
	sendMessage(ServerPlayer, t, "Callsign registration confirmed,"
		    " please /identify to login");
      } else {
	sendMessage(ServerPlayer, t, "Your password must be 3 or more characters");
      }
    }
  }
  return;
}


void handleGhostCmd(int t, const char *message)
{
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

    int user = getPlayerIDByRegName(ghostie);
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
		  " You have been ghosted by %s", player[t].getCallSign());
	  sendMessage(ServerPlayer, user, temp, true);
	  removePlayer(user, "Ghost");
	}
      }
    }
  }
  return;
}


void handleDeregisterCmd(int t, const char *message)
{
  if (!accessInfo[t].isAccessVerified()) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the deregister command");
    return;
  }

  if (strlen(message) == 11) {
    // removing own callsign
    PasswordMap::iterator itr1
      = passwordDatabase.find(accessInfo[t].getName());
    PlayerAccessMap::iterator itr2
      = userDatabase.find(accessInfo[t].getName());
    passwordDatabase.erase(itr1);
    userDatabase.erase(itr2);
    PlayerAccessInfo::updateDatabases();
    sendMessage(ServerPlayer, t, "Your callsign has been deregistered");
  } else if (strlen(message) > 12
	     && accessInfo[t].hasPerm(PlayerAccessInfo::setAll)) {
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
  } else if (!accessInfo[t].hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to deregister this user");
  }

  return;
}


void handleSetpassCmd(int t, const char *message)
{
  if (!accessInfo[t].isAccessVerified()) {
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
  accessInfo[t].setPasswd(pass);
  char text[MessageLen];
  snprintf(text, MessageLen, "Your password is now set to \"%s\"", pass.c_str());
  sendMessage(ServerPlayer, t, text, true);
  return;
}


void handleGrouplistCmd(int t, const char *)
{
  sendMessage(ServerPlayer, t, "Group List:");
  PlayerAccessMap::iterator itr = groupAccess.begin();
  while (itr != groupAccess.end()) {
    sendMessage(ServerPlayer, t, itr->first.c_str());
    itr++;
  }
  return;
}


void handleShowgroupCmd(int t, const char *message)
{
  std::string settie;

  if (strlen(message) == 10) {	 // show own groups
    if (accessInfo[t].isAccessVerified()) {
      settie = accessInfo[t].getName();
    } else {
      sendMessage(ServerPlayer, t, "You are not identified");
    }
  } else if (accessInfo[t].hasPerm(PlayerAccessInfo::showOthers)) {
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
      // FIXME let's hope that line is not too long (> MessageLen)
      sendMessage(ServerPlayer, t, line.c_str());
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return;
}


void handleGrouppermsCmd(int t, const char *)
{
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


void handleSetgroupCmd(int t, const char *message)
{
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
      if (!accessInfo[t].canSet(group)) {
	sendMessage(ServerPlayer, t, "You do not have permission to set this group");
      } else {
	PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

	if (info.addGroup(group)) {
	  sendMessage(ServerPlayer, t, "Group Add successful");
	  int getID = getPlayerIDByRegName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    sprintf(temp, "you have been added to the %s group, by %s",
		    group.c_str(), player[t].getCallSign());
	    sendMessage(ServerPlayer, getID, temp, true);
	    accessInfo[getID].addGroup(group);
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


void handleRemovegroupCmd(int t, const char *message)
{
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
      if (!accessInfo[t].canSet(group)) {
	sendMessage(ServerPlayer, t, "You do not have permission to remove this group");
      } else {
	PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

	if (info.removeGroup(group)) {
	  sendMessage(ServerPlayer, t, "Group Remove successful");
	  int getID = getPlayerIDByRegName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    sprintf(temp, "You have been removed from the %s group, by %s",
		    group.c_str(), player[t].getCallSign());
	    sendMessage(ServerPlayer, getID, temp, true);
	    accessInfo[getID].removeGroup(group);
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

void handleReloadCmd(int t, const char *)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::setAll)) {
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
  for (int p = 0; p < curMaxPlayers; p++) {
    accessInfo[p].reloadInfo();
  }
  sendMessage(ServerPlayer, t, "Databases reloaded");

  return;
}


void handlePollCmd(int t, const char *message)
{
  char reply[MessageLen] = {0};
  std::string callsign = std::string(player[t].getCallSign());

  DEBUG2("Entered poll command handler (MessageLen is %d)\n", MessageLen);

  /* make sure player has permission to request a poll */
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::poll)) {
    sprintf(reply,"%s, you are presently not authorized to run /poll", callsign.c_str());
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }

  DEBUG2("Player has permission\n");

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)", true);
    return;
  }

  DEBUG2("BZDB poll value is not empty\n");

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  DEBUG2("Arbiter was acquired with address 0x%x\n", (unsigned int)arbiter);

  /* make sure that there is not a poll active already */
  if (arbiter->knowsPoll()) {
    sprintf(reply,"A poll to %s %s is presently in progress", arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply, true);
    sendMessage(ServerPlayer, t, "Unable to start a new poll until the current one is over", true);
    return;
  }

  DEBUG2("The arbiter says there is not another poll active\n");

  // get available voter count
  unsigned short int available = 0;
  for (int i = 0; i < curMaxPlayers; i++) {
    // any registered/known users on the server (including observers) are eligible to vote

    if (player[i].exist() && accessInfo[i].exists()) {
      available++;
    }
  }

  DEBUG2("There are %d available players for %d votes required\n", available, clOptions->votesRequired);

  /* make sure there are enough players to even make a poll that has a chance
   * of succeeding (not counting the person being acted upon)
   */
  if (available - 1 < clOptions->votesRequired) {
    sendMessage(ServerPlayer, t, "Unable to initiate a new poll.  There are not enough registered players playing.", true);
    sprintf(reply,"There needs to be at least %d other %s and only %d %s available.",
	    clOptions->votesRequired,
	    clOptions->votesRequired - 1 == 1 ? "player" : "players",
	    available - 1,
	    available - 1 == 1 ? "is" : "are");
    sendMessage(ServerPlayer, t, reply, true);
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
      sendMessage(ServerPlayer, t, reply, true);
      sprintf(reply,"Usage: /poll %s target", cmd.c_str());
      sendMessage(ServerPlayer, t, reply, true);
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
	sendMessage(ServerPlayer, t, reply, true);
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
      sendMessage(ServerPlayer, t, reply, true);
      return;
    } else {
      sprintf(reply,"A poll to %s %s has been requested by %s", cmd.c_str(), target.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, AllPlayers, reply, true);
    }

    unsigned int necessaryToSucceed = (unsigned int)((clOptions->votePercentage / 100.0) * (double)available);
    sprintf(reply, "%d player%s available, %d additional affirming vote%s are required to pass the poll (%f %%)", available, available==1?"":"s", necessaryToSucceed, necessaryToSucceed==1?"":"s", clOptions->votePercentage);
    sendMessage(ServerPlayer, AllPlayers, reply, true);

    // set the number of available voters
    arbiter->setAvailableVoters(available);

    // keep track of who is allowed to vote
    for (int j = 0; j < curMaxPlayers; j++) {
      // any registered/known users on the server (including observers) are eligible to vote
      if (player[j].exist() && accessInfo[j].exists()) {
	arbiter->grantSuffrage(player[j].getCallSign());
      }
    }

    // automatically place a vote for the player requesting the poll
    DEBUG2("Attempting to automatically place a vote for [%s]\n", callsign.c_str());

    bool voted = arbiter->voteYes(callsign);
    if (!voted) {
      sendMessage(ServerPlayer, t, "Unable to automatically place your vote for some unknown reason", true);
      DEBUG2("Unable to automatically place a vote for [%s]\n", callsign.c_str());
    }

  } else if (cmd == "vote") {
    std::string voteCmd = "/vote ";
    voteCmd += arguments;
    handleVoteCmd(t, voteCmd.c_str());
    return;

  } else if (cmd == "veto") {
    std::string vetoCmd = "/veto ";
    vetoCmd += arguments;
    handleVetoCmd(t, vetoCmd.c_str());
    return;

  } else {
    sendMessage(ServerPlayer, t, "Invalid option to the poll command", true);
    sendMessage(ServerPlayer, t, "Usage: /poll ban|kick playername", true);
    sendMessage(ServerPlayer, t, "    or /poll set variable value", true);
    sendMessage(ServerPlayer, t, "    or /poll flagreset", true);
    sendMessage(ServerPlayer, t, "    or /poll vote yes|no", true);
    sendMessage(ServerPlayer, t, "    or /poll veto", true);

  } /* end handling of poll subcommands */

  return;
}


void handleVoteCmd(int t, const char *message)
{
  char reply[MessageLen] = {0};
  std::string callsign = std::string(player[t].getCallSign());

  if (!accessInfo[t].hasPerm(PlayerAccessInfo::vote)) {
    /* permission denied for /vote */
    sprintf(reply,"%s, you are presently not authorized to run /vote", callsign.c_str());
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)", true);
    return;
  }

  // only need to get this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure that there is a poll to vote upon */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sendMessage(ServerPlayer, t, "A poll is not presently in progress.  There is nothing to vote on", true);
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
      sendMessage(ServerPlayer, t, reply, true);
    }
  } else if (vote == 1) {
    if ((cast = arbiter->voteYes(callsign)) == true) {
      /* player voted yes */
      sprintf(reply,"%s, your vote in favor of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply, true);
    }
  } else {
    if (answer.length() == 0) {
      sprintf(reply,"%s, you did not provide a vote answer", callsign.c_str());
      sendMessage(ServerPlayer, t, reply, true);
    } else {
      sprintf(reply,"%s, you did not vote in favor or in opposition", callsign.c_str());
      sendMessage(ServerPlayer, t, reply, true);
    }
    sendMessage(ServerPlayer, t, "Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao", true);
    return;
  }

  if (!cast) {
    /* player was unable to cast their vote; probably already voted */
    sprintf(reply,"%s, you have already voted on the poll to %s %s", callsign.c_str(), arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }

  return;
}


void handleVetoCmd(int t, const char * /*message*/)
{
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::veto)) {
    /* permission denied for /veto */
    sendMessage(ServerPlayer, t,
		string_util::format
		("%s, you are presently not authorized to run /veto",
		 player[t].getCallSign()).c_str(), true);
    return;
  }

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)", true);
    return;
  }

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure there is an unexpired poll */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sendMessage(ServerPlayer, t,
		string_util::format
		("%s, there is presently no active poll to veto",
		 player[t].getCallSign()).c_str(), true);
    return;
  }

  sendMessage(ServerPlayer, t,
	      string_util::format("%s, you have cancelled the poll to %s %s",
				  player[t].getCallSign(),
				  arbiter->getPollAction().c_str(),
				  arbiter->getPollTarget().c_str()).c_str(),
	      true);

  /* poof */
  arbiter->forgetPoll();

  sendMessage(ServerPlayer, AllPlayers,
	      string_util::format("The poll was cancelled by %s",
				  player[t].getCallSign()).c_str(), true);

  return;
}

void handleViewReportsCmd(int t, const char * /*message*/)
{
  std::string line;
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::viewReports)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the viewreports command");
    return;
  }
  if (clOptions->reportFile.size() == 0 && clOptions->reportPipe.size() == 0) {
    line = "The /report command is disabled on this server or there are no reports filed.";
    sendMessage(ServerPlayer, t, line.c_str(), true);
  } 
  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  while (std::getline(ifs, line))
    sendMessage(ServerPlayer, t, line.c_str(), true);
}
 

void handleClientqueryCmd(int t, const char * /*message*/)
{
  DEBUG2("Clientquery requested by %s [%d]\n", player[t].getCallSign(), t);
  sendMessage(ServerPlayer, AllPlayers, "[Sent version information per request]");
  // send server's own version string just for kicks
  sendMessage(ServerPlayer, t, 
              string_util::format("BZFS Version: %s", getAppVersion()).c_str());
  // send all players' version strings
  // is faking a message from the remote client rude?
  // did that so that /clientquery and CLIENTQUERY look about the same.
  for (int i = 0; i < curMaxPlayers;i++) {
    if (player[i].isPlaying()) {
      sendMessage(i, t, string_util::format("Version: %s", player[i].getClientVersion()).c_str());
    }
  }
  return;
}


void handleRecordCmd(int t, const char * message)
{
  const char *buf = message + 8;
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::record)) {
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


void handleReplayCmd(int t, const char * message)
{
  const char *buf = message + 7;
  if (!accessInfo[t].hasPerm(PlayerAccessInfo::replay)) {
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
