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

#include "UserStorage.h"
#define LDAP_DEPRECATED 1
#include "ConfigMgr.h"
#include <Log.h>
#include <gcrypt.h>
#include "base64.h"
#include <assert.h>
#include <bzregex.h>
#include "Random.h"
#include "LDAPUtils.h"
#include "MailMan.h"
#include <curl/curl.h>
#include <set>

INSTANTIATE_GUARDED_SINGLETON(UserStore)

struct LDAPMod_UserLockSetReason : public LDAPMod1
{
  LDAPMod_UserLockSetReason(UserLockReason flag) : LDAPMod1(LDAP_MOD_REPLACE, "shadowFlag", getFlagStr(flag)) {}

  const char *getFlagStr(UserLockReason flag) {
    sprintf(flag_str, "%d", (int)flag);
    return flag_str;
  }

  char flag_str[30];
};

struct LDAPMod_UserUnlock : public LDAPMod1
{
  LDAPMod_UserUnlock() : LDAPMod1(LDAP_MOD_REPLACE, "shadowExpire", "0") {}
};

struct LDAPMod_UserLockDel : public LDAPMod1
{
  LDAPMod_UserLockDel(const char *lock_val) : LDAPMod1(LDAP_MOD_DELETE, "shadowExpire", lock_val) {}
};

struct LDAPMod_UserLockAdd : public LDAPMod1
{
  LDAPMod_UserLockAdd(int diff, char *buf = NULL) : LDAPMod1(LDAP_MOD_ADD, "shadowExpire", getTimeStr(diff, buf)) {}

  const char *getTimeStr(int diff, char *buf) {
    if(!buf) buf = time_str;
    // if diff = 0, just add the attribute but don't set a lock
    // TODO: TimeKeeper + 64bit
    sprintf(buf, "%d", diff > 0 ? (int)time(NULL) + diff : 0);
    return buf;
  }

  char time_str[30];
};

#define err_isset2(value, err_v) (((err_v) & (value)) != 0)
#define err_isset(value) err_isset2(value, err)
#define err_set2(value, err_v) err = ((err_v) | (value))
#define err_unset2(value, err_v) err = ((err_v) & (~(value)))
#define err_set(value) err_set2(value, err)
#define err_unset(value) err_unset2(value, err)

UserStore::UserStore() : rootld(NULL), nextuid(0)
{
}

UserStore::~UserStore()
{
  regfree(&re_callsign);
  regfree(&re_password);
  regfree(&re_email);
  unbind(rootld);
}

void UserStore::unbind(LDAP *& ld)
{
  if(ld) {
    LDAP_VCHECK( ldap_unbind(ld) );
    sLog.outLog("UserStore: unbound");
  }
}

bool UserStore::bind(LDAP *& ld, const uint8_t *addr, const uint8_t *dn, const uint8_t *pw)
{
  unbind(ld);
  sLog.outLog("UserStore: binding to %s, with root dn %s", addr, dn);

  int version = LDAP_VERSION3;
  LDAP_FCHECK( ldap_initialize(&ld, (char*)addr) );
	LDAP_FCHECK( ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version) );
  LDAP_FCHECK( ldap_simple_bind_s(ld, (char*)dn, (char*)pw) );
  return true;
}

bool UserStore::compile_reg(regex_t &reg, uint16_t config_key)
{
  const char *regex = (const char*)sConfig.getStringValue(config_key);
  int ret = regcomp(&reg, regex, REG_EXTENDED | REG_NOSUB);
  if(ret != 0) {
    char err[1024];
    size_t size = regerror (ret, &reg, err, 1024);
    sLog.outError("UserStore: failed to compile '%s': %s", regex, err);
    return false;
  }
  return true;
}

bool UserStore::initialize()
{
  nextuid_dn = getUserDN("NextUID");

  if(!compile_reg(re_callsign, CONFIG_CALLSIGN_REGEX)) return false;
  if(!compile_reg(re_password, CONFIG_PASSWORD_REGEX)) return false;
  if(!compile_reg(re_email, CONFIG_EMAIL_REGEX)) return false;
  if(!compile_reg(re_group, CONFIG_GROUP_REGEX)) return false;
  if(!compile_reg(re_organization, CONFIG_ORGANIZATION_REGEX)) return false;

  return bind(rootld, sConfig.getStringValue(CONFIG_LDAP_MASTER_ADDR), sConfig.getStringValue(CONFIG_LDAP_ROOTDN), sConfig.getStringValue(CONFIG_LDAP_ROOTPW));
}

size_t UserStore::hashLen()
{
  return (size_t)gcry_md_get_algo_dlen(GCRY_MD_MD5) / 2 * 3 + 5;
}

void UserStore::hash(uint8_t *message, size_t message_len, uint8_t *digest)
{
  int md5len = gcry_md_get_algo_dlen(GCRY_MD_MD5);
  uint8_t *tmpbuf = new uint8_t[md5len];
  gcry_md_hash_buffer(GCRY_MD_MD5, tmpbuf, message, message_len);
  strcpy((char*)digest, "{md5}");
  base64::encode(tmpbuf, tmpbuf + md5len, digest+5);
  digest[md5len / 2 * 3 + 5] = '\0';
  delete[] tmpbuf;
}

std::string UserStore::hash(const std::string &message)
{
  int md5len = gcry_md_get_algo_dlen(GCRY_MD_MD5);
  int hash_len = md5len / 2 * 3 + 5;
  int msg_len = (int)message.size();

  uint8_t *tmpbuf = new uint8_t[md5len + hash_len];
  gcry_md_hash_buffer(GCRY_MD_MD5, tmpbuf, message.c_str(), message.size());
  base64::encode(tmpbuf, tmpbuf + md5len, tmpbuf + md5len + 5);
  memcpy((char*)tmpbuf + md5len, "{md5}", 5);
  std::string digest((char*)tmpbuf + md5len, hash_len);
  delete[] tmpbuf;

  return digest;
}

bool UserStore::execute_reg(regex_t &reg, const char *str)
{
  int ret = regexec(&reg, str, 0, NULL, 0);
  if(ret != 0) {
    char err[1024];
    size_t size = regerror (ret, &reg, err, 1024);
    sLog.outDebug("UserStore: %s did not match", str);
    return false;
  }
  return true;
}

BzRegErrors UserStore::addUser(const std::string &user_dn, const char *name, const char *pass_digest, bool active, const char *email, uint32_t uid, int lock_time)
{
  char uid_str[20]; sprintf(uid_str, "%d", uid);
  return addUser(user_dn, name, pass_digest, active, email, uid_str, lock_time);
}

BzRegErrors UserStore::addUser(const std::string &user_dn, const char *name, const char *pass_digest, bool active, const char *email, const char * uid_str, int lock_time)
{
  const char *oc_vals[3] = {"person", "extensibleObject", NULL};
  LDAPModN attr_oc    (LDAP_MOD_ADD, "objectClass", oc_vals);
  LDAPMod1 attr_cn    (LDAP_MOD_ADD, "cn", name);
  LDAPMod1 attr_sn    (LDAP_MOD_ADD, "sn", name);
  LDAPMod1 attr_uid   (LDAP_MOD_ADD, "uid", uid_str);
  LDAPMod1 attr_mail  (LDAP_MOD_ADD, "mail", email);
  LDAPMod_UserLockAdd attr_lock(lock_time);
  LDAPMod_UserLockSetReason attr_lockr(USER_LOCK_REGISTER);
  LDAPMod1 attr_pass  (LDAP_MOD_ADD, "userPassword", pass_digest);
  LDAPMod1 attr_ipass (LDAP_MOD_ADD, "inactivePassword", pass_digest);

  // do not lock the user if the password was also given
  LDAPMod *user_mods[9] = { &attr_oc.mod, &attr_cn.mod, &attr_sn.mod,
    &attr_uid.mod, &attr_mail.mod, &attr_lock.mod, &attr_lockr.mod,
    pass_digest ? (active ? &attr_pass.mod : &attr_ipass.mod) : NULL, NULL };

  while(1) {
    uint64_t err;
    int ret = ldap_add_ext_s(rootld, user_dn.c_str(), user_mods, NULL, NULL);
    switch(ret) {
    case LDAP_SUCCESS:
      return REG_SUCCESS;
    case LDAP_ALREADY_EXISTS:
      err = acquireUserLock(user_dn, name, 0, USER_LOCK_NO_REASON);
      // if the function succeeded in deleting the existing user and its email
      // try inserting again
      if(err_isset(AQ_USER_DELETED))
        continue;
      if(!err_isset(AQ_ERROR)) {
        sLog.outDebug("User %s already exists", name);
        return REG_USER_EXISTS;
      }
      return REG_FAIL_GENERIC;
    default:
      ldap_check(ret);
      return REG_FAIL_GENERIC;
    }
  }
}

BzRegErrors UserStore::registerUser(const UserInfo &info)
{
  // first check if info is valid
  if(!execute_reg(re_callsign, info.name.c_str()))
    return REG_USER_INVALID;
  if(!execute_reg(re_password, info.password.c_str()))
    return REG_PASS_INVALID;
  if(!execute_reg(re_email, info.email.c_str()))
    return REG_MAIL_INVALID;

  sLog.outLog("UserStore: registering %s(%s)", info.name.c_str(), info.email.c_str());
  /* If multiple sources are allowed to register users,
     the highest uid must be atomically fetch/incremented.
     First the value stored in cn=NextUID's uid is retrieved then
     that value is deleted and the incremented value is added in one atomic operation.
     If the operation fails it is because someone changed the value of next uid
     between the search and modify operation and the fetch/increment is retried
  */

  /* NOTE: If the server crashes or the connection to ldap is lost
     the new bzid will have been generated with no user associated to it (holes). 
     To ensure that simply trying to register with an existing callsign/email
     doesn't create holes, the generated nextuid is saved and reused for subsequent
     registrations.
  */

  if(nextuid == 0) {
    for(int i = 0; i < 4; i++) {
      nextuid = getuid(rootld, nextuid_dn.c_str());
      if(nextuid > 0) {
        char curvalue[20];
        sprintf(curvalue, "%d", nextuid);
        char newvalue[20];
        sprintf(newvalue, "%d", nextuid + 1);

        LDAPMod1 attr_old(LDAP_MOD_DELETE, "uid", curvalue);
        LDAPMod1 attr_new(LDAP_MOD_ADD, "uid", newvalue);
        LDAPMod *mod_attrs[3] = { &attr_old.mod, &attr_new.mod, NULL };
        if(!ldap_check( ldap_modify_ext_s(rootld, nextuid_dn.c_str(), mod_attrs, NULL, NULL) )) {
          sLog.outDebug("UserStore: nextuid modify failed");
          nextuid = 0;
        } else
          break;
      }

      sLog.outDebug("UserStore: cannot fetch-increment NextUID, retry number %d", i + 1);
    }

    if(nextuid < 1) {
      nextuid = 0;
      return REG_FAIL_GENERIC;
    }
    sLog.outDebug("UserStore: fetch nextuid %d", nextuid);
  }

  /* NOTE: The user has an active and inactive password. The inactive one is used
    until the user activates his registration then it is deleted and replaced by the
    active one (userPassword instead of inactivePassword). This ensures that
    external auths will not pass the user before it activated. The inactive 
    password is used when authenticating from the game, which is allowed even before
    activation. It is added as the last step in the registration so that the user cannot
    login if it was not finished. */

  /* NOTE: As long as all the changes succeed or only fail when expected to (user/mail
     already exits), this should run without concurrency problems and will clean itself up.
     The acquireUserLock procedure also ensures that if after an interrupted registration,
     the same callsign or email is registered again, it will be superceded. This means
     that the corruption in the database does not affect the behavior of the registration,
     apart from the time delay in the soft lock. If however nobody registers the same
     name/mail again, the entries will accumulate and will need to be cleaned periodically.
     To do that without shutting down registration, after searching for the invalid
     passwords/emails, just run the userExists procedure, with the exception of waiting
     until the time lock expired and trying again if necessary.
  */

  // insert the user with the next uid
  std::string user_dn = getUserDN(info.name);
  std::string mail_dn = getMailDN(info.email);
  BzRegErrors err;

  err = addUser(user_dn, info.name.c_str(), NULL, false, info.email.c_str(), nextuid + 1, 
    (int)sConfig.getIntValue(CONFIG_REGISTER_LOCK_TIME));
  if(err != REG_SUCCESS)
    return err;

  err = registerMail(info, nextuid + 1, NULL, true, user_dn, mail_dn);
  if(err != REG_SUCCESS)
    return err;

  return finishReg(info, user_dn, mail_dn);

  /* TODO: The above mentioned solution to the holes problem can be further improved.
    Each separate entitry that wishes to do registration can acquire a lock on certain range 
    of BZIDs. E.g one server locks ids 1-1000 another 1001-2000 and so on. When a server owns
    a range it can hand them out to new users without needing to check with LDAP. This could
    be done by putting the ranges as multiple values of some attribute and atomically 
    appending LOCKED-timestamp to the value. Each lock will have say a 5 minute timeout...
    (todo: finish the todo, or maybe just do it)
  */
}

uint64_t UserStore::acquireUserLock(std::string const &user_dn, std::string const &callsign, int diff, UserLockReason reason)
{
  /*
    Try to acquire a lock for the given user for a given time/reason and
    at the same time, try to resume stalled operation on the user.
    If reason == USER_LOCK_NO_REASON, don't acquire the lock, just resume.
  */

  uint64_t err = 0;
  const int retry_count = 3;

  for(int i = 0; i < retry_count; i++) {
    if(i > 0 || reason == USER_LOCK_NO_REASON) {
      LDAPBaseSearchN<2> user_search(rootld, user_dn.c_str(), "(objectClass=*)",
        "shadowExpire", 1, 30,
        "shadowFlag", 1, 30);

      char *lock_value = user_search.getResult(0).getNextVal();
      char *lock_reason = user_search.getResult(1).getNextVal();
      if(!lock_value || !lock_reason) {
        sLog.outError("UserStore: cannot find lock value/reason for %s", callsign.c_str());
        return err_set(AQ_ERROR);
      }
      err_set( acquireOrCheckLock(user_dn, callsign, diff, reason, lock_value, lock_reason) );
    } else
      err_set( acquireOrCheckLock(user_dn, callsign, diff, reason, "0", "0") );

    // unless this was the last try and there was some error
    // remove the error state and try again
    if(i != retry_count - 1 && err_isset(AQ_ERROR))
      err_unset(AQ_ERROR);
    else
      break;
  }

  return err;
}

uint64_t UserStore::acquireOrCheckLock(std::string const &user_dn, std::string const &callsign, int new_diff, UserLockReason new_reason, const char *lock_value, const char *lock_reason)
{
  char buf_lock_val[30];
  int ret;
  uint64_t err = 0;

  if(lock_value[0] != '0') {
    int expire_time; // todo 64bit
    sscanf(lock_value, "%d", &expire_time);
    if(expire_time > (int)time(NULL)) {
      sLog.outDebug("UserStore: the lock for %s will only expire %d seconds later", callsign.c_str(), expire_time - time(NULL));
      return err_set(AQ_ALREADY_LOCKED);
    }
    
    // lock expired, try to resume stalled operation
    // lock with the old reason in case it fails again while resuming
    // lock with a proper diff for the old operation
    int old_reason, old_diff; sscanf(lock_reason, "%d", &old_reason);
    switch(old_reason) {
      case USER_LOCK_REGISTER: old_diff = (int)sConfig.getIntValue(CONFIG_REGISTER_LOCK_TIME); break;
      default:
        sLog.outError("UserStore: unhandled lock reason %d", old_reason);
        return err_set(AQ_ERROR);
    }

    LDAPMod_UserLockDel attr_lock_del(lock_value);
    LDAPMod_UserLockAdd attr_lock_add(old_diff, buf_lock_val);
    LDAPMod_UserLockSetReason attr_lock_reason((UserLockReason)old_reason);
    LDAPMod *old_lock_mods[4] = { &attr_lock_del.mod, &attr_lock_add.mod, &attr_lock_reason.mod, NULL };

    ret = ldap_modify_ext_s(rootld, user_dn.c_str(), old_lock_mods, NULL, NULL);
    if(ret != LDAP_SUCCESS) {
      if(ret == LDAP_NO_SUCH_ATTRIBUTE) {
        sLog.outDebug("UserStore: user %s already locked", callsign.c_str());
        return err_set(AQ_ALREADY_LOCKED);
      } else {
        ldap_check(ret);
        return err_set(AQ_ERROR);
      }
    }

    err_set(AQ_LOCKED_OLD);

    bool unlock = true;
    // try to resume the stalled lock 
    switch(old_reason) {
      case USER_LOCK_REGISTER: err_set( resume_register(callsign, user_dn) ); break;
      default:
        sLog.outError("UserStore: unhandled lock reason %d", old_reason);
        return err_set(AQ_ERROR);
    }

    if(!err_isset(AQ_RESUMED)) // failed to resume, don't unlock
      return false;

    if(new_reason == USER_LOCK_NO_REASON) {
      // no reason to lock again, try to unlock
      // unless the resume already unlocked
      if(err_isset(AQ_LOCKED_OLD))
        return releaseUserLock(user_dn, err_set(AQ_ERROR), err_unset(AQ_LOCKED_OLD));
      else
        return err;
    }

    lock_value = buf_lock_val;

  } else if(new_reason == USER_LOCK_NO_REASON) {
    // don't lock for no reason
    return err;
  }

  // try to lock the current player and set the lock to expire later
  // this will fail if somebody acquired the lock between now and the time its value was retrieved
  // or if the assumption that it's not locked is wrong
  LDAPMod_UserLockDel attr_lock_del(lock_value);
  LDAPMod_UserLockAdd attr_lock_add(new_diff);
  LDAPMod_UserLockSetReason attr_lock_reason(new_reason);
  LDAPMod *time_mods[4] = { &attr_lock_del.mod, &attr_lock_add.mod, &attr_lock_reason.mod, NULL };

  ret = ldap_modify_ext_s(rootld, user_dn.c_str(), time_mods, NULL, NULL);
  if(ret != LDAP_SUCCESS) {
    if(ret == LDAP_NO_SUCH_ATTRIBUTE) {
      sLog.outDebug("UserStore: user %s already locked", callsign.c_str());
      return err_set(AQ_ALREADY_LOCKED);
    } else {
      ldap_check(ret);
      return err_set(AQ_ERROR);
    }
  } else
    return err_unset2(AQ_LOCKED_OLD, err_set(AQ_SUCCESS));
}

template<class T>
T UserStore::releaseUserLock(std::string const &user_dn, T ret_error, T ret_success)
{
  LDAPMod_UserUnlock attr_unlock;
  LDAPMod *attrs[3] = { &attr_unlock.mod, NULL };
  if(!ldap_check( ldap_modify_s(rootld, user_dn.c_str(), attrs) ))
    return ret_error;
  else
    return ret_success;
}

uint64_t UserStore::resume_register(const std::string &callsign, const std::string &user_dn)
{
  // try to undo an unfinished registration

  uint64_t err = 0;

  LDAPBaseSearchN<2> user_search(rootld, user_dn.c_str(), "(objectClass=*)",
    "mail", 1, MAX_EMAIL_LEN,
    "uid", 1, 30);

  char *mail = user_search.getResult(0).getNextVal();
  char *uid_str = user_search.getResult(1).getNextVal();
  if(!mail || !uid_str) {
    sLog.outError("UserStore: cannot find mail or uid for %s", callsign.c_str());
    return err_set(AQ_ERROR);
  }

  sLog.outLog("UserStore: checking mail status for %s(%s)", callsign.c_str(), mail);
  std::string mail_dn = getMailDN(mail);
  LDAPBaseSearchN<1> mail_search(rootld, mail_dn.c_str(), "(objectClass=*)",
    "uid", 1, 30);

  // no mail entry found => the mail was not registered
  // found and registered to this user => the mail was registered and the entry should be removed
  // found and registered to another user => the mail was not registered, only later by another user
  // NOTE: this would have concurrency issues without the timed lock

  if(mail_search.getResultCount() != 0) {
    char *mail_uid = mail_search.getResult(0).getNextVal();
    if(!mail_uid) {
      sLog.outError("UserStore: cannot find mail uid for %s(%s)", callsign.c_str(), mail);
      return err_set(AQ_ERROR);
    }
    if(strcmp(uid_str, mail_uid) == 0) {
      sLog.outLog("UserStore: found mail from unfinished registration %s(%s), removing", callsign.c_str(), mail);
      if(!ldap_check(ldap_delete_ext_s(rootld, mail_dn.c_str(), NULL, NULL)))
        return err_set(AQ_ERROR);
      err_set(AQ_MAIL_DELETED);
    }
  }

  sLog.outLog("UserStore: removing user %s due to unfinished registration", callsign.c_str());
  // delete the user and try inserting again
  
  if(ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL))) {
    err_set(AQ_USER_DELETED);
    err_set(AQ_RESUMED);
    err_unset(AQ_LOCKED_OLD); // not locked because there's nothing to lock anymore
  } else
    err_set(AQ_ERROR);

  return err;
}

BzRegErrors UserStore::finishReg(const UserInfo &info, std::string const &user_dn, std::string const &mail_dn)
{
  sLog.outLog("finishing registration for %s", info.name.c_str());
  // unlock the player and update the password
  std::string digest = hash(info.password);
  LDAPMod_UserUnlock attr_unlock;
  LDAPMod1 attr_ipass (LDAP_MOD_ADD, "inactivePassword", digest.c_str());
  LDAPMod *reg_mods[3] = { &attr_unlock.mod, &attr_ipass.mod, NULL };

  if(!ldap_check(ldap_modify_ext_s(rootld, user_dn.c_str(), reg_mods, NULL, NULL))) {
    // undo in reverse order, both should succeed under normal circumstances
    if(ldap_check(ldap_delete_ext_s(rootld, mail_dn.c_str(), NULL, NULL)))
      ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));

    return REG_FAIL_GENERIC;
  }

  return REG_SUCCESS;
}

BzRegErrors UserStore::registerMail(const UserInfo &info, uint32_t uid, const char *act_key, bool send_mail, std::string const &user_dn, std::string const &mail_dn)
{
  char uid_str[30]; sprintf(uid_str, "%d", (int)uid);
  return registerMail(info, uid_str, act_key, send_mail, user_dn, mail_dn);
}

int UserStore::getActivationKeyLen()
{
  return 10;
}

void UserStore::getRandomKey(uint8_t *key, int len)
{
  static char alphanum[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  static int alpha_len = (int)strlen(alphanum);

  sRandom.get((char*)key, len);
  for(int i = 0; i < len; i++)
    key[i] = alphanum[key[i] % alpha_len];
  key[len] = '\0';
}

int UserStore::getNewPasswordLen()
{
  return 15;
}

bool UserStore::sendActivationMail(const UserInfo &info, const char *key, const char *tmpl)
{
  if(!execute_reg(re_callsign, info.name.c_str())) return false;
  if(!execute_reg(re_email, info.email.c_str())) return false;

  // it's better to send the confirmation mail multiple times
  // than not send it at all, so do it before registration is finalized
  Mail mail = sMailMan.newMail(tmpl ? tmpl : "user_welcome_inactive");
  mail.replace("{USERNAME}", info.name);
  mail.replace("{U_ACTIVATE}", getActivationURL(info, key));
  return mail.send(info.email);
}

int UserStore::activateUser(const UserInfo &info, const std::string &key)
{
  /* returns: 0 - succes
              1 - already confirmed
              2 - bad key
              3 - other error
   */

  LDAPBaseSearchN<1> mail_search(rootld, getMailDN(info.email).c_str(), "(objectClass=*)",
    "activationKey", 1, getActivationKeyLen());

  char *act_key = mail_search.getResult(0).getNextVal();
  if(!act_key) {
    sLog.outDebug("UserStore: mail %s has no activation key", info.email.c_str());
    return 3;
  }

  if(key != act_key) return 2;

  std::string user_dn = getUserDN(info.name);
  LDAPBaseSearchN<1> pass_search(rootld, user_dn.c_str(), "(inactivePassword=*)",
    "inactivePassword", 1, (int)hashLen());

  char *digest = pass_search.getResult(0).getNextVal();
  if(!digest) // could be some other error but usually it's because the user's active
    return 1;

  LDAPMod1 attr_del_inactive (LDAP_MOD_DELETE, "inactivePassword", digest);
  LDAPMod1 attr_add_active   (LDAP_MOD_ADD,    "userPassword",     digest);
  LDAPMod *attrs[3] = { &attr_del_inactive.mod, &attr_add_active.mod, NULL };

  if( ldap_check( ldap_modify_ext_s(rootld, user_dn.c_str(), attrs, NULL, NULL) ))
    return 0;
  else
    return 3;
}

std::string UserStore::getNamefromUID(uint32_t uid)
{
  char uid_str[30]; sscanf(uid_str, "%d", &uid);
  return getNamefromUID(uid_str);
}

std::string UserStore::getNamefromUID(const char * uid_str)
{
  // return the primary callsign of the user with the given uid
  LDAPSearchN<1> user_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_SUBTREE, "(&(objectClass=person)(userPassword=*))",
    "cn", 1, MAX_CALLSIGN_LEN);
  
  LDAPEntry *entry = user_search.getNextEntry();
  if(!entry) return "";
  char *cn = entry->getResult(0).getNextVal();
  return cn ? cn : "";
}

std::string UserStore::getNameFromMail(const std::string &mail, const std::string &mail_dn)
{ 
  // return the primary callsign of the user with the given email
  LDAPBaseSearchN<1> mail_search(rootld, mail_dn.c_str(), "(objectClass=*)",
    "uid", 1, 30);

  char *uid = mail_search.getResult(0).getNextVal();
  if(!uid) {
    sLog.outError("UserStore: mail %s has no uid", mail.c_str());
    return "";
  }

  return getNamefromUID(uid);
}

BzRegErrors UserStore::registerMail(const UserInfo &info, char * uid_str, const char *act_key, bool send_mail, std::string const &user_dn, std::string const &mail_dn)
{
  uint8_t buf[100];
  if(!act_key) {
    getRandomKey(buf, getActivationKeyLen());
    act_key = (const char*)buf;
  }

  sLog.outLog("UserStore: registering mail %s for %s", info.email.c_str(), info.name.c_str());
  LDAPMod1 attr_oc   (LDAP_MOD_ADD, "objectClass", "email");
  LDAPMod1 attr_uid  (LDAP_MOD_ADD, "uid", uid_str);
  LDAPMod1 attr_mail (LDAP_MOD_ADD, "mail", info.email.c_str());
  LDAPMod1 attr_akey (LDAP_MOD_ADD, "activationKey", act_key);
  LDAPMod *mail_mods[5] = { &attr_oc.mod, &attr_uid.mod, &attr_mail.mod, &attr_akey.mod, NULL };

  while(1) {
    int mail_ret = ldap_add_ext_s(rootld, mail_dn.c_str(), mail_mods, NULL, NULL);
    if(mail_ret != LDAP_SUCCESS) {
      if(mail_ret != LDAP_ALREADY_EXISTS) {
        ldap_check(mail_ret);
        return REG_FAIL_GENERIC;
      }
      
      // the email may come from an unfinished registration
      // if so, roll back the registration and continue
      sLog.outDebug("UserStore: finding owner of existing mail %s", info.email.c_str());

      std::string cn = getNameFromMail(info.email, mail_dn);
      if(cn == "")
        return REG_FAIL_GENERIC;

      if(info.name == cn) {
        sLog.outError("UserStore: mail=%s already registered to cn=%s?", info.email.c_str(), info.name.c_str());
        return REG_FAIL_GENERIC;
      }

      std::string mail_owner_dn = getUserDN(cn);
      uint64_t err = acquireUserLock(mail_owner_dn, cn, 0, USER_LOCK_NO_REASON);
      // if the function succeeded in deleting the existing mail
      // try inserting again 
      if(err_isset(AQ_MAIL_DELETED))
        continue;

      // if the mail was not deleted and there was no other error
      // the email exists and is valid
      if(!err_isset(AQ_ERROR)) {
        sLog.outLog("UserStore: email %s already exists", info.email.c_str());
        // undo: delete the user
        ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));
        return REG_MAIL_EXISTS;
      } else
        return REG_FAIL_GENERIC;
    }

    if(send_mail) sendActivationMail(info, act_key);
    return REG_SUCCESS;
  }
}

// find the uid for a given ldap connection and dn
uint32_t UserStore::getuid(LDAP *ld, const char *dn) const
{
  LDAPBaseSearchN<1> uid_search(ld, dn, "(objectClass=*)",
    "uid", 1, 20);

  uint32_t uid = 0;
  char *uid_str = uid_search.getResult(0).getNextVal();
  if(uid_str)
    sscanf(uid_str, "%u", &uid);
  return uid;
}

// auth the user but only take activated users into account 
uint32_t UserStore::authUser(const UserInfo &info)
{
  if(!execute_reg(re_callsign, info.name.c_str()))
    return 0;

  // bind to the user's dn
  std::string dn = getUserDN(info.name);

  LDAP *ld = NULL;
  if(!bind(ld, sConfig.getStringValue(CONFIG_LDAP_MASTER_ADDR), (const uint8_t*)dn.c_str(), (const uint8_t*)info.password.c_str()))
    return 0;

  uint32_t uid = getuid(ld, dn.c_str());
  unbind(ld);

  return uid;
}

// auth for both active/inactive users
uint32_t UserStore::authUserInGame(const UserInfo &info)
{
  if(!execute_reg(re_callsign, info.name.c_str()))
    return 0;

  LDAPBaseSearchN<3> pass_search(rootld, getUserDN(info.name).c_str(), "(objectClass=person)",
    "uid", 1, 30,
    "userPassword", 1, (int)hashLen(),
    "inactivePassword", 1, (int)hashLen());

  char *uid_str = pass_search.getResult(0).getNextVal();
  if(!uid_str) // no such user
    return 0;

  char *active_pass = pass_search.getResult(1).getNextVal();
  char *inactive_pass = pass_search.getResult(2).getNextVal();
  if(!active_pass && !inactive_pass) {
    sLog.outError("UserStore: user with id %s has no active/inactive password", uid_str);
    return 0;
  } else if(active_pass && inactive_pass) {
    sLog.outError("UserStore: user with id %s has both active/inactive password", uid_str);
    return 0;
  }

  char *pass = active_pass ? active_pass : inactive_pass;
  if(hash(info.password) != pass)
    return 0;

  int uid; sscanf(uid_str, "%d", &uid);
  return uid;
}

bool UserStore::isRegistered(std::string callsign)
{
  std::string dn = getUserDN(callsign);

  // filter out users without a userPassword

  LDAPBaseSearchN<0> user_search(rootld, dn.c_str(), "(userPassword=*)");
  return LDAP_SUCCESS == user_search.getError();
}

const std::string UserStore::getUIDfromName(const char *name) const
{
  LDAPBaseSearchN<1> uid_search(rootld, getUserDN(name).c_str(), "(objectClass=*)",
    "uid", 1, 30);

  char *uid_str = uid_search.getResult(0).getNextVal();
  return uid_str ? uid_str : getEmptyString();
}

std::list<GroupId> UserStore::intersectGroupList(std::string callsign, std::list<GroupId> const &groups, bool all_groups)
{
  if(!callsign.empty()) {
    if( !execute_reg(re_callsign, callsign.c_str()) )
      return std::list<GroupId>();

    return intersectGroupList(getUIDfromName(callsign.c_str()), groups, all_groups);
  } else
    return intersectGroupList(getEmptyString(), groups, all_groups);
}

std::list<GroupId> UserStore::intersectGroupListUID(const std::string & uid_str, std::list<GroupId> const &groups, bool all_groups)
{
  sLog.outLog("UserStore: getting group list for user %s", uid_str.c_str());

  std::list<GroupId> ret;
  if(groups.empty() && !all_groups) return ret;

  std::string filter;
  if(!uid_str.empty()) {
    if(!all_groups)
      filter = "(&(objectClass=groupMembership)(uid=" + uid_str + ")(|";
    else
      filter = "(&(objectClass=groupMembership)(uid=" + uid_str + "))";
  } else if(!all_groups)
    filter = "(&(objectClass=groupMembership)(|";
  else
    filter = "(objectClass=group)";

  if(!all_groups) {
    for(std::list<GroupId>::const_iterator itr = groups.begin(); itr != groups.end(); ++itr)
      if(GROUPID_VALID == validateGroupId(*itr))
        filter += "(&(ou=" + itr->ou + ")(grp=" + itr->grp + "))";
    filter += "))";
  }

  return getGroups(filter, ret);
}

bool UserStore::addToGroup(const std::string &uid_str, const GroupId & gid)
{
  LDAPMod1 attr_oc   (LDAP_MOD_ADD, "objectClass", "groupMembership");
  LDAPMod1 attr_uid  (LDAP_MOD_ADD, "uid", uid_str.c_str());
  LDAPMod1 attr_gn   (LDAP_MOD_ADD, "grp", gid.grp.c_str());
  LDAPMod1 attr_ou   (LDAP_MOD_ADD, "ou", gid.ou.c_str());

  LDAPMod *member_attrs[] = { &attr_oc.mod, &attr_uid.mod, &attr_gn.mod, &attr_ou.mod, NULL };

  return ldap_check( ldap_modify_ext_s(rootld, getMemberDN(uid_str.c_str(), gid).c_str(), member_attrs, NULL, NULL) );
}

std::string UserStore::getUserDN(std::string const &callsign) const
{
  return "cn=" + callsign + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
}

std::string UserStore::getMailDN(std::string const &email) const
{
  return "mail=" + email + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
}

std::string UserStore::getOrgDN(std::string const &org) const
{
  return "ou=" + org + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
}

std::string UserStore::getGroupDN(const GroupId &gid) const
{
  return "grp=" + gid.grp + "," + getOrgDN(gid.ou);
}

std::string UserStore::getMemberDN(const char *uid_str, const GroupId &gid) const
{
  return "uid=" + std::string(uid_str) + "," + getGroupDN(gid);
}

std::string UserStore::getMemberDN(uint32_t uid, const GroupId &gid) const
{
  char uid_str[30]; sprintf(uid_str, "%d", (int)uid);
  return getMemberDN(uid_str, gid);
}

std::list<std::string> UserStore::getUsers(const char *uid)
{
  std::list<std::string> ret;
  std::string filter = "(&(objectclass=person)(uid=" + std::string(uid) + "))";

  LDAPSearchN<1> user_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX), LDAP_SCOPE_ONELEVEL, filter.c_str(),
    "cn", 1, MAX_CALLSIGN_LEN);

  LDAPEntry *entry;
  while(entry = user_search.getNextEntry()) {
    char *cn = entry->getResult(0).getNextVal();
    if(cn && strcmp(cn, "") != 0)
      ret.push_back(cn);
  }

  return ret;
}

ChInfError UserStore::changeUserInfo(std::string for_name, const UserInfo &to_info)
{
  /* FIXME: - this is not interrupt safe */

  std::string &name = for_name;
  const UserInfo& info = to_info;

  bool got_name = true, got_mail = true, got_pass = true; 
  switch(validateUserInfo(info, &got_name, &got_mail, &got_pass)) {
    case INFO_INVALID_NAME: return CHINF_INVALID_CALLSIGN;
    case INFO_INVALID_MAIL: return CHINF_INVALID_EMAIL;
    case INFO_INVALID_PASS: return CHINF_INVALID_PASSWORD;
  }

  if(!got_name && !got_mail && !got_pass)
    return CHINF_SUCCESS;

  if(got_name) sLog.outLog("UserStore: changing the callsign %s to %s", name.c_str(), info.name.c_str());
  if(got_mail) sLog.outLog("UserStore: changing email for %s to %s", name.c_str(), info.email.c_str());
  if(got_pass) sLog.outLog("UserStore: changing password for %s", name.c_str());

  std::string old_user_dn = getUserDN(name);
  std::string new_user_dn = getUserDN(info.name);

  int lock_time = 0;
  if(got_pass) lock_time += (int)sConfig.getIntValue(CONFIG_CHINF_PASS_LOCK_TIME);
  if(got_mail) lock_time += (int)sConfig.getIntValue(CONFIG_CHINF_MAIL_LOCK_TIME);
  if(got_name) lock_time += (int)sConfig.getIntValue(CONFIG_CHINF_NAME_LOCK_TIME);

  // for now use the same lock type for all operations
  UserLockReason lock_type = USER_LOCK_CHINF;

  if(!err_isset2(AQ_SUCCESS, acquireUserLock(old_user_dn, name, lock_time, lock_type)))
    return CHINF_OTHER_ERROR;

  LDAPBaseSearchN<3> user_search; // put here so old_digest doesn't go out of scope
  char *old_digest = NULL;
  char *new_digest = NULL;

  if(got_name || got_mail) 
  {
    // search for the old user's uid,mail,pass
    user_search.run(rootld, old_user_dn.c_str(), "(userPassword=*)",
      "uid", 1, 20,
      "mail", 1, MAX_EMAIL_LEN,
      "userPassword", 1, (int)hashLen());

    char *old_uid = user_search.getResult(0).getNextVal();
    char *old_mail = user_search.getResult(1).getNextVal();
    old_digest = user_search.getResult(2).getNextVal();
    if(!old_uid || !old_mail || !old_digest) {
      sLog.outError("UserStore: cannot find uid,mail,pass for %s", name.c_str());
      return releaseUserLock(old_user_dn, CHINF_OTHER_ERROR, CHINF_OTHER_ERROR);
    }

    if(old_mail == info.email)
      got_mail = false;

    if(got_name) {
      // add a new user with either the same values as the old or
      // if the email or password is being changed as well
      // insert directly with the new value
      BzRegErrors err = addUser(new_user_dn, info.name.c_str(), 
        got_pass ? hash(info.password).c_str() : old_digest, true, 
        got_mail ? info.email.c_str() : old_mail, 
        old_uid, 0);

      if(err != REG_SUCCESS) {
        if(err == REG_USER_EXISTS)
          return releaseUserLock(old_user_dn, CHINF_OTHER_ERROR, CHINF_TAKEN_CALLSIGN);
        else
          return releaseUserLock(old_user_dn, CHINF_OTHER_ERROR, CHINF_OTHER_ERROR);
      }
    }

    if(got_mail) {
      BzRegErrors err = registerMail(UserInfo(name, "", info.email), old_uid, "", false, old_user_dn, getMailDN(info.email));
      if(err != REG_SUCCESS) {
        if(got_name) {
          if(ldap_check( ldap_delete_ext_s(rootld, new_user_dn.c_str(), NULL, NULL) ))
            // cannot release lock until finished or undone
            return CHINF_OTHER_ERROR; 
        }

        if(err == REG_MAIL_EXISTS)
          return releaseUserLock(old_user_dn, CHINF_OTHER_ERROR, CHINF_TAKEN_EMAIL);
        else
          return releaseUserLock(old_user_dn, CHINF_OTHER_ERROR, CHINF_OTHER_ERROR);
      }

      if(!ldap_check( ldap_delete_ext_s( rootld, getMailDN(old_mail).c_str(), NULL, NULL) )) {
        sLog.outError("UserStore: failed to delete mail %s", old_mail);
        // user remains locked, should try to finish this when next acquired
        return CHINF_OTHER_ERROR;
      }
    }
  
    // both groups and emails are liked to the user by uid so no need to move them
  }

  if(got_pass && !got_name) {
    // hash the password
    size_t digest_len = hashLen();
    new_digest = new char[digest_len+1];
    hash((uint8_t*)info.password.c_str(), info.password.length(), (uint8_t*)new_digest);
  }
  
  LDAPMod1 attr_del_pass (LDAP_MOD_DELETE, "userPassword", old_digest);
  LDAPMod1 attr_set_pass (LDAP_MOD_REPLACE, "userPassword", new_digest);
  LDAPMod1 attr_set_mail (LDAP_MOD_REPLACE, "mail", info.email.c_str());
  LDAPMod_UserUnlock attr_lock;

  LDAPMod *attrs[5] = { NULL, NULL, NULL, NULL, NULL };
  int poz = 0;
  // deactivate the old user if a new one was created
  if(got_name) attrs[poz++] = &attr_del_pass.mod;
  // the password should be changed if it was not already added to a new user
  if(got_pass && !got_name) attrs[poz++] = &attr_set_pass.mod;
  // should the mail be modified for the old user if a new one was created ?
  if(got_mail) attrs[poz++] = &attr_set_mail.mod;
  attrs[poz++] = &attr_lock.mod;
  
  bool ret = ldap_check( ldap_modify_ext_s(rootld, old_user_dn.c_str(), attrs, NULL, NULL) );
  if(got_pass && !got_name) delete[] new_digest;

  if(!ret)
    return releaseUserLock(old_user_dn, CHINF_OTHER_ERROR, CHINF_OTHER_ERROR);

  return CHINF_SUCCESS;
}

std::string UserStore::getActivationURL(const UserInfo &info, const std::string &key)
{
  char *escaped_name = curl_easy_escape(NULL, info.name.c_str(), (int)info.name.size());
  char *escaped_mail = curl_easy_escape(NULL, info.email.c_str(), (int)info.email.size());

  std::string ret = "http://" +
    (std::string)(char*)sConfig.getStringValue(CONFIG_WEB_SERVER_NAME) +
    (std::string)(char*)sConfig.getStringValue(CONFIG_WEB_SCRIPT_NAME) + 
    "?action=CONFIRM" 
    "&callsign=" + (std::string)escaped_name +
    "&email=" + (std::string)escaped_mail + 
    "&password=" + key;

  curl_free(escaped_name);
  curl_free(escaped_mail);
  return ret;
}

UserInfoValidity UserStore::validateUserInfo(const UserInfo &info, bool *got_name, bool *got_mail, bool *got_pass)
{
  bool name = true, mail = true, pass = true;
  if(got_name == NULL) got_name = &name;
  if(got_mail == NULL) got_mail = &name;
  if(got_pass == NULL) got_pass = &name;

  if(*got_name == true)
    *got_name = !info.name.empty();
  if(*got_mail == true)
    *got_mail = !info.email.empty();
  if(*got_pass == true)
    *got_pass = !info.password.empty();

  if(*got_name && !execute_reg(re_callsign, info.name.c_str()))
    return INFO_INVALID_NAME;
  if(*got_mail && !execute_reg(re_email, info.email.c_str()))
    return INFO_INVALID_MAIL;
  if(*got_pass && !execute_reg(re_password, info.password.c_str()))
    return INFO_INVALID_PASS;
  return INFO_VALID;
}

UserInfoValidity UserStore::validateUserInfo(const UserInfo &info, bool got_name, bool got_mail, bool got_pass)
{
  return validateUserInfo(info, &got_name, &got_mail, &got_pass);
}

int UserStore::resetPassword(const UserInfo &info)
{
  /* returns: 0 - success
              1 - no such user
              2 - no such email
              3 - ther user has not been activated
              4 - other error
  */

  if(INFO_VALID != validateUserInfo(info, true, true, false))
    return 4;

  sLog.outLog("UserStore: resetting password for %s", info.name.c_str());

  uint8_t pass[1000], key[1000];
  getRandomKey(pass, getNewPasswordLen());
  getRandomKey(key, getActivationKeyLen());

  // check if the mail is correct
  LDAPBaseSearchN<0> mail_search(rootld, getMailDN(info.email).c_str(), "(objectClass=*)");
  if(LDAP_SUCCESS != mail_search.getError())
    return 2;

  // send the reminder email
  Mail mail = sMailMan.newMail("user_activate_passwd");
  mail.replace("{USERNAME}", info.name);
  mail.replace("{PASSWORD}", (char*)pass);
  mail.replace("{U_ACTIVATE}", getActivationURL(info, (char*)key));
  mail.send(info.email);

  std::string digest = hash((char*)pass);

  // NOTE: this will only succeed if the user is active
  // is there any reason to allow inactive users to reset their passwords ?
  LDAPMod1 attr_del (LDAP_MOD_DELETE, "userPassword", NULL);
  LDAPMod1 attr_add (LDAP_MOD_ADD, "userPassword", digest.c_str());
  LDAPMod *attrs[3] = { &attr_del.mod, &attr_add.mod, NULL };

  int ret = ldap_modify_ext_s(rootld, getUserDN(info.name).c_str(), attrs, NULL, NULL);
  if(ret != LDAP_SUCCESS) {
    switch(ret) {
      case LDAP_NO_SUCH_OBJECT: return 1;
      case LDAP_NO_SUCH_ATTRIBUTE: return 3;
      default:
        ldap_check(ret);
        return 4;
    }
  } else
    return 0;
}

int UserStore::resendActivation(const UserInfo &info)
{
  /* returns: 0 - success
              1 - no such user
              2 - no such email
              3 - has already been activated
              4 - other error
  */

  if(INFO_VALID != validateUserInfo(info, true, false, true))
    return 4;

  LDAPBaseSearchN<1> key_search(rootld, getMailDN(info.email).c_str(), "(objectClass=*)",
    "activationKey", 1, getActivationKeyLen());

  char *key = key_search.getResult(0).getNextVal();
  if(!key) {
    if(key_search.getError() != LDAP_NO_SUCH_OBJECT) {
      sLog.outDebug("UserStore: cannot find activation key for email %s", info.email.c_str());
      return 4;
    }
    return 2;
  }

  return sendActivationMail(info, key, "user_resend_inactive") ? 0 : 4;
}

bool UserStore::createOrganization(const std::string &org)
{
  sLog.outLog("UserStore: creating the organization %s", org.c_str());

  std::string org_dn = getOrgDN(org);

  const char *oc_vals[3] = { "organizationalUnit", "extensibleObject", NULL };
  LDAPModN attr_oc      (LDAP_MOD_ADD, "objectClass", oc_vals);
  LDAPMod1 attr_ou      (LDAP_MOD_ADD, "ou", org.c_str());
  LDAPMod *org_mods[3] = { &attr_oc.mod, &attr_ou.mod, NULL };

  return ldap_check( ldap_add_ext_s(rootld, org_dn.c_str(), org_mods, NULL, NULL) );
}

bool UserStore::createGroup(const GroupId &gid)
{
  sLog.outLog("UserStore: creating the group %s.%s", gid.ou.c_str(), gid.grp.c_str());

  std::string group_dn = getGroupDN(gid);

  LDAPMod1 attr_oc     (LDAP_MOD_ADD, "objectClass", "group");
  LDAPMod1 attr_gn     (LDAP_MOD_ADD, "grp", gid.grp.c_str());
  LDAPMod1 attr_ou     (LDAP_MOD_ADD, "ou", gid.ou.c_str());
  LDAPMod *group_mods[] = { &attr_oc.mod, &attr_gn.mod, &attr_ou.mod, NULL };

  return ldap_check( ldap_add_ext_s(rootld, group_dn.c_str(), group_mods, NULL, NULL) );
}

std::list<GroupId> &UserStore::getGroups(const std::string &filter, std::list<GroupId> &ret)
{
  LDAPSearchN<2> group_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX), 
    LDAP_SCOPE_SUBTREE, filter.c_str(),
    "ou", 1, MAX_ORGNAME_LEN,
    "grp", 1, MAX_GROUPNAME_LEN);

  LDAPEntry *entry;
  while(entry = group_search.getNextEntry()) {
    char *ou = entry->getResult(0).getNextVal();
    char *grp = entry->getResult(1).getNextVal();
    if(ou && grp && strcmp(ou, "") != 0 && strcmp(grp, "") != 0)
      ret.push_back(GroupId(ou, grp));
  }
  return ret;
}

// TODO: reuse some code in the filters

OrgFilter::OrgFilter() {
  stream << "(&(objectClass=organizationalUnit)(|";
  changed = false;
}

void OrgFilter::add_org(const char *org)
{
  stream << "(ou=" << org << ")";
  changed = true;
}

bool OrgFilter::finish()
{
  if(!changed) return false;
  stream << "))";
  return true;
}

void OrgFilter::add_owner(const char *uid_str)
{
  stream << "(uidOwner=" << uid_str << ")";
  changed = true;
}

GroupFilter::GroupFilter()
{
  stream << "(&(objectClass=group)(|";
  changed = false;
}

void GroupFilter::add_org(const char *org)
{
  stream << "(ou=" << org << ")";
  changed = true;
}

void GroupFilter::add_org_group(const char *org, const char *grp)
{
  stream << "(&(ou=" << org << ")(grp=" << grp << "))";
  changed = true;
}

bool GroupFilter::finish()
{
  if(!changed) return false;
  stream << "))";
  return true;
}

MemberFilter::MemberFilter()
{
  stream << "(&(objectClass=groupMembership)(|";
  changed = false;
}

void MemberFilter::add_uid(const char *uid)
{
  stream << "(uid=" << uid << ")";
  changed = true;
}

void MemberFilter::add_org(const char *org)
{
  stream << "(ou=" << org << ")";
  changed = true; 
}

void MemberFilter::add_org_group(const char *org, const char *grp)
{
  stream << "(&(ou=" << org << ")(grp=" << grp << "))";
  changed = true;
}

bool MemberFilter::finish()
{
  if(!changed) return false;
  stream << "))";
  return true;
}

UserNameFilter::UserNameFilter()
{
  stream << "(&(objectClass=person)(userPassword=*)(|";
  changed = false;
}

void UserNameFilter::add_uid(const char *uid)
{
  stream << "(uid=" << uid << ")";
  changed = true;
}

bool UserNameFilter::finish()
{
  if(!changed) return false;
  stream << "))";
  return true;
}

bool UserStore::getOrgs(OrgFilter &filter, OrgCallback &callback)
{
  if(!filter.finish()) return true;

  LDAPSearchN<1> org_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_ONELEVEL, filter.stream.str().c_str(),
    "ou", 1, MAX_GROUPNAME_LEN);

  LDAPEntry *orgEntry;
  while(orgEntry = org_search.getNextEntry()) {
    char *ou = orgEntry->getResult(0).getNextVal();
    if(ou && *ou)
      callback.got_org(ou);
  }
  return true;
}

const std::string UserStore::uid2str(uint32_t uid) const
{
  char buf[30]; sprintf(buf, "%d", (int)uid);
  return buf;
}

uint32_t UserStore::checkPerm(char *perm, char * &arg, const char *type, const char *what)
{
  arg = strchr(perm, ' ');
  if(arg) { *arg = '\0'; arg++; }
  uint32_t perm_val; sscanf(perm, "%d", (int*)&perm_val);
  if(!arg) {
    switch(perm_val) {
      case PERM_ADMIN_OF:
        sLog.outError("UserStore: %s %s has perm %s but no arg", type, what, perm);
        return 0;
    }
  }
  return perm_val;
}

bool UserStore::getGroups(GroupFilter &filter, FindGroupsCallback &callback)
{
  if(!filter.finish()) return true;

  // get all groups of the org
  LDAPSearchN<2> org_group_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_SUBTREE, filter.stream.str().c_str(),
    "ou", 1, MAX_ORGNAME_LEN,
    "grp", 1, MAX_GROUPNAME_LEN);

  LDAPEntry *orgGroupEntry;
  while(orgGroupEntry = org_group_search.getNextEntry()) {
    char *ou = orgGroupEntry->getResult(0).getNextVal();
    char *grp = orgGroupEntry->getResult(1).getNextVal();
    if(ou && grp && *ou && *grp)
      callback.got_group(ou, grp);
  }
  return true;
}

bool UserStore::getMembershipInfo(MemberFilter &filter, GroupMemberCallback &callback)
{
  if(!filter.finish())
    return true;

  LDAPSearchN<4> member_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX), 
    LDAP_SCOPE_SUBTREE, filter.stream.str().c_str(),
    "uid", 1, 30,
    "ou", 1, MAX_ORGNAME_LEN,
    "grp", 1, MAX_GROUPNAME_LEN,
    "permission", -1, MAX_PERMISSION_LEN);

  LDAPEntry *memberEntry;
  while(memberEntry = member_search.getNextEntry()) {
    char *uid = memberEntry->getResult(0).getNextVal();
    char *ou = memberEntry->getResult(1).getNextVal();
    char *grp = memberEntry->getResult(2).getNextVal();
    if(!ou || !grp || !uid) {
      sLog.outError("UserStore: incomplete membership info (%s,%s,%s)", (uid ? uid : ""), (ou ? ou : ""), (grp ? grp : ""));
      continue;
    }

    callback.got_group(uid, ou, grp);
  
    LDAPAttr &attr = memberEntry->getResult(2);
    uint32_t perm_val;
    char *perm, *arg;
    while(perm = attr.getNextVal()) {
      if(!(perm_val = checkPerm(perm, arg, "user", uid)))
        continue;

      callback.got_perm(perm_val, perm, arg);
    }
  }
  return true;
}

bool UserStore::getGroupInfo(GroupFilter &filter, GroupInfoCallback &callback)
{
  if(!filter.finish()) return true;

  LDAPSearchN<4> group_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX), 
    LDAP_SCOPE_SUBTREE, filter.stream.str().c_str(),
    "ou", 1, MAX_ORGNAME_LEN,
    "grp", 1, MAX_GROUPNAME_LEN,
    "permission", -1, MAX_PERMISSION_LEN,
    "state", 1, 30);

  LDAPEntry *groupEntry;
  while(groupEntry = group_search.getNextEntry()) {
    char *ou = groupEntry->getResult(0).getNextVal();
    char *grp = groupEntry->getResult(1).getNextVal();
    char *state = groupEntry->getResult(3).getNextVal();
    if(!ou || !grp || !state || !*ou || !*grp || !*state)
      continue;

    uint32_t state_val; sscanf(state, "%d", (int*)&state_val);
    callback.got_group(ou, grp, state_val);

    uint32_t perm_val;
    char *perm, *arg;
    while(perm = groupEntry->getResult(2).getNextVal()) {
      if(perm_val = checkPerm(perm, arg, "group", grp))
        callback.got_perm(perm, arg);
    }
  }
  return true;
}

GroupId::GroupId(const std::string &org_dot_group)
{
  std::string::size_type pos = org_dot_group.find('.');
  if(pos == org_dot_group.npos || pos == org_dot_group.size() - 1) {
    sLog.outError("GroupId: invalid group %s", org_dot_group.c_str());
  } else {
    ou = org_dot_group.substr(0, pos);
    grp = org_dot_group.substr(pos + 1);
  }
}

GroupIdValidity UserStore::validateGroupId(const GroupId &gid)
{
  int ret = 0;
  if(gid.ou.empty() || !execute_reg(re_organization, gid.ou.c_str()))
    ret += GROUPID_INVALID_OU;
  if(gid.grp.empty() || !execute_reg(re_group, gid.grp.c_str()))
    ret += GROUPID_INVALID_GN;

  return (GroupIdValidity)ret;
}

uint32_t UserStore::getTotalOrgs()
{
  LDAPSearchN<0> org_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_ONELEVEL, "(objectClass=organizationalUnit)");
  return (uint32_t)org_search.getEntriesCount();
}

uint32_t UserStore::getTotalGroups()
{
  LDAPSearchN<0> group_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_SUBTREE, "(objectClass=group)");
  return (uint32_t)group_search.getEntriesCount();
}

bool UserStore::getMemberCount(MemberFilter &filter, MemberCountCallback &callback)
{
  if(!filter.finish())
    return true;

  LDAPSearchN<2> member_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_SUBTREE, filter.stream.str().c_str(),
    "ou", 1, MAX_ORGNAME_LEN,
    "grp", 1, MAX_GROUPNAME_LEN);

  // TODO: this can be made faster with ldap_create_sort_keylist
  typedef std::pair<std::string, std::string> GroupPair;
  typedef std::map<GroupPair, uint32_t> GroupMap;
  GroupMap groups;

  LDAPEntry *entry;
  while(entry = member_search.getNextEntry()) {
    char *ou = entry->getResult(0).getNextVal();
    char *grp = entry->getResult(1).getNextVal();
    if(ou && grp && *ou && *grp)
      groups[GroupPair(ou, grp)]++;
  }

  for(GroupMap::iterator itr = groups.begin(); itr != groups.end(); ++itr)
    callback.got_count(itr->first.first.c_str(), itr->first.second.c_str(), itr->second);

  return true;
}

bool UserStore::getUserNames(UserNameFilter &filter, UserNameCallback &callback)
{
  if(!filter.finish())
    return true;

  // return the primary callsign of the given users
  LDAPSearchN<2> user_search(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX),
    LDAP_SCOPE_SUBTREE, filter.stream.str().c_str(),
    "uid", 1, 30,
    "cn", 1, MAX_CALLSIGN_LEN);
  
  LDAPEntry *entry;
  while(entry = user_search.getNextEntry()) {
    char *uid = entry->getResult(0).getNextVal();
    char *cn = entry->getResult(1).getNextVal();
    if(!uid || !cn || !*uid || !*cn) continue;
    callback.got_userName(uid, cn);
  }
  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
