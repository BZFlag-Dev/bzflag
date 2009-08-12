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

#include "TokenMgr.h"
#include "EventHandler.h"
#include "Log.h"
#include "ConfigMgr.h"
#include "Random.h"

INSTANTIATE_GUARDED_SINGLETON(TokenMgr)

TokenMgr::TokenMgr()
{
}

TokenMgr::~TokenMgr()
{

}

uint32_t TokenMgr::newToken(std::string name, uint32_t bzid, uint32_t ip)
{
  std::pair<TokenMapType::iterator, bool> p;
  uint32_t token;
  do {
    token = 1 + sRandom.getU32() % ((uint32_t)(1 << 31) - 1);
    p = tokenMap.insert(TokenMapType::value_type(token, TokenInfo(name, bzid, ip)));
  } while(p.second == false); // try inserting a different number if failed

  sEventHandler.addDelta(&TokenMgr::expireCallback, (void *)new uint32_t(token), sConfig.getIntValue(CONFIG_TOKEN_EXPIRE_DELAY) / 1000.0);
  sLog.outLog("TokenMgr: adding token %d (%s)", token, name.c_str());
  // TODO: share tokens between daemons
  return token;
}

uint32_t TokenMgr::checkToken(uint32_t token, std::string name, uint32_t ip)
{
  TokenMapType::iterator itr = tokenMap.find(token);
  // callsigns should be case insesitive so use such a comparison
  if(itr != tokenMap.end() && (strcasecmp(itr->second.name.c_str(), name.c_str()) == 0))
    return itr->second.bzid;
  else
    return 0;
}

void TokenMgr::removeToken(uint32_t token)
{
  TokenMapType::iterator itr = tokenMap.find(token);
  if(itr != tokenMap.end()) removeToken(itr);
  else sLog.outLog("TokenMgr: can't remove token %d, not found!", token);
}

void TokenMgr::removeToken(TokenMapType::iterator &itr)
{
  sLog.outLog("TokenMgr: removing token %d", itr->first);
  tokenMap.erase(itr);
}

void TokenMgr::expireCallback(void *data)
{
  sTokenMgr.removeToken(*(uint32_t*)data);
  delete (uint32_t*)data;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
