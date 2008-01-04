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

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include "common.h"

/* system interface headers */
#include <time.h>
#include <string>
#include <vector>

/* common interface headers */
#include "Singleton.h"


#define CACHEMGR (CacheManager::instance())


class CacheManager : public Singleton<CacheManager> {
 public:

  typedef struct {
    std::string url;
    time_t usedDate;
    std::string name;
    int size;
    time_t date;
    std::string key;
  } CacheRecord;

  static bool isCacheFileType(const std::string name);

  void setCacheDirectory(const std::string dir);
  std::string getLocalName(const std::string name) const;

  bool loadIndex();
  bool saveIndex();

  bool findURL(const std::string& url, CacheRecord& record);
  bool addFile(CacheRecord& rec, const void* data);

  std::vector<CacheRecord> getCacheList() const;

  void limitCacheSize();

 protected:
  friend class Singleton<CacheManager>;

 private:
  CacheManager();
  ~CacheManager();

  int findRecord(const std::string& url);

  std::string cacheDir;
  std::string indexName;
  std::vector<CacheRecord> records;
};

#endif  /* __CACHEMANAGER_H__ */

/*
 * Local Variables: ***
 * mode: C++ ***
 * tab-width: 8 ***
 * c-basic-offset: 2 ***
 * indent-tabs-mode: t ***
 * End: ***
 * ex: shiftwidth=2 tabstop=8
 */
