/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"

/* class interface header */
#include "AccessControlList.h"

/* system interface headers */
#include <time.h>
#include <ctype.h>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

/* common interface headers */
#include "global.h"
#include "bzglob.h"
#include "network.h"
#include "Address.h"
#include "TimeKeeper.h"
#include "TextUtils.h"

// bzfs specific headers
#include "bzfs.h"


void AccessControlList::ban(in_addr &ipAddr, const char *bannedBy, int period,
                            const char *reason, bool fromMaster)
{
  BanInfo toban(ipAddr, bannedBy, period,fromMaster);
  if (reason) toban.reason = reason;
  banList_t::iterator oldit = std::find(banList.begin(), banList.end(), toban);
  if (oldit != banList.end()) // IP already in list? -> replace
    *oldit = toban;
  else
    banList.push_back(toban);
}


bool AccessControlList::ban(std::string &ipList, const char *bannedBy, int period,
                            const char *reason, bool fromMaster)
{
  return ban(ipList.c_str(), bannedBy, period, reason,fromMaster);
}


bool AccessControlList::ban(const char *ipList, const char *bannedBy, int period,
                            const char *reason, bool fromMaster)
{
  char *buf = strdup(ipList);
  char *pStart = buf;
  char *pSep;
  bool added = false;

  in_addr mask;
  while ((pSep = strchr(pStart, ',')) != NULL) {
    *pSep = 0;
    if (convert(pStart, mask)) {
      ban(mask, bannedBy, period, NULL, fromMaster);
      added = true;
    }
    *pSep = ',';
    pStart = pSep + 1;
  }
  if (convert(pStart, mask)) {
    ban(mask, bannedBy, period, reason, fromMaster);
    added = true;
  }
  free(buf);
  return added;
}


void AccessControlList::hostBan(std::string hostpat, const char *bannedBy, int period,
                                const char *reason, bool fromMaster)
{
  HostBanInfo toban(hostpat, bannedBy, period,fromMaster);
  if (reason) toban.reason = reason;
  hostBanList_t::iterator oldit = std::find(hostBanList.begin(), hostBanList.end(), toban);
  if (oldit != hostBanList.end()) {
    *oldit = toban;
  } else {
    hostBanList.push_back(toban);
  }
}


void AccessControlList::idBan(std::string idpat, const char *bannedBy, int period,
                              const char *reason, bool fromMaster)
{
  IdBanInfo toban(idpat, bannedBy, period, fromMaster);
  if (reason) toban.reason = reason;
  idBanList_t::iterator oldit = std::find(idBanList.begin(), idBanList.end(), toban);
  if (oldit != idBanList.end()) {
    *oldit = toban;
  } else {
    idBanList.push_back(toban);
  }
}


bool AccessControlList::unban(in_addr &ipAddr)
{
  banList_t::iterator it = std::remove(banList.begin(), banList.end(), BanInfo(ipAddr));
  if (it != banList.end()) {
    banList.erase(it, banList.end());
    return true;
  }
  return false;
}


bool AccessControlList::unban(std::string &ipList)
{
  return unban(ipList.c_str());
}


bool AccessControlList::unban(const char *ipList)
{
  char *buf = strdup(ipList);
  char *pStart = buf;
  char *pSep;
  bool success = false;

  in_addr mask;
  while ((pSep = strchr(pStart, ',')) != NULL) {
    *pSep = 0;
    if (convert(pStart, mask))
      success|=unban(mask);
    *pSep = ',';
    pStart = pSep + 1;
  }
  if (convert(pStart, mask))
    success|=unban(mask);
  free(buf);
  return success;
}


bool AccessControlList::hostUnban(std::string hostpat)
{
  hostBanList_t::iterator it = std::remove(hostBanList.begin(), hostBanList.end(), HostBanInfo(hostpat));
  if (it != hostBanList.end()) {
    hostBanList.erase(it, hostBanList.end());
    return true;
  }
  return false;
}


bool AccessControlList::idUnban(std::string idpat)
{
  idBanList_t::iterator it = std::remove(idBanList.begin(), idBanList.end(), IdBanInfo(idpat));
  if (it != idBanList.end()) {
    idBanList.erase(it, idBanList.end());
    return true;
  }
  return false;
}


bool AccessControlList::validate(const in_addr &ipAddr, BanInfo *info)
{
  expire();

  for (banList_t::iterator it = banList.begin(); it != banList.end(); ++it) {
    in_addr mask = it->addr;

    if ((ntohl(mask.s_addr) & 0x00ffffff) == 0x00ffffff)
      mask.s_addr = htonl((ntohl(mask.s_addr) & 0xff000000) | (ntohl(ipAddr.s_addr) & 0x00ffffff));
    else if ((ntohl(mask.s_addr) & 0x0000ffff) == 0x0000ffff)
      mask.s_addr = htonl((ntohl(mask.s_addr) & 0xffff0000) | (ntohl(ipAddr.s_addr) & 0x0000ffff));
    else if ((ntohl(mask.s_addr) & 0x000000ff) == 0x000000ff)
      mask.s_addr = htonl((ntohl(mask.s_addr) & 0xffffff00) | (ntohl(ipAddr.s_addr) & 0x000000ff));

    if (mask.s_addr == ipAddr.s_addr)	{
      if (info)
	*info = *it;
      return false;
    }
  }
  return true;
}


bool AccessControlList::does_match(const char *targ, int targlen, const char *pat, int patlen)
{
  if (!targlen)
    return patlen == 0;
  if (!patlen)
    return targlen == 0;

  while (*pat != '*') {
    if (*pat != *targ)
      return false;

    pat++; patlen--;
    targ++; targlen--;
    if (!targlen)
      return patlen == 0;
    if (!patlen)
      return targlen == 0;
  }

  // found a *, search for matches in the rest of the string
  for (int pos = 0; pos <= targlen; pos++)
    if (does_match(targ+pos, targlen-pos, pat+1, patlen-1))
      return true;
  return false;
}


bool AccessControlList::hostValidate(const char *hostname, HostBanInfo *info)
{
  expire();

  for (hostBanList_t::iterator it = hostBanList.begin(); it != hostBanList.end(); ++it) {
    if (does_match(hostname, strlen(hostname), it->hostpat.c_str(), it->hostpat.length())) {
      if (info)
	*info = *it;
      return false;
    }
  }

  return true;
}


bool AccessControlList::idValidate(const char *id, IdBanInfo *info)
{
  expire();
  if (strlen(id) == 0) {
    return true;
  }
  for (idBanList_t::iterator it = idBanList.begin(); it != idBanList.end(); ++it) {
    if (strcmp(id, it->idpat.c_str()) == 0) {
      if (info)
	*info = *it;
      return false;
    }
  }

  return true;
}


static std::string makeGlobPattern(const char* str)
{
  if (str == NULL) {
    return "*";
  }
  while ((*str != '\0') && isspace(*str)) str++;
  if (*str == '\0') {
    return "*";
  }
  std::string pattern = str;
  pattern = TextUtils::toupper(pattern);
  if (pattern.find('*') == std::string::npos) {
    pattern = "*" + pattern + "*";
  }
  printf ("PATTERN = \"%s\"\n", pattern.c_str());
  return pattern;
}


static std::string getBanMaskString(in_addr mask)
{
  std::ostringstream os;
  os << (ntohl(mask.s_addr) >> 24) << '.';
  if ((ntohl(mask.s_addr) & 0x00ffffff) == 0x00ffffff) {
    os << "*.*.*";
  } else {
    os << ((ntohl(mask.s_addr) >> 16) & 0xff) << '.';
    if ((ntohl(mask.s_addr) & 0x0000ffff) == 0x0000ffff) {
      os << "*.*";
    } else {
      os << ((ntohl(mask.s_addr) >> 8) & 0xff) << '.';
      if ((ntohl(mask.s_addr) & 0x000000ff) == 0x000000ff)
        os << "*";
      else
        os << (ntohl(mask.s_addr) & 0xff);
    }
  }
  return os.str();
}


void AccessControlList::sendBan(PlayerId id, const BanInfo &baninfo)
{
  std::ostringstream os;
  os << getBanMaskString(baninfo.addr);

  // print duration when < 1 year
  double duration = baninfo.banEnd - TimeKeeper::getCurrent();
  if (duration < 365.0f * 24 * 3600)
    os << std::setiosflags(std::ios::fixed) << std::setprecision(1)
       << " (" << duration/60 << " minutes)";
  if( baninfo.fromMaster )
    os << " (m)";
  if (baninfo.bannedBy.length())
    os << " banned by: " << baninfo.bannedBy;
  sendMessage(ServerPlayer, id, os.str().c_str());

  // add reason, if any
  if (baninfo.reason.size()) {
    std::ostringstream ost;
    ost << "   reason: " << baninfo.reason;
    sendMessage(ServerPlayer, id, ost.str().c_str());
  }
}


void AccessControlList::sendBans(PlayerId id, const char* pattern)
{
  expire();
  sendMessage(ServerPlayer, id, "IP Ban List");
  sendMessage(ServerPlayer, id, "-----------");
  
  const std::string glob = makeGlobPattern(pattern);
  
  for (banList_t::iterator it = banList.begin(); it != banList.end(); ++it) {
    const BanInfo& bi = *it;
    if (!glob_match(glob, getBanMaskString(bi.addr)) &&
        !glob_match(glob, TextUtils::toupper(bi.reason)) &&
        !glob_match(glob, TextUtils::toupper(bi.bannedBy))) {
      continue;
    }
    sendBan(id, *it);
  }
}


void AccessControlList::sendHostBans(PlayerId id, const char* pattern)
{
  expire();
  sendMessage(ServerPlayer, id, "Host Ban List");
  sendMessage(ServerPlayer, id, "-------------");

  const std::string glob = makeGlobPattern(pattern);

  char banlistmessage[MessageLen];
  for (hostBanList_t::iterator it = hostBanList.begin(); it != hostBanList.end(); ++it) {

    const HostBanInfo& bi = *it;
    if (!glob_match(glob, TextUtils::toupper(bi.hostpat)) &&
        !glob_match(glob, TextUtils::toupper(bi.reason)) &&
        !glob_match(glob, TextUtils::toupper(bi.bannedBy))) {
      continue;
    }
    
    char *pMsg = banlistmessage;
    sprintf(pMsg, "%s", bi.hostpat.c_str());

    // print duration when < 1 year
    double duration = bi.banEnd - TimeKeeper::getCurrent();
    if (duration < 365.0f * 24 * 3600)
      sprintf(pMsg + strlen(pMsg)," (%.1f minutes)", duration / 60);
    if (bi.bannedBy.length())
      sprintf(pMsg + strlen(pMsg), " banned by: %s", bi.bannedBy.c_str());
    if (bi.fromMaster)
      sprintf(pMsg + strlen(pMsg), "(m)");

    sendMessage(ServerPlayer, id, banlistmessage);

    // add reason, if any
    if (bi.reason.size()) {
      pMsg = banlistmessage;
      sprintf(pMsg, "   reason: %s", bi.reason.c_str());
      sendMessage(ServerPlayer, id, banlistmessage);
    }
  }
}


void AccessControlList::sendIdBans(PlayerId id, const char* pattern)
{
  expire();
  sendMessage(ServerPlayer, id, "BZID Ban List");
  sendMessage(ServerPlayer, id, "-------------");

  const std::string glob = makeGlobPattern(pattern);

  char banlistmessage[MessageLen];
  for (idBanList_t::iterator it = idBanList.begin(); it != idBanList.end(); ++it) {

    const IdBanInfo& bi = *it;
    if (!glob_match(glob, TextUtils::toupper(bi.idpat)) &&
        !glob_match(glob, TextUtils::toupper(bi.reason)) &&
        !glob_match(glob, TextUtils::toupper(bi.bannedBy))) {
      continue;
    }
    
    char *pMsg = banlistmessage;
    
    bool useQuotes = (bi.idpat.find_first_of(" \t") != std::string::npos);
    if (useQuotes) {
      sprintf(pMsg, "\"%s\"", bi.idpat.c_str());
    } else {
      sprintf(pMsg, "%s", bi.idpat.c_str());
    }

    // print duration when < 1 year
    double duration = bi.banEnd - TimeKeeper::getCurrent();
    if (duration < 365.0f * 24 * 3600)
      sprintf(pMsg + strlen(pMsg)," (%.1f minutes)", duration / 60);
    if (bi.bannedBy.length())
      sprintf(pMsg + strlen(pMsg), " banned by: %s", bi.bannedBy.c_str());
    if (bi.fromMaster)
      sprintf(pMsg + strlen(pMsg), "(m)");

    sendMessage(ServerPlayer, id, banlistmessage);

    // add reason, if any
    if (bi.reason.size()) {
      pMsg = banlistmessage;
      sprintf(pMsg, "   reason: %s", bi.reason.c_str());
      sendMessage(ServerPlayer, id, banlistmessage);
    }
  }
}


bool AccessControlList::load() {

  if (banFile.size() == 0)
    return true;

  // try to open the ban file
  std::ifstream is(banFile.c_str());
  if (!is.good())
    // file does not exist, but that's OK, we'll create it later if needed
    return true;

  // clear all current bans
  banList.clear();

  // try to read ban entries
  std::string ipAddress, hostpat, bzId, bannedBy, reason, tmp;
  long banEnd;
  is >> std::ws;
  while (!is.eof()) {
    is >> ipAddress;
    if (ipAddress == "host:") {
      is >> hostpat;
    } else if (ipAddress == "bzid:") {
      is.ignore(1);
      std::getline(is, bzId);
    }
    is >> tmp;
    if (tmp != "end:") {
      DEBUG3("Banfile: bad 'end:' line\n");
      return false;
    }
    is >> banEnd;
    if (banEnd != 0) {
      // banEnd is absolute time - get delay from now, in minute
      // ban command use minute as ban time
      banEnd -= long(time(NULL));
      banEnd /= 60;
      if (banEnd == 0) banEnd = -1;
    }
    is >> tmp;
    if (tmp != "banner:") {
      DEBUG3("Banfile: bad 'banner:' line\n");
      return false;
    }
    is.ignore(1);
    std::getline(is, bannedBy);
    is >> tmp;
    if (tmp != "reason:") {
      DEBUG3("Banfile: bad 'reason:' line\n");
      return false;
    }
    is.ignore(1);
    std::getline(is, reason);
    is >> std::ws;
    if (banEnd < 0) continue;
    if (ipAddress == "host:") {
      hostBan(hostpat, (bannedBy.size() ? bannedBy.c_str(): NULL), banEnd,
              (reason.size() > 0 ? reason.c_str() : NULL));
    } else if (ipAddress == "bzid:") {
      idBan(bzId, (bannedBy.size() ? bannedBy.c_str(): NULL), banEnd,
            (reason.size() > 0 ? reason.c_str() : NULL));
    } else {
      std::string::size_type n;
      while ((n = ipAddress.find('*')) != std::string::npos) {
	ipAddress.replace(n, 1, "255");
      }
      if (!ban(ipAddress, (bannedBy.size() ? bannedBy.c_str(): NULL), banEnd,
	       (reason.size() > 0 ? reason.c_str() : NULL))) {
        DEBUG3("Banfile: bad ban\n");
	return false;
      }
    }
  }
  return true;
}


int AccessControlList::merge(const std::string& banData) {
  if (!banData.size()) {
    return 0;
  }
  int bansAdded = 0;
  std::stringstream is(banData,std::stringstream::in);

  // try to read ban entries
  std::string ipAddress, hostpat, bzId, bannedBy, reason, tmp;
  long banEnd;
  is>>std::ws;
  while (!is.eof()) {
    is >> ipAddress;
    if (ipAddress == "host:") {
      is >> hostpat;
    } else if (ipAddress == "bzid:") {
      is.ignore(1);
      std::getline(is, bzId);
    }
    is >> tmp;
    if (tmp != "end:") {
      DEBUG3("Banfile: bad 'end:' line\n");
      return bansAdded;
    }
    is >> banEnd;
    if (banEnd != 0) {
      banEnd -= long(time(NULL));
      banEnd /= 60;
      if (banEnd == 0)
	banEnd = -1;
    }
    is >> tmp;
    if (tmp != "banner:") {
      DEBUG3("Banfile: bad 'banner:' line\n");
      return bansAdded;
    }
    is.ignore(1);
    std::getline(is, bannedBy);
    is >> tmp;
    if (tmp != "reason:") {
      DEBUG3("Banfile: bad 'reason:' line\n");
      return bansAdded;
    }
    is.ignore(1);
    std::getline(is, reason);
    is >> std::ws;
    if (banEnd < 0)
      continue;
    if (ipAddress == "host:") {
      hostBan(hostpat, (bannedBy.size() ? bannedBy.c_str(): NULL), banEnd,
	      (reason.size() > 0 ? reason.c_str() : NULL),true);
      bansAdded++;
    } else if (ipAddress == "bzid:") {
      idBan(bzId, (bannedBy.size() ? bannedBy.c_str(): NULL), banEnd,
            (reason.size() > 0 ? reason.c_str() : NULL));
    } else {
      std::string::size_type n;
      while ((n = ipAddress.find('*')) != std::string::npos) {
	ipAddress.replace(n, 1, "255");
      }
      if (!ban(ipAddress, (bannedBy.size() ? bannedBy.c_str(): NULL), banEnd,
	  (reason.size() > 0 ? reason.c_str() : NULL),true)) {
        DEBUG3("Banfile: bad ban\n");
	return bansAdded;
      }
      bansAdded++;
    }
  }
  return bansAdded;
}


void AccessControlList::save() {
  if (banFile.size() == 0)
    return;
  std::ofstream os(banFile.c_str());
  if (!os.good()) {
    std::cerr<<"Could not open "<<banFile<<std::endl;
    return;
  }
  for (banList_t::const_iterator it = banList.begin(); it != banList.end(); ++it) {
    if (!it->fromMaster) {	// don't save stuff from the master list
      // print address
      in_addr mask = it->addr;
      os << ((ntohl(mask.s_addr) >> 24) % 256) << '.';
      if ((ntohl(mask.s_addr) & 0x00ffffff) == 0x00ffffff) {
	os << "*.*.*";
      } else {
	os << ((ntohl(mask.s_addr) >> 16) % 256) << '.';
	if ((ntohl(mask.s_addr) & 0x0000ffff) == 0x0000ffff) {
	  os << "*.*";
	} else {
	  os << ((ntohl(mask.s_addr) >> 8) % 256) << '.';
	  if ((ntohl(mask.s_addr) & 0x000000ff) == 0x000000ff)
	    os << "*";
	  else
	    os << (ntohl(mask.s_addr) % 256);
	}
      }
      os << '\n';

      // print ban end, banner, and reason
      if (it->banEnd.getSeconds() ==
	  TimeKeeper::getSunExplodeTime().getSeconds()) {
	os << "end: 0" << '\n';
      } else {
	os << "end: " << (long(it->banEnd.getSeconds() + time(NULL) -
			   TimeKeeper::getCurrent().getSeconds()))<<'\n';
      }
      os << "banner: " << it->bannedBy << '\n';
      os << "reason: " << it->reason << '\n';
      os << '\n';
    }
  }
  for (hostBanList_t::const_iterator ith = hostBanList.begin(); ith != hostBanList.end(); ++ith) {
    // print address
    os << "host: " << ith->hostpat << '\n';

    // print ban end, banner, and reason
    if (ith->banEnd.getSeconds() ==
	TimeKeeper::getSunExplodeTime().getSeconds()) {
      os << "end: 0" << '\n';
    } else {
      os << "end: " << (long(ith->banEnd.getSeconds() + time(NULL) -
			 TimeKeeper::getCurrent().getSeconds()))<<'\n';
    }
    os << "banner: " << ith->bannedBy << '\n';
    os << "reason: " << ith->reason << '\n';
    os << '\n';
  }
  for (idBanList_t::const_iterator iti = idBanList.begin(); iti != idBanList.end(); ++iti) {
    // print bzid
    os << "bzid: " << iti->idpat << '\n';

    // print ban end, banner, and reason
    if (iti->banEnd.getSeconds() ==
	TimeKeeper::getSunExplodeTime().getSeconds()) {
      os << "end: 0" << '\n';
    } else {
      os << "end: " << (long(iti->banEnd.getSeconds() + time(NULL) -
			 TimeKeeper::getCurrent().getSeconds())) << '\n';
    }
    os << "banner: " << iti->bannedBy << '\n';
    os << "reason: " << iti->reason << '\n';
    os << '\n';
  }
}


void AccessControlList::purgeMasters(void) {
  // remove any bans from the master server
  banList_t::iterator	bItr = banList.begin();
  while (bItr != banList.end()){
    if (bItr->fromMaster)
      bItr = banList.erase(bItr);
    else
      bItr++;
  }
  hostBanList_t::iterator	hItr = hostBanList.begin();
  while (hItr != hostBanList.end()) {
    if (hItr->fromMaster)
      hItr = hostBanList.erase(hItr);
    else
      hItr++;
  }
  idBanList_t::iterator	iItr = idBanList.begin();
  while (iItr != idBanList.end()) {
    if (iItr->fromMaster) {
      iItr = idBanList.erase(iItr);
    } else {
      iItr++;
    }
  }
}


std::vector<std::pair<std::string, std::string> > AccessControlList::listMasterBans(void) const
{
  std::vector<std::pair<std::string, std::string> > bans;
  std::string explain;

  banList_t::const_iterator bItr;
  for (bItr = banList.begin(); bItr != banList.end(); bItr++) {
    if (bItr->fromMaster) {
      explain = TextUtils::format("%s (banned by %s)",
                                  bItr->reason.c_str(), bItr->bannedBy.c_str());
      const std::pair<std::string, std::string>
        baninfo = std::make_pair(std::string(inet_ntoa(bItr->addr)), explain);
      bans.push_back(baninfo);
    }
  }
  hostBanList_t::const_iterator hItr;
  for (hItr = hostBanList.begin(); hItr != hostBanList.end(); hItr++) {
    if (hItr->fromMaster) {
      explain = TextUtils::format("%s (banned by %s)",
                                  hItr->reason.c_str(), hItr->bannedBy.c_str());
      const std::pair<std::string, std::string>
        baninfo = std::make_pair(hItr->hostpat, explain);
      bans.push_back(baninfo);
    }
  }
  idBanList_t::const_iterator iItr;
  for (iItr = idBanList.begin(); iItr != idBanList.end(); iItr++) {
    if (iItr->fromMaster) {
      explain = TextUtils::format("%s (banned by %s)",
                                  iItr->reason.c_str(), iItr->bannedBy.c_str());
      const std::pair<std::string, std::string>
        baninfo = std::make_pair(iItr->idpat, explain);
      bans.push_back(baninfo);
    }
  }

  return bans;
}


bool AccessControlList::convert(char *ip, in_addr &mask) {
  unsigned char b[4];
  char *pPeriod;

  for (int i = 0; i < 3; i++) {
    pPeriod = strchr(ip, '.');
    if (pPeriod) {
      *pPeriod = 0;
      if (strcmp("*", ip) == 0)
	b[i] = 255;
      else
	b[i] = atoi(ip);
      *pPeriod = '.';
      ip = pPeriod + 1;
    } else {
      return false;
    }
  }
  if (strcmp("*", ip) == 0)
    b[3] = 255;
  else
    b[3] = atoi(ip);

  mask.s_addr= htonl(((unsigned int)b[0] << 24) |
		     ((unsigned int)b[1] << 16) |
		     ((unsigned int)b[2] << 8)  |
		      (unsigned int)b[3]);
  return true;
}


void AccessControlList::expire()
{
  TimeKeeper now = TimeKeeper::getCurrent();
  for (banList_t::iterator it = banList.begin(); it != banList.end();) {
    if (it->banEnd <= now) {
      it = banList.erase(it);
    } else {
      ++it;
    }
  }
  for (hostBanList_t::iterator ith = hostBanList.begin(); ith != hostBanList.end();) {
    if (ith->banEnd <= now) {
      ith = hostBanList.erase(ith);
    } else {
      ++ith;
    }
  }
  for (idBanList_t::iterator iti = idBanList.begin(); iti != idBanList.end();) {
    if (iti->banEnd <= now) {
      iti = idBanList.erase(iti);
    } else {
      ++iti;
    }
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
