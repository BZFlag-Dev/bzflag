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

/* common implementation headers */
#include "network.h"
#include "Address.h"
#include "AccessList.h"
#include "CacheManager.h"
#include "BzMaterial.h"
#include "URLManager.h"
#include "AnsiCodes.h"
#include "TextureManager.h"

/* local implementation headers */
#include "playing.h"
#include "HUDDialogStack.h"


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


/******************************************************************************/
#ifdef HAVE_CURL
/******************************************************************************/


// Function Prototypes
static void printAuthNotice();
static void setHudMessage(const std::string& msg);
static bool getFileTime(const std::string& url, time_t& t);
static bool getAndCacheURL(const std::string& url);
static bool authorizedServer(const std::string& hostname);
static bool checkAuthorizations(BzMaterialManager::TextureSet& set);


void Downloads::doDownloads()
{
  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();

  DownloadAccessList.reload();

  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, false /* ignore referencing */);

  const bool doDownloads =	BZDB.isTrue("doDownloads");
  const bool updateDownloads =  BZDB.isTrue("updateDownloads");

  // check hosts' access permissions
  bool authNotice = checkAuthorizations(set);

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {

      // use the cache?
      CacheManager::CacheRecord oldrec;
      if (CACHEMGR.findURL(texUrl, oldrec)) {
	time_t filetime = 0;
	if (doDownloads && updateDownloads) {
	  getFileTime(texUrl, filetime);
	}
	if (filetime <= oldrec.date) {
	  // use the cached file
	  MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
	  continue;
	}
      }

      // bail here if we can't download
      if (!doDownloads) {
	MATERIALMGR.setTextureLocal(texUrl, "");
	std::string msg = ColorStrings[GreyColor];
	msg += "not downloading: " + texUrl;
	addMessage(NULL, msg);
	continue;
      }

      // download and cache the URL
      if (getAndCacheURL(texUrl)) {
	const std::string localname = CACHEMGR.getLocalName(texUrl);
	MATERIALMGR.setTextureLocal(texUrl, localname);
      } else {
	MATERIALMGR.setTextureLocal(texUrl, "");
      }
    }
  }

  if (authNotice) {
    printAuthNotice();
  }

  CACHEMGR.saveIndex();

  return;
}


bool Downloads::updateDownloads(bool& rebuild)
{
  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();

  DownloadAccessList.reload();

  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, true /* only referenced materials */);

  TextureManager& TEXMGR = TextureManager::instance();

  rebuild = false;
  bool updated = false;

  // check hosts' access permissions
  bool authNotice = checkAuthorizations(set);

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {

      // use the cache or update?
      CacheManager::CacheRecord oldrec;
      if (CACHEMGR.findURL(texUrl, oldrec)) {
	time_t filetime;
	getFileTime(texUrl, filetime);
	if (filetime <= oldrec.date) {
	  // keep using the cached file
	  MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
	  if (!TEXMGR.isLoaded(oldrec.name)) {
	    rebuild = true;
	  }
	  continue;
	}
      }

      // download the file and update the cache
      if (getAndCacheURL(texUrl)) {
	updated = true;
	const std::string localname = CACHEMGR.getLocalName(texUrl);
	if (!TEXMGR.isLoaded(localname)) {
	  rebuild = true;
	} else {
	  TEXMGR.reloadTextureImage(localname); // reload with the new image
	}
	MATERIALMGR.setTextureLocal(texUrl, localname); // if it wasn't cached
      }
    }
  }

  if (authNotice) {
    printAuthNotice();
  }

  CACHEMGR.saveIndex();

  return updated;
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


static bool getFileTime(const std::string& url, time_t& t)
{
  setHudMessage("Update DNS check...");

  URLManager& URLMGR = URLManager::instance();
  if (URLMGR.getURLHeader(url)) {
    URLMGR.getFileTime(t);
    return true;
  } else {
    t = 0;
    return false;
  }

  setHudMessage("");
}


static bool getAndCacheURL(const std::string& url)
{
  bool result = false;

  setHudMessage("Download DNS check...");

  URLManager& URLMGR = URLManager::instance();
  URLMGR.setProgressFunc(curlProgressFunc, NULL);

  std::string msg = ColorStrings[GreyColor];
  msg += "downloading: " + url;
  addMessage(NULL, msg);

  void* urlData;
  unsigned int urlSize;

  if (URLMGR.getURL(url, &urlData, urlSize)) {
    time_t filetime;
    URLMGR.getFileTime(filetime);
    // CACHEMGR generates name, usedDate, and key
    CacheManager::CacheRecord rec;
    rec.url = url;
    rec.size = urlSize;
    rec.date = filetime;
    CACHEMGR.addFile(rec, urlData);
    free(urlData);
    result = true;
  }
  else {
    std::string msg = ColorStrings[RedColor] + "* failure *";
    addMessage(NULL, msg);
    result = false;
  }

  URLMGR.setProgressFunc(NULL, NULL);

  setHudMessage("");

  return result;
}


static bool authorizedServer(const std::string& hostname)
{
  setHudMessage("Access DNS check...");
  Address address(hostname); // get the address  (BLOCKING)
  std::string ip = address.getDotNotation();
  setHudMessage("");

  // make the list of strings to check
  std::vector<std::string> nameAndIp;
  if (hostname.size() > 0) {
    nameAndIp.push_back(hostname);
  }
  if (ip.size() > 0) {
    nameAndIp.push_back(ip);
  }

  return DownloadAccessList.authorized(nameAndIp);
}


static bool parseHostname(const std::string& url, std::string& hostname)
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
  
  std::map<std::string, bool> hostMap;
  std::map<std::string, bool>::iterator host_it;
  
  // get the list of hosts to check
  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& url = *set_it;
    std::string hostname;
    if (parseHostname(url, hostname)) {
      hostMap[hostname] = true;
    }
  }
  
  // check the hosts
  for (host_it = hostMap.begin(); host_it != hostMap.end(); host_it++) {
    const std::string& host = host_it->first;
    printf ("Checking host: %s ", host.c_str());//FIXME
    if (authorizedServer(host)) {
      host_it->second = true;
      printf ("passed\n");//FIXME
    } else {    
      host_it->second = false;
      printf ("failed\n");//FIXME
    }
  }
  
  // clear any unauthorized urls
  set_it = set.begin();
  while (set_it != set.end()) {
    BzMaterialManager::TextureSet::iterator next_it = set_it;
    next_it++;
    const std::string& url = *set_it;
    std::string hostname;
    if (parseHostname(url, hostname) && !hostMap[hostname]) {
      hostFailed = true;
      // remove the url
      set.erase(set_it);
      MATERIALMGR.setTextureLocal(url, "");
      // send a message
      std::string msg = ColorStrings[RedColor];
      msg += "local denial: ";
      msg += ColorStrings[GreyColor];
      msg += url;
      addMessage(NULL, msg);
    }
    set_it = next_it;
  }

  return hostFailed;  
}


static void setHudMessage(const std::string& msg)
{
  HUDDialogStack::get()->setFailedMessage(msg.c_str());
  drawFrame(0.0f);
  return;
}


/******************************************************************************/
#else // ! HAVE_CURL
/******************************************************************************/


void Downloads::doDownloads()
{
  bool needWarning = false;
  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, false /* ignore referencing */);

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {
      needWarning = true;
      // one time warning
      std::string msg = ColorStrings[GreyColor];
      msg += "not downloading: " + texUrl;
      addMessage(NULL, msg);
      // avoid future warnings
      MATERIALMGR.setTextureLocal(texUrl, "");
    }
  }

  if (needWarning && BZDB.isTrue("doDownloads")) {
    std::string msg = ColorStrings[RedColor];
    msg += "Downloads are not available for clients without libcurl";
    addMessage(NULL, msg);
    msg = ColorStrings[YellowColor];
    msg += "To disable this message, disable [Automatic Downloads] in";
    addMessage(NULL, msg);
    msg = ColorStrings[YellowColor];
    msg += "Options -> Cache Options, or get a client with libcurl";
    addMessage(NULL, msg);
  }

  return;
}


bool Downloads::updateDownloads(bool& /*rebuild*/)
{
  std::string msg = ColorStrings[RedColor];
  msg += "Downloads are not available for clients without libcurl";
  addMessage(NULL, msg);
  return false;
}


void Downloads::removeTextures()
{
  return;
}


/******************************************************************************/
#endif // HAVE_CURL
/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
