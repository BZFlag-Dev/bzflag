/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#ifdef HAVE_CMATH
#  include <cmath>
#else
#  include <math.h>
#endif
#include <iostream>
#include <sstream>

#include "BZAdminClient.h"
#include "StateDatabase.h"
#include "TextUtils.h"
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
  
  // tell BZDB to shut up, we can't have debug data printed to stdout
  BZDB.setDebug(false);
  
  // set a default message mask
  showMessageType(MsgAddPlayer);
  showMessageType(MsgKilled);
  showMessageType(MsgMessage);
  showMessageType(MsgNewRabbit);
  showMessageType(MsgPause);
  showMessageType(MsgRemovePlayer);
  showMessageType(MsgSuperKill);
  
  // initialise the colormap
  colorMap[NoTeam] = Yellow;
  colorMap[RogueTeam] = Yellow;
  colorMap[RedTeam] = Red;
  colorMap[GreenTeam] = Green;
  colorMap[BlueTeam] = Blue;
  colorMap[PurpleTeam] = Purple;
  colorMap[ObserverTeam] = Cyan;
  
  // initialise the msg type map
  msgTypeMap["chat"] = MsgMessage;
  msgTypeMap["join"] = MsgAddPlayer;
  msgTypeMap["kill"] = MsgKilled;
  msgTypeMap["leave"] = MsgRemovePlayer;
  msgTypeMap["pause"] = MsgPause;
  msgTypeMap["ping"] = MsgLagPing;
  msgTypeMap["rabbit"] = MsgNewRabbit;
  msgTypeMap["spawn"] = MsgAlive;
}


PlayerId BZAdminClient::getMyId() {
  return sLink.getId();
}


BZAdminClient::ServerCode 
BZAdminClient::getServerString(std::string& str, ColorCode& colorCode) {
  uint16_t code, len;
  char inbuf[MaxPacketLen];
  std::string dstName, srcName;
  int i;
  PlayerIdMap::iterator iter;
  std::string returnString;

  /* read until we have a package that we want, or until there are no more
     packages for 100 ms */
  while (sLink.read(code, len, inbuf, 100) == 1) {

    colorCode = Default;
    void* vbuf = inbuf;
    PlayerId p;
    PlayerIdMap::const_iterator it;
    std::string victimName, killerName;
    returnString = ""; 
     
    switch (code) {

    case MsgNewRabbit:
      vbuf = nboUnpackUByte(vbuf, p);
      returnString = "*** '" + players[p].name + "' is now the rabbit.";
      break;
      
    case MsgPause:
      uint8_t paused;
      vbuf = nboUnpackUByte(vbuf, p);
      vbuf = nboUnpackUByte(vbuf, paused);
      returnString = "*** '" + players[p].name + "': " + 
	(paused ? "paused" : "resumed") + ".";
      break;
      
    case MsgAlive:
      vbuf = nboUnpackUByte(vbuf, p);
      returnString = "*** '" + players[p].name + "' has respawned.";
      break;

    case MsgLagPing:
      returnString = "*** Received lag ping from server.";
      break;
      
    case MsgSetVar:
      // code stolen from playing.cxx
      uint16_t numVars;
      uint8_t nameLen, valueLen;
      
      char name[MaxPacketLen];
      char value[MaxPacketLen];
      
      vbuf = nboUnpackUShort(vbuf, numVars);
      for (i = 0; i < numVars; i++) {
	vbuf = nboUnpackUByte(vbuf, nameLen);
	vbuf = nboUnpackString(vbuf, name, nameLen);
	name[nameLen] = '\0';
	
	vbuf = nboUnpackUByte(vbuf, valueLen);
	vbuf = nboUnpackString(vbuf, value, valueLen);
	value[valueLen] = '\0';
	
	BZDB.set(name, value);
	BZDB.setPersistent(name, false);
	BZDB.setPermission(name, StateDatabase::Locked);
      }
      return NoMessage;

    case MsgAddPlayer:
      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      char email[EmailLen];
      vbuf = nboUnpackUByte(vbuf, p);
      vbuf = nboUnpackUShort(vbuf, type);
      vbuf = nboUnpackUShort(vbuf, team);
      vbuf = nboUnpackUShort(vbuf, wins);
      vbuf = nboUnpackUShort(vbuf, losses);
      vbuf = nboUnpackUShort(vbuf, tks);
      vbuf = nboUnpackString(vbuf, callsign, CallSignLen);
      vbuf = nboUnpackString(vbuf, email, EmailLen);
      players[p].name.resize(0);
      players[p].name.append(callsign);
      players[p].team = TeamColor(team);
      players[p].wins = wins;
      players[p].losses = losses;
      players[p].tks = tks;
      if (ui != NULL)
	ui->addedPlayer(p);
      returnString = returnString + "*** '" + callsign + "' joined the game.";
      break;

    case MsgRemovePlayer:
      vbuf = nboUnpackUByte(vbuf, p);
      returnString = returnString + "*** '" + players[p].name + 
	"' left the game.";
      if (ui != NULL)
	ui->removingPlayer(p);
      players.erase(p);
      break;

    case MsgKilled:
      PlayerId victim, killer;
      int16_t shotId, reason;
      vbuf = nboUnpackUByte(vbuf, victim);
      vbuf = nboUnpackUByte(vbuf, killer);
      vbuf = nboUnpackShort(vbuf, reason);
      vbuf = nboUnpackShort(vbuf, shotId);

      // find the player names and build a kill message string
      it = players.find(victim);
      victimName = (it != players.end() ? it->second.name : "<unknown>");
      it = players.find(killer);
      killerName = (it != players.end() ? it->second.name : "<unknown>");
      returnString = "*** ";
      returnString = returnString + "'" + victimName + "' ";
      if (killer == victim)
	returnString += "blew myself up.";
      else
	returnString = returnString + "destroyed by '" + killerName + "'.";
      break;

    case MsgSuperKill:
      return Superkilled;

    case MsgScore: {
      uint8_t numScores;
      vbuf = nboUnpackUByte(vbuf, numScores);
      for (uint8_t i = 0; i < numScores; i++) {
	uint16_t wins, losses, tks;
	vbuf = nboUnpackUByte(vbuf, p);
	vbuf = nboUnpackUShort(vbuf, wins);
	vbuf = nboUnpackUShort(vbuf, losses);
	vbuf = nboUnpackUShort(vbuf, tks);
	if ((iter = players.find(p)) != players.end()) {
	  iter->second.wins = wins;
	  iter->second.losses = losses;
	  iter->second.tks = tks;
	}
      }
      return NoMessage;
    }

    case MsgMessage:

      // unpack the message header
      PlayerId src;
      PlayerId dst;
      PlayerId me = sLink.getId();
      vbuf = nboUnpackUByte(vbuf, src);
      vbuf = nboUnpackUByte(vbuf, dst);
      returnString = std::string((char*)vbuf);
      
      // parse playerlist
      std::istringstream iss(returnString);
      char c, d;
      int pt;
      if (iss >> c >> pt >> d) {
        PlayerIdMap::iterator iter;
        if (c == '[' && (iter = players.find(pt)) != players.end() && d == ']') {
          std::vector<std::string> tokens = string_util::tokenize(returnString, " ");
          if (!tokens.empty()) {
            if (*tokens.rbegin() == "udp" || *tokens.rbegin() == "udp+")
              tokens.pop_back();
            if (tokens.size() >= 2) {
              std::string callsign = *(++tokens.rbegin());
              if (*callsign.rbegin() == ':') {
                iter->second.ip = *tokens.rbegin();
              }
            }
          }
        }
      }
      
      // format the message depending on src and dst
      TeamColor dstTeam = (dst >= 244 && dst <= 250 ?
			   TeamColor(250 - dst) : NoTeam);
      if (returnString == "CLIENTQUERY") {
	sendMessage(std::string("bzadmin ") + getAppVersion(), src);
	if (ui != NULL)
	  ui->outputMessage("    [Sent versioninfo per request]", Default);
	return NoMessage;
      }
      else {
	returnString = formatMessage((char*)vbuf, src, dst, dstTeam, me);
	PlayerIdMap::const_iterator iter = players.find(src);
	colorCode = (iter == players.end() ? 
		     colorMap[NoTeam] : colorMap[iter->second.team]);
      }
      break;
    }
    
    if (messageMask[code]) {
      str = returnString;
      return GotMessage;
    }
  }
  
  if (sLink.getState() != ServerLink::Okay) {
    str = returnString;
    return CommError;
  }
  
  return NoMessage;
}


BZAdminClient::ServerCode 
BZAdminClient::getServerString(std::string& str) {
  ColorCode cc;
  return getServerString(str, cc);
}


PlayerIdMap& BZAdminClient::getPlayers() {
  return players;
}


bool BZAdminClient::isValid() const {
  return valid;
}


void BZAdminClient::runLoop() {
  std::string str;
  ColorCode color;
  ServerCode what(NoMessage);
  while (true) {
    while ((what = getServerString(str, color)) == GotMessage) {
      if (ui != NULL)
	ui->outputMessage(str, color);
    }
    if (what == Superkilled || what == CommError)
      break;
    if (ui != NULL && ui->checkCommand(str)) {
      if (str == "/quit")
	break;
      if (str != "")
	sendMessage(str, ui->getTarget());
    }
  }

  // why did we leave the loop?
  switch (what) {
  case Superkilled:
    if (ui != NULL)
      ui->outputMessage("--- ERROR: Server forced disconnect", Red);
    break;
  case CommError:
    if (ui != NULL)
      ui->outputMessage("--- ERROR: Connection to server lost", Red);
    break;
  default:
    waitForServer();
  }
}


void BZAdminClient::sendMessage(const std::string& msg,
				PlayerId target) {
  // local commands:
  // /set lists all BZDB variables
  if (msg == "/set") {
    if (ui != NULL)
      BZDB.iterate(listSetVars, this);
    return;
  }
  
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
			       (players.count(src) ? players[src].name : 
				"(UNKNOWN)"));
  const std::string dstName = (players.count(dst) ? players[dst].name : 
			       "(UNKNOWN)");

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
    if (dst == AdminPlayers)
      formatted += "[Admin] ";
    else if (dstTeam != NoTeam)
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
  bool tmp = messageMask[MsgMessage];
  messageMask[MsgMessage] = true;
  PlayerId me = sLink.getId();
  if (sLink.getState() == ServerLink::Okay) {
    sendMessage("bzadminping", me);
    std::string expected = formatMessage("bzadminping", me, me, NoTeam, me);
    std::string str;
    do {
      getServerString(str);
    } while (str != expected);
  }
  messageMask[MsgMessage] = tmp;
}


void BZAdminClient::ignoreMessageType(uint16_t type) {
  messageMask[type] = false;
}


void BZAdminClient::showMessageType(uint16_t type) {
  messageMask[type] = true;
}


void BZAdminClient::ignoreMessageType(std::string type) {
  ignoreMessageType(msgTypeMap[type]);
}


void BZAdminClient::showMessageType(std::string type) {
  showMessageType(msgTypeMap[type]);
}


void BZAdminClient::listSetVars(const std::string& name, void* thisObject) {
  char message[MessageLen];
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    sprintf(message, "/set %s %f", name.c_str(), BZDB.eval(name));
    ((BZAdminClient*)thisObject)->ui->outputMessage(message, Default);
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
