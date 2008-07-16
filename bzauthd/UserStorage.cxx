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

#include "Common.h"
#include "UserStorage.h"
#define LDAP_DEPRECATED 1
#include <ldap.h>
#include "Config.h"
#include "Log.h"
#include <gcrypt.h>
#include "base64.h"

INSTANTIATE_SINGLETON(UserStore)

UserStore::UserStore() : ld(NULL)
{
}

bool ldap_check(int ret)
{
  if(ret != LDAP_SUCCESS)
  {
    sLog.outError("LDAP %d: %s", ret, ldap_err2string(ret));
    return false;
  }
  else return true;
}

#define LDAP_FCHECK(x) if(!ldap_check(x)) return false
#define LDAP_VCHECK(x) if(!ldap_check(x)) return

void UserStore::unbind()
{
  if(ld)
  {
    LDAP_VCHECK( ldap_unbind(ld) );
    sLog.outLog("UserStore: unbound");
  }
}

bool UserStore::bind(const uint8 *master_addr, const uint8 *root_dn, const uint8 *root_pw)
{
  unbind();
  sLog.outLog("UserStore: binding to %s, with root dn %s", master_addr, root_dn);

  int version = LDAP_VERSION3;
  LDAP_FCHECK( ldap_initialize(&ld, (char*)master_addr) );
	LDAP_FCHECK( ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version) );
  LDAP_FCHECK( ldap_simple_bind_s(ld, (char*)root_dn, (char*)root_pw) );
  return true;
}

bool UserStore::initialize()
{
  return bind(sConfig.getStringValue(CONFIG_LDAP_MASTER_ADDR), sConfig.getStringValue(CONFIG_LDAP_ROOTDN), sConfig.getStringValue(CONFIG_LDAP_ROOTPW));
}

size_t UserStore::hashLen()
{
  return (size_t)gcry_md_get_algo_dlen(GCRY_MD_MD5) / 2 * 3;
}

void UserStore::hash(uint8 *message, size_t message_len, uint8 *digest)
{
  int md5len = gcry_md_get_algo_dlen(GCRY_MD_MD5);
  uint8 *tmpbuf = new uint8[md5len];
  gcry_md_hash_buffer(GCRY_MD_MD5, tmpbuf, message, message_len);
  base64::encode(tmpbuf, tmpbuf + md5len, digest);
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

void UserStore::registerUser(UserInfo &info)
{
  std::string dn = "cn=" + info.name + "," + std::string((const char*)sConfig.getStringValue(CONFIG_LDAP_SUFFIX));

  LDAPMod1 attr_cn(LDAP_MOD_ADD, "cn", info.name.c_str());
  LDAPMod1 attr_sn(LDAP_MOD_ADD, "sn", info.name.c_str());
  LDAPMod1 attr_pwd(LDAP_MOD_ADD, "userPassword", info.password.c_str());
  LDAPMod *attrs[3] = { &attr_cn.mod, &attr_sn.mod, &attr_pwd.mod };

  LDAP_VCHECK( ldap_add_ext_s(ld, dn.c_str(), attrs, NULL, NULL) );
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8