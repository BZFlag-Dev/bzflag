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

#ifndef __BZAUTHD_USERSTORAGE_H__
#define __BZAUTHD_USERSTORAGE_H__

#include <string>

typedef struct ldap LDAP;

#define MAX_PASSWORD_LEN 32
#define MAX_CALLSIGN_LEN 32
#define MIN_PASSWORD_LEN 2
#define MIN_CALLSIGN_LEN 2

struct UserInfo
{
  std::string name;
  std::string password;
};

class UserStore : public Singleton<UserStore>
{
public:
  UserStore();
  ~UserStore();
  bool initialize();
  
  void registerUser(UserInfo &info);
  bool authUser(UserInfo &info);
  void update();

  size_t hashLen();
  void hash(uint8 *message, size_t message_len, uint8 *digest);
private:
  bool bind(LDAP *&ld, const uint8 *addr, const uint8 *dn, const uint8 *pw);
  void unbind(LDAP *&ld);
  LDAP *rootld;
};

#define sUserStore UserStore::instance()

#endif // __BZAUTHD_USERSTORAGE_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8