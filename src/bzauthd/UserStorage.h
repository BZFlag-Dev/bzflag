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
#include <list>
#include <AuthProtocol.h>
#include "Thread.h"
#include <bzregex.h>

typedef struct ldap LDAP;

#define MAX_PASSWORD_LEN 32
#define MAX_CALLSIGN_LEN 32
#define MIN_PASSWORD_LEN 2
#define MIN_CALLSIGN_LEN 2
#define MIN_EMAIL_LEN 3
#define MAX_EMAIL_LEN 254 // RFC
#define MAX_GROUPNAME_LEN 20 /* TODO */

struct UserInfo
{
  std::string name;
  std::string password;
  std::string email;
  UserInfo(const std::string &n, const std::string &p, const std::string &e) :
    name(n), password(p), email(e) {}
  UserInfo() {}
};

enum UserLockReason
{
  USER_LOCK_REGISTER,
  USER_LOCK_NAMECHANGE
};

/** The UserStore abstracts the method used for storing users */
class UserStore : public GuardedSingleton<UserStore>
{ 
public:
  UserStore();
  ~UserStore();
  bool initialize();
  void update();
  
  BzRegErrors registerUser(const UserInfo &info, std::string *rand_text = NULL);
  uint32_t authUser(const UserInfo &info);
  bool isRegistered(std::string callsign);
  std::list<std::string> intersectGroupList(std::string callsign, std::list<std::string> const &groups, bool all_groups, bool ids);
  bool updateName(const std::string &old_name, const std::string &new_name);
  bool addToGroup(const std::string &callsign, const std::string &group, const std::string &user_dn, const std::string &group_dn);
  std::list<std::string> getUsers(const char *uid);

  int acquireUserLock(std::string const &user_dn, std::string const &callsign, int diff, UserLockReason reason, const char *lock_value = NULL, const char *user_filter = NULL);
  void releaseUserLock(std::string const &user_dn);

  std::string getUserDN(std::string const &callsign);
  std::string getMailDN(std::string const &email);
  std::string getGroupDN(std::string const &group);

  size_t hashLen();
  void hash(uint8_t *message, size_t message_len, uint8_t *digest);

private:
  bool bind(LDAP *&ld, const uint8_t *addr, const uint8_t *dn, const uint8_t *pw);
  void unbind(LDAP *&ld);

  uint32_t getuid(LDAP *ld, const char *dn);
  BzRegErrors registerMail(const UserInfo &info, uint32_t uid, std::string const &user_dn, std::string const &mail_dn);
  BzRegErrors updatePassword(const UserInfo &info, std::string const &user_dn, std::string const &mail_dn);
  BzRegErrors userExists(std::string const &user_dn, std::string const &callsign, uint32_t uid);
  BzRegErrors userExists(std::string const &user_dn, std::string const &callsign, const char *uid_str);
  bool compile_reg(regex_t &reg, uint16_t config_key);
  bool execute_reg(regex_t &reg, const char *str);
  BzRegErrors addUser(const std::string &user_dn, const char *name, const char *pass, const char *email, uint32_t uid, int lock_time);
  BzRegErrors addUser(const std::string &user_dn, const char *name, const char *pass, const char *email, const char * uid_str, int lock_time);

  LDAP *rootld;
  uint32_t nextuid;
  regex_t re_callsign;
  regex_t re_password;
  regex_t re_email;
  std::string nextuid_dn;
};

#define sUserStore UserStore::guard().instance()

#endif // __BZAUTHD_USERSTORAGE_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
