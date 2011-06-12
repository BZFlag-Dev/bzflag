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

// interface headers
#include "HUDuiServerListCache.h"

#include "ServerMenu.h"

#include "net/Pack.h"
#include "version.h"
#include "game/DirectoryNames.h"
#include "common/FileManager.h"
#include <algorithm>

//
// HUDuiServerListCache
//

// initialize the singleton
template <>
HUDuiServerListCache* Singleton<HUDuiServerListCache>::_instance = (HUDuiServerListCache*)0;

HUDuiServerListCache::HUDuiServerListCache() {
  cacheLoaded = false;
}

HUDuiServerListCache::~HUDuiServerListCache() {
  // do nothing
}

void HUDuiServerListCache::saveCache() {
  if (!cacheLoaded) {
    return;
  }

  std::string fileName = getCacheFilename();
  if (fileName == "") { return; }

  std::ostream* outFile = FILEMGR.createDataOutStream(fileName, true, true);

  if (outFile != NULL) {
    char buffer[max_string + 1];

    // write out normal list sort mode
    memset(buffer, 0, sizeof(buffer));
    nboPackInt32(buffer, cachedLists[0].first->getSortMode());
    outFile->write(&buffer[0], 4);

    // write out normal list reverse sort flag
    nboPackUInt8(buffer, cachedLists[0].first->getReverseSort());
    outFile->write(buffer, 1);

    // write out recent list sort mode
    memset(buffer, 0, sizeof(buffer));
    nboPackInt32(buffer, cachedLists[1].first->getSortMode());
    outFile->write(&buffer[0], 4);

    // write out recent list reverse sort flag
    nboPackUInt8(buffer, cachedLists[1].first->getReverseSort());
    outFile->write(buffer, 1);

    // write out favorites list sort mode
    memset(buffer, 0, sizeof(buffer));
    nboPackInt32(buffer, cachedLists[2].first->getSortMode());
    outFile->write(&buffer[0], 4);

    // write out favorites list reverse sort flag
    nboPackUInt8(buffer, cachedLists[2].first->getReverseSort());
    outFile->write(buffer, 1);

    for (size_t i = 3; i < cachedLists.size(); i++) {
      // write out the list's tab name
      memset(buffer, 0, sizeof(buffer));
      int copyLength = int(cachedLists[i].second.size() < max_string ? cachedLists[i].second.size() : max_string);
      strncpy(&buffer[0], cachedLists[i].second.c_str(), copyLength);
      outFile->write(buffer, sizeof(buffer));

      // write out the list's sort mode
      memset(buffer, 0, sizeof(buffer));
      nboPackInt32(buffer, cachedLists[i].first->getSortMode());
      outFile->write(&buffer[0], 4);

      // write out the list's reverse sort flag
      nboPackUInt8(buffer, cachedLists[i].first->getReverseSort());
      outFile->write(buffer, 1);

      // write out the list's filter options
      memset(buffer, 0, sizeof(buffer));
      nboPackInt32(buffer, cachedLists[i].first->getFilterOptions());
      outFile->write(&buffer[0], 4);

      // write out the list's server name filter
      memset(buffer, 0, sizeof(buffer));
      copyLength = int(cachedLists[i].first->getFilterPatterns().second.size() < max_string ? cachedLists[i].first->getFilterPatterns().second.size() : max_string);
      strncpy(&buffer[0], cachedLists[i].first->getFilterPatterns().second.c_str(), copyLength);
      outFile->write(buffer, sizeof(buffer));

      // write out the list's domain name filter
      memset(buffer, 0, sizeof(buffer));
      copyLength = int(cachedLists[i].first->getFilterPatterns().first.size() < max_string ? cachedLists[i].first->getFilterPatterns().first.size() : max_string);
      strncpy(&buffer[0], cachedLists[i].first->getFilterPatterns().first.c_str(), copyLength);
      outFile->write(buffer, sizeof(buffer));

      // write out the number of servers in the list
      memset(buffer, 0, sizeof(buffer));
      nboPackInt32(buffer, (int)cachedLists[i].first->getSize());
      outFile->write(&buffer[0], 4);

      // write out the server keys of servers in the list
      for (size_t j = 0; j < cachedLists[i].first->getSize(); j++) {
        memset(buffer, 0, sizeof(buffer));
        copyLength = int(cachedLists[i].first->get(j)->getServerKey().size() < max_string ? cachedLists[i].first->get(j)->getServerKey().size() : max_string);
        strncpy(&buffer[0], cachedLists[i].first->get(j)->getServerKey().c_str(), copyLength);
        outFile->write(buffer, sizeof(buffer));
      }
    }
    delete outFile;
  }
}

bool HUDuiServerListCache::loadCache() {
  std::string fileName = getCacheFilename();
  if (fileName == "") { return false; }
  std::ifstream inFile(fileName.c_str(), std::ios::in | std::ios::binary);

  if (inFile) {
    char buffer[max_string + 1];

    // Create the standard server lists
    cachedLists.clear();
    cachedLists.push_back(std::pair<HUDuiServerList*, std::string>(new HUDuiServerList, "")); // All server list
    cachedLists.push_back(std::pair<HUDuiServerList*, std::string>(new HUDuiServerList, "")); // Recent server list
    cachedLists.push_back(std::pair<HUDuiServerList*, std::string>(new HUDuiServerList, "")); // Favorites server list

    // read in normal list sort mode
    inFile.read(&buffer[0], 4);
    if (inFile.gcount() < 4) { return false; }
    int32_t sort;
    nboUnpackInt32(&buffer[0], sort);

    // read in normal list reverse sort flag
    uint8_t rev;
    inFile.read(buffer, 1);
    nboUnpackUInt8(buffer, rev);
    bool reverse = (rev != 0);

    // apply sort mode
    applySort(0, sort, reverse);

    // read in recent list sort mode
    inFile.read(&buffer[0], 4);
    if (inFile.gcount() < 4) { return false; }
    nboUnpackInt32(&buffer[0], sort);

    // read in recent list reverse sort flag
    inFile.read(buffer, 1);
    nboUnpackUInt8(buffer, rev);
    reverse = (rev != 0);

    // apply sort mode
    applySort(1, sort, reverse);

    // read in favorites list sort mode
    inFile.read(&buffer[0], 4);
    if (inFile.gcount() < 4) { return false; }
    nboUnpackInt32(&buffer[0], sort);

    // read in favorites list reverse sort flag
    inFile.read(buffer, 1);
    nboUnpackUInt8(buffer, rev);
    reverse = (rev != 0);

    // apply sort mode
    applySort(2, sort, reverse);

    while (inFile) {
      //read in the list's tab names
      memset(buffer, 0, sizeof(buffer));
      inFile.read(buffer, sizeof(buffer));
      if ((size_t)inFile.gcount() < sizeof(buffer)) { return false; } // failed to read entire string
      std::string tabName = buffer;

      // read in the sort mode
      inFile.read(&buffer[0], 4);
      if (inFile.gcount() < 4) { return false; }
      nboUnpackInt32(&buffer[0], sort);

      // read in the reverse sort flag
      inFile.read(buffer, 1);
      nboUnpackUInt8(buffer, rev);
      reverse = (rev != 0);

      cachedLists.push_back(std::pair<HUDuiServerList*, std::string>(new HUDuiServerList, tabName));
      int i = (int) cachedLists.size() - 1;

      // apply sort mode
      applySort(i, sort, reverse);

      // read in the list's filter options
      inFile.read(&buffer[0], 4);
      if (inFile.gcount() < 1) { return false; }
      int32_t filters_signed;
      nboUnpackInt32(&buffer[0], filters_signed);
      uint32_t filters = (uint32_t) filters_signed;

      cachedLists[i].first->applyFilters(filters);

      //read in the list's server name filter
      memset(buffer, 0, sizeof(buffer));
      inFile.read(buffer, sizeof(buffer));
      if ((size_t)inFile.gcount() < sizeof(buffer)) { return false; } // failed to read entire string
      std::string serverFilter = buffer;

      cachedLists[i].first->serverNameFilter(serverFilter);

      //read in the list's domain name filter
      memset(buffer, 0, sizeof(buffer));
      inFile.read(buffer, sizeof(buffer));
      if ((size_t)inFile.gcount() < sizeof(buffer)) { return false; } // failed to read entire string
      std::string domainFilter = buffer;

      cachedLists[i].first->domainNameFilter(domainFilter);

      // read in the number of servers in the list
      inFile.read(&buffer[0], 4);
      if (inFile.gcount() < 1) { return false; }
      int32_t serverCount;
      nboUnpackInt32(&buffer[0], serverCount);

      int j = 0;
      while (j < serverCount) {
        //read in the server keys
        memset(buffer, 0, sizeof(buffer));
        inFile.read(buffer, sizeof(buffer));
        if ((size_t)inFile.gcount() < sizeof(buffer)) { return false; } // failed to read entire string
        std::string key = buffer;

        ServerList::instance().addServerKeyCallback(key, ServerMenu::newServer , cachedLists[i].first);
        j++;
      }
    }
    inFile.close();
  }
  cacheLoaded = true;
  return true;
}

void HUDuiServerListCache::clearCache() {
  // Do nothing
}

std::string HUDuiServerListCache::getCacheFilename() const {
  std::string fileName = getCacheDirName();
  if (fileName == "") { return ""; }
  std::string verString = getServerVersion();
  fileName += verString + "-ServerLists.bzs";
  return fileName;
}

void HUDuiServerListCache::applySort(int index, int _sort, bool reverse) {
  switch (_sort) {
    case HUDuiServerList::DomainName:
      cachedLists[index].first->sortBy(HUDuiServerList::DomainName);
      break;

    case HUDuiServerList::ServerName:
      cachedLists[index].first->sortBy(HUDuiServerList::ServerName);
      break;

    case HUDuiServerList::PlayerCount:
      cachedLists[index].first->sortBy(HUDuiServerList::PlayerCount);
      break;

    case HUDuiServerList::Ping:
      cachedLists[index].first->sortBy(HUDuiServerList::Ping);
      break;
  }
  cachedLists[index].first->setReverseSort(reverse);
}

void HUDuiServerListCache::addNewList(HUDuiServerList* newList, std::string tabName) {
  cachedLists.push_back(std::pair<HUDuiServerList*, std::string>(newList, tabName));
}

void HUDuiServerListCache::removeList(HUDuiServerList* list, std::string tabName) {
  std::vector<std::pair<HUDuiServerList*, std::string> >::iterator it = std::find(cachedLists.begin(), cachedLists.end(), std::pair<HUDuiServerList*, std::string>(list, tabName));

  // Do not remove the first 3 lists: normal, recent, favorites
  if ((it == cachedLists.begin()) || (it == cachedLists.begin() + 1) || (it == cachedLists.begin() + 2)) {
    return;
  }
  else if (it != cachedLists.end()) {
    cachedLists.erase(it);
  }
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
