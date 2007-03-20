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
#include "ServerListCache.h"

/* system headers */
#include <fstream>

// invoke persistent rebuilding for build versioning
#include "version.h"

/* common implementation headers */
#include "FileManager.h"
#include "Protocol.h"
#include "DirectoryNames.h"
#include "TextUtils.h"


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
void ServerListCache::insert(const std::string &serverAddress, const ServerItem &info)
{
  serverCache.insert(SRV_STR_MAP::value_type(serverAddress,info));
}


ServerListCache::SRV_STR_MAP::iterator ServerListCache::find(const std::string &serverAddress)
{
  return serverCache.find(serverAddress);
}


ServerListCache::SRV_STR_MAP::iterator ServerListCache::begin()
{
  return serverCache.begin();
}


ServerListCache::SRV_STR_MAP::iterator ServerListCache::end()
{
  return serverCache.end();
}

bool ServerListCache::isFavorite(const std::string &serverAddress) const
{
  SRV_STR_MAP::const_iterator i = serverCache.find(serverAddress);
  return i!=serverCache.end() && i->second.favorite;
}

std::string             ServerListCache::getCacheFilename(int subrevision) const
{
  // get a file named e.g. BZFS1910Server.bzs in the cache dir
  // allows separation of server caches by protocol version
  std::string fileName = getCacheDirName();
  if (fileName == "") return "";
  std::string verString = getServerVersion();
  // subrevision to differentiate file formats for same protocol version
  if (subrevision > 0)
    verString += TextUtils::format("_%d", subrevision);
  fileName += DirectorySeparator;
  fileName += verString + "-Servers.bzs";
  return fileName;
}

void			ServerListCache::saveCache()
{
  const int subrevision = 1;
  std::string fileName = getCacheFilename(subrevision);
  if (fileName == "") return;

  std::ostream* outFile = FILEMGR.createDataOutStream(fileName, true, true);
  int lenCpy = max_string;

  if (outFile != NULL){
    char buffer[max_string+1];
    for (SRV_STR_MAP::iterator iter = serverCache.begin(); iter != serverCache.end(); iter++){

      // skip items that are more than 30 days old, but always save favorites
      if (!iter->second.favorite && iter->second.getAgeMinutes() > 60*24*30) {
	continue;
      }

      // write out the index of the map
      memset(buffer, 0, sizeof(buffer));
      lenCpy = int((iter->first).size() < max_string ? (iter->first).size() : max_string);
      strncpy(buffer, (iter->first.c_str()), lenCpy);
      outFile->write(buffer, sizeof(buffer));

      ServerItem x = iter->second;
      // write out the serverinfo -- which is mapped by index
      iter->second.writeToFile(*outFile);
    }
    delete outFile;
  }
}


void			ServerListCache::loadCache()
{
  int subrevision = 1;
  std::string fileName = getCacheFilename(subrevision);
  if (fileName == "") return;
  std::ifstream inFile(fileName.c_str(), std::ios::in | std::ios::binary);

  // could not open, try again with older revision
  if (!inFile) {
    subrevision = 0;
    std::string fileName = getCacheFilename(subrevision);
    if (fileName == "") return;
    inFile.clear();
    inFile.open(fileName.c_str(), std::ios::in | std::ios::binary);
  }

  if (inFile) {
    char buffer[max_string+1];
    while(inFile) {
      std::string serverIndex;
      ServerItem info;

      inFile.read(buffer,sizeof(buffer)); //read the index of the map
      if ((size_t)inFile.gcount() < sizeof(buffer)) break; // failed to read entire string
      serverIndex = buffer;

      bool infoWorked = info.readFromFile(inFile, subrevision);
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
