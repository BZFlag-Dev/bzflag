/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "menus.h"
#include "ServerListCache.h"
#include "playing.h"
#include "Protocol.h"

//
// ServerListCache
//

ServerListCache	ServerListCache::globalCache;


ServerListCache::ServerListCache() : cacheAddedNum (0)
{

}

ServerListCache::~ServerListCache ()
{

}

// get the global instance of the cache
ServerListCache*  ServerListCache::get()
{
  return &globalCache;
}

// insert a serverItem mapped by the serverAddress
void ServerListCache::insert(std::string serverAddress,ServerItem& info)
{
  serverCache.insert(SRV_STR_MAP::value_type(serverAddress,info));
}

// a wrapper that allows access to the maps find method
SRV_STR_MAP::iterator ServerListCache::find(std::string serverAddress)
{
  return serverCache.find(serverAddress);
}

// a wrapper that allows access to the maps begin method
SRV_STR_MAP::iterator ServerListCache::begin()
{
  return serverCache.begin();
}

// a wrapper that allows access to the maps end method
SRV_STR_MAP::iterator ServerListCache::end()
{
  return serverCache.end();
}



// load the server cache from the file fileName
void			ServerListCache::saveCache()
{
  // get a file named e.g. BZFS107Server.bzs in the cache dir
  // allows separation of server caches by version
  std::string fileName = getCacheDirectoryName();
  if (fileName == "") return;
  std::string verString = ServerVersion;
  verString = verString.substr(0,7);
  fileName = fileName + "/" + verString + "Servers.bzs";

  char buffer[MAX_STRING+1];

  ofstream outFile (fileName.c_str(), ios::out|ios::binary);
  int lenCpy = MAX_STRING;
  bool doWeed = (cacheAddedNum >0); // weed out as many items as were added

  if (outFile){
    for (SRV_STR_MAP::iterator iter = serverCache.begin(); iter != serverCache.end(); iter++){
      // weed out after 30 days *if* if we should
      if (doWeed && iter->second.getAgeMinutes() > 60*24*30) {
       cacheAddedNum --;
       doWeed = (cacheAddedNum >0);
       continue;
      }

      // write out the index of the map
      memset(&buffer,0, sizeof(buffer));
      lenCpy = (iter->first).size() < MAX_STRING ? (iter->first).size() : MAX_STRING;
      strncpy(&buffer[0],(iter->first.c_str()),lenCpy);
      outFile.write(buffer,sizeof(buffer));

      ServerItem x = iter->second;
      // write out the serverinfo -- which is mapped by index
      (iter->second).writeToFile(outFile);

    }
    outFile.close();
  }
}

// load the server cache
void			ServerListCache::loadCache()
{
  // get a file named BZFS107Server.bzs in the cache dir
  // allows separation of server caches by version
  std::string fileName = getCacheDirectoryName();
  if (fileName == "") return;
  std::string verString = ServerVersion;
  verString = verString.substr(0,7);
  fileName = fileName + "/" + verString + "Servers.bzs";

  char buffer[MAX_STRING+1];

  ifstream inFile (fileName.c_str(),ios::in|ios::binary);
  bool infoWorked;

  if (inFile) {
    while(inFile) {
      std::string serverIndex;
      ServerItem info;

      inFile.read(buffer,sizeof(buffer)); //read the index of the map
      if ((size_t)inFile.gcount() < sizeof(buffer)) break; // failed to read entire string
      serverIndex = buffer;

      infoWorked = info.readFromFile(inFile);
      // after a while it is doubtful that player counts are accurate
      if (info.getAgeMinutes() > (time_t)30) info.ping.zeroPlayerCounts();
      if (!infoWorked) break;

      serverCache.insert(SRV_STR_MAP::value_type(serverIndex,info));
    }
    inFile.close();
  }
}

// increases the maximum # of items that will be weeded out on save
void			ServerListCache::incAddedNum()
{
  cacheAddedNum++;
}

// clear the server list cache
bool			ServerListCache::clearCache()
{
  if (serverCache.size() > 0){
    serverCache.clear();
    return true;

  } else {
    return false;
  }

}

// set the max age to # of minutes after which items in cache
// are this old they are no longer shown on the find menu-- 0 disables
void			ServerListCache::setMaxCacheAge(time_t time)
{
  maxCacheAge = time;
}

time_t			ServerListCache::getMaxCacheAge()
{
  return maxCacheAge;
}
// ex: shiftwidth=2 tabstop=8
