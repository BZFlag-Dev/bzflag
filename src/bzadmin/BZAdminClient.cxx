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
#include "BZAdminUI.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "version.h"
#include "Team.h"


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
    std::cerr << "Rejected." << std::endl;
    return;
  }
  
  std::string reason;
  uint16_t code, rejcode;
  if (sLink.readEnter (reason, code, rejcode)) {
    valid = true;
  } else {
    std::cerr << reason << std::endl;
  }
  
  
  // tell BZDB to shut up, we can't have debug data printed to stdout
  BZDB.setDebug(false);
  
  //send our version string for /clientquery
  sLink.sendVersionString();

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
  msgTypeMap["bzdb"] = MsgSetVar;
  msgTypeMap["chat"] = MsgMessage;
  msgTypeMap["admin"] = MsgAdminInfo;
  msgTypeMap["join"] = MsgAddPlayer;
  msgTypeMap["kill"] = MsgKilled;
  msgTypeMap["leave"] = MsgRemovePlayer;
  msgTypeMap["pause"] = MsgPause;
  msgTypeMap["ping"] = MsgLagPing;
  msgTypeMap["rabbit"] = MsgNewRabbit;
  msgTypeMap["score"] = MsgScore;
  msgTypeMap["spawn"] = MsgAlive;
  msgTypeMap["time"] = MsgTimeUpdate;
  msgTypeMap["over"] = MsgScoreOver;
}


PlayerId BZAdminClient::getMyId() {
  return sLink.getId();
}


BZAdminClient::ServerCode BZAdminClient::checkMessage() {
  uint16_t code, len;
  char inbuf[MaxPacketLen];
  std::string dstName, srcName;
  int i;
  PlayerIdMap::iterator iter;

  // read until we have a package, or until we have waited 100 ms
  if (sLink.read(code, len, inbuf, 100) == 1) {
    lastMessage.first = "";
    lastMessage.second = Default;
    void* vbuf = inbuf;
    PlayerId p;
    PlayerIdMap::const_iterator it;
    std::string victimName, killerName;
    Address a;
    
    switch (code) {

    case MsgNewRabbit:
      if (messageMask[MsgNewRabbit]) {
	vbuf = nboUnpackUByte(vbuf, p);
	lastMessage.first = std::string("*** '") + players[p].name + 
	  "' is now the rabbit.";
      }
      break;
      
    case MsgPause:
      if (messageMask[MsgPause]) {
	uint8_t paused;
	vbuf = nboUnpackUByte(vbuf, p);
	vbuf = nboUnpackUByte(vbuf, paused);
	lastMessage.first = std::string("*** '") + players[p].name + "': " + 
	  (paused ? "paused" : "resumed") + ".";
      }
      break;
      
    case MsgAlive:
      if (messageMask[MsgAlive]) {
	vbuf = nboUnpackUByte(vbuf, p);
	lastMessage.first = std::string("*** '") + players[p].name + 
	  "' has respawned.";
      }
      break;

    case MsgLagPing:
      if (messageMask[MsgLagPing])
	lastMessage.first = "*** Received lag ping from server.";
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
      if (messageMask[MsgSetVar]) {
	lastMessage.first = std::string("*** Received BZDB update, ") +
	  string_util::format("%d", numVars) + " variable" +
	  (numVars == 1 ? "" : "s") + " updated.";
      }
      break;

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
      if (messageMask[MsgAddPlayer]) {
      Team temp;
  std::string joinMsg = std::string("*** \'") + callsign + "\' joined the game as " +
		temp.getName(players[p].team) + ".";
	lastMessage.first = joinMsg;
      }
      break;
      
    case MsgRemovePlayer:
      vbuf = nboUnpackUByte(vbuf, p);
      if (ui != NULL)
	ui->removingPlayer(p);
      if (messageMask[MsgRemovePlayer]) {
	lastMessage.first = std::string("*** '") + players[p].name + 
	  "' left the game.";
      }
      players.erase(p);
      break;

    case MsgAdminInfo:
      uint8_t numIPs;
      uint8_t tmp;
      vbuf = nboUnpackUByte(vbuf, numIPs);
      for (int i = 0; i < numIPs; ++i) {
	vbuf = nboUnpackUByte(vbuf, tmp);
	vbuf = nboUnpackUByte(vbuf, p);
	// FIXME - actually parse the bitfield
	vbuf = nboUnpackUByte(vbuf, tmp);
	vbuf = a.unpack(vbuf);
	players[p].ip = a.getDotNotation();
      }
      if (messageMask[MsgAdminInfo]) {
	lastMessage.first = std::string("*** IP update received, ") + 
	  string_util::format("%d", numIPs) + " IP" +(numIPs == 1 ? "" : "s") +
	  " updated.";
      }
      break;
      
    case MsgScoreOver:
      if (messageMask[MsgScoreOver]) {
 	PlayerId id;
	uint16_t team;
	vbuf = nboUnpackUByte(vbuf, id);
	vbuf = nboUnpackUShort(vbuf, team);
	it = players.find(id);
	victimName = (it != players.end() ? it->second.name : "<unknown>");
	if (team == (uint16_t)NoTeam) {
	  Team temp;
	  victimName = temp.getName((TeamColor)team);
	}
	lastMessage.first = std::string("*** ") + victimName + " won the game.";
      }
      break;
      
    case MsgTimeUpdate:
      if (messageMask[MsgTimeUpdate]) {
	uint16_t timeLeft;
	vbuf = nboUnpackUShort(vbuf, timeLeft);
	if (timeLeft == 0)
	  lastMessage.first = "*** Time Expired.";
	else
	  lastMessage.first = std::string("*** ") + 
	    string_util::format("%d", timeLeft) + " seconds remaining.";
      }
      break;
      
    case MsgKilled:
      if (messageMask[MsgKilled]) {
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
	lastMessage.first = std::string("*** ") + "'" + victimName + "' ";
	if (killer == victim) {
	  lastMessage.first = lastMessage.first + "blew myself up.";
	}
	else {
	  lastMessage.first = lastMessage.first + "destroyed by '" + 
	    killerName + "'.";
	}
      }
      break;

    case MsgSuperKill:
      return Superkilled;

    case MsgScore:
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
      if (messageMask[MsgScore]) {
	lastMessage.first = 
	  std::string("*** Received score update, score for ")+
	  string_util::format("%d", numScores) + " player" + 
	  (numScores == 1 ? "s" : "") + " updated.";
      }
      break;
      
    case MsgMessage:

      // unpack the message header
      PlayerId src;
      PlayerId dst;
      PlayerId me = sLink.getId();
      vbuf = nboUnpackUByte(vbuf, src);
      vbuf = nboUnpackUByte(vbuf, dst);
      
      // format the message depending on src and dst
      TeamColor dstTeam = (dst >= 244 && dst <= 250 ?
			   TeamColor(250 - dst) : NoTeam);
      if (messageMask[MsgMessage]) {
	lastMessage.first = formatMessage((char*)vbuf, src, dst,dstTeam, me);
	PlayerIdMap::const_iterator iter = players.find(src);
	lastMessage.second = (iter == players.end() ? 
			      colorMap[NoTeam] : 
			      colorMap[iter->second.team]);
      }
      break;
    }
    if (ui != NULL)
      ui->handleNewPacket(code);
    return GotMessage;
  }
  
  if (sLink.getState() != ServerLink::Okay) {
    if (ui != NULL)
      ui->outputMessage("--- ERROR: Communication error", Red);
    return CommError;
  }
  
  return NoMessage;
}


std::pair<std::string, ColorCode> BZAdminClient::getLastMessage() const {
  return lastMessage;
}


PlayerIdMap& BZAdminClient::getPlayers() {
  return players;
}


bool BZAdminClient::isValid() const {
  return valid;
}


void BZAdminClient::runLoop() {
  std::string cmd;
  std::map<std::string, uint16_t>::iterator iter;
  ServerCode what(NoMessage);
  while (true) {
    what = checkMessage();
    if (what == Superkilled || what == CommError)
      break;
    if (ui != NULL && ui->checkCommand(cmd)) {
      if (cmd == "/quit")
	break;
      else if (cmd.substr(0, 6) == "/show ") {
	if ((iter = msgTypeMap.find(cmd.substr(6))) == msgTypeMap.end()) {
	  ui->outputMessage(std::string("--- ERROR: ") + cmd.substr(6) + 
			    " is an unknown message type", Red);
	}
	else {
	  showMessageType(cmd.substr(6));
	  ui->outputMessage(std::string("--- Will now show messages of the ")
			    + "type " + cmd.substr(6), Yellow);
	}
      }
      else if (cmd.substr(0, 6) == "/hide ") {
	if ((iter = msgTypeMap.find(cmd.substr(6))) == msgTypeMap.end()) {
	  ui->outputMessage(std::string("--- ERROR: ") + cmd.substr(6) + 
			    " is an unknown message type", Red);
	}
	else {
	  ignoreMessageType(cmd.substr(6));
	  ui->outputMessage(std::string("--- Will now hide messages of the ")
			    + "type " + cmd.substr(6), Yellow);
	}
      }
      else if (cmd != "")
	sendMessage(cmd, ui->getTarget());
    }
  }

  // why did we leave the loop?
  switch (what) {
  case Superkilled:
    lastMessage.first = "--- ERROR: Server forced disconnect";
    lastMessage.second = Red;
    break;
  case CommError:
    lastMessage.first = "--- ERROR: Connection to server lost";
    lastMessage.second = Red;
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

  // public or admin or team message
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
    BZAdminUI* tmpUI = ui;
    ui = NULL;
    do {
      checkMessage();
    } while (lastMessage.first != expected);
    ui = tmpUI;
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


const std::map<std::string, uint16_t>& BZAdminClient::getMessageTypeMap() const
{
  return msgTypeMap;
}
  
bool BZAdminClient::getFilterStatus(uint16_t msgType) const {
  std::map<uint16_t, bool>::const_iterator iter = messageMask.find(msgType);
  if (iter == messageMask.end())
    return false;
  else 
    return iter->second;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
