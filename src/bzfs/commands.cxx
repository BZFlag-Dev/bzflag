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

// FIXME -- need to pull communication out of bzfs.cxx...

// externs that poll, veto, and vote require
extern void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer=false);
extern bool hasPerm(int playerIndex, PlayerAccessInfo::AccessPerm right);
extern PlayerInfo player[MaxPlayers];
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;
extern int NotConnected;

// externs that removegroup needs
extern int getPlayerIDByRegName(const std::string &regName);

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


void handlePasswordCmd(int t, const char *message)
{
  if (player[t].passwordAttempts >=5 ){	// see how many times they have tried, you only get 5
    sendMessage(ServerPlayer, t, "Too many attempts");
  }else{
    player[t].passwordAttempts++;
    if (clOptions->password && strncmp(message + 10, clOptions->password, strlen(clOptions->password)) == 0){
      player[t].passwordAttempts = 0;
      player[t].Admin = true;
      sendMessage(ServerPlayer, t, "You are now an administrator!");
    }else{
      sendMessage(ServerPlayer, t, "Wrong Password!");
    }
  }
  return;
}


void handleSetCmd(int t, const char *message)
{
  if (!hasPerm(t, PlayerAccessInfo::setVar) && !hasPerm(t, PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the set command");
    return;
  }
  sendMessage(ServerPlayer, t, CMDMGR.run(message+1).c_str());
  return;
}


void handleResetCmd(int t, const char *message)
{
  if (!hasPerm(t, PlayerAccessInfo::setVar) && !hasPerm(t, PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the reset command");
    return;
  }
  sendMessage(ServerPlayer, t, CMDMGR.run(message+1).c_str());
  return;
}


void handleShutdownserverCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::shutdownServer)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the reset command");
    return;
  }
  done = true;
  return;
}


void handleSuperkillCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::superKill)) {
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
  if (!hasPerm(t, PlayerAccessInfo::endGame)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the gameover command");
    return;
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, t);
  buf = nboPackUShort(buf, uint16_t(NoTeam));
  broadcastMessage(MsgScoreOver, (char*)buf-(char*)bufStart, bufStart);
  gameOver = true;
  if (clOptions->timeManualStart)
    countdownActive = false;
  return;
}


void handleCountdownCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::countdown)) {
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
  for (i=RedTeam;i<=PurpleTeam;i++) {
    team[i].team.lost = team[i].team.won=0;
  }
  sendTeamUpdate();

  sprintf(reply, "Countdown started.");
  sendMessage(ServerPlayer, t, reply, true);

  // CTF game -> simulate flag captures to return ppl to base
  if (clOptions->gameStyle & int(TeamFlagGameStyle)) {
    // get someone to can do virtual capture
    for (j=0;j<curMaxPlayers;j++) {
      if (player[j].state > PlayerInLimbo)
	break;
    }
    if (j < curMaxPlayers) {
      for (int i=0;i<curMaxPlayers;i++) {
	if (player[i].playedEarly) {
	  void *buf, *bufStart = getDirectMessageBuffer();
	  buf = nboPackUByte(bufStart, j);
	  buf = nboPackUShort(buf, uint16_t(int(player[i].team)-1));
	  buf = nboPackUShort(buf, uint16_t(1+((int(player[i].team))%4)));
	  directMessage(i, MsgCaptureFlag, (char*)buf-(char*)bufStart, bufStart);
	  player[i].playedEarly = false;
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
  if (!hasPerm(t, PlayerAccessInfo::flagMod)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flag command");
    return;
  }
  if (strncmp(message + 6, "reset", 5) == 0) {
    bool onlyUnused = strncmp(message + 11, " unused", 7) == 0;
    for (int i = 0; i < numFlags; i++) {
      // see if someone had grabbed flag,
      const int playerIndex = flag[i].player;
      if ((playerIndex != -1) && (!onlyUnused)) {
	//	tell 'em to drop it.
	flag[i].player = -1;
	flag[i].flag.status = FlagNoExist;
	player[playerIndex].flag = -1;

	void *buf, *bufStart = getDirectMessageBuffer();
	buf = nboPackUByte(bufStart, playerIndex);
	buf = nboPackUShort(buf, uint16_t(i));
	buf = flag[i].flag.pack(buf);
	broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
	player[playerIndex].lastFlagDropTime = TimeKeeper::getCurrent();

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
	  player[playerIndex].flag = -1;

	  void *buf, *bufStart = getDirectMessageBuffer();
	  buf = nboPackUByte(bufStart, playerIndex);
	  buf = nboPackUShort(buf, uint16_t(i));
	  buf = flag[i].flag.pack(buf);
	  broadcastMessage(MsgDropFlag, (char*)buf-(char*)bufStart, bufStart);
	  player[playerIndex].lastFlagDropTime = TimeKeeper::getCurrent();
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


void handleKickCmd(int t, const char *message)
{
  if (!hasPerm(t, PlayerAccessInfo::kick)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the kick command");
    return;
  }
  int i;
  std::vector<std::string> argv = string_util::tokenize( message, " \t", 3, true);
  
  if( argv.size() < 2 ){
    sendMessage(ServerPlayer, t, "Syntax: /kick <PlayerName/\"Player Name\"> [reason]", true);
    sendMessage(ServerPlayer, t, "        Please keep in mind that reason is displayed to the user.", true);
    return;
  }

  const char *victimname = argv[1].c_str();

  for (i = 0; i < curMaxPlayers; i++) {
    if (player[i].fd != NotConnected && strcasecmp(player[i].callSign, victimname) == 0) {
      break;
    }
  }
  
  if (i < curMaxPlayers) {
    char kickmessage[MessageLen];
    sprintf(kickmessage,"You were kicked off the server by %s", player[t].callSign);
    sendMessage(ServerPlayer, i, kickmessage, true);
    if (argv.size() > 2){
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
  if (!hasPerm(t, PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendBans(t);
  return;
}


void handleHostBanlistCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the banlist command");
    return;
  }
  clOptions->acl.sendHostBans(t);
  return;
}


void handleBanCmd(int t, const char *message)
{
  if (!hasPerm(t, PlayerAccessInfo::ban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the ban command");
    return;
  }
  char reply[MessageLen] = {0};

  std::string msg = message;
  std::vector<std::string> argv = string_util::tokenize( msg, " \t", 4 );

  if( argv.size() < 2 ){
    strcpy(reply, "Syntax: /ban <ip> [duration] [reason]");
    sendMessage(ServerPlayer, t, reply, true);
    strcpy(reply, "        Please keep in mind that reason is displayed to the user.");
    sendMessage(ServerPlayer, t, reply, true);
  }
  else {
    int durationInt = 0;
    std::string ip = argv[1];
    std::string reason;

    if( argv.size() >= 3 )
      durationInt = string_util::parseDuration(argv[2]);

    if( argv.size() == 4 )
      reason = argv[3];

    if (clOptions->acl.ban(ip, player[t].callSign, durationInt, reason.c_str())){
      clOptions->acl.save();
      strcpy(reply, "IP pattern added to banlist");
      char kickmessage[MessageLen];
      for (int i = 0; i < curMaxPlayers; i++) {
	if ((player[i].fd != NotConnected) && (!clOptions->acl.validate(player[i].taddr.sin_addr))) {
	  sprintf(kickmessage,"You were banned from this server by %s", player[t].callSign);
	  sendMessage(ServerPlayer, i, kickmessage, true);
	  if( reason.length() > 0 ){
	    sprintf(kickmessage,"Reason given: %s", reason.c_str());
	    sendMessage(ServerPlayer, i, kickmessage, true);
	  }
	  removePlayer(i, "/ban");
	}
      }
    }
    else {
      strcpy(reply, "malformed address");
    }
    sendMessage(ServerPlayer, t, reply, true);
  }
  return;
}


void handleHostBanCmd(int t, const char *message)
{
  if (!hasPerm(t, PlayerAccessInfo::ban)) {
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

    clOptions->acl.hostBan(hostpat, player[t].callSign, durationInt, reason.c_str());
    clOptions->acl.save();
#ifdef HAVE_ADNS_H
    strcpy(reply, "Host pattern added to banlist");
    char kickmessage[MessageLen];
    for (int i = 0; i < curMaxPlayers; i++) {
      if ((player[i].fd != NotConnected) && player[i].hostname && (!clOptions->acl.hostValidate(player[i].hostname))) {
	sprintf(kickmessage,"You were banned from this server by %s", player[t].callSign);
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
  if (!hasPerm(t, PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the unban command");
    return;
  }
  char reply[MessageLen] = {0};

  if (clOptions->acl.unban(message + 7)) {
    strcpy(reply, "removed IP pattern");
    clOptions->acl.save();
  }
  else
    strcpy(reply, "no pattern removed");
  sendMessage(ServerPlayer, t, reply, true);
  return;
}

void handleHostUnbanCmd(int t, const char *message)
{
  if (!hasPerm(t, PlayerAccessInfo::unban)) {
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
  if (!hasPerm(t, PlayerAccessInfo::lagwarn)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagwarn command");
    return;
  }

  char reply[MessageLen] = {0};

  if (message[8] == ' ') {
    const char *maxlag = message + 9;
    clOptions->lagwarnthresh = (float) (atoi(maxlag) / 1000.0);
    sprintf(reply,"lagwarn is now %d ms",int(clOptions->lagwarnthresh * 1000 + 0.5));
    sendMessage(ServerPlayer, t, reply, true);
  }
  else
    {
      sprintf(reply,"lagwarn is set to %d ms",int(clOptions->lagwarnthresh * 1000 +  0.5));
      sendMessage(ServerPlayer, t, reply, true);
    }
  return;
}


void handleLagstatsCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::lagStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagstats command");
    return;
  }

  char reply[MessageLen] = {0};

  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].state > PlayerInLimbo && player[i].type == TankPlayer) {
      sprintf(reply,"%-16s : %3d +- %2dms %s", player[i].callSign,
	      int(player[i].lagavg*1000),
	      int(player[i].jitteravg*1000),
	      player[i].accessInfo.verified ? "(R)" : "");
      if (player[i].lostavg>=0.01f)
	sprintf(reply+strlen(reply), " %d%% lost/ooo", int(player[i].lostavg*100));
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
  return;
}


void handleIdlestatsCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::idleStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the idlestats command");
    return;
  }

  TimeKeeper now=TimeKeeper::getCurrent();
  std::string reply;
  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].state > PlayerInLimbo && player[i].team != ObserverTeam) {
      reply = string_util::format("%-16s : %4ds", player[i].callSign, 
				  int(now - player[i].lastupdate));
      if (player[i].paused) {
	reply += string_util::format("  paused %4ds",
				     int(now - player[i].pausedSince));
      }
      sendMessage(ServerPlayer, t, reply.c_str(), true);
    }
  }
  return;
}


void handleFlaghistoryCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::flagHistory)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flaghistory command");
    return;
  }

  char reply[MessageLen] = {0};

  for (int i = 0; i < curMaxPlayers; i++)
    if (player[i].state > PlayerInLimbo && player[i].team != ObserverTeam) {
      char flag[MessageLen];
      sprintf(reply,"%-16s : ",player[i].callSign );
      std::vector<FlagType*>::iterator fhIt = player[i].flagHistory.begin();

      while (fhIt != player[i].flagHistory.end()) {
	FlagType * fDesc = (FlagType*)(*fhIt);
	if (fDesc->endurance == FlagNormal)
	  sprintf(flag, "(*%c) ", fDesc->flagName[0] );
	else
	  sprintf(flag, "(%s) ", fDesc->flagAbbv );
	strcat(reply, flag );
	fhIt++;
      }
      sendMessage(ServerPlayer, t, reply, true);
    }
  return;
}


void handlePlayerlistCmd(int t, const char *)
{
  if (!hasPerm(t, PlayerAccessInfo::playerList)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the playerlist command");
    return;
  }

  char reply[MessageLen] = {0};

  for (int i = 0; i < curMaxPlayers; i++) {
    if (player[i].state > PlayerInLimbo) {
      sprintf(reply,"[%d]%-16s: %s%s%s%s%s%s",i,player[i].callSign,
	      player[i].peer.getDotNotation().c_str(),
#ifdef HAVE_ADNS_H
	      player[i].hostname ? " (" : "",
	      player[i].hostname ? player[i].hostname : "",
	      player[i].hostname ? ")" : "",
#else
	      "", "", "",
#endif
	      player[i].udpin ? " udp" : "",
	      player[i].udpout ? "+" : "");
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
  return;
}


void handleReportCmd(int t, const char *message)
{
  char reply[MessageLen] = {0};

  if (strlen(message+1) < 8) {
    sprintf(reply, "Nothing reported");
  }
  else {
    time_t now = time(NULL);
    char* timeStr = ctime(&now);
    std::string reportStr;
    reportStr = reportStr + timeStr + "Reported by " +
      player[t].callSign + ": " + (message + 8);
    if (clOptions->reportFile.size() > 0) {
      std::ofstream ofs(clOptions->reportFile.c_str(), std::ios::out | std::ios::app);
      ofs<<reportStr<<std::endl<<std::endl;
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
    if (!foundChunk){
      sprintf(reply, "help command %s not found", message + 6);
      sendMessage(ServerPlayer, t, reply, true);
    }
  }
  return;
}


void handleIdentifyCmd(int t, const char *message)
{
  // player is trying to send an ID
  if (player[t].accessInfo.verified) {
    sendMessage(ServerPlayer, t, "You have already identified");
  } else if (player[t].accessInfo.loginAttempts >= 5) {
    sendMessage(ServerPlayer, t, "You have attempted to identify too many times");
    DEBUG1("Too Many Identifys %s\n",player[t].regName.c_str());
  } else {
    // get their info
    if (!userExists(player[t].regName)) {
      // not in DB, tell them to reg
      sendMessage(ServerPlayer, t, "This callsign is not registered,"
		  " please register it with a /register command");
    } else {
      if (verifyUserPassword(player[t].regName.c_str(), message + 10)) {
	sendMessage(ServerPlayer, t, "Password Accepted, welcome back.");
	player[t].accessInfo.verified = true;

	// get their real info
	PlayerAccessInfo &info = getUserInfo(player[t].regName);
	player[t].accessInfo.explicitAllows = info.explicitAllows;
	player[t].accessInfo.explicitDenys = info.explicitDenys;
	player[t].accessInfo.groups = info.groups;

	DEBUG1("Identify %s\n",player[t].regName.c_str());
      } else {
	player[t].accessInfo.loginAttempts++;
	sendMessage(ServerPlayer, t, "Identify Failed, please make sure"
		    " your password was correct");
      }
    }
  }
  return;
}


void handleRegisterCmd(int t, const char *message)
{
  if (player[t].accessInfo.verified) {
    sendMessage(ServerPlayer, t, "You have allready registered and"
		" identified this callsign");
  } else {
    if (userExists(player[t].regName)) {
      sendMessage(ServerPlayer, t, "This callsign is allready registered,"
		  " if it is yours /identify to login");
    } else {
      if (strlen(message) > 12) {
	PlayerAccessInfo info;
	info.groups.push_back("DEFAULT");
	info.groups.push_back("REGISTERED");
	std::string pass = message + 10;
	setUserPassword(player[t].regName.c_str(), pass.c_str());
	setUserInfo(player[t].regName, info);
	DEBUG1("Register %s %s\n",player[t].regName.c_str(),pass.c_str());

	sendMessage(ServerPlayer, t, "Callsign registration confirmed,"
		    " please /identify to login");
	  updateDatabases();
      } else {
	  sendMessage(ServerPlayer, t, "your password must be 3 or more characters");
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
    std::string ghostie(p1+1,p2-p1-1);
    std::string ghostPass=p2+2;

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
		  " You have been ghosted by %s", player[t].callSign);
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
  if (!player[t].accessInfo.verified) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the deregister command");
    return;
  }

  if (strlen(message) == 11) {
    // removing own callsign
    PasswordMap::iterator itr1 = passwordDatabase.find(player[t].regName);
    PlayerAccessMap::iterator itr2 = userDatabase.find(player[t].regName);
    passwordDatabase.erase(itr1);
    userDatabase.erase(itr2);
    updateDatabases();
      sendMessage(ServerPlayer, t, "Your callsign has been deregistered");
  } else if (strlen(message) > 12 && hasPerm(t, PlayerAccessInfo::setAll)) {
    // removing someone else's
    std::string name = message + 12;
    makeupper(name);
    if (userExists(name)) {
      PasswordMap::iterator itr1 = passwordDatabase.find(name);
      PlayerAccessMap::iterator itr2 = userDatabase.find(name);
      passwordDatabase.erase(itr1);
      userDatabase.erase(itr2);
      updateDatabases();
      char text[MessageLen];
      sprintf(text, "%s has been deregistered", name.c_str());
      sendMessage(ServerPlayer, t, text);
    } else {
      char text[MessageLen];
      sprintf(text, "user %s does not exist", name.c_str());
      sendMessage(ServerPlayer, t, text);
    }
  }
  return;
}


void handleSetpassCmd(int t, const char *message)
{
  if (!player[t].accessInfo.verified) {
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
  setUserPassword(player[t].regName.c_str(), pass);
  updateDatabases();
  char text[MessageLen];
  sprintf(text, "Your password is now set to \"%s\"", pass.c_str());
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
    if (player[t].accessInfo.verified) {
      settie = player[t].regName;
    } else {
      sendMessage(ServerPlayer, t, "You are not identified");
    }
  } else if (hasPerm(t, PlayerAccessInfo::showOthers)) { // show groups for other player
    char *p1 = strchr(message + 1, '\"');
    char *p2 = 0;
    if (p1) p2 = strchr(p1 + 1, '\"');
    if (p2) {
      settie = std::string(p1+1, p2-p1-1);
      makeupper(settie);
    } else {
      sendMessage(ServerPlayer, t, "wrong format, usage"
		  " /showgroup  or  /showgroup \"CALLSIGN\"");
    }
  } else {
    sendMessage(ServerPlayer, t, "No permission!");
  }

  // something is wrong
  if (settie!="") {
    if (userExists(settie)) {
      PlayerAccessInfo &info = getUserInfo(settie);

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
    std::string settie(p1+1, p2-p1-1);
    std::string group=p2+2;

    makeupper(settie);
    makeupper(group);

    if (userExists(settie)) {
      bool canset = true;
      if (!hasPerm(t, PlayerAccessInfo::setAll)) {
	canset = hasGroup(player[t].accessInfo, group.c_str())
	  && hasPerm(t, PlayerAccessInfo::setPerms);
      }
      if (!canset) {
	sendMessage(ServerPlayer, t, "You do not have permission to set this group");
      } else {
	PlayerAccessInfo &info = getUserInfo(settie);

	if (addGroup(info, group)) {
	  sendMessage(ServerPlayer, t, "Group Add successful");
	  int getID = getPlayerIDByRegName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    sprintf(temp, "you have been added to the %s group, by %s", group.c_str(), player[t].callSign);
	    sendMessage(ServerPlayer, getID, temp, true);
	    addGroup(player[getID].accessInfo, group);
	  }
	  updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "Group Add failed (user may allready have that group)");
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
    std::string settie(p1+1, p2-p1-1);
    std::string group=p2+2;

    makeupper(settie);
    makeupper(group);
    if (userExists(settie)) {
      bool canset = true;
      if (!hasPerm(t, PlayerAccessInfo::setAll)) {
	canset = hasGroup(player[t].accessInfo, group.c_str())
	  && hasPerm(t, PlayerAccessInfo::setPerms);
      }
      if (!canset) {
	sendMessage(ServerPlayer, t, "You do not have permission to remove this group");
      } else {
	PlayerAccessInfo &info = getUserInfo(settie);

	if (removeGroup(info, group)) {
	  sendMessage(ServerPlayer, t, "Group Remove successful");
	  int getID = getPlayerIDByRegName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    sprintf(temp, "you have been removed from the %s group, by %s", group.c_str(), player[t].callSign);
	    sendMessage(ServerPlayer, getID, temp, true);
	    removeGroup(player[getID].accessInfo, group);
	  }
	  updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "Group Remove failed ( user may not have had group)");
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
  if (!hasPerm(t, PlayerAccessInfo::setAll)) {
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
    readGroupsFile(groupsFile);
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
    readPermsFile(userDatabaseFile);
  for (int p = 0; p < curMaxPlayers; p++) {
    if (player[p].accessInfo.verified && userExists(player[p].regName)) {
      player[p].accessInfo = getUserInfo(player[p].regName);
      player[p].accessInfo.verified = true;
    }
  }
  sendMessage(ServerPlayer, t, "Databases reloaded");

  return;
}


void handlePollCmd(int t, const char *message)
{
  char reply[MessageLen] = {0};

  DEBUG2("Entered poll command handler (MessageLen is %d)\n", MessageLen);

  /* make sure player has permission to request a poll */
  if (!hasPerm(t, PlayerAccessInfo::poll)) {
    sprintf(reply,"%s, you are presently not authorized to run /poll", player[t].callSign);
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
  for (int i=0; i < curMaxPlayers; i++) {
    // any registered/known users on the server (including observers) are eligible to vote
    if ((player[i].fd != NotConnected) && userExists(player[i].regName)) {
      available++;
    }
  }

  DEBUG2("There are %d available players for %d votes required\n", available, clOptions->votesRequired);

  /* make sure there are enough players to even make a poll that has a chance
   * of succeeding (not counting the person being acted upon)
   */
  if (available - 1 < clOptions->votesRequired) {
    sendMessage(ServerPlayer, t, "Unable to initiate a new poll.  There are not enough regsitered players playing.", true);
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

  if ((cmd == "ban") || (cmd == "kick")) {
    std::string nick;

    arguments = arguments.substr(endPosition);

    DEBUG2("Command arguments rguments is [%s]\n", arguments.c_str());

    /* find the start of the player name */
    startPosition = 0;
    while ((startPosition < arguments.size()) &&
	   (isspace(arguments[startPosition]))) {
      startPosition++;
    }
    // do not include a starting quote, if given
    if ( arguments[startPosition] == '"' ) {
      startPosition++;
    }

    DEBUG2("Start position for player name is %d\n", (int)startPosition);

    /* find the end of the player name */
    endPosition = arguments.size() - 1;
    while ((endPosition > 0) &&
	   (isspace(arguments[endPosition]))) {
      endPosition--;
    }
    // do not include a trailing quote, if given
    if ( arguments[endPosition] == '"' ) {
      endPosition--;
    }

    DEBUG2("End position for player name is %d\n", (int)endPosition);

    nick = arguments.substr(startPosition, endPosition - startPosition + 1);

    DEBUG2("Player specified to vote upon is [%s]\n", nick.c_str());

    if (nick.length() == 0) {
      sprintf(reply,"%s, no player was specified for the [%s] vote", player[t].callSign, cmd.c_str());
      sendMessage(ServerPlayer, t, reply, true);
      sprintf(reply,"Usage: /poll %s playername", cmd.c_str());
      sendMessage(ServerPlayer, t, reply, true);
      return;
    }

    /* make sure the requested player is actually here */
    bool foundPlayer=false;
    std::string playerIP = "";
    for (int v = 0; v < curMaxPlayers; v++) {
      if (strncasecmp(nick.c_str(), player[v].callSign, 256) == 0) {
	playerIP = player[v].peer.getDotNotation().c_str();
	foundPlayer=true;
	break;
      }
    }

    if (!foundPlayer) {
      /* wrong name? */
      sprintf(reply, "The player specified for a %s vote is not here", cmd.c_str());
      sendMessage(ServerPlayer, t, reply, true);
      return;
    }

    /* create and announce the new poll */
    if (cmd == "ban") {
      if (arbiter->pollToBan(nick.c_str(), player[t].callSign, playerIP) == false) {
	sprintf(reply,"You are not able to request a ban poll right now, %s", player[t].callSign);
	sendMessage(ServerPlayer, t, reply, true);
	return;
      } else {
	sprintf(reply,"A poll to temporarily ban %s has been requested by %s", nick.c_str(), player[t].callSign);
	sendMessage(ServerPlayer, AllPlayers, reply, true);
      }
    } else {
      if (arbiter->pollToKick(nick.c_str(), player[t].callSign) == false) {
	sprintf(reply,"You are not able to request a kick poll right now, %s", player[t].callSign);
	sendMessage(ServerPlayer, t, reply, true);
	return;
      } else {
	sprintf(reply,"A poll to %s %s has been requested by %s", cmd.c_str(), nick.c_str(), player[t].callSign);
	sendMessage(ServerPlayer, AllPlayers, reply, true);
      }
    }

    unsigned int necessaryToSucceed = (unsigned int)((clOptions->votePercentage / 100.0) * (double)available);
    sprintf(reply, "%d player%s available, %d additional affirming vote%s are required to pass the poll (%f %%)", available, available==1?"":"s", necessaryToSucceed, necessaryToSucceed==1?"":"s", clOptions->votePercentage);
    sendMessage(ServerPlayer, AllPlayers, reply, true);

    // set the number of available voters
    arbiter->setAvailableVoters(available);

    // keep track of who is allowed to vote
    for (int j=0; j < curMaxPlayers; j++) {
      // any registered/known users on the server (including observers) are eligible to vote
      if ((player[j].fd != NotConnected) && userExists(player[j].regName)) {
	arbiter->grantSuffrage(player[j].callSign);
      }
    }

    // automatically place a vote for the player requesting the poll
    DEBUG2("Attempting to automatically place a vote for [%s]\n", player[t].callSign);

    bool voted = arbiter->voteYes(player[t].callSign);
    if (!voted) {
      sendMessage(ServerPlayer, t, "Unable to automatically place your vote for some unknown reason", true);
      DEBUG2("Unable to  to automatically place a vote for [%s]\n", player[t].callSign);
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
    sendMessage(ServerPlayer, t, "    or /poll vote yes|no", true);
    sendMessage(ServerPlayer, t, "    or /poll veto", true);

  } /* end handling of poll subcommands */

  return;
}


void handleVoteCmd(int t, const char *message)
{
  char reply[MessageLen] = {0};

  if (!hasPerm(t, PlayerAccessInfo::vote)) {
    /* permission denied for /vote */
    sprintf(reply,"%s, you are presently not authorized to run /vote", player[t].callSign);
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
  int vote=-1;
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
    if ((cast = arbiter->voteNo(player[t].callSign)) == true) {
      /* player voted no */
      sprintf(reply,"%s, your vote in opposition of the %s has been recorded", player[t].callSign, arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply, true);
    }
  } else if (vote == 1) {
    if ((cast = arbiter->voteYes(player[t].callSign)) == true) {
      /* player voted yes */
      sprintf(reply,"%s, your vote in favor of the %s has been recorded", player[t].callSign, arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply, true);
    }
  } else {
    if (answer.length() == 0) {
      sprintf(reply,"%s, you did not provide a vote answer", player[t].callSign);
      sendMessage(ServerPlayer, t, reply, true);
    } else {
      sprintf(reply,"%s, you did not vote in favor or in opposition", player[t].callSign);
      sendMessage(ServerPlayer, t, reply, true);
    }
    sendMessage(ServerPlayer, t, "Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao", true);
    return;
  }

  if (!cast) {
    /* player was unable to cast their vote; probably already voted */
    sprintf(reply,"%s, you have already voted on the poll to %s %s", player[t].callSign, arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }

  return;
}


void handleVetoCmd(int t, const char * /*message*/)
{
  if (!hasPerm(t, PlayerAccessInfo::veto)) {
    /* permission denied for /veto */
    sendMessage(ServerPlayer, t, string_util::format("%s, you are presently not authorized to run /veto", player[t].callSign).c_str(), true);
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
    sendMessage(ServerPlayer, t, string_util::format("%s, there is presently no active poll to veto", player[t].callSign).c_str(), true);
    return;
  }

  sendMessage(ServerPlayer, t, string_util::format("%s, you have cancelled the poll to %s %s", player[t].callSign, arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str()).c_str(), true);

  /* poof */
  arbiter->forgetPoll();

  sendMessage(ServerPlayer, AllPlayers, string_util::format("The poll was cancelled by %s", player[t].callSign).c_str(), true);

  return;
}




// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

