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
#include <ldap.h>
#include "ConfigMgr.h"
#include <Log.h>
#include <gcrypt.h>
#include "base64.h"
#include <assert.h>
#include <bzregex.h>

INSTANTIATE_GUARDED_SINGLETON(UserStore)

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

bool ldap_check(int ret)
{
  if(ret != LDAP_SUCCESS) {
    sLog.outError("LDAP %d: %s", ret, ldap_err2string(ret));
    return false;
  }
  else return true;
}

#define LDAP_FCHECK(x) if(!ldap_check(x)) return false
#define LDAP_VCHECK(x) if(!ldap_check(x)) return

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
  if(!compile_reg(re_callsign, CONFIG_CALLSIGN_REGEX)) return false;
  if(!compile_reg(re_password, CONFIG_PASSWORD_REGEX)) return false;
  if(!compile_reg(re_email, CONFIG_EMAIL_REGEX)) return false;

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
  delete[] tmpbuf;
}

struct LDAPMod1
{
  LDAPMod1(int op, const char *type, const char *value)
  {
    mod.mod_op = op;
    mod.mod_type = (char*)type;
    mod.mod_values = values;
    values[0] = (char*)value;
    values[1] = NULL;
  }

  LDAPMod mod;
  char *values[2];
};

struct LDAPModN
{
  LDAPModN(int op, const char *type, const char **values)
  {
    mod.mod_op = op;
    mod.mod_type = (char*)type;
    mod.mod_values = (char**)values;
  }

  LDAPMod mod;
};

class LDAPAttr
{
public:
  friend class LDAPBaseSearch;
  LDAPAttr() {}
  
  LDAPAttr(int req_value_cnt, int max_value_len, const char *attr_name) {
    init(req_value_cnt, max_value_len, attr_name);
  }

  void init(int req_value_cnt, int max_value_len, const char *attr_name) {
    val_req_cnt = req_value_cnt;
    val_max_len = max_value_len;
    cur_val = 0;
    values = NULL;
    attr = (char*)attr_name;
  }

  ~LDAPAttr() {
    ldap_value_free(values);
  }

  int count() {
    return ldap_count_values(values); 
  }

  char * getNext() {
    while(values && values[cur_val] != NULL) {
      
      if(val_max_len != -1) {
        int i;
        for(i = 0; i < val_max_len+1; i++)
          if(values[cur_val][i] == '\0')
            break;
        if(i == val_max_len+1) {
          sLog.outError("invalid value for %s, potential buffer overflow", attr);
          cur_val++;
          continue;
        }
      }

      return values[cur_val++];
    }
    return NULL;
  }

private:
  void setValues(char **vals) {
    values = vals;
    if(val_req_cnt != -1) {
      int cnt = count();
      if(cnt != val_req_cnt) {
        sLog.outError("value count for %s is %d != %d", attr, cnt, val_req_cnt);
        ldap_value_free(values);
        values = NULL;
      }
    }

  }

  char *attr;
  char **values;
  int cur_val;
  int val_max_len;
  int val_req_cnt;
};

class LDAPBaseSearch
{
public:
  LDAPBaseSearch(LDAP *ldap, const char *dn, const char *filter, int attr_count, const char **attrs, LDAPAttr *ldap_attrs)
  {
    ld = ldap;
    result = NULL;
    attr_cnt = attr_count;
    attr_results = ldap_attrs;

    err = ldap_search_s(ld, dn, LDAP_SCOPE_BASE, filter, (char**)attrs, 0, &result);
    
    if(err != LDAP_SUCCESS) {
      sLog.outError("LDAP %d: %s (at search)", err, ldap_err2string(err));
    } else {
      for (LDAPMessage *msg = ldap_first_message(ld, result); msg; msg = ldap_next_message(ld, msg)) {
        if(ldap_msgtype(msg) == LDAP_RES_SEARCH_ENTRY) {
          for(int i = 0; i < attr_cnt; i++)
            attr_results[i].setValues(ldap_get_values(ld, msg, attrs[i]));

          break; // don't care about other messages
        }
      }
    }
  }

  int getError() {
    return err;
  }

  LDAPAttr& getResult(int i) {
    assert(i < attr_cnt);
    return attr_results[i];
  }

  int getResultCount() {
    return err == LDAP_SUCCESS;
  }

  ~LDAPBaseSearch()
  {
    ldap_msgfree(ldap_first_message(ld, result));
  }

private:
  LDAP *ld;
  LDAPMessage *result;
  LDAPAttr *attr_results;
  int attr_cnt;
  int err;
};

template< int N >
class LDAPBaseSearchN {
};

template<>
class LDAPBaseSearchN<1> : public LDAPAttr, public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter,
    const char *attr_name, int req_value_cnt, int max_value_len) :
    LDAPAttr(req_value_cnt, max_value_len, attr_name),
    LDAPBaseSearch(ldap, dn, filter, 1, init_attrs(attr_name), (LDAPAttr*)this)
  {
  }
    
private:
  const char **init_attrs(const char *attr_name)
  {
    attrs[0] = (char*)attr_name;
    attrs[1] = NULL;
    return (const char**)attrs;
  }

  char* attrs[2];
};

template<>
class LDAPBaseSearchN<2> : public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2) :
    LDAPBaseSearch(ldap, dn, filter, 2, init_attrs(attr1, attr2), init_result(
      req_value_cnt1, max_value_len1, attr1,
      req_value_cnt2, max_value_len2, attr2))
  {
  }
    
private:
  const char **init_attrs(const char *attr1, const char *attr2)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = NULL;
    return (const char**)attrs;
  }

  LDAPAttr *init_result(int req_value_cnt1, int max_value_len1, const char *attr1, int req_value_cnt2, int max_value_len2, const char *attr2)
  {
    attr_res[0].init(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init(req_value_cnt2, max_value_len2, attr2);
    return attr_res;
  }

  char* attrs[3];
  LDAPAttr attr_res[2];
};

template<>
class LDAPBaseSearchN<3> : public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ldap, const char *dn, const char *filter, 
    const char *attr1, int req_value_cnt1, int max_value_len1,
    const char *attr2, int req_value_cnt2, int max_value_len2,
    const char *attr3, int req_value_cnt3, int max_value_len3) :
    LDAPBaseSearch(ldap, dn, filter, 2, init_attrs(attr1, attr2, attr3), init_result(
      req_value_cnt1, max_value_len1, attr1,
      req_value_cnt2, max_value_len2, attr2,
      req_value_cnt3, max_value_len3, attr3))
  {
  }
    
private:
  const char **init_attrs(const char *attr1, const char *attr2, const char *attr3)
  {
    attrs[0] = (char*)attr1;
    attrs[1] = (char*)attr2;
    attrs[2] = (char*)attr3;
    attrs[3] = NULL;
    return (const char**)attrs;
  }

  LDAPAttr *init_result(int req_value_cnt1, int max_value_len1, const char *attr1, int req_value_cnt2, int max_value_len2, const char *attr2, int req_value_cnt3, int max_value_len3, const char *attr3)
  {
    attr_res[0].init(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init(req_value_cnt2, max_value_len2, attr2);
    attr_res[3].init(req_value_cnt3, max_value_len3, attr3);
    return attr_res;
  }

  char* attrs[4];
  LDAPAttr attr_res[3];
};

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

  std::string nextuid_dn = "cn=NextUID," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));

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
        if(!ldap_check( ldap_modify_s(rootld, nextuid_dn.c_str(), mod_attrs) )) {
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

  /* NOTE: the "active state" of the account is determined by whether the password is
     given a valid hash. This means that no additional attributes and no additional checks
     are needed during authentication. */

  /* Because mails need to be unique the following algorithms is used:
     - insert the user with an invalidated password (append something to the end)
       (since we need multiple modifications this makes sure that the account cannot
       be used until everything is finishes) and also a timestamp set to NOW() which
       is used as a soft timed lock to avoid concurrency problems
     loop1:
     if (succeeded) {
       loop2:
       - insert the entry having mail=email and cn=user's callsign
       if(failed) {
         - run userExists with the mail's cn to delete the mail if it belongs to
           an unfinished registration
         if(userExists succeeded)
           - goto loop2
         else if(it failed with user exists)
           - fail with mail exists
         else 
           - fail with given error

         - delete the user
         if(with already exists)
           - fail with email exits
         else
           - fail
       }
       - change the user's password and shadowLastChange to the real value
     } else if(failed because it already exists) {
        - run userExists to check for an unfinished registration
        if(userExists succeeded)
          - goto loop1
        else
          - fail with the given error
     } else if(failed for some other reason) {
        - fail;
     } 

     NOTE: As long as all the changes succeed or only fail when expected to (user/mail
     already exits), this should run without concurrency problems and will clean itself up.
     The userExists procedure also ensures that if after an interrupted registration,
     the same callsign or email is registered again, it will be superceded. This means
     that the corruption in the database does not affect the behavior of the registration,
     apart from the time delay in the soft lock. If however nobody registers the same
     name/mail again, the entries will accumulate and will need to be cleaned periodically.
     To do that without shutting down registration, after searching for the invalid
     passwords/emails, just run the userExists procedure, with the exception of waiting
     until the time lock expired and trying again if necessary.
  */

  // insert the user with the next uid
  std::string user_dn = "cn=" + info.name + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
  std::string mail_dn = "mail=" + info.email + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
  BzRegErrors err;

  const char *oc_vals[3] = {"person", "extensibleObject", NULL};
  LDAPModN attr_oc    (LDAP_MOD_ADD, "objectClass", oc_vals);
  LDAPMod1 attr_cn    (LDAP_MOD_ADD, "cn", info.name.c_str());
  LDAPMod1 attr_sn    (LDAP_MOD_ADD, "sn", info.name.c_str());
  char nextuid_str[20]; sprintf(nextuid_str, "%d", nextuid + 1);
  LDAPMod1 attr_uid   (LDAP_MOD_ADD, "uid", nextuid_str);
  LDAPMod1 attr_mail  (LDAP_MOD_ADD, "mail", info.email.c_str());
  // use a negative value as a flag for unfinished registration
  char time_str[30]; sprintf(time_str, "-%d", (int)time(NULL)); // TODO: TimeKeeper + 64bit
  LDAPMod1 attr_time  (LDAP_MOD_ADD, "shadowLastChange", time_str);
  LDAPMod *user_mods[7] = { &attr_oc.mod, &attr_cn.mod, &attr_sn.mod, &attr_uid.mod, &attr_mail.mod, &attr_time.mod, NULL };

  while(1) {
    int user_add_ret = ldap_add_ext_s(rootld, user_dn.c_str(), user_mods, NULL, NULL);
    if(user_add_ret != LDAP_SUCCESS) {
      switch(user_add_ret) {
      case LDAP_ALREADY_EXISTS:
        err = userExists(user_dn, info.name);
        // if the function succeeded in deleting the existing user and its email
        // try inserting again
        if(err == REG_SUCCESS)
          continue;
        if(err == REG_USER_EXISTS) {
          sLog.outDebug("User %s already exists", info.name.c_str());
          return REG_USER_EXISTS;
        }
        return err;
      default:
        ldap_check(user_add_ret);
        return REG_FAIL_GENERIC;
      }
    }
    break;
  }

  err = registerMail(info, user_dn, mail_dn);
  if(err != REG_SUCCESS)
    return err;

  // it's better to send the confirmation mail multiple times
  // than not send it at all, so do it before registration is finalized

  FILE *cmd_pipe;
  std::string cmd = "mail -s \"BZFlag player registration\" " + 
info.email;
  if( (cmd_pipe = popen( cmd.c_str(), "w" )) != NULL ) {
    char alphanum[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string randtext(8, ' ');
    for(int i = 0; i < 8; i++)
      randtext[i] = alphanum[rand() % 35];

    std::string msg = "You have just registered a BZFlag player account with\n"
      "    callsign: " + info.name + "\n"
      "To activate this account, please go to the following URL:\n\n" 
      "http://" +(std::string)(char*)sConfig.getStringValue(CONFIG_WEB_SERVER_NAME) +
      (std::string)(char*)sConfig.getStringValue(CONFIG_WEB_SCRIPT_NAME) + 
      "?action=CONFIRM&email=" + info.email + "&password=" + 
      (std::string)randtext + "\n";
    pclose(cmd_pipe);
  } else
    sLog.outError("UserStore: could not send mail, failed to open pipe");  

  err = updatePassword(info, user_dn, mail_dn);
  if(err == REG_SUCCESS)
    nextuid = 0;

  return err;

  /* TODO: The above mentioned solution to the holes problem can be further improved.
    Each separate entitry that wishes to do registration can acquire a lock on certain range 
    of BZIDs. E.g one server locks ids 1-1000 another 1001-2000 and so on. When a server owns
    a range it can hand them out to new users without needing to check with LDAP. This could
    be done by putting the ranges as multiple values of some attribute and atomically 
    appending LOCKED-timestamp to the value. Each lock will have say a 5 minute timeout...
    (todo: finish the todo, or maybe just do it)
  */
}

BzRegErrors UserStore::userExists(std::string const &user_dn, std::string const &callsign)
{
  sLog.outDebug("checking registration state for %s", callsign.c_str());
  // retrieve the last change timestamp of the user and its email

  /*char buf[100]; 
  // TODO: TimeKeeper + 64bit
  sprintf(time_str, "(shadowLastChange>=%d)", (int)time(NULL) - sConfig.getIntValue(CONFIG_REG_PERIOD)); */
 
  LDAPBaseSearchN<2> user_search(rootld, user_dn.c_str(), "(objectClass=*)",
    "shadowLastChange", 1, 30,
    "mail", 1, MAX_EMAIL_LEN);

  /*
  if(user_search.getResultCount() == 0) {
    // 
    return REG_USER_EXISTS;
  }
  */

  char *lastchange = user_search.getResult(0).getNext();
  char *mail = user_search.getResult(1).getNext();

  if(!lastchange || !mail) {
    sLog.outError("cannot find lastchange or mail for %s", callsign.c_str());
    // without the timestamp, cannot delete email due to concurrency issue
    return REG_FAIL_GENERIC;
  }

  if(lastchange[0] != '-') {
    // a positive timestamp means the registration was finished
    return REG_USER_EXISTS;
  }

  int change_time; // todo 64bit
  sscanf(lastchange, "-%d", &change_time);
  if(change_time >= (int)time(NULL) - (int)sConfig.getIntValue(CONFIG_REG_PERIOD)) {
    sLog.outDebug("registration for %s not complete but not enough time has passed from %d", callsign.c_str(), change_time);
    // the user's registration may not have finished
    // if the last change was not too long ago
    // assume it was successful
    return REG_USER_EXISTS;
  }

  // "soft lock" the current player by changing the timestamp to "-NOW"
  LDAPMod1 attr_time1 (LDAP_MOD_DELETE, "shadowLastChange", lastchange);
  char time_str[30]; sprintf(time_str, "-%d", (int)time(NULL)); // TODO: TimeKeeper + 64bit
  LDAPMod1 attr_time2 (LDAP_MOD_ADD, "shadowLastChange", time_str);
  LDAPMod *time_mods[3] = { &attr_time1.mod, &attr_time2.mod, NULL };

  int ret = ldap_modify_ext_s(rootld, user_dn.c_str(), time_mods, NULL, NULL);
  if(ret != LDAP_SUCCESS) {
    if(ret == LDAP_NO_SUCH_ATTRIBUTE) {
      sLog.outDebug("timestamp already locked for %s", callsign.c_str());
      // some other daemon already locked the timestamp
      // assume it will finish the registration
      return REG_USER_EXISTS;
    } else {
      ldap_check(ret);
      return REG_FAIL_GENERIC;
    }
  }

  sLog.outDebug("checking mail status for %s(%s)", callsign.c_str(), mail);
  std::string mail_dn = "mail=" + std::string(mail) + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
  LDAPBaseSearchN<1> mail_search(rootld, mail_dn.c_str(), "(objectClass=*)",
    "cn", 1, MAX_CALLSIGN_LEN);

  // no mail entry found => the mail was not registered
  // found and registered to this user => the mail was registered and the entry should be removed
  // found and registered to another user => the mail was not registered, only later by another user
  // NOTE: this would have concurrency issues without the "soft lock"

  if(mail_search.getResultCount() != 0) {
    char *cn = mail_search.getResult(0).getNext();
    if(!cn) 
      return REG_FAIL_GENERIC;
    if(callsign == cn) {
      sLog.outLog("found mail from unfinished registration %s(%s), removing", callsign.c_str(), mail);
      if(!ldap_check(ldap_delete_ext_s(rootld, mail_dn.c_str(), NULL, NULL)))
        return REG_FAIL_GENERIC;
    }
  }

  sLog.outLog("removing user %s due to unfinished registration", callsign.c_str());
  // delete the user and try inserting again
  ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));
  return REG_SUCCESS;
}

BzRegErrors UserStore::updatePassword(const UserInfo &info, std::string const &user_dn, std::string const &mail_dn)
{
  // hash the password
  size_t digest_len = hashLen();
  uint8_t *digest = new uint8_t[digest_len];
  hash((uint8_t*)info.password.c_str(), info.password.length(), digest);

  sLog.outLog("finishing registration for %s", info.name.c_str());
  // add the password and change the sign of the timestamp
  LDAPMod1 attr_pwd (LDAP_MOD_ADD, "userPassword", (const char*)digest);
  char time_str[30]; sprintf(time_str, "%d", (int)time(NULL)); // TODO: TimeKeeper + 64bit
  LDAPMod1 attr_time  (LDAP_MOD_REPLACE, "shadowLastChange", time_str);
  LDAPMod *pwd_mods[3] = { &attr_pwd.mod, &attr_time.mod, NULL };

  bool success = ldap_check(ldap_modify_ext_s(rootld, user_dn.c_str(), pwd_mods, NULL, NULL));
  delete[] digest;

  if(!success) {
    // undo in reverse order, both should succeed under normal circumstances
    if(ldap_check(ldap_delete_ext_s(rootld, mail_dn.c_str(), NULL, NULL)))
      ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));

    return REG_FAIL_GENERIC;
  }

  return REG_SUCCESS;
}

BzRegErrors UserStore::registerMail(const UserInfo &info, std::string 
const &user_dn, std::string const &mail_dn)
{
  sLog.outLog("registering mail %s for %s", info.email.c_str(), info.name.c_str());
  const char *oc_vals[3] = {"applicationProcess", "extensibleObject", NULL};
  LDAPModN attr_oc   (LDAP_MOD_ADD, "objectClass", oc_vals);
  LDAPMod1 attr_cn   (LDAP_MOD_ADD, "cn", info.name.c_str());
  LDAPMod1 attr_mail (LDAP_MOD_ADD, "mail", info.email.c_str());
  LDAPMod *mail_mods[4] = { &attr_oc.mod, &attr_cn.mod, 
&attr_mail.mod, NULL };

  while(1) {
    int mail_ret = ldap_add_ext_s(rootld, mail_dn.c_str(), mail_mods, NULL, NULL);
    if(mail_ret != LDAP_SUCCESS) {
      if(mail_ret != LDAP_ALREADY_EXISTS) {
        ldap_check(mail_ret);
        return REG_FAIL_GENERIC;
      }
      
      // the email may come from an unfinished registration
      // if so, roll back the registration and continue
      sLog.outDebug("finding owner of existing mail %s", info.email.c_str());
      LDAPBaseSearchN<1> mail_search(rootld, mail_dn.c_str(), "(objectClass=*)",
        "cn", 1, MAX_CALLSIGN_LEN);

      char *cn = mail_search.getResult(0).getNext();
      if(!cn)
        return REG_FAIL_GENERIC;

      if(info.name == cn) {
        sLog.outError("mail=%s already registered to cn=%s?", info.email.c_str(), info.name.c_str());
        return REG_FAIL_GENERIC;
      }

      std::string mail_owner_dn = "cn=" + std::string(cn) + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
      BzRegErrors err = userExists(mail_owner_dn, std::string(cn));
      // if the function succeeded in deleting the existing user and its email
      // try inserting again 
      if(err == REG_SUCCESS)
        continue;

      // if the registration is finished then the email is considered already registered
      if(err == REG_USER_EXISTS) {
        // delete the user
        sLog.outLog("Email %s already exists", info.email.c_str());
        ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));
        return REG_MAIL_EXISTS;
      } else
        return err;
    }
    return REG_SUCCESS;
  }
}

// find the uid for a given ldap connection and dn
uint32_t UserStore::getuid(LDAP *ld, const char *dn)
{
  LDAPBaseSearchN<1> uid_search(ld, dn, "(objectClass=*)",
    "uid", 1, 20);

  uint32_t uid = 0;
  char *uid_str = uid_search.getResult(0).getNext();
  if(uid_str)
    sscanf(uid_str, "%u", &uid);
  return uid;
}

uint32_t UserStore::authUser(UserInfo &info)
{
  // bind to the user's dn
  std::string dn = "cn=" + info.name + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));

  LDAP *ld = NULL;
  if(!bind(ld, sConfig.getStringValue(CONFIG_LDAP_MASTER_ADDR), (const uint8_t*)dn.c_str(), (const uint8_t*)info.password.c_str()))
    return 0;

  uint32_t uid = getuid(ld, dn.c_str());
  unbind(ld);

  return uid;
}

bool UserStore::isRegistered(std::string callsign)
{
  std::string dn = "cn=" + callsign + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));

  // filter out users without a userPassword

  char *attrs[2] = { (char*)LDAP_NO_ATTRS, NULL };
  LDAPMessage *res = NULL, *msg;
  if(!ldap_check( ldap_search_s(rootld, dn.c_str(), LDAP_SCOPE_BASE, "(userPassword=*)", attrs, 0, &res) )) {
    if(res) ldap_msgfree(ldap_first_message(rootld, res));
    return false;
  }

  bool found = false;
  for (msg = ldap_first_message(rootld, res); msg; msg = ldap_next_message(rootld, msg)) {
    if(ldap_msgtype(msg) == LDAP_RES_SEARCH_RESULT) {
      int errcode;
      char *errmsg;
      if(!ldap_check( ldap_parse_result(rootld, msg, &errcode, NULL, &errmsg, NULL, NULL, 0) ))
        break;
      if(errmsg) {
        if(errmsg[0]) printf("ERROR: %s\n", errmsg);
        ldap_memfree(errmsg);
      }
      if(errcode == LDAP_SUCCESS)
        found = true;
    }
  }

  ldap_msgfree(ldap_first_message(rootld, res));
  return found;
}

std::list<std::string> UserStore::intersectGroupList(std::string callsign, std::list<std::string> const &groups)
{
  sLog.outLog("getting group list for %s", callsign.c_str());

  std::list<std::string> ret;
  if(groups.empty()) return ret;

  /* It seems here is no memberOf attribute for users in OpenLdap, but the groupOfUniqueNames
     objectClass has a member attribute. So getting the groups is still possible but slower. */
  /* It is also possible to retrieve all of the user's groups from the directory
     and intersect it with the interest list on the daemon, but (unless the overhead
     of sending more data is too much) LDAP should be able to find the groups faster if
     it only needs to check a subset for membership (there are likely many more groups than
     what a particular server is interested in) */

  std::string dn = "cn=" + callsign + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
  std::string filter = "(&(objectClass=groupOfUniqueNames)(uniqueMember=" + dn + ")(|";
  for(std::list<std::string>::const_iterator itr = groups.begin(); itr != groups.end(); ++itr)
    filter += "(cn=" + *itr + ")";
  filter += "))";
  
  char *attrs[2] = { (char*)LDAP_NO_ATTRS, NULL };
  LDAPMessage *res, *msg;

  if(!ldap_check( ldap_search_s(rootld, (const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX), LDAP_SCOPE_ONELEVEL, filter.c_str(), attrs, 0, &res) )) {
    if(res) ldap_msgfree(ldap_first_message(rootld, res));
    return ret;
  }

  for (msg = ldap_first_message(rootld, res); msg; msg = ldap_next_message(rootld, msg)) {
    switch(ldap_msgtype(msg)) {
      case LDAP_RES_SEARCH_ENTRY: {
	// found the dn of a group, extract its cn
        char *dn_str = ldap_get_dn(rootld, msg);
        if(!dn_str) 
          sLog.outError("null dn in search result");
        else {
          // TODO: maybe use ldap_str2dn
          char *cn = strstr(dn_str, "cn=");
          if(cn != NULL) {
            char *comma = strchr(cn, ',');
            if(comma) *comma = NULL;
            ret.push_back(cn+3);
          } else
            sLog.outError("found group with no cn, dn=%s", dn_str);
          
          ldap_memfree(dn_str);
        }
      } break;
      case LDAP_RES_SEARCH_RESULT: {
        int errcode;
        char *errmsg;
        if(!ldap_check( ldap_parse_result(rootld, msg, &errcode, NULL, &errmsg, NULL, NULL, 0) ))
          continue;
        if(errmsg) {
          if(errmsg[0]) printf("ERROR: %s\n", errmsg);
          ldap_memfree(errmsg);
        }
        if(errcode != LDAP_SUCCESS) {
          // TODO: handle this
        }
      }
    }
  }

  ldap_msgfree(ldap_first_message(rootld, res));
  return ret;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
