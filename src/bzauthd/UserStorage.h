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
  USER_LOCK_NO_REASON,
  USER_LOCK_REGISTER,
  USER_LOCK_CHINF,
};

enum ChInfError
{
  CHINF_SUCCESS = 0x0,
  CHINF_INVALID_CALLSIGN = 0x1,
  CHINF_INVALID_EMAIL = 0x2,
  CHINF_INVALID_PASSWORD = 0x4,
  CHINF_TAKEN_CALLSIGN = 0x8,
  CHINF_TAKEN_EMAIL = 0x10,
  CHINF_OTHER_ERROR = 0x1000
};

enum AcquireError
{
  AQ_SUCCESS            = 0x1,    // the user was locked with the given time / reason
  AQ_ERROR              = 0x2,    // some error occurred
  AQ_ALREADY_LOCKED     = 0x4,    // the user was already locked
  AQ_RESUMED            = 0x8,    // a stalled operation was resumed
  AQ_LOCKED_OLD         = 0x10,   // the lock from a stalled op was not removed
  AQ_MAIL_DELETED       = 0x20,   // the email entry for the user was deleted
  AQ_USER_DELETED       = 0x40    // the user entry was deleted
};

/** The UserStore abstracts the method used for storing users */
class UserStore : public GuardedSingleton<UserStore>
{ 
public:
  UserStore();
  ~UserStore();
  bool initialize();
  void update();
  
  BzRegErrors registerUser(const UserInfo &info);
  uint32_t authUser(const UserInfo &info);
  uint32_t authUserInGame(const UserInfo &info);
  bool isRegistered(std::string callsign);
  bool addToGroup(const std::string &callsign, const std::string &group, const std::string &user_dn, const std::string &group_dn);
  ChInfError changeUserInfo(std::string for_name, const UserInfo &to_info);
  bool activateUser(const UserInfo &info, const std::string &key);
  bool resetPassword(const UserInfo &info);
  bool resendActivation(const UserInfo &info);

  uint64_t acquireUserLock(std::string const &user_dn, std::string const &callsign, int diff, UserLockReason reason);
  template<class T>
    T releaseUserLock(std::string const &user_dn, T ret_error, T ret_success);

  std::string getUserDN(std::string const &callsign);
  std::string getMailDN(std::string const &email);
  std::string getGroupDN(std::string const &group);

  std::list<std::string> intersectGroupList(std::string callsign, std::list<std::string> const &groups, bool all_groups, bool ids);
  std::list<std::string> getUsers(const char *uid);
  std::string getNamefromUID(uint32_t uid);
  std::string getNamefromUID(const char * uid_str);
  std::string getNameFromMail(const std::string &mail, const std::string &mail_dn);

  size_t hashLen();
  void hash(uint8_t *message, size_t message_len, uint8_t *digest);
  std::string hash(const std::string &message);



private:
  bool bind(LDAP *&ld, const uint8_t *addr, const uint8_t *dn, const uint8_t *pw);
  void unbind(LDAP *&ld);

  uint32_t getuid(LDAP *ld, const char *dn);
  BzRegErrors registerMail(const UserInfo &info, uint32_t uid, const char *act_key, bool send_mail, std::string const &user_dn, std::string const &mail_dn);
  BzRegErrors registerMail(const UserInfo &info, char * uid, const char *act_key, bool send_mail, std::string const &user_dn, std::string const &mail_dn);
  BzRegErrors finishReg(const UserInfo &info, std::string const &user_dn, std::string const &mail_dn);
  bool compile_reg(regex_t &reg, uint16_t config_key);
  bool execute_reg(regex_t &reg, const char *str);
  BzRegErrors addUser(const std::string &user_dn, const char *name, const char *pass_digest, bool active, const char *email, uint32_t uid, int lock_time);
  BzRegErrors addUser(const std::string &user_dn, const char *name, const char *pass_digest, bool active, const char *email, const char * uid_str, int lock_time);
  void getActivationKey(uint8_t *key, int len);
  int getActivationKeyLen() const;
  bool sendActivationMail(const UserInfo &info, const char *key);
  uint64_t acquireOrCheckLock(std::string const &user_dn, std::string const &callsign, int new_diff, UserLockReason new_reason, const char *lock_value, const char *lock_reason);

  uint64_t resume_register(const std::string &callsign, const std::string &user_dn);

  LDAP *rootld;
  uint32_t nextuid;
  regex_t re_callsign;
  regex_t re_password;
  regex_t re_email;
  regex_t re_group;
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
