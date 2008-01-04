/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
#include "bzfsAPI.h"
#include "WorldEventManager.h"

/* local implementation headers */
#include "bzfs.h"

const int ListServerLink::NotConnected = -1;

extern bz_eTeamType convertTeam ( TeamColor team );
extern TeamColor convertTeam( bz_eTeamType team );

ListServerLink::ListServerLink(std::string listServerURL,
			       std::string publicizedAddress,
			       std::string publicizedTitle,
			       std::string _advertiseGroups)
{

  std::string bzfsUserAgent = "bzfs ";
  bzfsUserAgent	    += getAppVersion();

  setURL(listServerURL);
  setUserAgent(bzfsUserAgent);
  setDNSCachingTime(-1);
  setTimeout(10);

  publiclyDisconnected = false;

  if (clOptions->pingInterface != "")
    setInterface(clOptions->pingInterface);

  publicizeAddress     = publicizedAddress;
  publicizeDescription = publicizedTitle;
  advertiseGroups      = _advertiseGroups;

  //if this c'tor is called, it's safe to publicize
  publicizeServer      = true;
  queuedRequest	= false;
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
  publiclyDisconnected = !good;

  queuedRequest = false;
  if (good && (length < 2048)) {
    char buf[2048];
    memcpy(buf, data, length);
    int bytes = length;
    buf[bytes]=0;
    char* base = buf;
    const char *tokGoodIdentifier = "TOKGOOD: ";
    const char *tokBadIdentifier = "TOKBAD: ";
    const char *unknownPlayer = "UNK: ";
    const char *bzIdentifier = "BZID: ";
    // walks entire reply including HTTP headers
    while (*base) {
      // find next newline
      char* scan = base;
      while (*scan && *scan != '\r' && *scan != '\n') scan++;
      // if no newline then no more complete replies
      if (*scan != '\r' && *scan != '\n') break;
      while (*scan && (*scan == '\r' || *scan == '\n')) *scan++ = '\0';
      logDebugMessage(4,"Got line: \"%s\"\n", base);
      // TODO don't do this if we don't want central logins

      // is player globally registered ?
      bool  registered = false;
      // is player authenticated ?
      bool  verified   = false;
      // this is a reply to an authentication request ?
      bool  authReply  = false;

      char *callsign = (char *)NULL;
      if (strncmp(base, tokGoodIdentifier, strlen(tokGoodIdentifier)) == 0) {
	callsign = base + strlen(tokGoodIdentifier);
	registered = true;
	verified   = true;
	authReply  = true;
      } else if (!strncmp(base, tokBadIdentifier, strlen(tokBadIdentifier))) {
	callsign = base + strlen(tokBadIdentifier);
	registered = true;
	authReply  = true;
      } else if (!strncmp(base, unknownPlayer, strlen(unknownPlayer))) {
	callsign = base + strlen(unknownPlayer);
	authReply  = true;
      } else if (!strncmp(base, bzIdentifier, strlen(bzIdentifier))) {
	std::string line = base;
	std::vector<std::string> args = TextUtils::tokenize(line, " \t", 3, true);
	if (args.size() < 3) {
	  logDebugMessage(3,"Bad BZID string: %s\n", line.c_str());
	} else {
	  const std::string& bzId = args[1];
	  const std::string& nick = args[2];
	  logDebugMessage(4,"Got BZID: \"%s\" || \"%s\"\n", bzId.c_str(), nick.c_str());
	  for (int i = 0; i < curMaxPlayers; i++) {
	    GameKeeper::Player* gkp = GameKeeper::Player::getPlayerByIndex(i);
	    if ((gkp != NULL) &&
		(TextUtils::compare_nocase(gkp->player.getCallSign(), nick.c_str()) == 0)) {
	      gkp->setBzIdentifier(bzId);
	      logDebugMessage(3,"Set player (%s [%i]) bzId to (%s)\n",
		     nick.c_str(), i, bzId.c_str());
	      break;
	    }
	  }
	}
      }

      if (authReply) {
	logDebugMessage(3,"Got: %s", base);
	char *group = (char *)NULL;

	// Isolate callsign from groups
	if (verified) {
	  group = callsign;
	  if (group) {
	    while (*group && (*group != ':')) group++;
	    while (*group && (*group == ':')) *group++ = 0;
	  }
	}
	GameKeeper::Player *playerData = NULL;
	int playerIndex;
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
	logDebugMessage(3,"[%d]\n", playerIndex);

	if (playerIndex < curMaxPlayers) {
	  if (registered) {
	    // don't accept the global auth if there's a local account
	    // of the same name and the local account is not marked as
	    // being the same as the global account
	    if (!playerData->accessInfo.hasRealPassword()
		|| playerData->accessInfo.getUserInfo(callsign).hasGroup("LOCAL.GLOBAL")) {
	      if (!playerData->accessInfo.isRegistered())
		// Create an entry on the user database even if
		// authentication wenk ko. Make the "isRegistered"
		// things work
		playerData->accessInfo.storeInfo(NULL);
	      if (verified) {
		playerData->_LSAState = GameKeeper::Player::verified;
		playerData->accessInfo.setPermissionRights();
		while (group && *group) {
		  char *nextgroup = group;
		  if (nextgroup) {
		    while (*nextgroup && (*nextgroup != ':')) nextgroup++;
		    while (*nextgroup && (*nextgroup == ':')) *nextgroup++ = 0;
		  }
		  playerData->accessInfo.addGroup(group);
		  group = nextgroup;
		}
		playerData->authentication.global(true);
		sendMessage(ServerPlayer, playerIndex, "Global login approved!");
	      } else {
		playerData->_LSAState = GameKeeper::Player::failed;
		sendMessage(ServerPlayer, playerIndex, "Global login rejected, bad token.");
	      }
	    } else {
	      playerData->_LSAState = GameKeeper::Player::failed;
	      sendMessage(ServerPlayer, playerIndex, "Global login rejected. This callsign is registered locally on this server.");
	      sendMessage(ServerPlayer, playerIndex, "If the local account is yours, /identify, /deregister and reconnnect, or ask an admin for the LOCAL.GLOBAL group.");
	      sendMessage(ServerPlayer, playerIndex, "If it is not yours, please ask an admin to deregister it so that you may use your global callsign.");
	    }
	  } else {
	    playerData->_LSAState = GameKeeper::Player::notRequired;
	    if (!playerData->player.isBot()) {
	      sendMessage(ServerPlayer, playerIndex, "This callsign is not registered.");
	      sendMessage(ServerPlayer, playerIndex, "You can register it at http://my.bzflag.org/bb/");
	    }
	  }
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
    playerData->_LSAState = GameKeeper::Player::timedOut;
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
    logDebugMessage(3,"There is a message already queued to the list server: not sending this one yet.\n");
}

void ListServerLink::sendQueuedMessages()
{
  queuedRequest = true;
  if (nextMessageType == ListServerLink::ADD) {
    logDebugMessage(3,"Queuing ADD message to list server\n");

    bz_ListServerUpdateEvent_V1	updateEvent;
    updateEvent.address = publicizeAddress;
    updateEvent.description = publicizeDescription;
    updateEvent.groups = verifyGroupPermissions(advertiseGroups);

    worldEventManager.callEvents(bz_eListServerUpdateEvent,&updateEvent);

    addMe(getTeamCounts(), std::string(updateEvent.address.c_str()), std::string(updateEvent.description.c_str()), std::string(updateEvent.groups.c_str()));
    lastAddTime = TimeKeeper::getCurrent();
  } else if (nextMessageType == ListServerLink::REMOVE) {
    logDebugMessage(3,"Queuing REMOVE message to list server\n");
    removeMe(publicizeAddress);
  }
  nextMessageType = ListServerLink::NONE;
}

std::string ListServerLink::verifyGroupPermissions(const std::string& groups)
{
  // replay servers can have any crazy permissions they want
  if (Replay::enabled())
    return groups;

  // check to make sure these groups are actually good
  std::vector<std::string> vgroups = TextUtils::tokenize(groups, ",");
  std::vector<std::string>::iterator delitr = vgroups.end();
  // case-insensitive comparisons the cheap way
  for (std::vector<std::string>::iterator itr = vgroups.begin(); itr != vgroups.end(); ++itr) {
    (*itr) = TextUtils::toupper(*itr);
  }

  // if EVERYONE is included, delist it if EVERYONE can neither spawn nor talk
  delitr = std::find(vgroups.begin(), vgroups.end(), "EVERYONE");
  if (delitr != vgroups.end()) {
    if (!groupHasPermission("EVERYONE", PlayerAccessInfo::spawn) &&
	!groupHasPermission("EVERYONE", PlayerAccessInfo::talk))
      vgroups.erase(delitr);
  }

  // if nothing is left, add VERIFIED
  if (vgroups.size() < 1) {
    vgroups.push_back("VERIFIED");
  }

  // if VERIFIED is included, delist it if VERIFIED can neither spawn nor talk
  delitr = std::find(vgroups.begin(), vgroups.end(), "VERIFIED");
  if (delitr != vgroups.end()) {
    if (!groupHasPermission("VERIFIED", PlayerAccessInfo::spawn) &&
	!groupHasPermission("VERIFIED", PlayerAccessInfo::talk))
      vgroups.erase(delitr);
  }

  // if there's nothing left, add any non-local groups who CAN either spawn or talk
  if (vgroups.size() < 1) {
    for (PlayerAccessMap::iterator itr = groupAccess.begin(); itr != groupAccess.end(); ++itr) {
      if ((*itr).first.compare(0, 6, "LOCAL.") == 0)
	continue;
      if ((*itr).second.hasPerm(PlayerAccessInfo::spawn) ||
	  (*itr).second.hasPerm(PlayerAccessInfo::talk)) {
	vgroups.push_back((*itr).first);
      }
    }
  }

  // if there's still nothing left, this is one screwed up configuration, so just bail with an error
  if (vgroups.size() < 1) {
    std::cout << "No groups found who can spawn or talk." << std::endl << std::flush;
    exit(2);
  }

  // shove it all back into a string
  std::string retgroups = "";
  for (std::vector<std::string>::iterator itr = vgroups.begin(); itr != vgroups.end(); ++itr) {
    retgroups += (*itr) + ",";
  }
  retgroups.erase(retgroups.length() - 1, 1); // last comma

  return retgroups;
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
  msg += TextUtils::url_encode(_advertiseGroups);
  msg += "&title=";
  msg += TextUtils::url_encode(publicizedTitle);

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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
