/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#include "ServerListCache.h"

/* system headers */
#include <fstream>

// invoke persistent rebuilding for build versioning
#include "version.h"

/* common implementation headers */
#include "FileManager.h"
#include "Protocol.h"
#include "DirectoryNames.h"


ServerListCache	ServerListCache::globalCache;


ServerListCache::ServerListCache()
{
}

ServerListCache::~ServerListCache ()
{
}


ServerListCache*  ServerListCache::get()
{
  return &globalCache;
}


// insert a serverItem mapped by the serverAddress
void ServerListCache::insert(std::string serverAddress,ServerItem& info)
{
  serverCache.insert(SRV_STR_MAP::value_type(serverAddress,info));
}


SRV_STR_MAP::iterator ServerListCache::find(std::string serverAddress)
{
  return serverCache.find(serverAddress);
}


SRV_STR_MAP::iterator ServerListCache::begin()
{
  return serverCache.begin();
}


SRV_STR_MAP::iterator ServerListCache::end()
{
  return serverCache.end();
}


void			ServerListCache::saveCache()
{
  // get a file named e.g. BZFS1910Server.bzs in the cache dir
  // allows separation of server caches by protocol version
  std::string fileName = getCacheDirName();
  if (fileName == "") return;
  std::string verString = getServerVersion();
#ifdef _WIN32
  fileName = fileName + "\\" + verString + "-Servers.bzs";
#else
  fileName = fileName + "/" + verString + "-Servers.bzs";
#endif

  char buffer[MAX_STRING+1];

  std::ostream* outFile = FILEMGR.createDataOutStream(fileName, true, true);
  int lenCpy = MAX_STRING;

  if (outFile != NULL){
    for (SRV_STR_MAP::iterator iter = serverCache.begin(); iter != serverCache.end(); iter++){

      // skip items that are more than 30 days old
      if (iter->second.getAgeMinutes() > 60*24*30) {
	continue;
      }

      // write out the index of the map
      memset(buffer, 0, sizeof(buffer));
      lenCpy = int((iter->first).size() < MAX_STRING ? (iter->first).size() : MAX_STRING);
      strncpy(buffer, (iter->first.c_str()), lenCpy);
      outFile->write(buffer, sizeof(buffer));

      ServerItem x = iter->second;
      // write out the serverinfo -- which is mapped by index
      (iter->second).writeToFile(*outFile);

    }
    delete outFile;
  }
}


void			ServerListCache::loadCache()
{
  // get a file named BZFS1910Server.bzs in the cache dir
  // allows separation of server caches by protocol version
  std::string fileName = getCacheDirName();
  if (fileName == "") return;
  std::string verString = getServerVersion();
#ifdef _WIN32
  fileName = fileName + "\\" + verString + "-Servers.bzs";
#else
  fileName = fileName + "/" + verString + "-Servers.bzs";
#endif

  char buffer[MAX_STRING+1];

  std::ifstream inFile (fileName.c_str(), std::ios::in | std::ios::binary);
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


bool			ServerListCache::clearCache()
{
  if (serverCache.size() > 0){
    serverCache.clear();
    return true;

  } else {
    return false;
  }

}


void			ServerListCache::setMaxCacheAge(time_t time)
{
  maxCacheAge = time;
}

time_t			ServerListCache::getMaxCacheAge()
{
  return maxCacheAge;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
