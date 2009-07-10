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

INSTANTIATE_SINGLETON(UserStore)

UserStore::UserStore() : rootld(NULL)
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


BzRegErrors UserStore::registerUser(UserInfo &info)
{
  /* If multiple sources are allowed to register users,
     the highest uid must be atomically fetch/incremented.
     First the value stored in cn=NextUID's uid is retrieved then
     that value is deleted and the incremented value is added in one atomic operation.
     If the operation fails it is because someone changed the value of next uid
     between the search and modify operation and the fetch/increment is retried */

  std::string dn = "cn=NextUID," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));
  char *search_attrs[2] = { "uid", NULL };

  int nextuid = 0;
  for(int i = 0; i < 4; i++) {
    LDAPMessage *res = NULL, *msg;
    if(!ldap_check( ldap_search_s(rootld, dn.c_str(), LDAP_SCOPE_BASE, "(objectClass=*)", search_attrs, 0, &res) )) {
      if(res) ldap_msgfree(ldap_first_message(rootld, res));
      sLog.outError("cannot find NextUID");
      continue;
    }

    for (msg = ldap_first_message(rootld, res); msg; msg = ldap_next_message(rootld, msg)) {
      if(ldap_msgtype(msg) == LDAP_RES_SEARCH_ENTRY) {
        char **values = ldap_get_values(rootld, msg, "uid");
        int nrvalues = ldap_count_values(values);

        if(nrvalues != 1) {
          sLog.outError("invalid number of values for uid of NextUID");
          ldap_value_free(values);
          break;
        }
        
        if(strnlen(values[0], 20) >= 20) {
          sLog.outError("invalid uid value in NextUID, potential buffer overflow");
          ldap_value_free(values);
          break;
        }

        sscanf(values[0], "%d", &nextuid);
        if(nextuid < 1) {
          sLog.outError("invalid nextuid found: %d", nextuid);
          ldap_value_free(values);
          break;
        }

        char newvalue[20];
        sprintf(newvalue, "%d", nextuid + 1);

        LDAPMod1 attr_old(LDAP_MOD_DELETE, "uid", values[0]);
        LDAPMod1 attr_new(LDAP_MOD_ADD, "uid", newvalue);
        LDAPMod *mod_attrs[3] = { &attr_old.mod, &attr_new.mod, NULL };
        if(!ldap_check( ldap_modify_s(rootld, dn.c_str(), mod_attrs) )) {
          sLog.outDebug("nextuid modify failed");
          nextuid = 0;
        }

        ldap_value_free(values);
        break;  // don't care about any other messages
      }
    }
    ldap_msgfree(ldap_first_message(rootld, res));

    if(nextuid < 1) {
      sLog.outDebug("cannot fetch-increment NextUID, retry number %d", i + 1);
      nextuid = 0;
    } else
      break;
  }

  if(nextuid < 1)
    return REG_FAIL_GENERIC;

  // insert the user with the next uid
  dn = "cn=" + info.name + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));

  char *oc_vals[4] = {"pilotPerson", "uidObject", "shadowAccount", NULL};
  LDAPModN attr_oc    (LDAP_MOD_ADD, "objectClass", oc_vals);
  LDAPMod1 attr_cn    (LDAP_MOD_ADD, "cn", info.name.c_str());
  LDAPMod1 attr_sn    (LDAP_MOD_ADD, "sn", info.name.c_str());
  LDAPMod1 attr_pwd   (LDAP_MOD_ADD, "userPassword", info.password.c_str());
  char nextuid_str[20]; sprintf(nextuid_str, "%d", nextuid);
  LDAPMod1 attr_uid   (LDAP_MOD_ADD, "uid", nextuid_str);
  LDAPMod1 attr_mail  (LDAP_MOD_ADD, "rfc822Mailbox", "nobody@nowhere.com");
  LDAPMod *add_attrs[7] = { &attr_oc.mod, &attr_cn.mod, &attr_sn.mod, &attr_pwd.mod, &attr_uid.mod, &attr_mail.mod, NULL };

  int ret = ldap_add_ext_s(rootld, dn.c_str(), add_attrs, NULL, NULL);
  switch(ret) {
    case LDAP_SUCCESS:
      return REG_SUCCESS; 
    default:
      sLog.outError("LDAP %d: %s", ret, ldap_err2string(ret));
      return REG_FAIL_GENERIC;
  }
}

bool UserStore::authUser(UserInfo &info)
{
  std::string dn = "cn=" + info.name + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));

  LDAP *ld = NULL;
  bool ret = bind(ld, sConfig.getStringValue(CONFIG_LDAP_MASTER_ADDR), (const uint8_t*)dn.c_str(), (const uint8_t*)info.password.c_str()); 
  unbind(ld);
  return ret;
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
