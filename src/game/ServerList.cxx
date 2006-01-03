/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "ServerList.h"

/* system headers */
#include <iostream>
#include <vector>
#include <string>
#if !defined(_WIN32)
#include <errno.h>
#endif
#include <ctype.h>

/* common implementation headers */
#include "version.h"
#include "bzsignal.h"
#include "Ping.h"
#include "Protocol.h"
#include "TimeKeeper.h"
#include "TextUtils.h"
#include "ErrorHandler.h"

/* local implementation headers */
#include "ServerListCache.h"
#include "StartupInfo.h"


ServerList::ServerList() :
	phase(-1),
	serverCache(ServerListCache::get()),
	pingBcastSocket(-1)
{
}

ServerList::~ServerList() {
  _shutDown();
}

void ServerList::startServerPings(StartupInfo *info) {

  // schedule lookup of server list url.  dereference URL chain every
  // time instead of only first time just in case one of the pointers
  // has changed.
  if (phase > -1 && phase < 4)
    return;
  if (info->listServerURL.size() == 0)
    phase = -1;
  else
    phase = 0;

  // also try broadcast
  pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
  if (pingBcastSocket != -1)
    PingPacket::sendRequest(pingBcastSocket, &pingBcastAddr);

}

void ServerList::readServerList()
{
  {
    char *base = (char *)theData;
    char *endS = base + theLen;
    static char *tokenIdentifier = "TOKEN: ";
    static char *noTokenIdentifier = "NOTOK: ";
    static char *errorIdentifier = "ERROR: ";
    static char *noticeIdentifier = "NOTICE: ";
    // walks entire reply including HTTP headers
    while (base < endS) {
      // find next newline
      char* scan = base;
      while (scan < endS && *scan != '\n')
	scan++;

      // if no newline then no more complete replies
      if (scan >= endS)
	break;
      *scan++ = '\0';

      // look for TOKEN: and save token if found also look for NOTOK:
      // and record "badtoken" into the token string and print an
      // error
      if (strncmp(base, tokenIdentifier, strlen(tokenIdentifier)) == 0) {
	strncpy(startupInfo->token, (char *)(base + strlen(tokenIdentifier)),
		TokenLen);
#ifdef DEBUG
	printError("got token:");
	printError(startupInfo->token);
#endif
	base = scan;
	continue;
      } else if (!strncmp(base, noTokenIdentifier,
			  strlen(noTokenIdentifier))) {
	printError("ERROR: did not get token:");
	printError(base);
	strcpy(startupInfo->token, "badtoken\0");
	base = scan;
	continue;
      } else if (!strncmp(base, errorIdentifier, strlen(errorIdentifier))) {
	printError(base);
	strcpy(startupInfo->token, "badtoken\0");
	base = scan;
	continue;
      } else if (!strncmp(base, noticeIdentifier, strlen(noticeIdentifier))) {
        printError(base);
        base = scan;
        continue;
      }
      // parse server info
      char *scan2, *name, *version, *infoServer, *address, *title;
      name = base;
      version = name;
      while (*version && !isspace(*version))  version++;
      while (*version &&  isspace(*version)) *version++ = 0;
      infoServer = version;
      while (*infoServer && !isspace(*infoServer))  infoServer++;
      while (*infoServer &&  isspace(*infoServer)) *infoServer++ = 0;
      address = infoServer;
      while (*address && !isspace(*address))  address++;
      while (*address &&  isspace(*address)) *address++ = 0;
      title = address;
      while (*title && !isspace(*title))  title++;
      while (*title &&  isspace(*title)) *title++ = 0;

      // extract port number from address
      int port = ServerPort;
      scan2 = strchr(name, ':');
      if (scan2) {
	port = atoi(scan2 + 1);
	*scan2 = 0;
      }

      // check info
      if (strcmp(version, getServerVersion()) == 0 &&
	  (int)strlen(infoServer) == PingPacketHexPackedSize &&
	  port >= 1 && port <= 65535) {
	// store info
	ServerItem serverInfo;
	serverInfo.ping.unpackHex(infoServer);
	int dot[4] = {127,0,0,1};
	if (sscanf(address, "%d.%d.%d.%d", dot+0, dot+1, dot+2, dot+3) == 4) {
	  if (dot[0] >= 0 && dot[0] <= 255 &&
	      dot[1] >= 0 && dot[1] <= 255 &&
	      dot[2] >= 0 && dot[2] <= 255 &&
	      dot[3] >= 0 && dot[3] <= 255) {
	    InAddr addr;
	    unsigned char* paddr = (unsigned char*)&addr.s_addr;
	    paddr[0] = (unsigned char)dot[0];
	    paddr[1] = (unsigned char)dot[1];
	    paddr[2] = (unsigned char)dot[2];
	    paddr[3] = (unsigned char)dot[3];
	    serverInfo.ping.serverId.serverHost = addr;
	  }
	}
	serverInfo.ping.serverId.port = htons((int16_t)port);
	serverInfo.name = name;

	// construct description
	serverInfo.description = serverInfo.name;
	if (port != ServerPort) {
	  char portBuf[20];
	  sprintf(portBuf, "%d", port);
	  serverInfo.description += ":";
	  serverInfo.description += portBuf;
	}
	if (strlen(title) > 0) {
	  serverInfo.description += "; ";
	  serverInfo.description += title;
	}

	serverInfo.cached = false;
	// add to list & add it to the server cache
	addToList(serverInfo,true);
      }

      // next reply
      base = scan;
    }

    // remove parsed replies
    theLen -= int(base - (char *)theData);
    memmove(theData, base, theLen);
  }
}

void ServerList::addToList(ServerItem& info, bool doCache)
{
  // update if we already have it
  int i;

  // search and delete entry for this item if it exists
  // (updating info in place may "unsort" the list)
  for (i = 0; i < (int)servers.size(); i++) {
    ServerItem& server = servers[i];
    if (server.ping.serverId.serverHost.s_addr
	== info.ping.serverId.serverHost.s_addr
	&& server.ping.serverId.port == info.ping.serverId.port) {
      servers.erase(servers.begin() + i); // erase this item
    }
  }

  // find point to insert new player at
  int insertPoint = -1; // point to insert server into

  // insert new item before the first serveritem with is deemed to be less
  // in value than the item to be inserted -- cached items are less than
  // non-cached, items that have more players are more, etc..
  for (i = 0; i < (int)servers.size(); i++) {
    ServerItem& server = servers[i];
    if (server < info){
      insertPoint = i;
      break;
    }
  }

  if (insertPoint == -1){ // no spot to insert it into -- goes on back
    servers.push_back(info);
  } else {  // found a spot to insert it into
    servers.insert(servers.begin() + insertPoint,info);
  }

  // update display
  /*
  char buffer [80];
  std::vector<std::string> args;
  sprintf(buffer, "%d", (int)servers.size());
  args.push_back(buffer);
  setStatus("Servers found: {1}", &args);
  */

  // force update
  /*
  const int oldSelectedIndex = selectedIndex;
  selectedIndex = -1;
  setSelected(oldSelectedIndex);
  */

  if (doCache) {
    // make string like "sdfsd.dmsd.com:123"
    char buffer [100];
    std::string serverAddress = info.name;
    sprintf(buffer,":%d",  ntohs((unsigned short) info.ping.serverId.port));
    serverAddress += buffer;
    info.cached = true; // values in cache are "cached"
    // update the last updated time to now
    info.setUpdateTime();

    SRV_STR_MAP::iterator iter;
    iter = serverCache->find(serverAddress);  // erase entry to allow update
    if (iter != serverCache->end()){ // if we find it, update it
      iter->second = info;
    } else {
      // insert into cache -- wasn't found
      serverCache->insert(serverAddress,info);
    }
  }
}

void			ServerList::checkEchos(StartupInfo *info)
{
  startupInfo = info;

  // *** NOTE *** searching spinner update was here

  // lookup server list in phase 0
  if (phase == 0) {

    std::string url = info->listServerURL;

    std::string msg = "action=LIST&version=";
    msg	    += getServerVersion();
    msg	    += "&callsign=";
    msg	    += TextUtils::url_encode(info->callsign);
    msg	    += "&password=";
    msg	    += TextUtils::url_encode(info->password);
    setPostMode(msg);
    setURL(url);
    addHandle();

    // do phase 1 only if we found a valid list server url
    phase = 1;
  }

  // get echo messages
  while (1) {
    // *** NOTE *** searching spinner update was here

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250;

    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (pingBcastSocket != -1) {
      FD_SET((unsigned int)pingBcastSocket, &read_set);
    }
    int fdMax = pingBcastSocket;

    const int nfound = select(fdMax+1, (fd_set*)&read_set,
					(fd_set*)&write_set, 0, &timeout);
    if (nfound <= 0)
      break;

    // check broadcast sockets
    ServerItem serverInfo;
    sockaddr_in addr;

    if (pingBcastSocket != -1 && FD_ISSET(pingBcastSocket, &read_set)) {
      if (serverInfo.ping.read(pingBcastSocket, &addr)) {
	serverInfo.ping.serverId.serverHost = addr.sin_addr;
	serverInfo.cached = false;
	addToListWithLookup(serverInfo);
      }
    }
  } // end loop waiting for input/output on any list server
}

void			ServerList::addToListWithLookup(ServerItem& info)
{
  info.name = Address::getHostByAddress(info.ping.serverId.serverHost);

  // tack on port number to description if not default
  info.description = info.name;
  const int port = (int)ntohs((unsigned short)info.ping.serverId.port);
  if (port != ServerPort) {
    char portBuf[20];
    sprintf(portBuf, "%d", port);
    info.description += ":";
    info.description += portBuf;
  }

  addToList(info); // do not cache network lan - etc. servers
}

// add the entire cache to the server list
void			ServerList::addCacheToList()
{
  if (addedCacheToList)
    return;
  addedCacheToList = true;
  for (SRV_STR_MAP::iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++){
    addToList(iter->second);
  }
}

void ServerList::collectData(char *ptr, int len)
{
  phase = 2;

  cURLManager::collectData(ptr, len);

  readServerList();
}

void ServerList::finalization(char *, unsigned int, bool good)
{
  if (!good) {
    printError("Can't talk with list server");
    addCacheToList();
    phase = -1;
  } else {
    phase = 4;
  }
}

const std::vector<ServerItem>& ServerList::getServers() {
  return servers;
}

std::vector<ServerItem>::size_type ServerList::size() {
  return servers.size();
}

void ServerList::clear() {
  servers.clear();
}

int ServerList::updateFromCache() {
  // clear server list
  clear();

  int numItemsAdded = 0;

  for (SRV_STR_MAP::const_iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++) {
    // if maxCacheAge is 0 we add nothing
    // if the item is young enough we add it
    if (serverCache->getMaxCacheAge() != 0
	&& iter->second.getAgeMinutes() < serverCache->getMaxCacheAge()) {
      ServerItem aCopy = iter->second;
      addToList(aCopy);
      numItemsAdded ++;
    }
  }

  return numItemsAdded;
}

bool ServerList::searchActive() const {
  return (phase < 4) ? true : false;
}

bool ServerList::serverFound() const {
  return (phase >= 2) ? true : false;
}

void ServerList::_shutDown() {
  // close broadcast socket
  closeBroadcast(pingBcastSocket);
  pingBcastSocket = -1;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
