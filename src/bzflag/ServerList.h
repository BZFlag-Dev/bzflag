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

#ifndef __SERVERLIST_H__
#define __SERVERLIST_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "BzfEvent.h"
#include "Protocol.h"
#include "version.h"

/* local interface headers */
#include "ServerItem.h"
#include "ServerListCache.h"
#include "ListServer.h"

/** The ServerList class contains links to the list server as well as
 * any fetched list of servers.  The list handles cacheing of those
 * server entries in case of list server unavailability.
 */
class ServerList {

public:
  ServerList();
  virtual ~ServerList();

  void readServerList(int index);
  void addToList(ServerItem&, bool doCache=false);
  void addToListWithLookup(ServerItem&);
  void checkEchos();
  void addCacheToList();
  void startServerPings();
  bool searchActive();
  const std::vector<ServerItem>& getServers();
  std::vector<ServerItem>::size_type size();
  int updateFromCache();
  void clear();

private:
  void _shutDown();

private:
  bool addedCacheToList;
  long numListServers;
  int phase;
  std::vector<ServerItem> servers;
  ListServer listServers[MaxListServers];
  ServerListCache* serverCache;
  struct sockaddr_in pingInAddr;
  int pingBcastSocket;
  struct sockaddr_in pingBcastAddr;

};

#endif  /* __SERVERLIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
