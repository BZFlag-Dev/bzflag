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
extern PlayerInfo *player;
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;
extern int NotConnected;


void handlePollCmd(int t, const char *message)
{
  char reply[MessageLen];

  static VotingArbiter *arbiter = (VotingArbiter *)BZDB->getPointer("poll");
    /* make sure player has permission to request a poll */
  if (!hasPerm(t, PlayerAccessInfo::poll)) {
      sprintf(reply,"%s, you are presently not authorized to run /poll", player[t].callSign);
      sendMessage(ServerPlayer, t, reply, true);
      return;
  }

    /* make sure that there is a poll arbiter */
  if (BZDB->isEmpty("poll")) {
      sprintf(reply, "ERROR: the poll arbiter has disappeared (this should never happen)");
      sendMessage(ServerPlayer, t, reply, true);
      return;
  }

    /* make sure that there is not a poll active already */
  if (arbiter->knowsPoll()) {
      sprintf(reply,"A poll to %s %s is presently in progress", arbiter->getPollAction().c_str(), arbiter->getPollPlayer().c_str());
      sendMessage(ServerPlayer, t, reply, true);
      sprintf(reply,"Unable to start a new poll until the current one is over");
      sendMessage(ServerPlayer, t, reply, true);
      return;
  }

    // get available voter count
  unsigned short int available = 0;
  for (int i=0; i < curMaxPlayers; i++) {
      // anyone on the server (even observers) are eligible to vote
      if (player[i].fd != NotConnected) {
	available++;
      }
  }

    // make sure there are enough players to even make a poll (don't count the pollee)
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

  std::string pollCmd = &message[5];
  std::string cmd;
  std::string nick;

  unsigned int endPos;
  unsigned int startPos = pollCmd.find_first_not_of(" \t");
  if (startPos != std::string::npos) {
      endPos = pollCmd.find_first_of(" \t", startPos);
      if (endPos == std::string::npos)
	endPos = pollCmd.length();

      cmd = pollCmd.substr(startPos,endPos-startPos);
      pollCmd = pollCmd.substr(endPos);
  }
  else {
      sprintf(reply,"Invalid poll syntax: /poll (kick|ban|vote|veto) playername");
      sendMessage(ServerPlayer, t, reply, true);
      return;
  }

  if ((cmd == "ban") || (cmd == "kick")) {

      startPos = pollCmd.find_first_not_of(" \t");
      if (startPos != std::string::npos) {
	std::string votePlayer = pollCmd.substr(startPos);
	if (votePlayer.length() == 0) {
	  sprintf(reply,"%s, no player was specified for the %s vote", player[t].callSign, cmd.c_str());
	  sendMessage(ServerPlayer, t, reply, true);
	  sprintf(reply,"Usage: /poll %s [playername]", cmd.c_str());
	  sendMessage(ServerPlayer, t, reply, true);
	  return;
	}

        /* make sure the requested player is actually here */
        bool foundPlayer=false;
        std::string playerIP = "";
        for (int v = 0; v < curMaxPlayers; v++) {
	  if (votePlayer == player[v].callSign) {
	    playerIP = player[v].peer.getDotNotation().c_str();
	    foundPlayer=true;
	    break;
	  }
	}

        if (!foundPlayer) {
	  /* wrong name? */
	  sprintf(reply, "The player specified for a %s vote is not here", cmd.c_str());
	  sendMessage(ServerPlayer, t, reply, true);
	  sprintf(reply,"Usage: /poll %s [playername]", cmd.c_str());
	  sendMessage(ServerPlayer, t, reply, true);
	  return;
	}

        /* create and announce the new poll */
        if (cmd == "ban") {
	  if (arbiter->pollToBan(votePlayer.c_str(), player[t].callSign, playerIP) == false) {
	    sprintf(reply,"You are not able to request a ban poll right now, %s", player[t].callSign);
	    sendMessage(ServerPlayer, t, reply, true);
	  } else {
	    sprintf(reply,"A poll to temporarily ban %s has been requested by %s", votePlayer.c_str(), player[t].callSign);
	    sendMessage(ServerPlayer, AllPlayers, reply, true);
	  }
	} else {
	  if (arbiter->pollToKick(votePlayer.c_str(), player[t].callSign) == false) {
	    sprintf(reply,"You are not able to request a kick poll right now, %s", player[t].callSign);
	    sendMessage(ServerPlayer, t, reply, true);
	  } else {
	    sprintf(reply,"A poll to %s %s has been requested by %s", cmd.c_str(), votePlayer.c_str(), player[t].callSign);
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
      }
      else {
        sprintf(reply,"Invalid poll syntax: /poll %s playername", cmd.c_str());
        sendMessage(ServerPlayer, t, reply, true);
	return;
      }
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
      sprintf(reply,"Usage: /poll ban|kick [playername]");
      sendMessage(ServerPlayer, t, reply, true);
    } /* end handling of poll subcommands */
}

void handleVoteCmd(int t, const char *message)
{
  char reply[MessageLen];

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

  VotingArbiter *arbiter = (VotingArbiter *)BZDB->getPointer("poll");

  /* make sure that there is a poll to vote upon */
  if (!arbiter->knowsPoll()) {
      sprintf(reply,"A poll is not presently in progress.  There is nothing to vote on");
      sendMessage(ServerPlayer, t, reply, true);
      return;
  }

  /* find the start of the vote answer */

  std::string answer;
  std::string voteCmd = &message[5];

  unsigned int startPos = voteCmd.find_first_not_of(" \t");
  if (startPos != std::string::npos) {
    unsigned int endPos = voteCmd.find_first_of(" \t", startPos);
    if (endPos == std::string::npos)
      endPos = voteCmd.length();
    answer = voteCmd.substr(startPos, endPos-startPos);
  }

  /* XXX answer arrays should be static const but it'll do for now */
  static const unsigned int yesCount = 8;
  char yesAnswers[8][5];
  sprintf(yesAnswers[0], "y");
  sprintf(yesAnswers[1], "1");
  sprintf(yesAnswers[2], "yes");
  sprintf(yesAnswers[3], "yea");
  sprintf(yesAnswers[4], "si");
  sprintf(yesAnswers[5], "ja");
  sprintf(yesAnswers[6], "oui");
  sprintf(yesAnswers[7], "sim");

  static const unsigned int noCount = 7;
  char noAnswers[7][5];
  sprintf(noAnswers[0], "n");
  sprintf(noAnswers[1], "0");
  sprintf(noAnswers[2], "no");
  sprintf(noAnswers[3], "nay");
  sprintf(noAnswers[4], "nein");
  sprintf(noAnswers[5], "non");
  sprintf(noAnswers[6], "nao");

  // see if the vote response is a valid yes or no answer
  int vote=-1;
  for (unsigned int v = 0; v < (noCount > yesCount ? noCount : yesCount); v++) {
      if (v < yesCount) {
	if (answer == yesAnswers[v]) {
	  vote = 1;
	  break;
	}
      }
      if (v < noCount) {
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
	sprintf(reply,"Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao");
	sendMessage(ServerPlayer, t, reply, true);
      } else {
	sprintf(reply,"%s, you did not vote in favor or in opposition", player[t].callSign);
	sendMessage(ServerPlayer, t, reply, true);
	sprintf(reply,"Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao");
	sendMessage(ServerPlayer, t, reply, true);
      }
      return;
  }

  if (!cast) {
      /* player was unable to cast their vote; probably already voted */
      sprintf(reply,"%s, you have already voted on the poll to %s %s", player[t].callSign, arbiter->getPollAction().c_str(), arbiter->getPollPlayer().c_str());
      sendMessage(ServerPlayer, t, reply, true);
      return;
  }
}

void handleVetoCmd(int t, const char * /*message*/)
{
    char reply[MessageLen];
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

    VotingArbiter *arbiter = (VotingArbiter *)BZDB->getPointer("poll");

    /* make sure there is an unexpired poll */
    if (!arbiter->knowsPoll()) {
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

}



// ex: shiftwidth=2 tabstop=8
