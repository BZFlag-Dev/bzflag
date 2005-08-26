/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "Downloads.h"

/* system headers */
#include <map>
#include <vector>

/* common implementation headers */
#include "network.h"
#include "AccessList.h"
#include "CacheManager.h"
#include "BzMaterial.h"
#include "AnsiCodes.h"
#include "TextureManager.h"
#include "cURLManager.h"

/* local implementation headers */
#include "playing.h"
#include "HUDDialogStack.h"


// stupid globals for stupid file tracker
int totalTex = 0;
int currentTex = 0;
int runs = 0;



// FIXME - someone write a better explanation
static const char DownloadContent[] =
  "#\n"
  "# This file controls the access to servers for downloads.\n"
  "# Patterns are attempted in order against both the hostname\n"
  "# and ip. The first matching pattern sets the state. If no\n"
  "# patterns are matched, then the server is authorized. There\n"
  "# are four types of matches:\n"
  "#\n"
  "#   simple globbing (* and ?)\n"
  "#     allow\n"
  "#     deny\n"
  "#\n"
  "#   regular expressions\n"
  "#     allow_regex\n"
  "#     deny_regex\n"
  "#\n"
  "\n"
  "#\n"
  "# To authorize all servers, remove the last 3 lines.\n"
  "#\n"
  "\n"
  "allow *.bzflag.bz\n"
  "allow *.bzflag.org\n"
  "deny *\n";

static AccessList DownloadAccessList("DownloadAccess.txt", DownloadContent);

static bool       textureDownloading = false;


// Function Prototypes
static void printAuthNotice();
static bool checkAuthorizations(BzMaterialManager::TextureSet& set);


class CachedTexture : cURLManager {
public:
  CachedTexture(const std::string &texUrl);

  virtual void finalization(char *data, unsigned int length, bool good);

  static void  setParams(bool check, long timeout);
  static int   activeTransfer();
private:

  virtual void collectData(char* ptr, int len);

  std::string               url;
  static bool               checkForCache;
  static long               httpTimeout;
  static int                textureCounter;
  static int				byteTransferred;
  bool                      timeRequest;
};
bool CachedTexture::checkForCache   = false;
long CachedTexture::httpTimeout     = 0;
int CachedTexture::textureCounter = 0;
int CachedTexture::byteTransferred = 0;

CachedTexture::CachedTexture(const std::string &texUrl) : cURLManager()
{
  CacheManager::CacheRecord oldrec;

  setURL(texUrl);
  url         = texUrl;

  // use the cache?
  bool cached = CACHEMGR.findURL(texUrl, oldrec);
  if (cached && !checkForCache) {
    // use the cached file
    MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
  } else {
    textureCounter++;
    if (httpTimeout > 0.0)
      setTimeout(httpTimeout);
    setRequestFileTime(true);
    timeRequest = cached;
    std::string msg = ColorStrings[GreyColor];
    msg            += "downloading: " + url;
    addMessage(NULL, msg);
    if (cached) {
      // use the cached file -- just in case
      MATERIALMGR.setTextureLocal(url, oldrec.name);
      setTimeCondition(ModifiedSince, oldrec.date);
    }
    addHandle();
  }
}

void CachedTexture::setParams(bool check, long timeout)
{
  checkForCache   = check;
  httpTimeout     = timeout;
  textureCounter  = 0;
  byteTransferred = 0;
}

void CachedTexture::finalization(char *data, unsigned int length, bool good)
{
  time_t filetime;

  textureCounter--;
  if (good) {
    if (length) {
      getFileTime(filetime);
      // CACHEMGR generates name, usedDate, and key
      CacheManager::CacheRecord rec;
      rec.url  = url;
      rec.size = length;
      rec.date = filetime;
      CACHEMGR.addFile(rec, data);
      const std::string localname = CACHEMGR.getLocalName(url);
      TextureManager& TEXMGR = TextureManager::instance();
      if (TEXMGR.isLoaded(localname)) {
	TEXMGR.reloadTextureImage(localname); // reload with the new image
      }
      MATERIALMGR.setTextureLocal(url, localname);
    }
  } else {
    MATERIALMGR.setTextureLocal(url, "");
  }
}

int CachedTexture::activeTransfer()
{
  return textureCounter;
}

void CachedTexture::collectData(char* ptr, int len)
{
  char buffer[128];

  if(runs == 0)
	  totalTex = textureCounter;

  cURLManager::collectData(ptr, len);
  byteTransferred += len;

  //Make it so it counts textures in reverse order (0 to max instead of max to 0)
  currentTex = totalTex - textureCounter + 1;

  //Turn bytes into kilobytes
  sprintf (buffer, "Downloading texture (%d of %d): %d KB", currentTex, totalTex, byteTransferred/1024);
  runs++;

  HUDDialogStack::get()->setFailedMessage(buffer);
}

std::vector<CachedTexture*> cachedTexVector;

void Downloads::startDownloads(bool doDownloads, bool updateDownloads,
			       bool referencing)
{
  totalTex = 0;
  currentTex = 0;
  runs = 0;

  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();

  DownloadAccessList.reload();

  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, referencing);

  float timeout = 15;
  if (BZDB.isSet("httpTimeout")) {
    timeout = BZDB.eval("httpTimeout");
  }
  CachedTexture::setParams(updateDownloads, (long)timeout);

  // check hosts' access permissions
  bool authNotice = checkAuthorizations(set);

  if (!referencing) {
    // Clear old cached texture
    // This is the first time is called after joining
    int  texNo = cachedTexVector.size();
    for (int i = 0; i < texNo; i++)
      delete cachedTexVector[i];
    cachedTexVector.clear();
  }

  if (doDownloads)
    for (set_it = set.begin(); set_it != set.end(); set_it++) {
      const std::string& texUrl = set_it->c_str();
      if (CACHEMGR.isCacheFileType(texUrl)) {
	if (!referencing)
	  MATERIALMGR.setTextureLocal(texUrl, "");
	cachedTexVector.push_back(new CachedTexture(texUrl));
      }
    }
  else
    for (set_it = set.begin(); set_it != set.end(); set_it++) {
      const std::string& texUrl = set_it->c_str();
      if (CACHEMGR.isCacheFileType(texUrl)) {

	// use the cache?
	CacheManager::CacheRecord oldrec;
	if (CACHEMGR.findURL(texUrl, oldrec)) {
	  // use the cached file
	  MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
	} else {
	  // bail here if we can't download
	  MATERIALMGR.setTextureLocal(texUrl, "");
	  std::string msg = ColorStrings[GreyColor];
	  msg += "not downloading: " + texUrl;
	  addMessage(NULL, msg);
	}
      }
    }

  if (authNotice) {
    printAuthNotice();
  }
  textureDownloading = true;
}

void Downloads::finalizeDownloads()
{
  textureDownloading = false;

  int  texNo = cachedTexVector.size();
  for (int i = 0; i < texNo; i++)
    delete cachedTexVector[i];
  cachedTexVector.clear();

  CACHEMGR.saveIndex();
}

bool Downloads::requestFinalized()
{
  return textureDownloading && (CachedTexture::activeTransfer() == 0);
}

void Downloads::removeTextures()
{
  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, false /* ignore referencing */);

  TextureManager& TEXMGR = TextureManager::instance();

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {
      const std::string& localname = CACHEMGR.getLocalName(texUrl);
      if (TEXMGR.isLoaded(localname)) {
	TEXMGR.removeTexture(localname);
      }
    }
  }

  return;
}


static void printAuthNotice()
{
  std::string msg = ColorStrings[WhiteColor];
  msg += "NOTE: ";
  msg += ColorStrings[GreyColor];
  msg += "download access is controlled by ";
  msg += ColorStrings[YellowColor];
  msg += DownloadAccessList.getFileName();
  addMessage(NULL, msg);
  return;
}


bool authorizedServer(const std::string& hostname)
{
  // Don't do here a DNS lookup, it can block the client
  // DNS is temporary removed until someone code it unblocking

  // make the list of strings to check
  std::vector<std::string> nameAndIp;
  if (hostname.size() > 0) {
    nameAndIp.push_back(hostname);
  }

  return DownloadAccessList.authorized(nameAndIp);
}


bool parseHostname(const std::string& url, std::string& hostname)
{
  std::string protocol, path, ip;
  int port;
  if (BzfNetwork::parseURL(url, protocol, hostname, port, path)) {
    if ((protocol == "http") || (protocol == "ftp")) {
      return true;
    }
  }
  return false;
}


static bool checkAuthorizations(BzMaterialManager::TextureSet& set)
{
  // avoid the DNS lookup
  if (DownloadAccessList.alwaysAuthorized()) {
    return false;
  }
  
  bool hostFailed = false;
  
  BzMaterialManager::TextureSet::iterator set_it;
  
  std::map<std::string, bool> hostAccess;
  std::map<std::string, bool>::iterator host_it;
  
  // get the list of hosts to check
  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& url = *set_it;
    std::string hostname;
    if (parseHostname(url, hostname)) {
      hostAccess[hostname] = true;
    }
  }
  
  // check the hosts
  for (host_it = hostAccess.begin(); host_it != hostAccess.end(); host_it++) {
    const std::string& host = host_it->first;
    host_it->second = authorizedServer(host);
  }
  
  // clear any unauthorized urls
  set_it = set.begin();
  while (set_it != set.end()) {
    BzMaterialManager::TextureSet::iterator next_it = set_it;
    next_it++;
    const std::string& url = *set_it;
    std::string hostname;
    if (parseHostname(url, hostname) && !hostAccess[hostname]) {
      hostFailed = true;
      // send a message
      std::string msg = ColorStrings[RedColor];
      msg += "local denial: ";
      msg += ColorStrings[GreyColor];
      msg += url;
      addMessage(NULL, msg);
      // remove the url
      MATERIALMGR.setTextureLocal(url, "");
      set.erase(set_it);
    }
    set_it = next_it;
  }

  return hostFailed;  
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
