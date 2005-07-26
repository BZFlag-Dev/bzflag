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

// Provide BZFS with a list server connection

/* class header */
#include "ListServerConnection.h"

/* system implementation headers */
#include <string.h>
#include <string>
#include <math.h>
#include <errno.h>
#include <set>

/* common implementation headers */
#include "bzfio.h"
#include "version.h"
#include "TextUtils.h"
#include "Protocol.h"
#include "GameKeeper.h"
#include "bzfsAPI.h"
#include "WorldEventManager.h"

/* local implementation headers */
#include "CmdLineOptions.h"

// FIXME remove externs!
extern PingPacket getTeamCounts();
extern uint16_t curMaxPlayers;
extern void sendMessage(int playerIndex, PlayerId targetPlayer,
			const char *message);
extern void sendPlayerInfo(void);
extern void sendIPUpdate(int targetPlayer, int playerIndex);
extern CmdLineOptions *clOptions;

const int ListServerLink::NotConnected = -1;

ListServerLink::ListServerLink(std::string listServerURL,
			       std::string publicizedAddress,
			       std::string publicizedTitle,
             std::string _advertiseGroups)
{

  std::string bzfsUserAgent = "bzfs ";
  bzfsUserAgent            += getAppVersion();

  setURL(listServerURL);
  setUserAgent(bzfsUserAgent);
  setDNSCachingTime(-1);

  if (clOptions->pingInterface != "")
    setInterface(clOptions->pingInterface);

  publicizeAddress     = publicizedAddress;
  publicizeDescription = publicizedTitle;
  advertiseGroups      = _advertiseGroups;

  //if this c'tor is called, it's safe to publicize
  publicizeServer      = true;
  queuedRequest        = false;
  // schedule initial ADD message
  queueMessage(ListServerLink::ADD);
}

ListServerLink::ListServerLink()
{
  // does not create a usable link, so checks should be placed
  // in  all public member functions to ensure that nothing tries
  // to happen if publicizeServer is false
  publicizeServer = false;
}

ListServerLink::~ListServerLink()
{
  // now tell the list server that we're going away.  this can
  // take some time but we don't want to wait too long.  we do
  // our own multiplexing loop and wait for a maximum of 3 seconds
  // total.

  // if we aren't supposed to be publicizing, skip the whole thing
  // and don't waste 3 seconds.
  if (!publicizeServer)
    return;

  queueMessage(ListServerLink::REMOVE);
  for (int i = 0; i < 12; i++) {
    cURLManager::perform();
    if (!queuedRequest)
      break;
    TimeKeeper::sleep(0.25f);
  }
}

void ListServerLink::finalization(char *data, unsigned int length, bool good)
{
  queuedRequest = false;
  if (good && (length < 2048)) {
    char buf[2048];
    memcpy(buf, data, length);
    int bytes = length;
    buf[bytes]=0;
    char* base = buf;
    static char *tokGoodIdentifier = "TOKGOOD: ";
    static char *tokBadIdentifier = "TOKBAD: ";
    // walks entire reply including HTTP headers
    while (*base) {
      // find next newline
      char* scan = base;
      while (*scan && *scan != '\r' && *scan != '\n') scan++;
      // if no newline then no more complete replies
      if (*scan != '\r' && *scan != '\n') break;
      while (*scan && (*scan == '\r' || *scan == '\n')) *scan++ = '\0';
      DEBUG4("Got line: \"%s\"\n", base);
      // TODO don't do this if we don't want central logins
      if (strncmp(base, tokGoodIdentifier, strlen(tokGoodIdentifier)) == 0) {
	char *callsign, *group;
	callsign = base + strlen(tokGoodIdentifier);
	DEBUG3("Got: %s\n", base);
	group = callsign;
	while (*group && (*group != ':')) group++;
	while (*group && (*group == ':')) *group++ = 0;
	int playerIndex;
	GameKeeper::Player *playerData;
	for (playerIndex = 0; playerIndex < curMaxPlayers; playerIndex++) {
	  playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
	  if (!playerData)
	    continue;
	  if (playerData->_LSAState != GameKeeper::Player::checking)
	    continue;
	  if (!TextUtils::compare_nocase(playerData->player.getCallSign(),
					 callsign))
	    break;
	}
	if (playerIndex < curMaxPlayers) {
	  // don't accept the global auth if there's a local account
	  // of the same name and the local account is not marked as
	  // being the same as the global account
	  if (!playerData->accessInfo.hasRealPassword()
              || playerData->accessInfo.getUserInfo(callsign)
	      .hasGroup("LOCAL.GLOBAL")) {
	    playerData->_LSAState = GameKeeper::Player::verified;
	    if (!playerData->accessInfo.isRegistered())
	      playerData->accessInfo.storeInfo(NULL);
	    playerData->accessInfo.setPermissionRights();
	    while (*group) {
              char *nextgroup = group;
	      while (*nextgroup && (*nextgroup != ':')) nextgroup++;
	      while (*nextgroup && (*nextgroup == ':')) *nextgroup++ = 0;
	      playerData->accessInfo.addGroup(group);
	      group = nextgroup;
	    }
	    playerData->authentication.global(true);
	    sendMessage(ServerPlayer, playerIndex, "Global login approved!");
	    sendIPUpdate(playerIndex, -1);
	    sendPlayerInfo();
	  } else {
	    playerData->_LSAState = GameKeeper::Player::failed;
	    sendMessage(ServerPlayer, playerIndex, "Global login rejected. "
	                "This callsign is registered locally on this "
			"server.");
	    sendMessage(ServerPlayer, playerIndex,
			"If the local account is yours, "
			"/identify, /deregister and reconnnect, "
			"or ask an admin for the LOCAL.GLOBAL group.");
	    sendMessage(ServerPlayer, playerIndex,
			"If it is not yours, please ask an admin "
			"to deregister it so that you may use your global "
			"callsign.");
	  }
	  playerData->setNeedThisHostbanChecked(true);
	  playerData->player.clearToken();
	}
      } else if (!strncmp(base, tokBadIdentifier, strlen(tokBadIdentifier))) {
	char *callsign;
	callsign = base + strlen(tokBadIdentifier);
	int playerIndex;
	GameKeeper::Player *playerData;
	for (playerIndex = 0; playerIndex < curMaxPlayers; playerIndex++) {
	  playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
	  if (!playerData)
	    continue;
	  if (playerData->_LSAState != GameKeeper::Player::checking)
	    continue;
	  if (!TextUtils::compare_nocase(playerData->player.getCallSign(),
					 callsign))
	    break;
	}
	DEBUG3("Got: [%d] %s\n", playerIndex, base);
	if (playerIndex < curMaxPlayers) {
	  playerData->_LSAState = GameKeeper::Player::failed;
	  sendMessage(ServerPlayer, playerIndex,
		      "Global login rejected, bad token.");
	  playerData->setNeedThisHostbanChecked(true);
	  playerData->player.clearToken();
	}
      }

      // next reply
      base = scan;
    }
  }
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    if (playerData->_LSAState != GameKeeper::Player::checking)
      continue;
    playerData->_LSAState = GameKeeper::Player::timed;
  }
  if (nextMessageType != ListServerLink::NONE) {
    // There was a pending request arrived after we write:
    // we should redo all the stuff
    sendQueuedMessages();
  }
}

void ListServerLink::queueMessage(MessageType type)
{
  // ignore if the server is not public
  if (!publicizeServer) return;

  // record next message to send.
  nextMessageType = type;

  if (!queuedRequest)
    sendQueuedMessages();
  else
    DEBUG3("There is a message already queued to the list server: not sending this one yet.\n");
}

void ListServerLink::sendQueuedMessages()
{
  queuedRequest = true;
  if (nextMessageType == ListServerLink::ADD) {
    DEBUG3("Queuing ADD message to list server\n");

	bz_ListServerUpdateEvent	updateEvent;
	updateEvent.address = publicizeAddress;
	updateEvent.description = publicizeDescription;
	updateEvent.groups = advertiseGroups;

	worldEventManager.callEvents(bz_eListServerUpdateEvent,-1,&updateEvent);

	addMe(getTeamCounts(), std::string(updateEvent.address.c_str()), std::string(updateEvent.description.c_str()), std::string(updateEvent.groups.c_str()));
    lastAddTime = TimeKeeper::getCurrent();
  } else if (nextMessageType == ListServerLink::REMOVE) {
    DEBUG3("Queuing REMOVE message to list server\n");
    removeMe(publicizeAddress);
  }
  nextMessageType = ListServerLink::NONE;
}

void ListServerLink::addMe(PingPacket pingInfo,
			   std::string publicizedAddress,
			   std::string publicizedTitle,
         std::string _advertiseGroups)
{
  std::string msg;
  std::string hdr;

  // encode ping reply as ascii hex digits plus NULL
  char gameInfo[PingPacketHexPackedSize + 1];
  pingInfo.packHex(gameInfo);

  // send ADD message (must send blank line)
  msg  = "action=ADD&nameport=";
  msg += publicizedAddress;
  msg += "&version=";
  msg += getServerVersion();
  msg += "&gameinfo=";
  msg += gameInfo;
  msg += "&build=";
  msg += getAppVersion();
  msg += "&checktokens=";

  std::set<std::string> callSigns;
  // callsign1@ip1=token1%0D%0Acallsign2@ip2=token2%0D%0A
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (!playerData)
      continue;
    if ((playerData->_LSAState != GameKeeper::Player::required)
	&& (playerData->_LSAState != GameKeeper::Player::requesting))
      continue;
    if (callSigns.count(playerData->player.getCallSign()))
      continue;
    callSigns.insert(playerData->player.getCallSign());
    playerData->_LSAState = GameKeeper::Player::checking;
    NetHandler *handler = playerData->netHandler;
    msg += TextUtils::url_encode(playerData->player.getCallSign());
    Address addr = handler->getIPAddress();
    if (!addr.isPrivate()) {
	msg += "@";
	msg += handler->getTargetIP();
    }
    msg += "=";
    msg += playerData->player.getToken();
    msg += "%0D%0A";
  }

  msg += "&groups=";
  // *groups=GROUP0%0D%0AGROUP1%0D%0A
  PlayerAccessMap::iterator itr = groupAccess.begin();
  for ( ; itr != groupAccess.end(); itr++) {
    if (itr->first.substr(0, 6) != "LOCAL.") {
      msg += itr->first.c_str();
      msg += "%0D%0A";
    }
  }

  msg += "&advertgroups=";
  msg += _advertiseGroups;
  msg += "&title=";
  msg += publicizedTitle;

  setPostMode(msg);
  addHandle();
}

void ListServerLink::removeMe(std::string publicizedAddress)
{
  std::string msg;
  
  msg  = "action=REMOVE&nameport=";
  msg += publicizedAddress;

  setPostMode(msg);
  addHandle();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
