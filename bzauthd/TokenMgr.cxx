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

#include "common.h"
#include "TokenMgr.h"

INSTANTIATE_SINGLETON(TokenMgr);

TokenMgr::TokenMgr()
{
  nextToken = 0;
}

TokenMgr::~TokenMgr()
{

}

void TokenMgr::update()
{

}

uint32 TokenMgr::newToken(std::string name)
{
  tokenMap[nextToken] = name;
  // TODO: schedule expiry
  return nextToken++;
}

bool TokenMgr::checkToken(std::string name, uint32 token)
{
  TokenMapType::iterator itr = tokenMap.find(token);
  return itr != tokenMap.end() && itr->second == name;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8