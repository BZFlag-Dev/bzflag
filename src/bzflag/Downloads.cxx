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


#ifdef HAVE_CURL


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


static bool authorizedServer(const std::string& url)
{
  // parse url
  std::string protocol, hostname, path, ip;
  int port = 80;
  if (BzfNetwork::parseURL(url, protocol, hostname, port, path) &&
      ((protocol == "http") || (protocol == "ftp")) &&
      (port >= 1) && (port <= 65535)) {
    // get the address
    Address address(hostname);
    ip = address.getDotNotation();
  }
  
  std::vector<std::string> nameAndIp;
  if (hostname.size() > 0) {
    nameAndIp.push_back(hostname);
  }
  if (ip.size() > 0) {
    nameAndIp.push_back(ip);
  }

  return DownloadAccessList.authorized(nameAndIp);
}


void doDownloads()
{
  const bool updateDownloads = BZDB.isTrue("updateDownloads");
  
  DownloadAccessList.reload();

  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set);
  
  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();

  URLManager& urlMgr = URLManager::instance();
  urlMgr.setProgressFunc(curlProgressFunc, NULL);
  
  bool authNotice = false;


  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFile(texUrl)) {

      // check access authorization
      if (!DownloadAccessList.alwaysAuthorized()) {
        if (!authorizedServer(texUrl)) {
          MATERIALMGR.setTextureLocal(texUrl, "");
          std::string msg = ColorStrings[RedColor];
          msg += "local server denial: ";
          msg += ColorStrings[GreyColor];
          msg += texUrl;
          addMessage(NULL, msg);
          authNotice = true;
          continue;
        }
      }

      // get last modification timestamp
      bool gotFileTime = false;
      time_t filetime = 0;
      if (updateDownloads) {
        gotFileTime = true;
        urlMgr.setProgressFunc(NULL, NULL);
        if (!urlMgr.getFileTime(texUrl, filetime)) {
          filetime = 0;
        }
        urlMgr.setProgressFunc(curlProgressFunc, NULL);
      }
      
      // use the cache?
      CacheManager::CacheRecord oldrec;
      if (CACHEMGR.findURL(texUrl, oldrec)) {
        if ((filetime <= oldrec.date) || !updateDownloads) {
          // use the cached file
          MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
          continue;
        }
      }

      std::string msg = ColorStrings[YellowColor];
      msg += "downloading: " + texUrl;
      addMessage(NULL, msg);
      
      // download the URL
      void* urlData;
      unsigned int urlSize;
      if (urlMgr.getURL(texUrl, &urlData, urlSize)) {
        // if we haven't gotten the filetime yet, do it now
        // FIXME - setup URLManager to get the filetime
        //         during downloads if it is requested?
        if (!gotFileTime) {
          urlMgr.setProgressFunc(NULL, NULL);
          if (!urlMgr.getFileTime(texUrl, filetime)) {
            filetime = 0;
          }
          urlMgr.setProgressFunc(curlProgressFunc, NULL);
        }
        
        // CACHEMGR generates name, usedDate, and key
        CacheManager::CacheRecord rec;
        rec.url = texUrl;
        rec.size = urlSize;
        rec.date = filetime;
        CACHEMGR.addFile(rec, urlData);
        MATERIALMGR.setTextureLocal(texUrl, rec.name);
        free(urlData);
      }
      else {
        MATERIALMGR.setTextureLocal(texUrl, "");
        msg = ColorStrings[RedColor] + "* download failed *";
        addMessage(NULL, msg);
      }
    }
  }
  
  urlMgr.setProgressFunc(NULL, NULL);
  CACHEMGR.saveIndex();

  // print a handy notice  
  if (authNotice) {
    std::string msg = ColorStrings[WhiteColor];
    msg += "NOTE: ";
    msg += ColorStrings[GreyColor];
    msg += "download access is controlled by ";
    msg += ColorStrings[YellowColor];
    msg += DownloadAccessList.getFileName();
    addMessage(NULL, msg);
  }

  return;
}


bool updateDownloads()
{
  bool updated = false;
  
  BzMaterialManager::TextureSet set;
  MATERIALMGR.makeTextureList(set);
  
  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();
  
  std::vector<CacheManager::CacheRecord> records = CACHEMGR.getCacheList();
  
  URLManager& urlMgr = URLManager::instance();
  urlMgr.setProgressFunc(curlProgressFunc, NULL);

  TextureManager& texMgr = TextureManager::instance();

  for (unsigned int i = 0; i < records.size(); i++) {
    CacheManager::CacheRecord& rec = records[i];

    // only update textures in the current world
    if (set.find(rec.url) == set.end()) {
      continue;
    }

    // get last modification timestamp
    time_t filetime = 0;
    urlMgr.setProgressFunc(NULL, NULL);
    if (!urlMgr.getFileTime(rec.url, filetime)) {
      continue;
    }
    urlMgr.setProgressFunc(curlProgressFunc, NULL);
    
    // download the file and update the cache
    if (filetime > rec.date) {
      std::string msg = ColorStrings[YellowColor];
      msg += "downloading: " + rec.url;
      addMessage(NULL, msg);
      
      void* urlData;
      unsigned int urlSize;
      if (urlMgr.getURL(rec.url, &urlData, urlSize)) {
        // CACHEMGR generates name, usedDate, and key
        rec.size = urlSize;
        rec.date = filetime;
        CACHEMGR.addFile(rec, urlData);
        texMgr.reloadTextureImage(rec.name);
        updated = true;
        free(urlData);
      } else {
        msg = ColorStrings[RedColor] + "* failure *";
        addMessage(NULL, msg);
      }
    }
  }
  
  urlMgr.setProgressFunc(NULL, NULL);
  CACHEMGR.saveIndex();
  
  return updated;
}

#else // HAVE_CURL

void doDownloads()
{
  return;
}

bool updateDownloads()
{
  return false;
}

#endif // HAVE_CURL


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
