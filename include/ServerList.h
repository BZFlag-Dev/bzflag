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

#ifndef __SERVERLIST_H__
#define __SERVERLIST_H__

#include "common.h"

/* system interface headers */
#include <vector>

/* common interface headers */
#include "ListServer.h"
#include "StartupInfo.h"
#include "ServerItem.h"
#include "ServerListCache.h"
#include "cURLManager.h"


/** The ServerList class contains links to the list server as well as
 * any fetched list of servers.  The list handles cacheing of those
 * server entries in case of list server unavailability.
 */
class ServerList : cURLManager {

public:
  ServerList();
  virtual ~ServerList();

  void checkEchos(StartupInfo *_info);
  void startServerPings(StartupInfo *_info);
  bool searchActive() const;
  bool serverFound() const;
  const std::vector<ServerItem>& getServers();
  std::vector<ServerItem>::size_type size();
  int updateFromCache();
  void collectData(char *ptr, int len);
  void finalization(char *data, unsigned int length, bool good);

public:
  void addToList(ServerItem, bool doCache=false);
  void markFav(const std::string &, bool);
  void clear();

private:
  void readServerList();
  void addToListWithLookup(ServerItem&);
  void addCacheToList();
  void _shutDown();

private:
  bool addedCacheToList;
  int phase;
  std::vector<ServerItem> servers;
  ServerListCache* serverCache;
  int pingBcastSocket;
  struct sockaddr_in pingBcastAddr;
  StartupInfo *startupInfo;
};

#endif  /* __SERVERLIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
