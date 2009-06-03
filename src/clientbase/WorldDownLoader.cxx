/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* local implementation headers */
#include "WorldDownLoader.h"

// system includes
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#  include <shlobj.h>
#  include <direct.h>
#else
#  include <pwd.h>
#  include <dirent.h>
#  include <utime.h>
#endif

/* common headers */
#include "Downloads.h"

/* local implementation headers */
#include "playing.h"
#include "clientvars.h"

WorldDownLoader::WorldDownLoader() :
  worldUrl(""),
  worldHash(""),
  worldCachePath(""),
  md5Digest(""),
  worldPtr(0),
  worldDatabase(NULL),
  isCacheTemp(false),
  cacheOut(NULL),
	cURLManager()
{
}

WorldDownLoader::~WorldDownLoader()
{
  if (worldBuilder) {
    delete worldBuilder;
    worldBuilder = NULL;
  }
  if (worldDatabase) {
    delete[] worldDatabase;
    worldDatabase = NULL;
  }
  if (cacheOut) {
    delete cacheOut;
    cacheOut = NULL;
  }
}

void WorldDownLoader::start(char *hexDigest)
{
  md5Digest = &hexDigest[1];
  if (isCached(hexDigest)) {
    loadCached();
  } else if (worldUrl.size()) {
    showError(("Loading world from " + worldUrl).c_str());
    setProgressFunction(curlProgressFunc, (char*)worldUrl.c_str());
    setURL(worldUrl);
    addHandle();
    worldUrl = ""; // clear the state
  } else {
    askToBZFS();
  }
}

void WorldDownLoader::stop()
{
  if (joiningGame) {
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    if (worldDatabase) {
      delete[] worldDatabase;
      worldDatabase = NULL;
    }
  }
}

void WorldDownLoader::setCacheURL(char *cacheURL)
{
  worldUrl = cacheURL;
}


void WorldDownLoader::setCacheTemp(bool cacheTemp)
{
  isCacheTemp = cacheTemp;
}


uint32_t WorldDownLoader::processChunk(void *buf, uint16_t len, int bytesLeft)
{
  int totalSize = worldPtr + len + bytesLeft;
  int doneSize  = worldPtr + len;
  if (cacheOut)
    cacheOut->write((char *)buf, len);
  
  showError(TextUtils::format("Downloading World (%2d%% complete/%d kb remaining)...",
    (100 * doneSize / totalSize),bytesLeft / 1024).c_str());
  
  if(bytesLeft == 0) {
    if (cacheOut) {
      delete cacheOut;
      cacheOut = NULL;
    }
    loadCached();
    if (isCacheTemp)
      markOld(worldCachePath);
  }
  
  return worldPtr;
}


void WorldDownLoader::cleanCache()
{
  // setup the world cache limit
  int cacheLimit = (10 * 1024 * 1024);
  if (BZDB.isSet("worldCacheLimit")) {
    const int dbCacheLimit = BZDB.evalInt("worldCacheLimit");
    // the old limit was 100 Kbytes, too small
    if (dbCacheLimit == (100 * 1024))
      BZDB.setInt("worldCacheLimit", cacheLimit);
    else
      cacheLimit = dbCacheLimit;
  } else {
    BZDB.setInt("worldCacheLimit", cacheLimit);
  }

  const std::string worldPath = getCacheDirName();

  while (true) {
    char *oldestFile = NULL;
    int oldestSize = 0;
    int totalSize = 0;

#ifdef _WIN32
    std::string pattern = worldPath + "*.bwc";
    WIN32_FIND_DATA findData;
    HANDLE h = FindFirstFile(pattern.c_str(), &findData);
    if (h != INVALID_HANDLE_VALUE) {
      FILETIME oldestTime;
      while (FindNextFile(h, &findData)) {
	if ((oldestFile == NULL) ||
	  (CompareFileTime(&oldestTime, &findData.ftLastAccessTime) > 0)) {
	    if (oldestFile)
	      free(oldestFile);

	    oldestFile = strdup(findData.cFileName);
	    oldestSize = findData.nFileSizeLow;
	    oldestTime = findData.ftLastAccessTime;
	}
	totalSize += findData.nFileSizeLow;
      }
      FindClose(h);
    }
#else
    DIR *directory = opendir(worldPath.c_str());
    if (directory) {
      struct dirent *contents;
      struct stat statbuf;
      time_t oldestTime = time(NULL);
      while ((contents = readdir(directory))) {
	const std::string filename = contents->d_name;
	const std::string fullname = worldPath + filename;
	stat(fullname.c_str(), &statbuf);
	if (S_ISREG(statbuf.st_mode) && (filename.size() > 4) &&
	  (filename.substr(filename.size() - 4) == ".bwc")) {
	    if ((oldestFile == NULL) || (statbuf.st_atime < oldestTime)) {
	      if (oldestFile) {
		free(oldestFile);
	      }
	      oldestFile = strdup(contents->d_name);
	      oldestSize = statbuf.st_size;
	      oldestTime = statbuf.st_atime;
	    }
	    totalSize += statbuf.st_size;
	}
      }
      closedir(directory);
    }
#endif

    // any valid cache files?
    if (oldestFile == NULL)
      return;

    // is the cache small enough?
    if (totalSize < cacheLimit) {
      if (oldestFile != NULL) {
	free(oldestFile);
	oldestFile = NULL;
      }
      return;
    }

    // remove the oldest file
    logDebugMessage(1, "cleanWorldCache: removed %s\n", oldestFile);
    remove((worldPath + oldestFile).c_str());
    free(oldestFile);
    totalSize -= oldestSize;
  }
}


void WorldDownLoader::askToBZFS()
{
  // Why do we use the error status for this?
  showError("Downloading World...");
  char message[MaxPacketLen];
  // ask for world
  nboPackUInt32(message, 0);
  serverLink->send(MsgGetWorld, sizeof(uint32_t), message);
  worldPtr = 0;
  if (cacheOut) {
    delete cacheOut;
    cacheOut = NULL;
  }
  cacheOut = FILEMGR.createDataOutStream(worldCachePath, true, true);
}


bool WorldDownLoader::isCached(char *hexDigest)
{
  std::istream *cachedWorld;
  bool cached    = false;
  worldHash = hexDigest;
  worldCachePath = getCacheDirName();
  worldCachePath += hexDigest;
  worldCachePath += ".bwc";
  if ((cachedWorld = FILEMGR.createDataInStream(worldCachePath, true))) {
    cached = true;
    delete cachedWorld;
    cachedWorld = NULL;
  }
  return cached;
}


void WorldDownLoader::loadCached()
{
  // can't get a cache from nothing
  if (worldCachePath.size() == 0) {
    joiningGame = false;
    return;
  }

  // lookup the cached world
  std::istream *cachedWorld = FILEMGR.createDataInStream(worldCachePath, true);
  if (!cachedWorld) {
    showError("World cache files disappeared.  Join canceled",true);
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }

  // status update
  showError("Loading world into memory...",true);

  // get the world size
  cachedWorld->seekg(0, std::ios::end);
  std::streampos size = cachedWorld->tellg();
  unsigned long charSize = std::streamoff(size);

  // load the cached world
  cachedWorld->seekg(0);
  char *localWorldDatabase = new char[charSize];
  if (!localWorldDatabase) {
    showError("Error loading cached world.  Join canceled",true);
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }
  cachedWorld->read(localWorldDatabase, charSize);
  delete cachedWorld;
  cachedWorld = NULL;

  // verify
  showError("Verifying world integrity...",true);
  MD5 md5;
  md5.update((unsigned char *)localWorldDatabase, charSize);
  md5.finalize();
  std::string digest = md5.hexdigest();
  if (digest != md5Digest) {
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    delete[] localWorldDatabase;
    localWorldDatabase = NULL;
    showError("Error on md5. Removing offending file.");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }

  // make world
  showError("Preparing world...",true);
  if (!worldBuilder->unpack(localWorldDatabase)) {
    // world didn't make for some reason
    if (worldBuilder) {
      delete worldBuilder;
      worldBuilder = NULL;
    }
    delete[] localWorldDatabase;
    localWorldDatabase = NULL;
    showError("Error unpacking world database. Join canceled.");
    remove(worldCachePath.c_str());
    joiningGame = false;
    return;
  }
  delete[] localWorldDatabase;
  localWorldDatabase = NULL;

  // return world
  if (worldBuilder) {
    world = worldBuilder->getWorld();
    world->setMapHash(worldHash);
    delete worldBuilder;
    worldBuilder = NULL;
  }

  showError("Downloading files...");

  const bool doDownloads = BZDB.isTrue("doDownloads");
  const bool updateDownloads =  BZDB.isTrue("updateDownloads");
  Downloads::instance().startDownloads(doDownloads, updateDownloads, false);
  downloadingData  = true;
}


void WorldDownLoader::markOld(std::string &fileName)
{
#ifdef _WIN32
  FILETIME ft;
  HANDLE h = CreateFile(fileName.c_str(),
    FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA, 0, NULL,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h != INVALID_HANDLE_VALUE) {
    SYSTEMTIME st;
    memset(&st, 0, sizeof(st));
    st.wYear = 1900;
    st.wMonth = 1;
    st.wDay = 1;
    SystemTimeToFileTime(&st, &ft);
    SetFileTime(h, &ft, &ft, &ft);
    GetLastError();
    CloseHandle(h);
  }
#else
  struct utimbuf times;
  times.actime = 0;
  times.modtime = 0;
  utime(fileName.c_str(), &times);
#endif
}


void WorldDownLoader::finalization(char *data, unsigned int length, bool good)
{
  if (good) {
    worldDatabase = data;
    theData       = NULL;
    MD5 md5;
    md5.update((unsigned char *)worldDatabase, length);
    md5.finalize();
    std::string digest = md5.hexdigest();
    if (digest != md5Digest) {
      showError("Download from URL failed");
      askToBZFS();
    } else {
      std::ostream *cache =
	FILEMGR.createDataOutStream(worldCachePath, true, true);
      if (cache != NULL) {
	cache->write(worldDatabase, length);
	delete cache;
	cache = NULL;
	loadCached();
      } else {
	showError("Problem writing cache");
	askToBZFS();
      }
    }
  } else {
    askToBZFS();
  }
}
