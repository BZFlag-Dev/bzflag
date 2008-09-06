/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HUDUISERVERLISTCACHE_H__
#define __HUDUISERVERLISTCACHE_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <string>

/* common interface headers */
#include "HUDuiServerList.h"
#include "Singleton.h"

/** The ServerList class contains links to the list server as well as
 * any fetched list of servers.  The list handles cacheing of those
 * server entries in case of list server unavailability.
 */
class HUDuiServerListCache : public Singleton<HUDuiServerListCache> {

public:
  HUDuiServerListCache();
  ~HUDuiServerListCache();

  void	saveCache();
  bool	loadCache();
  void	clearCache();

  std::vector<std::pair<HUDuiServerList*, std::string> > readCachedLists() { return cachedLists; }
  
  void addNewList(HUDuiServerList* newList, std::string tabName);
  void removeList(HUDuiServerList* list, std::string tabName);

protected:
  friend class Singleton<HUDuiServerListCache>;

private:
  void applySort(int index, int _sort, bool reverse);
  std::string getCacheFilename() const;
  static const size_t max_string = 200;
  std::vector<std::pair<HUDuiServerList*, std::string> > cachedLists;
  bool cacheLoaded;
};

#endif  /* __SERVERLIST_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
