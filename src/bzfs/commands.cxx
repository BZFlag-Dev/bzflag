/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

// interface header
#include "commands.h"

// implementation-specific system headers
#include <string>
#include <cstdio>
#include <cstring>

// implementation-specific bzflag headers
#include "global.h"
#include "Address.h"

// implementation-specific bzfs-specific headers
#include "VotingArbiter.h"
#include "Permissions.h"
#include "CmdLineOptions.h"
#include "PlayerInfo.h"

// FIXME -- need to pull communication out of bzfs.cxx...
extern void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer);
extern bool hasPerm(int playerIndex, PlayerAccessInfo::AccessPerm right);
extern PlayerInfo player[MaxPlayers];
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;
extern int NotConnected;


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
  if (BZDB->isEmpty("poll")) {
    sprintf(reply, "ERROR: the poll arbiter has disappeared (this should never happen)");
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  DEBUG2("BZDB poll value is not empty\n");

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB->getPointer("poll");

  DEBUG2("Arbiter was acquired with address 0x%x\n", (unsigned int)arbiter);

  /* make sure that there is not a poll active already */
  if (arbiter->knowsPoll()) {
    sprintf(reply,"A poll to %s %s is presently in progress", arbiter->getPollAction().c_str(), arbiter->getPollPlayer().c_str());
    sendMessage(ServerPlayer, t, reply, true);
    sprintf(reply,"Unable to start a new poll until the current one is over");
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  DEBUG2("The arbiter says there is not another poll active\n");

  // get available voter count
  unsigned short int available = 0;
  for (int i=0; i < curMaxPlayers; i++) {
    // anyone on the server (even observers) are eligible to vote
    if (player[i].fd != NotConnected) {
      available++;
    }
  }

  DEBUG2("There are %d available players for %d votes required\n", available, clOptions->votesRequired);
  
  /* make sure there are enough players to even make a poll that has a chance
   * of succeeding (not counting the person being acted upon)
   */
  if (available - 1 < clOptions->votesRequired) {
    sprintf(reply,"Unable to initiate a new poll.  There are not enough players.");
    sendMessage(ServerPlayer, t, reply, true);
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
	 (isWhitespace(arguments[startPosition]))) {
    startPosition++;
  }
  
  DEBUG2("Start position is %d\n", (int)startPosition);

  /* find the end of the command */
  size_t endPosition = startPosition + 1;
  while ((endPosition < arguments.size()) &&
	 (!isWhitespace(arguments[endPosition]))) {
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
	   (isWhitespace(arguments[startPosition]))) {
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
	   (isWhitespace(arguments[endPosition]))) {
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
      
    // set the number of available voters
    arbiter->setAvailableVoters(available);
      
    // keep track of who is allowed to vote
    for (int j=0; j < curMaxPlayers; j++) {
      // anyone on the server (even observers) are eligible to vote
      if (player[j].fd != NotConnected) {
	arbiter->grantSuffrage(player[j].callSign);
      }
    }
    
    // automatically place a vote for the player requesting the poll
    arbiter->voteYes(player[t].callSign);

  } else if (cmd == "vote") {
    
    if (!hasPerm(t, PlayerAccessInfo::vote)) {
      sprintf(reply,"%s, you do not presently have permission to vote (must /identify first)", player[t].callSign);
      sendMessage(ServerPlayer, t, reply, true);
      return;
    }
    
    /* !!! needs to be handled by the /vote command  */
    sprintf(reply,"%s, your vote has been recorded -- unimplemented", player[t].callSign);
    sendMessage(ServerPlayer, t, reply, true);
    
    
  } else if (cmd == "veto") {
    
    if (!hasPerm(t, PlayerAccessInfo::veto)) {
      sprintf(reply,"%s, you do not have permission to veto the poll", player[t].callSign);
      sendMessage(ServerPlayer, t, reply, true);
    }
    
    /* !!! needs to be handled by the /veto command  */
    sprintf(reply,"%s, you have aborted the poll -- unimplemented", player[t].callSign);
    sendMessage(ServerPlayer, t, reply, true);
    
    
  } else {
    
    sprintf(reply,"Invalid option to the poll command");
    sendMessage(ServerPlayer, t, reply, true);
    sprintf(reply,"Usage: /poll ban|kick playername");
    sendMessage(ServerPlayer, t, reply, true);
    sprintf(reply,"    or /poll vote yes|no");
    sendMessage(ServerPlayer, t, reply, true);
    sprintf(reply,"    or /poll veto");
    sendMessage(ServerPlayer, t, reply, true);
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
  if (BZDB->isEmpty("poll")) {
    sprintf(reply, "ERROR: the poll arbiter has disappeared (this should never happen)");
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }

  // only need to get this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB->getPointer("poll");

  /* make sure that there is a poll to vote upon */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sprintf(reply,"A poll is not presently in progress.  There is nothing to vote on");
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  std::string voteCmd = &message[5];
  std::string answer;

  /* find the start of the vote answer */  
  size_t startPosition = 0;
  while ((startPosition < voteCmd.size()) &&
	 (isWhitespace(voteCmd[startPosition]))) {
    startPosition++;
  }
  
  /* stash the answer ('yes', 'no', etc) in lowercase to simplify comparison */
  for (size_t i = startPosition;  i < voteCmd.size() && !isWhitespace(voteCmd[i]); i++) {
    answer += tolower(voteCmd[i]);
  }

  std::vector<std::string> yesAnswers;
  yesAnswers.push_back("y");
  yesAnswers.push_back("1");
  yesAnswers.push_back("yes");
  yesAnswers.push_back("yea");
  yesAnswers.push_back("si");
  yesAnswers.push_back("ja");
  yesAnswers.push_back("oui");
  yesAnswers.push_back("sim");

  std::vector<std::string> noAnswers;
  noAnswers.push_back("n");
  noAnswers.push_back("0");
  noAnswers.push_back("no");
  noAnswers.push_back("nay");
  noAnswers.push_back("nein");
  noAnswers.push_back("non");
  noAnswers.push_back("nao");

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
    sprintf(reply,"Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao");
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  if (!cast) {
    /* player was unable to cast their vote; probably already voted */
    sprintf(reply,"%s, you have already voted on the poll to %s %s", player[t].callSign, arbiter->getPollAction().c_str(), arbiter->getPollPlayer().c_str());
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }

  return;
}


void handleVetoCmd(int t, const char * /*message*/)
{
  char reply[MessageLen] = {0};

  if (!hasPerm(t, PlayerAccessInfo::veto)) {
    /* permission denied for /veto */
    sprintf(reply,"%s, you are presently not authorized to run /veto", player[t].callSign);
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  /* make sure that there is a poll arbiter */
  if (BZDB->isEmpty("poll")) {
    sprintf(reply, "ERROR: the poll arbiter has disappeared (this should never happen)");
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB->getPointer("poll");
  
  /* make sure there is an unexpired poll */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sprintf(reply, "%s, there is presently no active poll to veto", player[t].callSign);
    sendMessage(ServerPlayer, t, reply, true);
    return;
  }
  
  /* poof */
  arbiter->forgetPoll();
  
  sprintf(reply,"%s, you have cancelled the poll to %s %s", player[t].callSign, arbiter->getPollAction().c_str(), arbiter->getPollPlayer().c_str());
  sendMessage(ServerPlayer, t, reply, true);
  
  sprintf(reply,"The poll was cancelled by %s", player[t].callSign);
  sendMessage(ServerPlayer, AllPlayers, reply, true);
 
  return;
}



// ex: shiftwidth=2 tabstop=8
