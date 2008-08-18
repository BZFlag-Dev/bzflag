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
  */


#ifndef __BZAUTHD_TOKENMGR_H__
#define __BZAUTHD_TOKENMGR_H__

#include "Singleton.h"
#include <hash_map>

class TokenMgr : public Singleton<TokenMgr>
{
public:
  TokenMgr();
  ~TokenMgr();
  uint32 newToken(std::string name);
  void addToken(std::string name, uint32 token);
  bool checkToken(std::string name, uint32 token);
  void removeToken(uint32 token);
  static void expireCallback(void *data);
private:
  uint32 nextToken;
  typedef HM_NAMESPACE::hash_map<uint32, std::string> TokenMapType;
  TokenMapType tokenMap;
  void removeToken(TokenMapType::iterator &itr);
};

#define sTokenMgr TokenMgr::instance()

#endif // __BZAUTHD_TOKENMGR_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8