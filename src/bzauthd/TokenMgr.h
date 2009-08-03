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

/** The TokenMgr stores session tokens
  * that invalidate after some time.
  */


#ifndef __BZAUTHD_TOKENMGR_H__
#define __BZAUTHD_TOKENMGR_H__

#include <string>
#include "Thread.h"

class TokenMgr : public GuardedSingleton<TokenMgr>
{
public:
  TokenMgr();
  ~TokenMgr();
  uint32_t newToken(std::string name, uint32_t bzid, uint32_t ip);
  uint32_t checkToken(uint32_t token, std::string name, uint32_t ip);
  void removeToken(uint32_t token);
  static void expireCallback(void *data);
private:
  struct TokenInfo
  {
    std::string name;
    uint32_t bzid;
    uint32_t ip;
    TokenInfo(std::string name, uint32_t bzid, uint32_t ip)
      : name(name), bzid(bzid), ip(ip) {}
  };

  typedef UNORDERED_MAP<uint32_t /*token*/, TokenInfo> TokenMapType;
  TokenMapType tokenMap;
  void removeToken(TokenMapType::iterator &itr);
};

#define sTokenMgr TokenMgr::guard().instance()

#endif // __BZAUTHD_TOKENMGR_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
