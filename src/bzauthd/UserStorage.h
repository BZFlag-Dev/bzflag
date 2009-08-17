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
#include <sstream>

typedef struct ldap LDAP;

#define MAX_PASSWORD_LEN 32
#define MAX_CALLSIGN_LEN 32
#define MIN_PASSWORD_LEN 2
#define MIN_CALLSIGN_LEN 2
#define MIN_EMAIL_LEN 3
#define MAX_EMAIL_LEN 254 // RFC
#define MAX_GROUPNAME_LEN 30
#define MAX_ORGNAME_LEN 30
#define MAX_PERMISSION_LEN 255

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

enum UserInfoValidity
{
  INFO_VALID,
  INFO_INVALID_NAME,
  INFO_INVALID_MAIL,
  INFO_INVALID_PASS
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

enum GroupIdValidity
{
  GROUPID_VALID = 0x0,
  GROUPID_INVALID_OU = 0x1,
  GROUPID_INVALID_GN = 0x2
};

struct GroupId
{
  std::string ou;
  std::string grp;
  GroupId(const std::string &organization, const std::string &group_name) : ou(organization), grp(group_name) {}
  GroupId(const std::string &org_dot_group);
  std::string getDotNotation() {
    return ou + "." + grp;
  }
};

struct GroupMemberCallback
{
  virtual void got_group(char *uid, char* ou, char* grp) = 0;
  virtual void got_perm(uint32_t perm_val, char* val, char* arg) = 0;
};

struct GroupInfoCallback
{
  virtual void got_group(char* ou, char* grp, uint32_t state) = 0;
  virtual void got_perm(char* val, char* arg) = 0;
};

struct FindGroupsCallback
{
  virtual void got_group(char *ou, char *grp) = 0;
};

struct OrgCallback
{
  virtual void got_org(char *ou) = 0;
};

struct MemberCountCallback
{
  virtual void got_count(const char *ou, const char *grp, uint32_t count) = 0;
};

struct UserNameCallback
{
  virtual void got_userName(const char *uid, const char *name) = 0;
};

struct FilterStream
{
  std::ostringstream stream;
  bool changed;
};

struct OrgFilter : public FilterStream
{
  OrgFilter();
  void add_org(const char *org);
  void add_owner(const char *uid_str);
  bool finish();
};

struct GroupFilter : public FilterStream
{
  GroupFilter();
  void add_org(const char *org);
  void add_org_group(const char *org, const char *grp);
  bool finish();
};

struct MemberFilter : public FilterStream
{
  MemberFilter();
  void add_uid(const char *uid);
  void add_org(const char *org);
  void add_org_group(const char *org, const char *grp);
  bool finish();
};

struct UserNameFilter : public FilterStream
{
  UserNameFilter();
  void add_uid(const char *uid);
  bool finish();
};

enum PermissionTypes
{
  PERM_ADMIN_OF     = 1,
  PERM_ADMIN        = 2,
  PERM_ORG_ADMIN    = 3,
  PERM_GLOBAL_ADMIN = 4
};

#define MAX_ORGS_PER_USER 255

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
  bool addToGroup(const std::string &uid_str, const GroupId & gid);
  ChInfError changeUserInfo(std::string for_name, const UserInfo &to_info);
  int activateUser(const UserInfo &info, const std::string &key);
  int resetPassword(const UserInfo &info);
  int resendActivation(const UserInfo &info);
  std::string getActivationURL(const UserInfo &info, const std::string &key);
  bool createGroup(const GroupId &gid);
  bool createOrganization(const std::string &org);

  uint64_t acquireUserLock(std::string const &user_dn, std::string const &callsign, int diff, UserLockReason reason);
  template<class T>
    T releaseUserLock(std::string const &user_dn, T ret_error, T ret_success);

  std::string getUserDN(std::string const &callsign) const;
  std::string getMailDN(std::string const &email) const;
  std::string getGroupDN(const GroupId &gid) const;
  std::string getOrgDN(std::string const &org) const;
  std::string getMemberDN(const char *uid_str, const GroupId &gid) const;
  std::string getMemberDN(uint32_t uid, const GroupId &gid) const;

  std::list<GroupId> intersectGroupListUID(const std::string & uid_str, std::list<GroupId> const &groups, bool all_groups);
  std::list<GroupId> intersectGroupList(std::string callsign, std::list<GroupId> const &groups, bool all_groups);
  std::list<std::string> getUsers(const char *uid);
  uint32_t getTotalGroups();
  uint32_t getTotalOrgs();
  bool getGroups(GroupFilter &filter, FindGroupsCallback &callback);
  bool getOrgs(OrgFilter &filter, OrgCallback &callback);
  bool getMembershipInfo(MemberFilter &filter, GroupMemberCallback &callback);
  bool getGroupInfo(GroupFilter &filter, GroupInfoCallback &callback);
  bool getMemberCount(MemberFilter &filter, MemberCountCallback &callback);
  bool getUserNames(UserNameFilter &filter, UserNameCallback &callback);

  std::string getNamefromUID(uint32_t uid);
  std::string getNamefromUID(const char * uid_str);
  std::string getNameFromMail(const std::string &mail, const std::string &mail_dn);
  const std::string getUIDfromName(const char *name) const;

  size_t hashLen();
  void hash(uint8_t *message, size_t message_len, uint8_t *digest);
  std::string hash(const std::string &message);

  void getRandomKey(uint8_t *key, int len);
  int getActivationKeyLen();
  int getNewPasswordLen();
  const std::string uid2str(uint32_t uid) const;

  UserInfoValidity validateUserInfo(const UserInfo &info, bool *got_name = NULL, bool *got_mail = NULL, bool *got_pass = NULL);
  UserInfoValidity validateUserInfo(const UserInfo &info, bool got_name, bool got_mail, bool got_pass);
  GroupIdValidity validateGroupId(const GroupId &gid);

private:
  bool bind(LDAP *&ld, const uint8_t *addr, const uint8_t *dn, const uint8_t *pw);
  void unbind(LDAP *&ld);

  uint32_t getuid(LDAP *ld, const char *dn) const;
  BzRegErrors registerMail(const UserInfo &info, uint32_t uid, const char *act_key, bool send_mail, std::string const &user_dn, std::string const &mail_dn);
  BzRegErrors registerMail(const UserInfo &info, char * uid, const char *act_key, bool send_mail, std::string const &user_dn, std::string const &mail_dn);
  BzRegErrors finishReg(const UserInfo &info, std::string const &user_dn, std::string const &mail_dn);
  bool compile_reg(regex_t &reg, uint16_t config_key);
  bool execute_reg(regex_t &reg, const char *str);
  BzRegErrors addUser(const std::string &user_dn, const char *name, const char *pass_digest, bool active, const char *email, uint32_t uid, int lock_time);
  BzRegErrors addUser(const std::string &user_dn, const char *name, const char *pass_digest, bool active, const char *email, const char * uid_str, int lock_time);
  bool sendActivationMail(const UserInfo &info, const char *key, const char *tmpl = NULL);
  uint64_t acquireOrCheckLock(std::string const &user_dn, std::string const &callsign, int new_diff, UserLockReason new_reason, const char *lock_value, const char *lock_reason);
  std::list<GroupId> &getGroups(const std::string &filter, std::list<GroupId> &ret);
  uint32_t checkPerm(char *perm, char * &arg, const char *type, const char *what);
  void getAllOrgGroups(const std::string &org, FindGroupsCallback &callback);

  uint64_t resume_register(const std::string &callsign, const std::string &user_dn);

  const std::string &getEmptyString() const {
    static std::string str;
    return str;
  }

  LDAP *rootld;
  uint32_t nextuid;
  regex_t re_callsign;
  regex_t re_password;
  regex_t re_email;
  regex_t re_group;
  regex_t re_organization;
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
