/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef	BZF_SERVER_LIST_CACHE_H
#define	BZF_SERVER_LIST_CACHE_H

#include "common.h"

/* system interface headers */
#include <string>
#include <map>

/* local interface headers */
#include "ServerItem.h"


/** The ServerListCache is a simple aging container of server entries.
 * The class can load from and safe to file.  Entries are culled based
 * on a specified cache age.
 */
class ServerListCache {

public:
  /** size of description/name */
  static const size_t max_string = 200;
  
  /** convenience map type */
  typedef std::map<std::string, ServerItem> SRV_STR_MAP;


  ServerListCache();
  ~ServerListCache();

  /** returns a pointer to the global instance of the list cache */
  static ServerListCache* get();

  /** save the cache to file */
  void			saveCache();

  /** read the cache from file */
  void			loadCache();

  /** set the max age to # of minutes after which items in cache.  are
   * this old they are no longer shown on the find menu-- 0
   * disables */
  void setMaxCacheAge(time_t time);

  /** get the set maximum cache age */
  time_t		getMaxCacheAge();

  /** clear the server list cache */
  bool			clearCache();

  /** first item in the cache.  this is a wrapper that allows access
   * to the maps begin method */
  SRV_STR_MAP::iterator begin();

  /** last item in the cache.  this is a wrapper that allows access to
   * the maps end method */
  SRV_STR_MAP::iterator end();

  /** search for some address in the cache.  this is a wrapper that
   * allows access to the maps find method */
  SRV_STR_MAP::iterator find(const std::string &ServerAddress);

  /** add an entry to the cache list */
  void			insert(const std::string &serverAddress, const ServerItem &info);

  /** is given server in cache and marked as favorite? */
  bool                  isFavorite(const std::string &serverAddress) const;

private:
  /** the full path of the file the cache is stored in **/
  std::string getCacheFilename(int subrevision) const;

  /** the actual map container of entries */
  SRV_STR_MAP		serverCache;

  /** age after we don't show servers in cache */
  time_t		maxCacheAge;

  /** one cache to rule them all */
  static ServerListCache globalCache;
};


#endif // BZF_SERVER_LIST_CACHE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

