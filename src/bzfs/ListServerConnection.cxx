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

// Provide BZFS with a list server connection

/* class header */
#include "ListServerConnection.h"

/* system implementation headers */
#include <string.h>
#include <string>
#include <math.h>
#include <errno.h>

/* common implementation headers */
#include "bzfio.h"
#include "version.h"
#include "TextUtils.h"
#include "Protocol.h"
#include "GameKeeper.h"

// FIXME remove externs!
extern Address serverAddress;
extern PingPacket getTeamCounts();
extern uint16_t curMaxPlayers;
extern int getTarget(const char *victimname);
extern void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message);

ListServerLink::ListServerLink(std::string listServerURL, std::string publicizedAddress, std::string publicizedTitle)
{
  // parse url
  std::string protocol, hostname, pathname;
  int port = 80;
  bool useDefault = false;

  // use default if it can't be parsed
  if (!BzfNetwork::parseURL(listServerURL, protocol, hostname, port, pathname))
    useDefault = true;

  // use default if wrong protocol or invalid port
  if ((protocol != "http") || (port < 1) || (port > 65535))
    useDefault = true;

  // use default if bad address
  Address address = Address::getHostAddress(hostname);
  if (address.isAny())
    useDefault = true;

  // parse default list server URL if we need to; assume default works
  if (useDefault) {
    BzfNetwork::parseURL(DefaultListServerURL, protocol, hostname, port, pathname);
    DEBUG1("Provided list server URL (%s) is invalid.  Using default of %s.\n", listServerURL.c_str(), DefaultListServerURL);
  }

  // add to list
  this->address	   = address;
  this->port	   = port;
  this->pathname   = pathname;
  this->hostname   = hostname;
  this->linkSocket = NotConnected;

  this->publicizeAddress     = publicizedAddress;
  this->publicizeDescription = publicizedTitle;
  this->publicizeServer	     = true;  //if this c'tor is called, it's safe to publicize

  // schedule initial ADD message
  queueMessage(ListServerLink::ADD);
}

ListServerLink::ListServerLink()
{
  // does not create a usable link, so checks should be placed
  // in  all public member functions to ensure that nothing tries
  // to happen if publicizeServer is false
  this->linkSocket = NotConnected;
  this->publicizeServer = false;
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
  TimeKeeper start = TimeKeeper::getCurrent();
  do {
    // compute timeout
    float waitTime = 3.0f - (TimeKeeper::getCurrent() - start);
    if (waitTime <= 0.0f)
      break;
    if (!isConnected()) //queueMessage should have connected us
      break;
    // check for list server socket connection
    int fdMax = -1;
    fd_set write_set;
    fd_set read_set;
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);
    if (phase == ListServerLink::CONNECTING)
      _FD_SET(linkSocket, &write_set);
    else
      _FD_SET(linkSocket, &read_set);
    fdMax = linkSocket;

    // wait for socket to connect or timeout
    struct timeval timeout;
    timeout.tv_sec = long(floorf(waitTime));
    timeout.tv_usec = long(1.0e+6f * (waitTime - floorf(waitTime)));
    int nfound = select(fdMax + 1, (fd_set*)&read_set, (fd_set*)&write_set,
			0, &timeout);
    if (nfound == 0)
      // Time has gone, close and go
      break;
    // check for connection to list server
    if (FD_ISSET(linkSocket, &write_set))
      sendQueuedMessages();
    else if (FD_ISSET(linkSocket, &read_set))
      read();
  } while (true);

  // stop list server communication
  closeLink();
}

void ListServerLink::closeLink()
{
  if (isConnected()) {
    close(linkSocket);
    DEBUG4("Closing List Server\n");
    linkSocket = NotConnected;
  }
}

void ListServerLink::read()
{
  if (isConnected()) {
    char    buf[2048];
    int bytes = recv(linkSocket, buf, sizeof(buf)-1, 0);
    // TODO don't close unless we've got it all
    closeLink();
    buf[bytes]=0;
    char* base = buf;
    static char *tokGoodIdentifier = "TOKGOOD: ";
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
	DEBUG3("Got: %s %d\n", base, getTarget(base));
        char *callsign, *group;
        callsign = (char *)(base + strlen(tokGoodIdentifier));
	group = callsign;
        while (*group && !isspace(*group)) group++;
        while (*group && isspace(*group)) *group++ = 0;
	if (getTarget(callsign) < curMaxPlayers) {
	  int playerIndex = getTarget(callsign);
	  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
	  if (!playerData->accessInfo.isRegistered())
	    playerData->accessInfo.storeInfo(NULL);
	  playerData->accessInfo.setPermissionRights();
	  // TODO walk list in *group and add user to the ones we care about
	  DEBUG3("Got: \"%s\" \"%s\" %d\n", callsign, group, getTarget(callsign));
	  sendMessage(ServerPlayer, playerIndex, "Global login approved!");
	}
      }
      // next reply
      base = scan;
    }
    if (nextMessageType != ListServerLink::NONE) {
      // There was a pending request arrived after we write:
      // we should redo all the stuff
      openLink();
    }
  }
}

void ListServerLink::openLink()
{
  // start opening connection if not already doing so
  if (!isConnected()) {
    linkSocket = socket(AF_INET, SOCK_STREAM, 0);
    DEBUG4("Opening List Server\n");
    if (!isConnected()) {
      return;
    }

    // set to non-blocking for connect
    if (BzfNetwork::setNonBlocking(linkSocket) < 0) {
      closeLink();
      return;
    }

    // Make our connection come from our serverAddress in case we have
    // multiple/masked IPs so the list server can verify us.
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr = serverAddress;

    // assign the address to the socket
    if (bind(linkSocket, (CNCTType*)&addr, sizeof(addr)) < 0) {
      closeLink();
      return;
    }

    // connect.  this should fail with EINPROGRESS but check for
    // success just in case.
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr   = address;
    if (connect(linkSocket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
      if (getErrno() != EINPROGRESS) {
	nerror("connecting to list server");
	// try to lookup dns name again in case it moved
	this->address = Address::getHostAddress(this->hostname);
	closeLink();
      } else {
	phase = CONNECTING;
      }
    } else {
      // shouldn't arrive here. Just in case, clean
      DEBUG3("list server connect and close?");
      closeLink();
    }
  }
}

void ListServerLink::queueMessage(MessageType type)
{
  // ignore if the server is not public
  if (!publicizeServer) return;

  // Open network connection only if closed
  if (!isConnected()) openLink();

  // record next message to send.
  nextMessageType = type;
}

void ListServerLink::sendQueuedMessages()
{
  if (!isConnected())
    return;

  if (nextMessageType == ListServerLink::ADD) {
    DEBUG3("Queuing ADD message to list server\n");
    addMe(getTeamCounts(), publicizeAddress, TextUtils::url_encode(publicizeDescription));
    lastAddTime = TimeKeeper::getCurrent();
  } else if (nextMessageType == ListServerLink::REMOVE) {
    DEBUG3("Queuing REMOVE message to list server\n");
    removeMe(publicizeAddress);
  }
}

void ListServerLink::addMe(PingPacket pingInfo,
			   std::string publicizedAddress,
			   std::string publicizedTitle)
{
  std::string msg;

  // encode ping reply as ascii hex digits plus NULL
  char gameInfo[PingPacketHexPackedSize + 1];
  pingInfo.packHex(gameInfo);

  // TODO we probably should convert to a POST. List server now allows either
  // send ADD message (must send blank line)
  msg = TextUtils::format("GET %s?action=ADD&nameport=%s&version=%s&gameinfo=%s&build=%s",
    pathname.c_str(), publicizedAddress.c_str(),
    getServerVersion(), gameInfo,
    getAppVersion());
  msg += "&checktokens=";
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(i);
    if (playerData && strlen(playerData->player.getCallSign()) && strlen(playerData->player.getToken())) {
      msg += TextUtils::format(playerData->player.getCallSign());
      msg += "=";
      msg += TextUtils::format(playerData->player.getToken());
      msg += "%0D%0A";
    }
  }
  // TODO loop through groups we are interested in and request them
  // *groups=GROUP0%0D%0AGROUP1%0D%0A
  // see handleGrouplistCmd()
  msg += "&groups=";
  PlayerAccessMap::iterator itr = groupAccess.begin();
  for ( ; itr != groupAccess.end(); itr++) {
    msg += itr->first.c_str();
    msg += "%0D%0A";
  }
  msg += TextUtils::format("&title=%s HTTP/1.1\r\n"
      "User-Agent: bzfs %s\r\n"
      "Host: %s\r\n"
      "Cache-Control: no-cache\r\n"
      "Connection: close\r\n"
      "\r\n",
    publicizedTitle.c_str(),
    getAppVersion(),
    hostname.c_str());
  sendLSMessage(msg);
}

void ListServerLink::removeMe(std::string publicizedAddress)
{
  std::string msg;
  // send REMOVE (must send blank line)
  msg = TextUtils::format("GET %s?action=REMOVE&nameport=%s HTTP/1.1\r\n"
    "User-Agent: bzfs %s\r\n"
    "Host: %s\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "\r\n",
    pathname.c_str(),
    publicizedAddress.c_str(),
    getAppVersion(),
    hostname.c_str());
  sendLSMessage(msg);
}

void ListServerLink::sendLSMessage(std::string message)
{
  const int bufsize = 4096;
  char msg[bufsize];
  strncpy(msg, message.c_str(), bufsize);
  msg[bufsize - 1] = 0;
  if (strlen(msg) > 0) {
    DEBUG3("%s\n", msg);
    if (send(linkSocket, msg, strlen(msg), 0) == -1) {
      perror("List server send failed");
      DEBUG3("Unable to send to the list server!\n");
      closeLink();
    } else {
      nextMessageType = ListServerLink::NONE;
      phase	   = ListServerLink::WRITTEN;
    }
  } else {
    closeLink();
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
