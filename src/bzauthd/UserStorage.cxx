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
#include "UserStorage.h"
#define LDAP_DEPRECATED 1
#include <ldap.h>
#include "ConfigMgr.h"
#include <Log.h>
#include <gcrypt.h>
#include "base64.h"
#include <AuthProtocol.h>
#include <assert.h>

INSTANTIATE_SINGLETON(UserStore)

UserStore::UserStore() : rootld(NULL), nextuid(0)
{
}

UserStore::~UserStore()
{
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

bool UserStore::initialize()
{
  mail_dn = "cn=MailMaster," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
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
  LDAPModN(int op, const char *type, char **values)
  {
    mod.mod_op = op;
    mod.mod_type = (char*)type;
    mod.mod_values = values; 
  }

  LDAPMod mod;
};

class LDAPAttr
{
public:
  friend class LDAPBaseSearch;
  LDAPAttr() {}
  
  LDAPAttr(int req_value_cnt, int max_value_len, char *attr_name) {
    init(req_value_cnt, max_value_len, attr_name);
  }

  void init(int req_value_cnt, int max_value_len, char *attr_name) {
    val_req_cnt = req_value_cnt;
    val_max_len = max_value_len;
    cur_val = 0;
    values = NULL;
    attr = attr_name;
  }

  ~LDAPAttr() {
    ldap_value_free(values);
  }

  int count() {
    return ldap_count_values(values); 
  }

  char * getNext() {
    while(values && values[cur_val] != NULL) {
      if((int)strnlen(values[cur_val], val_max_len) < val_max_len)
        return values[cur_val];
      sLog.outError("invalid value for %s, potential buffer overflow", attr);
      cur_val++;
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
  LDAPBaseSearch(LDAP *ldap, const char *dn, const char *filter, int attr_count, char **attrs, LDAPAttr *ldap_attrs)
  {
    ld = ldap;
    result = NULL;
    attr_cnt = attr_count;

    err = ldap_search_s(ld, dn, LDAP_SCOPE_BASE, filter, attrs, 0, &result);
    
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
  LDAPBaseSearchN(LDAP *ld, const char *dn, const char *filter,
    char *attr, int req_value_cnt, int max_value_len) :
    LDAPAttr(req_value_cnt, max_value_len, attr),
    LDAPBaseSearch(ld, dn, filter, 1, init_attrs(attr), (LDAPAttr*)this)
  {
  }
    
private:
  char **init_attrs(char *attr)
  {
    attrs[0] = attr;
    attrs[1] = NULL;
    return attrs;
  }

  char* attrs[2];
};

template<>
class LDAPBaseSearchN<2> : public LDAPBaseSearch {
public:
  LDAPBaseSearchN(LDAP *ld, const char *dn, const char *filter, 
    char *attr1, int req_value_cnt1, int max_value_len1,
    char *attr2, int req_value_cnt2, int max_value_len2) :
    LDAPBaseSearch(ld, dn, filter, 2, init_attrs(attr1, attr2), init_result(
      req_value_cnt1, max_value_len1, attr1,
      req_value_cnt2, max_value_len2, attr2))
  {
  }
    
private:
  char **init_attrs(char *attr1, char *attr2)
  {
    attrs[0] = attr1;
    attrs[1] = attr2;
    attrs[2] = NULL;
    return attrs;
  }

  LDAPAttr *init_result(int req_value_cnt1, int max_value_len1, char *attr1, int req_value_cnt2, int max_value_len2, char *attr2)
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
  LDAPBaseSearchN(LDAP *ld, const char *dn, const char *filter, 
    char *attr1, int req_value_cnt1, int max_value_len1,
    char *attr2, int req_value_cnt2, int max_value_len2,
    char *attr3, int req_value_cnt3, int max_value_len3) :
    LDAPBaseSearch(ld, dn, filter, 2, init_attrs(attr1, attr2, attr3), init_result(
      req_value_cnt1, max_value_len1, attr1,
      req_value_cnt2, max_value_len2, attr2,
      req_value_cnt3, max_value_len3, attr3))
  {
  }
    
private:
  char **init_attrs(char *attr1, char *attr2, char *attr3)
  {
    attrs[0] = attr1;
    attrs[1] = attr2;
    attrs[2] = attr3;
    attrs[3] = NULL;
    return attrs;
  }

  LDAPAttr *init_result(int req_value_cnt1, int max_value_len1, char *attr1, int req_value_cnt2, int max_value_len2, char *attr2, int req_value_cnt3, int max_value_len3, char *attr3)
  {
    attr_res[0].init(req_value_cnt1, max_value_len1, attr1);
    attr_res[1].init(req_value_cnt2, max_value_len2, attr2);
    attr_res[3].init(req_value_cnt3, max_value_len3, attr3);
    return attr_res;
  }

  char* attrs[4];
  LDAPAttr attr_res[3];
};

BzRegErrors UserStore::registerUser(UserInfo &info)
{
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
          sLog.outDebug("nextuid modify failed");
          nextuid = 0;
        } else
          break;
      }

      sLog.outDebug("cannot fetch-increment NextUID, retry number %d", i + 1);
    }

    if(nextuid < 1) {
      nextuid = 0;
      return REG_FAIL_GENERIC;
    }
  }

  /* NOTE: the "active state" of the account is determined by whether the password is
     given a valid hash. This means that no additional attributes and no additional checks
     are needed during authentication. */

  /* Because mails need to be unique the following algorithms is used:
     - insert the user with an invalidated password (append something to the end)
       (since we need multiple modifications this makes sure that the account cannot
        be used until everything is finishes) ...
     if (succeeded) {
       - insert the entry having mail=email and uid=user's bzid
       if(failed) {
         - delete the user
         if(with already exists)
           - fail with email exits
         else
           - fail
       }
       - change the user's password and shadowLastChange to the real value
     } else if(failed because it already exists) {
       ...
     } else if(failed for some other reason) {
        - fail;
     } 

     NOTE: As long as all the changes succeed or only fail when expected to (user/mail
     already exits), this should run without concurrency problems and will clean itself up.
     To clean out corrupted user/email entries periodically without shutting down
     registration, after searching for the invalid passwords/emails, the entries should 
     not be deleted immediately but only at the next period. The period should be big enough
     to ensure that any ongoing registration at the previous period has been finished, 
     and small enough for minimal discomfort to the user in such events.
  */

  // insert the user with the next uid
  std::string user_dn = "cn=" + info.name + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
  BzRegErrors err;

  while(1) {
    char *oc_vals[3] = {"person", "extensibleObject", NULL};
    LDAPModN attr_oc    (LDAP_MOD_ADD, "objectClass", oc_vals);
    LDAPMod1 attr_cn    (LDAP_MOD_ADD, "cn", info.name.c_str());
    LDAPMod1 attr_sn    (LDAP_MOD_ADD, "sn", info.name.c_str());
    LDAPMod1 attr_pwd   (LDAP_MOD_ADD, "userPassword", (info.password+"::::").c_str());
    char nextuid_str[20]; sprintf(nextuid_str, "%d", nextuid + 1);
    LDAPMod1 attr_uid   (LDAP_MOD_ADD, "uid", nextuid_str);
    LDAPMod1 attr_mail  (LDAP_MOD_ADD, "mail", info.email.c_str());
    // use a negative value as a flag for unfinished registration
    char time_str[30]; sprintf(time_str, "-%d", (int)time(NULL)); // TODO: TimeKeeper + 64bit
    LDAPMod1 attr_time  (LDAP_MOD_ADD, "shadowLastChange", time_str);
    LDAPMod *user_mods[8] = { &attr_oc.mod, &attr_cn.mod, &attr_sn.mod, &attr_pwd.mod, &attr_uid.mod, &attr_mail.mod, &attr_time.mod, NULL };

    int user_add_ret = ldap_add_ext_s(rootld, user_dn.c_str(), user_mods, NULL, NULL);
    if(user_add_ret != LDAP_SUCCESS) {
      switch(user_add_ret) {
      case LDAP_ALREADY_EXISTS:
        err = userExists(user_dn, nextuid+1);
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
    
    err = registerMail(info, nextuid + 1, user_dn);
    if(err != REG_SUCCESS)
      return err;

    err = updatePassword(info, user_dn);
    if(err == REG_SUCCESS)
      nextuid = 0;

    return err;
  }

  /* TODO: The above mentioned solution to the holes problem can be further improved.
    Each separate entitry that wishes to do registration can acquire a lock on certain range 
    of BZIDs. E.g one server locks ids 1-1000 another 1001-2000 and so on. When a server owns
    a range it can hand them out to new users without needing to check with LDAP. This could
    be done by putting the ranges as multiple values of some attribute and atomically 
    appending LOCKED-timestamp to the value. Each lock will have say a 5 minute timeout...
    (todo: finish the todo, or maybe just do it)
  */
}

BzRegErrors UserStore::userExists(std::string &user_dn, uint32_t uid)
{
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
      // some other daemon already locked the password
      // assume it will finish the registration
      return REG_USER_EXISTS;
    } else {
      ldap_check(ret);
      return REG_FAIL_GENERIC;
    }
  }
  
  LDAPMod1 attr_mail (LDAP_MOD_DELETE, "mail", mail);
  char uid_str[20]; sprintf(uid_str, "%d", uid);
  LDAPMod1 attr_uid (LDAP_MOD_DELETE, "mail", uid_str);
  LDAPMod *mail_mods[3] = { &attr_mail.mod, &attr_uid.mod, NULL };

  int mail_ret = ldap_modify_ext_s(rootld, mail_dn.c_str(), mail_mods, NULL, NULL);
  // both values present => the mail was registered and now it was successfully removed
  // only uid or both absent => the registration was interrupted before registering the mail
  // only email is present => somebody registered the mail to another uid since then
  if(mail_ret != LDAP_SUCCESS && mail_ret != LDAP_NO_SUCH_ATTRIBUTE) {
    ldap_check(mail_ret);
    return REG_FAIL_GENERIC;
  }

  // delete the user and try inserting again
  ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));
  return REG_SUCCESS;
}

BzRegErrors UserStore::updatePassword(UserInfo &info, std::string &user_dn)
{
  // atomically replace the invalidated password
  LDAPMod1 attr_pwd1 (LDAP_MOD_DELETE, "userPassword", (info.password+"::::").c_str());
  LDAPMod1 attr_pwd2 (LDAP_MOD_ADD, "userPassword", info.password.c_str());
  char time_str[30]; sprintf(time_str, "%d", (int)time(NULL)); // TODO: TimeKeeper + 64bit
  LDAPMod1 attr_time  (LDAP_MOD_REPLACE, "shadowLastChange", time_str);
  LDAPMod *pwd_mods[4] = { &attr_pwd1.mod, &attr_pwd2.mod, &attr_time.mod, NULL };

  if(!ldap_check(ldap_modify_ext_s(rootld, user_dn.c_str(), pwd_mods, NULL, NULL))) {
    // undo in reverse order, both should succeed under normal circumstances
    if(ldap_check(ldap_delete_ext_s(rootld, mail_dn.c_str(), NULL, NULL)))
      ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));

    return REG_FAIL_GENERIC;
  }

  return REG_SUCCESS;
}

BzRegErrors UserStore::registerMail(UserInfo &info, uint32_t uid, std::string &user_dn)
{
  LDAPMod1 attr_mail (LDAP_MOD_ADD, "mail", info.email.c_str());
  char uid_str[20]; sprintf(uid_str, "%d", uid);
  LDAPMod1 attr_uid (LDAP_MOD_ADD, "mail", uid_str);
  LDAPMod *mail_mods[3] = { &attr_mail.mod, &attr_uid.mod, NULL };

  int mail_ret = ldap_modify_ext_s(rootld, mail_dn.c_str(), mail_mods, NULL, NULL);
  if(mail_ret != LDAP_SUCCESS) {
    // delete the user
    ldap_check(ldap_delete_ext_s(rootld, user_dn.c_str(), NULL, NULL));

    if(mail_ret == LDAP_ALREADY_EXISTS) {
      sLog.outDebug("Email %s already exists", info.email.c_str());
      return REG_MAIL_EXISTS;
    } else {
      ldap_check(mail_ret);
      return REG_FAIL_GENERIC;
    }
  }
  return REG_SUCCESS;
}

// find the uid for a given ldap connection and dn
uint32_t UserStore::getuid(LDAP *ld, const char *dn)
{
  char *search_attrs[2] = { "uid", NULL };

  LDAPMessage *res = NULL, *msg;
  if(!ldap_check( ldap_search_s(ld, dn, LDAP_SCOPE_BASE, "(objectClass=*)", search_attrs, 0, &res) )) {
    if(res) ldap_msgfree(ldap_first_message(ld, res));
    sLog.outError("cannot find uid for %s", dn);
    return 0;
  }

  uint32_t uid = 0;
  for (msg = ldap_first_message(ld, res); msg; msg = ldap_next_message(ld, msg)) {
    if(ldap_msgtype(msg) == LDAP_RES_SEARCH_ENTRY) {
      char **values = ldap_get_values(ld, msg, "uid");
      int nrvalues = ldap_count_values(values);

      if(nrvalues != 1) {
        sLog.outError("invalid number of uids for %s", dn);
        break;
      }
      
      if(strnlen(values[0], 20) >= 20) {
        sLog.outError("invalid uid value in %s, potential buffer overflow", dn);
        break;
      }

      sscanf(values[0], "%d", &uid);
      if(uid < 1) {
        sLog.outError("invalid uid found for %s: %d", dn, uid);
        uid = 0;
        break;
      }

      break;  // don't care about any other messages
    }
  }
  ldap_msgfree(ldap_first_message(ld, res));
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

  char *attrs[2] = { (char*)LDAP_NO_ATTRS, NULL };
  LDAPMessage *res = NULL, *msg;
  if(!ldap_check( ldap_search_s(rootld, dn.c_str(), LDAP_SCOPE_BASE, "(objectClass=*)", attrs, 0, &res) )) {
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
