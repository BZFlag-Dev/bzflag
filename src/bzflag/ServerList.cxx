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

/* interface header */
#include "ServerList.h"

/* system headers */
#include <iostream>
#include <vector>
#include <string>

/* common implementation headers */
#include "version.h"
#include "bzsignal.h"
#include "Ping.h"
#include "Protocol.h"
#include "TimeKeeper.h"
#include "TextUtils.h"

/* local implementation headers */
#include "ServerListCache.h"
#include "StartupInfo.h"
#include "ErrorHandler.h"
#if !defined(_WIN32)
#include <errno.h>
#endif

/* from playing.h */
StartupInfo* getStartupInfo();
typedef void (*PlayingCallback)(void*);
void addPlayingCallback(PlayingCallback, void* data);
void removePlayingCallback(PlayingCallback, void* data);

ServerList::ServerList() :
	numListServers(0),
	phase(-1),
	serverCache(ServerListCache::get()),
	pingBcastSocket(-1)
{
}

ServerList::~ServerList() {
  _shutDown();
}

void ServerList::startServerPings() {

  // schedule lookup of server list url.  dereference URL chain every
  // time instead of only first time just in case one of the pointers
  // has changed.
  const StartupInfo* info = getStartupInfo();
  if (info->listServerURL.size() == 0)
    phase = -1;
  else
    phase = 0;

  // also try broadcast
  pingBcastSocket = openBroadcast(BroadcastPort, NULL, &pingBcastAddr);
  if (pingBcastSocket != -1)
    PingPacket::sendRequest(pingBcastSocket, &pingBcastAddr);

}

void ServerList::readServerList(int index)
{
  ListServer& listServer = listServers[index];

  // read more data into server list buffer
  int n = recv(listServer.socket, listServer.buffer + listServer.bufferSize,
				sizeof(listServer.buffer) -
					listServer.bufferSize - 1, 0);
  if (n > 0) {
    listServer.bufferSize += n;
    listServer.buffer[listServer.bufferSize] = 0;

    char* base = listServer.buffer;
    static char *tokenIdentifier = "TOKEN:";
    // walks entire reply including HTTP headers
    while (*base) {
      // find next newline
      char* scan = base;
      while (*scan && *scan != '\n') scan++;

      // if no newline then no more complete replies
      if (*scan != '\n') break;
      *scan++ = '\0';

      // look for ^TOKEN: and save token if found
      if (strncmp(base, tokenIdentifier, strlen(tokenIdentifier)) == 0) {
	StartupInfo* info = getStartupInfo();
	printError("got token. TODO: give it to the server");
        //printError(base);
	strncpy(info->token, (char *)(base + strlen(tokenIdentifier)), TokenLen);
	printError(info->token);
	base=scan;
	continue;
      }
      // parse server info
      char *scan2, *name, *version, *info, *address, *title;
      name = base;
      version = name;
      while (*version && !isspace(*version))  version++;
      while (*version &&  isspace(*version)) *version++ = 0;
      info = version;
      while (*info && !isspace(*info))  info++;
      while (*info &&  isspace(*info)) *info++ = 0;
      address = info;
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
	  (int)strlen(info) == PingPacketHexPackedSize &&
	  port >= 1 && port <= 65535) {
	// store info
	ServerItem serverInfo;
	serverInfo.ping.unpackHex(info);
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
    listServer.bufferSize -= (base - listServer.buffer);
    memmove(listServer.buffer, base, listServer.bufferSize);
  } else if (n == 0) {
    // server hungup
    close(listServer.socket);
    listServer.socket = -1;
    listServer.phase = 4;
  } else if (n < 0) {
    close(listServer.socket);
    listServer.socket = -1;
    listServer.phase = -1;
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
    if (server.ping.serverId.serverHost.s_addr == info.ping.serverId.serverHost.s_addr
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
    // update server cache if asked for
    // on save we delete at most as many items as we added
    // if the added list is normal, we weed most out, if we
    // get few items, we weed few items out
    serverCache->incAddedNum();

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

void			ServerList::checkEchos()
{
  // *** NOTE *** searching spinner update was here

  // lookup server list in phase 0
  if (phase == 0) {
    int i;

    std::vector<std::string> urls;
    urls.push_back(getStartupInfo()->listServerURL);

    // check urls for validity
    numListServers = 0;
    for (i = 0; i < (int)urls.size(); ++i) {
      // parse url
      std::string protocol, hostname, path;
      int port = 80;
      Address address;
      if (!BzfNetwork::parseURL(urls[i], protocol, hostname, port, path) ||
	protocol != "http" || port < 1 || port > 65535 ||
	(address = Address::getHostAddress(hostname)).isAny()) {
	std::vector<std::string> args;
	args.push_back(urls[i]);
	printError("Can't open list server: {1}", &args);
	if (!addedCacheToList) {
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // add to list
      listServers[numListServers].address = address;
      listServers[numListServers].hostname = hostname;
      listServers[numListServers].pathname = path;
      listServers[numListServers].port    = port;
      listServers[numListServers].socket  = -1;
      listServers[numListServers].phase   = 2;
      numListServers++;
    }

    // do phase 1 only if we found a valid list server url
    if (numListServers > 0)
      phase = 1;
    else
      phase = -1;
  }

  // connect to list servers in phase 1
  else if (phase == 1) {
    phase = -1;
    for (int i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];

      // create socket.  give up on failure.
      listServer.socket = socket(AF_INET, SOCK_STREAM, 0);
      if (listServer.socket < 0) {
	printError("Can't create list server socket");
	listServer.socket = -1;
	if (!addedCacheToList) {
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // set to non-blocking.  we don't want to wait for the connection.
      if (BzfNetwork::setNonBlocking(listServer.socket) < 0) {
	printError("Error with list server socket");
	close(listServer.socket);
	listServer.socket = -1;
	if (!addedCacheToList){
	  addedCacheToList = true;
	  addCacheToList();
	}
	continue;
      }

      // start connection
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(listServer.port);
      addr.sin_addr = listServer.address;
      if (connect(listServer.socket, (CNCTType*)&addr, sizeof(addr)) < 0) {
#if defined(_WIN32)
#undef EINPROGRESS
#define EINPROGRESS EWOULDBLOCK
#endif
	if (getErrno() != EINPROGRESS) {
	  printError("Can't connect list server socket");
	  close(listServer.socket);
	  listServer.socket = -1;
	  if (!addedCacheToList){
	    addedCacheToList = true;
	    addCacheToList();
	  }
	  continue;
	}
      }

      // at least this socket is okay so proceed to phase 2
      phase = 2;
    }
  }

  // get echo messages
  while (1) {
    int i;

    // *** NOTE *** searching spinner update was here

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250;

    fd_set read_set, write_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    if (pingBcastSocket != -1) _FD_SET(pingBcastSocket, &read_set);
    int fdMax = pingBcastSocket;

    // check for list server connection or data
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	if (listServer.phase == 2) {
	  _FD_SET(listServer.socket, &write_set);
	} else if (listServer.phase == 3) {
	  _FD_SET(listServer.socket, &read_set);
	}
	if (listServer.socket > fdMax) {
	  fdMax = listServer.socket;
	}
      }
    }

    const int nfound = select(fdMax+1, (fd_set*)&read_set,
					(fd_set*)&write_set, 0, &timeout);
    if (nfound <= 0) {
      break;
    }

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

    // check list servers
    for (i = 0; i < numListServers; i++) {
      ListServer& listServer = listServers[i];
      if (listServer.socket != -1) {
	// read more data from server
	if (FD_ISSET(listServer.socket, &read_set)) {
	  readServerList(i);
	}

	// send list request
	else if (FD_ISSET(listServer.socket, &write_set)) {
#if !defined(_WIN32)
	  // ignore SIGPIPE for this send
	  SIG_PF oldPipeHandler = bzSignal(SIGPIPE, SIG_IGN);
#endif
	  bool errorSending;
	  {
	    const StartupInfo* info = getStartupInfo();
	    char url[1024];
	    snprintf(url, sizeof(url),
		     "GET %s?action=LIST&version=%s&callsign=%s&password=%s HTTP/1.1\r\nHost: %s\r\nCache-control: no-cache\r\n\r\n",
		     listServer.pathname.c_str(), getServerVersion(),
		     info->callsign, info->password,
		     listServer.hostname.c_str());
	    //printError(url);
	    errorSending = send(listServer.socket, url, strlen(url), 0)
	      != (int) strlen(url);
	  }
	  if (errorSending) {
	    // probably unable to connect to server
	    close(listServer.socket);
	    listServer.socket = -1;
	    if (!addedCacheToList){
	      addedCacheToList = true;
	      addCacheToList();
	     }
	  } else {
	    listServer.phase = 3;
	    listServer.bufferSize = 0;
	  }
#if !defined(_WIN32)
	  bzSignal(SIGPIPE, oldPipeHandler);
#endif
	}
      }
    }
  }
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
  for (SRV_STR_MAP::iterator iter = serverCache->begin();
       iter != serverCache->end(); iter++){
    addToList(iter->second);
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
    if (serverCache->getMaxCacheAge() != 0 && iter->second.getAgeMinutes() < serverCache->getMaxCacheAge()) {
      ServerItem aCopy = iter->second;
      addToList(aCopy);
      numItemsAdded ++;
    }
  }

  return numItemsAdded;
}

bool ServerList::searchActive() {
  return ((phase < 2) ? true : false);
}

void ServerList::_shutDown() {
  // close server list sockets
  for (int i = 0; i < numListServers; i++)
    if (listServers[i].socket != -1) {
      close(listServers[i].socket);
      listServers[i].socket = -1;
    }
  numListServers = 0;

  // close input multicast socket
  closeMulticast(pingBcastSocket);
  pingBcastSocket = -1;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
