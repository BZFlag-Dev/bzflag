/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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
#include "Singleton.h"

typedef void (*ServerListCallback)(ServerItem* addedServer, void*);

typedef std::list< std::pair<ServerListCallback, void*> > ServerCallbackList;
typedef std::map<std::string, std::vector<std::pair<ServerListCallback, void*> > > ServerKeyCallbackList;

/** The ServerList class contains links to the list server as well as
 * any fetched list of servers.  The list handles cacheing of those
 * server entries in case of list server unavailability.
 */
class ServerList : private cURLManager, public Singleton<ServerList> {

  public:
    ServerList();
    virtual ~ServerList();

    void checkEchos(StartupInfo* _info);
    void startServerPings(StartupInfo* _info);
    bool searchActive() const;
    bool serverFound() const;
    const std::map<std::string, ServerItem>& getServers();
    std::map<std::string, ServerItem>::size_type size();
    ServerItem* lookupServer(std::string key);
    int updateFromCache();
    void collectData(char* ptr, int len);
    void finalization(char* data, unsigned int length, bool good);

    ServerItem* getServerAt(size_t index);

    void addServerCallback(ServerListCallback cb, void* data);
    void removeServerCallback(ServerListCallback cb, void* data);

    void addServerKeyCallback(std::string key, ServerListCallback cb, void* data);
    void removeServerKeyCallback(std::string key, ServerListCallback cb, void* data);

    void addFavoriteServerCallback(ServerListCallback cb, void* data);
    void removeFavoriteServerCallback(ServerListCallback cb, void* data);

    void addRecentServerCallback(ServerListCallback cb, void* data);
    void removeRecentServerCallback(ServerListCallback cb, void* data);

    void addClearedListCallback(ServerListCallback cb, void* data);
    void removeClearedListCallback(ServerListCallback cb, void* data);

  public:
    void markAsFavorite(ServerItem* item);
    void markAsRecent(ServerItem* item);
    void unmarkAsFavorite(ServerItem* item);
    void unmarkAsRecent(ServerItem* item);

    void addToList(ServerItem, bool doCache = false);
    //void markFav(const std::string &, bool);
    void clear();
    void sort();

  protected:
    friend class Singleton<ServerList>;

  private:
    void readServerList();
    void addToListWithLookup(ServerItem&);
    void addCacheToList();
    void _shutDown();

  private:
    bool addedCacheToList;
    int phase;
    std::map<std::string, ServerItem> servers;
    ServerListCache* serverCache;
    int pingBcastSocket;
    struct sockaddr_in pingBcastAddr;
    StartupInfo* startupInfo;

    ServerCallbackList serverCallbackList;
    ServerKeyCallbackList serverKeyCallbackList;
    ServerCallbackList favoritesCallbackList;
    ServerCallbackList recentCallbackList;
    ServerCallbackList clearedCallbacklist;
};

#endif  /* __SERVERLIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
