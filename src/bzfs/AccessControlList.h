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

#ifndef __ACCESSCONTROLLIST_H__
#define __ACCESSCONTROLLIST_H__

#ifdef _WIN32
#pragma warning( 4: 4786)
#endif


#include <vector>
#include <string>
#include <algorithm>

#include "global.h"
#include "network.h"

#include "Address.h"
#include "TimeKeeper.h"

extern void sendMessage(int playerIndex, PlayerId targetPlayer, const char *message, bool fullBuffer);

struct BanInfo
{
  BanInfo( in_addr &banAddr, const char *bannedBy = NULL, int period = 0 ) {
    memcpy( &addr, &banAddr, sizeof( in_addr ));
    if( bannedBy )
       this->bannedBy = bannedBy;
    if (period == 0) {
      banEnd = TimeKeeper::getSunExplodeTime();
    } else {
      banEnd = TimeKeeper::getCurrent();
      banEnd += period * 60.0f;
    }
  }
  // BanInfos with same IP are identical
  bool operator==(const BanInfo &rhs) const {
    return addr.s_addr == rhs.addr.s_addr;
  }

  in_addr	addr;
  TimeKeeper	banEnd;
  std::string	bannedBy;	// Who did perform the ban
  std::string	reason;		// reason for banning
};

/* FIXME the AccessControlList assumes that 255 is a wildcard. it "should"
 * include a cidr mask with each address. it's still useful as is, though
 * see wildcard conversion occurs in convert().
 */
class AccessControlList
{
public:
  void ban(in_addr &ipAddr, const char *bannedBy, int period = 0, const char *reason=NULL ) {
    BanInfo toban(ipAddr, bannedBy, period);
    if( reason ) toban.reason = reason;
    banList_t::iterator oldit = std::find(banList.begin(), banList.end(), toban);
    if (oldit != banList.end()) // IP already in list? -> replace
      *oldit = toban;
    else
      banList.push_back(toban);
  }

  bool ban(std::string &ipList, const char *bannedBy=NULL, int period = 0, const char *reason=NULL) {
    return ban(ipList.c_str(), bannedBy, period, reason);
  }

  bool ban(const char *ipList, const char *bannedBy=NULL, int period = 0, const char *reason=NULL) {
    char *buf = strdup(ipList);
    char *pStart = buf;
    char *pSep;
    bool added = false;

    in_addr mask;
    while ((pSep = strchr(pStart, ',')) != NULL) {
      *pSep = 0;
      if (convert(pStart, mask)) {
	ban(mask, bannedBy, period);
	added = true;
      }
      *pSep = ',';
      pStart = pSep + 1;
    }
    if (convert(pStart, mask)) {
      ban(mask,bannedBy,period,reason);
      added = true;
    }
    free(buf);
    return added;
  }

  bool unban(in_addr &ipAddr) {
    banList_t::iterator it = std::remove(banList.begin(), banList.end(), BanInfo(ipAddr));
    if (it != banList.end()) {
      banList.erase(it, banList.end());
      return true;
    }
    return false;
  }

  bool unban(std::string &ipList) {
    return unban(ipList.c_str());
  }

  bool unban(const char *ipList) {
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

  bool validate(in_addr &ipAddr) {
    expire();

    for (banList_t::iterator it = banList.begin(); it != banList.end(); ++it) {
      in_addr mask = it->addr;

      if ((ntohl(mask.s_addr) & 0x00ffffff) == 0x00ffffff)
	mask.s_addr = htonl((ntohl(mask.s_addr) & 0xff000000) | (ntohl(ipAddr.s_addr) & 0x00ffffff));
      else if ((ntohl(mask.s_addr) & 0x0000ffff) == 0x0000ffff)
	mask.s_addr = htonl((ntohl(mask.s_addr) & 0xffff0000) | (ntohl(ipAddr.s_addr) & 0x0000ffff));
      else if ((ntohl(mask.s_addr) & 0x000000ff) == 0x000000ff)
	mask.s_addr = htonl((ntohl(mask.s_addr) & 0xffffff00) | (ntohl(ipAddr.s_addr) & 0x000000ff));

      if (mask.s_addr == ipAddr.s_addr)
	return false;
    }
    return true;
  }

  void sendBans(PlayerId id)
  {
    expire();

    char banlistmessage[MessageLen];
    sendMessage(ServerPlayer, id, "IP Ban List", false);
    sendMessage(ServerPlayer, id, "-----------", false);
    for (banList_t::iterator it = banList.begin(); it != banList.end(); ++it) {
      char *pMsg = banlistmessage;
      in_addr mask = it->addr;

      sprintf( pMsg, "%d.", ((unsigned char)(ntohl(mask.s_addr) >> 24)));
      pMsg+=strlen(pMsg);

      if ((ntohl(mask.s_addr) & 0x00ffffff) == 0x00ffffff) {
        strcat( pMsg, "*.*.*" );
      } else {
        sprintf( pMsg, "%d.", ((unsigned char)(ntohl(mask.s_addr) >> 16)));
        pMsg+=strlen(pMsg);
        if ((ntohl(mask.s_addr) & 0x0000ffff) == 0x0000ffff) {
          strcat( pMsg, "*.*" );
        } else {
          sprintf( pMsg, "%d.", ((unsigned char)(ntohl(mask.s_addr) >> 8)));
          pMsg+=strlen(pMsg);
          if ((ntohl(mask.s_addr) & 0x000000ff) == 0x000000ff)
	    strcat( pMsg, "*" );
          else
	    sprintf( pMsg, "%d", ((unsigned char)ntohl(mask.s_addr)));
        }
      }
      // print duration when < 1 year
      double duration = it->banEnd - TimeKeeper::getCurrent();
      if (duration < 365.0f * 24 * 3600)
	sprintf(pMsg + strlen(pMsg)," (%.1f minutes)", duration / 60);
      if( it->bannedBy.length() )
	sprintf(pMsg + strlen(pMsg), " banned by: %s", it->bannedBy.c_str());

      sendMessage(ServerPlayer, id, banlistmessage, true);

      // add reason, if any
      if( it->reason.size() ) { 
	char *pMsg = banlistmessage;
	sprintf(pMsg, "   reason: %s", it->reason.c_str() );
	sendMessage(ServerPlayer, id, banlistmessage, true );
      }
    }
  }

private:
  bool convert(char *ip, in_addr &mask);

  void expire();

  typedef std::vector<BanInfo> banList_t;
  banList_t banList;
};

#else
class AccessControlList;
#endif /* __ACCESSCONTROLLIST_H__ */
// ex: shiftwidth=2 tabstop=8
