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

#ifndef	BZF_SERVER_LIST_CACHE_H
#define	BZF_SERVER_LIST_CACHE_H

#if defined(_WIN32)
  #pragma warning(disable: 4786)
#endif

#include "menus.h"


static const size_t MAX_STRING = 200; // size of description/name
typedef std::map<std::string, ServerItem> SRV_STR_MAP;

class ServerListCache {


public:

  ServerListCache();
  ~ServerListCache();
  static ServerListCache* get();
  void			saveCache();
  void			loadCache();
  void			setMaxCacheAge(time_t time);
  time_t		getMaxCacheAge();
  bool			clearCache();
  SRV_STR_MAP::iterator begin();
  SRV_STR_MAP::iterator end();
  SRV_STR_MAP::iterator find(std::string ServerAddress);
  void			insert(std::string serverAddress,ServerItem& info);
  void			incAddedNum();


public:

private:

private:
  SRV_STR_MAP		serverCache;
  time_t		maxCacheAge; // age after we don't show servers in cache
  int			cacheAddedNum; // how many items were added to cache
  static ServerListCache globalCache;
};


#endif // BZF_SERVER_LIST_CACHE_H
// ex: shiftwidth=2 tabstop=8
