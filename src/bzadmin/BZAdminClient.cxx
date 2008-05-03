/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "BZAdminClient.h"

/* system implementation headers */
#ifdef HAVE_CMATH
#  include <cmath>
#else
#  include <math.h>
#endif
#include <iostream>
#include <sstream>

/* common implementation headers */
#include "BZAdminUI.h"
#include "StateDatabase.h"
#include "TextUtils.h"
#include "version.h"
#include "Team.h"
#include "ServerList.h"
#include "ErrorHandler.h"
#include "cURLManager.h"

StartupInfo startupInfo;

BZAdminClient::BZAdminClient(BZAdminUI* bzInterface)
  : myTeam(ObserverTeam), sLink(Address(startupInfo.serverName), startupInfo.serverPort), valid(false), ui(bzInterface) {

  if (sLink.getState() != ServerLink::Okay) {
    switch (sLink.getState()) {
      case ServerLink::BadVersion: {
	static char versionError[] = "Incompatible server version XXXXXXXX";
	strncpy(versionError + strlen(versionError) - 8,
	    sLink.getVersion(), 8);
	std::cout << versionError;
	break;
      }
      case ServerLink::Refused: {
	std::string banMessage = "Server Refused connection due to ban: ";
	banMessage += sLink.getRejectionMessage();
	std::cout << banMessage;
	break;
      }
      case ServerLink::Rejected:
	std::cout << "Game is full or over.  Try again later.";
	break;
      case ServerLink::SocketError:
	std::cout << "Error connecting to server.";
	break;
      case ServerLink::CrippledVersion:
	std::cout << "Cannot connect to full version server.";
	break;
      default:
	std::cout << "Internal error connecting to server.";
	break;
    }
    std::cout << std::endl;
    return;
  }
  if ((startupInfo.token[0] == '\0') && (startupInfo.password[0] != '\0')) {
    // won't really output anything, just gets token
    outputServerList();
  }
  sLink.sendEnter(sLink.getId(), ChatPlayer, myTeam, startupInfo.callsign, startupInfo.token);
  sLink.flush();
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

  // set a default message mask
  showMessageType(MsgAddPlayer);
  showMessageType(MsgAdminInfo);
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
  // FIXME MsgPlayerInfo
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
  int i = 0;
  PlayerIdMap::iterator iter;

  sLink.flush();
  // read until we have a package, or until we have waited 100 ms
  if (sLink.read(code, len, inbuf, 100) == 1) {
    lastMessage.first = "";
    lastMessage.second = Default;
    void* vbuf = inbuf;
    PlayerId p;
    PlayerIdMap::const_iterator it;
    std::string victimName, killerName;
    Address a;
    if ((messageMask.find(code) != messageMask.end()) &&
       (code == MsgSetVar || code == MsgRemovePlayer ||
	code == MsgAddPlayer || code == MsgAdminInfo ||
	code == MsgPlayerInfo || (*(messageMask.find(code))).second))
    switch (code) {

    case MsgNewRabbit:
	vbuf = nboUnpackUByte(vbuf, p);
	if (p != NoPlayer)
	  lastMessage.first = std::string("*** '") + players[p].name +
	    "' is now the rabbit.";
      break;

    case MsgPause:
	uint8_t paused;
	vbuf = nboUnpackUByte(vbuf, p);
	vbuf = nboUnpackUByte(vbuf, paused);
	lastMessage.first = std::string("*** '") + players[p].name + "': " +
	  (paused ? "paused" : "resumed") + ".";
      break;

    case MsgAlive:
	vbuf = nboUnpackUByte(vbuf, p);
	lastMessage.first = std::string("*** '") + players[p].name +
	  "' has respawned.";
      break;

    case MsgLagPing:
	lastMessage.first = "*** Received lag ping from server.";
      break;

    case MsgSetVar: {
      // code stolen from playing.cxx
      uint16_t numVars;
      std::string name;
      std::string value;

      vbuf = nboUnpackUShort(vbuf, numVars);
      for (i = 0; i < numVars; i++) {
	vbuf = nboUnpackStdString(vbuf, name);
	vbuf = nboUnpackStdString(vbuf, value);

	BZDB.set(name, value);
	BZDB.setPersistent(name, false);
	BZDB.setPermission(name, StateDatabase::Locked);
      }

      if (messageMask[MsgSetVar]) {
	lastMessage.first = std::string("*** Received BZDB update, ") +
	  TextUtils::format("%d", numVars) + " variable" +
	  (numVars == 1 ? "" : "s") + " updated.";
      }
      break;
    }

    case MsgAddPlayer:
      uint16_t team, type, wins, losses, tks;
      char callsign[CallSignLen];
      vbuf = nboUnpackUByte(vbuf, p);
      vbuf = nboUnpackUShort(vbuf, type);
      vbuf = nboUnpackUShort(vbuf, team);
      vbuf = nboUnpackUShort(vbuf, wins);
      vbuf = nboUnpackUShort(vbuf, losses);
      vbuf = nboUnpackUShort(vbuf, tks);
      vbuf = nboUnpackString(vbuf, callsign, CallSignLen);
      players[p].name = callsign;
      players[p].team = TeamColor(team);
      players[p].wins = wins;
      players[p].losses = losses;
      players[p].tks = tks;
      players[p].isRegistered = false;
      players[p].isVerified = false;
      players[p].isAdmin = false;
      if (ui != NULL)
	ui->addedPlayer(p);
      // If you are an admin, then MsgAdminInfo will output the message
      if (messageMask[MsgAddPlayer] && !players[getMyId()].isAdmin) {
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

    case MsgPlayerInfo:
      uint8_t numPlayers;
      vbuf = nboUnpackUByte(vbuf, numPlayers);
      for (i = 0; i < numPlayers; ++i) {
	vbuf = nboUnpackUByte(vbuf, p);
	uint8_t info;
	// parse player info bitfield
	vbuf = nboUnpackUByte(vbuf, info);
	players[p].isAdmin = ((info & IsAdmin) != 0);
	players[p].isRegistered = ((info & IsRegistered) != 0);
	players[p].isVerified = ((info & IsVerified) != 0);
      }
      break;

    case MsgAdminInfo:
      uint8_t numIPs;
      uint8_t tmp;
      vbuf = nboUnpackUByte(vbuf, numIPs);
      if(numIPs > 1){
	for (i = 0; i < numIPs; ++i) {
	  vbuf = nboUnpackUByte(vbuf, tmp);
	  vbuf = nboUnpackUByte(vbuf, p);
	  vbuf = a.unpack(vbuf);
	  players[p].ip = a.getDotNotation();
	  if ((ui != NULL) && messageMask[MsgAdminInfo]){
	    ui->outputMessage("*** IPINFO: " + players[p].name + " from "  +
	      players[p].ip, Default);
	  }
	}
      }
      //Alternative to the MsgAddPlayer message
      else if(numIPs == 1){
	vbuf = nboUnpackUByte(vbuf, tmp);
	vbuf = nboUnpackUByte(vbuf, p);
	vbuf = a.unpack(vbuf);
	players[p].ip = a.getDotNotation();
	Team temp;
	if (messageMask[MsgAdminInfo]){
	  std::string joinMsg = std::string("*** \'") + players[p].name + "\' joined the game as " +
	    temp.getName(players[p].team) + " from " + players[p].ip + ".";
	  lastMessage.first = joinMsg;
	}
      }
      break;

    case MsgScoreOver:
	PlayerId id;
	uint16_t _team;
	vbuf = nboUnpackUByte(vbuf, id);
	vbuf = nboUnpackUShort(vbuf, _team);
	it = players.find(id);
	victimName = (it != players.end() ? it->second.name : "<unknown>");
	if (_team != (uint16_t)NoTeam) {
	  Team temp;
	  victimName = temp.getName((TeamColor)_team);
	}
	lastMessage.first = std::string("*** \'") + victimName + "\' won the game.";
      break;

    case MsgTimeUpdate:
	uint32_t timeLeft;
	vbuf = nboUnpackUInt(vbuf, timeLeft);
	if (timeLeft == 0)
	  lastMessage.first = "*** Time Expired.";
	else if (timeLeft == ~0u)
	  lastMessage.first = "*** Paused.";
	else
	    lastMessage.first = std::string("*** ") +
	      TextUtils::format("%u", timeLeft) + " seconds remaining.";
      break;

    case MsgKilled:
	PlayerId victim, killer;
	FlagType* flagType;
	int16_t shotId, reason;
	int phydrv;
	vbuf = nboUnpackUByte(vbuf, victim);
	vbuf = nboUnpackUByte(vbuf, killer);
	vbuf = nboUnpackShort(vbuf, reason);
	vbuf = nboUnpackShort(vbuf, shotId);
	vbuf = FlagType::unpack(vbuf, flagType);
	if (reason == PhysicsDriverDeath) {
	  int32_t inPhyDrv;
	  vbuf = nboUnpackInt(vbuf, inPhyDrv);
	  phydrv = int(inPhyDrv);
	}

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
      break;

    case MsgSuperKill:
      return Superkilled;

    case MsgScore:
      uint8_t numScores;
      vbuf = nboUnpackUByte(vbuf, numScores);
      for (i = 0; i < numScores; i++) {
	uint16_t winners, loosers, teamkillers;
	vbuf = nboUnpackUByte(vbuf, p);
	vbuf = nboUnpackUShort(vbuf, winners);
	vbuf = nboUnpackUShort(vbuf, loosers);
	vbuf = nboUnpackUShort(vbuf, teamkillers);
	if ((iter = players.find(p)) != players.end()) {
	  iter->second.wins   = winners;
	  iter->second.losses = loosers;
	  iter->second.tks    = teamkillers;
	}
      }
      if (messageMask[MsgScore]) {
	lastMessage.first =
	  std::string("*** Received score update, score for ")+
	  TextUtils::format("%d", numScores) + " player" +
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
	lastMessage.first = formatMessage((char*)vbuf, src, dst,dstTeam, me);
	PlayerIdMap::const_iterator iterator = players.find(src);
	lastMessage.second = (iterator == players.end() ?
			      colorMap[NoTeam] :
			      colorMap[iterator->second.team]);
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

void BZAdminClient::outputServerList() const {
  if (ui)
    ui->outputMessage(std::string("Server List:"), Yellow);
  ServerList serverList;

  serverList.startServerPings(&startupInfo);

  // wait no more than 20 seconds for the list server
  for (int i = 0; i < 20; i++) {
    if (!serverList.searchActive() && serverList.serverFound()) {
      break;
    }
    if (ui) {
      if (!serverList.serverFound()) {
	ui->outputMessage(std::string("...waiting on the list server..."), Yellow);
      } else {
	ui->outputMessage(TextUtils::format("...retrieving list of servers... (found %d)", (int)serverList.size()), Yellow);
      }
    }
    serverList.checkEchos(&startupInfo);
    cURLManager::perform();
    TimeKeeper::sleep(1.0);
  }
  // what is your final answer?
  serverList.checkEchos(&startupInfo);

  if (ui) {
    std::vector<ServerItem> servers = serverList.getServers();
    for (std::vector<ServerItem>::const_iterator server = servers.begin();
	 server != servers.end();
	 server++) {
      ui->outputMessage(std::string("  ") + server->description, Yellow);
    }
    ui->outputMessage(std::string("End Server List."), Yellow);
  }

  return;
}

void BZAdminClient::runLoop() {

  sLink.sendVarRequest();
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
      else if (cmd == "/list") {
	outputServerList();
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
  memset(buffer, 0, MessageLen);
  strncpy(buffer, msg.c_str(), MessageLen - 1);
  sLink.sendMessage(target, buffer);
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

  // display action messages differently
  bool isAction = false;
  std::string message;
  if ((msg[0] == '*') && (msg[1] == ' ') &&
      (msg[msg.size () - 1] == '*') && (msg[msg.size () - 2] == '\t')) {
    isAction = true;
    message = msg.substr(2, msg.size() - 4);
  } else {
    message = msg;
  }

  // direct message to or from me
  if (dst == me || players.count(dst)) {
    if (!(src == me && dst == me)) {
      if (src == me) {
	if (isAction) {
	  formatted += "[->" + message + "]";
	} else {
	  formatted += "[->" + dstName + "] " + message;
	}
      } else {
	if (isAction) {
	  formatted += "[" + message + "->]";
	} else {
	  formatted += "[" + srcName + "->] " + message;
	}
      }
    } else {
      formatted += message;
    }
  }

  // public or admin or team message
  else {
    if (dst == AdminPlayers)
      formatted += "[Admin] ";
    else if (dstTeam != NoTeam)
      formatted += "[Team] ";

    if (!isAction) {
      formatted += srcName;
      formatted += ": ";
    }
    formatted += message;
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
    std::string noTalk = formatMessage("We're sorry, you are not allowed to talk!", ServerPlayer, me, NoTeam, me);
    std::string str;
    BZAdminUI* tmpUI = ui;
    ui = NULL;
    do {
      checkMessage();
    } while (lastMessage.first != expected && lastMessage.first != noTalk);
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
    snprintf(message, MessageLen, "/set %s %f", name.c_str(), BZDB.eval(name));
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
