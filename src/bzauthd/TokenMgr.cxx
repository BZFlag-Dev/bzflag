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

#include <common.h>
#include "TokenMgr.h"
#include "EventHandler.h"
#include "Log.h"
#include "ConfigMgr.h"

INSTANTIATE_SINGLETON(TokenMgr);

TokenMgr::TokenMgr()
{
  // reserve token 0 for unregistered users
  nextToken = 1;
}

TokenMgr::~TokenMgr()
{

}

void TokenMgr::addToken(std::string name, uint32_t token)
{
  sLog.outLog("TokenMgr: adding token %d (%s)", token, name.c_str());
  tokenMap[nextToken] = name;
  sEventHandler.addDelta(&TokenMgr::expireCallback, (void *)new uint32_t(token), sConfig.getIntValue(CONFIG_TOKEN_EXPIRE_DELAY) / 1000.0);
}

uint32_t TokenMgr::newToken(std::string name)
{
  addToken(name, nextToken);
  return nextToken++;
}

bool TokenMgr::checkToken(std::string name, uint32_t token)
{
  TokenMapType::iterator itr = tokenMap.find(token);
  return itr != tokenMap.end() && itr->second == name;
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