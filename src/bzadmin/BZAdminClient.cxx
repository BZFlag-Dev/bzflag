/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _WIN32
#pragma warning( 4: 4786)
#endif

#include <iostream>

#include "BZAdminClient.h"
#include "version.h"


BZAdminClient::BZAdminClient(std::string callsign, std::string host,
			     int port, BZAdminUI* bzInterface)
  : myTeam(ObserverTeam), sLink(Address(host), port), valid(false),
    ui(bzInterface) {

  if (sLink.getState() != ServerLink::Okay) {
    std::cerr<<"Could not connect to "<<host<<':'<<port<<'.'<<std::endl;
    return;
  }
  sLink.sendEnter(TankPlayer, myTeam, callsign.c_str(), "");
  if (sLink.getState() != ServerLink::Okay) {
    std::cerr<<"Rejected."<<std::endl;
    return;
  }
  valid = true;
}


PlayerId BZAdminClient::getMyId() {
  return sLink.getId();
}


BZAdminClient::ServerCode BZAdminClient::getServerString(std::string& str) {
  uint16_t code, len;
  char inbuf[MaxPacketLen];
  int e;
  std::string dstName, srcName;
  str = "";

  /* read until we have a package that we want, or until there are no more
     packages for 100 ms */
  while ((e = sLink.read(code, len, inbuf, 100)) == 1) {
    void* vbuf = inbuf;
    PlayerId p;
    switch (code) {

    case MsgAddPlayer:

      vbuf = nboUnpackUByte(vbuf, p);

      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      char email[EmailLen];
      vbuf = nboUnpackUShort(vbuf, type);
      vbuf = nboUnpackUShort(vbuf, team);
      vbuf = nboUnpackUShort(vbuf, wins);
      vbuf = nboUnpackUShort(vbuf, losses);
      vbuf = nboUnpackUShort(vbuf, tks);
      vbuf = nboUnpackString(vbuf, callsign, CallSignLen);
      vbuf = nboUnpackString(vbuf, email, EmailLen);
      players[p].resize(0);
      players[p].append(callsign);
      if (ui != NULL)
	ui->addedPlayer(p);
      str = str + "*** '" + callsign + "' joined the game.";
      return GotMessage;

    case MsgRemovePlayer:
      vbuf = nboUnpackUByte(vbuf, p);
      str = str + "*** '" + players[p] + "' left the game.";
      if (ui != NULL)
	ui->removingPlayer(p);
      players.erase(p);
      return GotMessage;

    case MsgSuperKill:
      return Superkilled;

    case MsgMessage:

      // unpack the message header
      PlayerId src;
      PlayerId dst;
      PlayerId me = sLink.getId();
      vbuf = nboUnpackUByte(vbuf, src);
      vbuf = nboUnpackUByte(vbuf, dst);

      // is the message for me?
      TeamColor dstTeam = (dst >= 244 && dst <= 250 ?
			   TeamColor(250 - dst) : NoTeam);
      if (dst == AllPlayers || src == me || dst == me || dstTeam == myTeam) {
	str = (char*)vbuf;
	if (str == "CLIENTQUERY") {
	  sendMessage(std::string("bzadmin ") + getAppVersion(), src);
	  if (ui != NULL)
	    ui->outputMessage("    [Sent versioninfo per request]");
	}
	else {
	  str = formatMessage((char*)vbuf, src, dst, dstTeam, me);
	  return GotMessage;
	}
      }
    }
  }

  if (sLink.getState() != ServerLink::Okay) {
    return CommError;
  }

  return NoMessage;
}


std::map<PlayerId, std::string>& BZAdminClient::getPlayers() {
  return players;
}


bool BZAdminClient::isValid() const {
  return valid;
}


void BZAdminClient::runLoop() {
  std::string str;
  ServerCode what(NoMessage);
  while (true) {
    while ((what = getServerString(str)) == GotMessage) {
      if (ui != NULL)
	ui->outputMessage(str);
    }
    if (what == Superkilled || what == CommError)
      break;
    if (ui != NULL && ui->checkCommand(str)) {
      if (str == "/quit")
	break;
      sendMessage(str, ui->getTarget());
    }
  }

  // why did we leave the loop?
  switch (what) {
  case Superkilled:
    if (ui != NULL)
      ui->outputMessage("--- ERROR: Server forced disconnect");
    break;
  case CommError:
    if (ui != NULL)
      ui->outputMessage("--- ERROR: Connection to server lost");
    break;
  default:
    waitForServer();
  }
}


void BZAdminClient::sendMessage(const std::string& msg,
				PlayerId target) {
  char buffer[MessageLen];
  char buffer2[1 + MessageLen];
  void* buf = buffer2;

  buf = nboPackUByte(buf, target);
  memset(buffer, 0, MessageLen);
  strncpy(buffer, msg.c_str(), MessageLen - 1);
  nboPackString(buffer2 + 1, buffer, MessageLen);
  sLink.send(MsgMessage, sizeof(buffer2), buffer2);
}


std::string BZAdminClient::formatMessage(const std::string& msg, PlayerId src,
				    PlayerId dst, TeamColor dstTeam,
				    PlayerId me) {
  std::string formatted = "    ";

  // get sender and receiver
  const std::string srcName = (src == ServerPlayer ? "SERVER" :
			  (players.count(src) ? players[src] : "(UNKNOWN)"));
  const std::string dstName = (players.count(dst) ? players[dst] : "(UNKNOWN)");

  // direct message to or from me
  if (dst == me || players.count(dst)) {
    if (!(src == me && dst == me)) {
      formatted += "[";
      if (src == me) {
	formatted += "->";
	formatted += dstName;
      }
      else {
	formatted += srcName;
	formatted += "->";
      }
      formatted += "] ";
    }
    formatted += msg;
  }

  // public or team message
  else {
    if (dstTeam != NoTeam)
      formatted += "[Team] ";
    formatted += srcName;
    formatted += ": ";
    formatted += msg;
  }

  return formatted;
}


void BZAdminClient::setUI(BZAdminUI* bzInterface) {
  ui = bzInterface;
}


void BZAdminClient::waitForServer() {
  // we need to know that the server has processed all our messages
  // send a private message to ourself and wait for it to come back
  // this assumes that the order of messages isn't changed along the way
  PlayerId me = sLink.getId();
  if (sLink.getState() == ServerLink::Okay) {
    sendMessage("bzadminping", me);
    std::string expected = formatMessage("bzadminping", me, me, NoTeam, me);
    std::string str;
    do {
      getServerString(str);
    } while (str != expected);
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
