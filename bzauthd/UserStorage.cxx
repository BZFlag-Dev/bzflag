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

void UserStore::registerUser(UserInfo &info)
{
  /*LDAPMod *attrs[NUM_ATTRS+1];
  LDAPMod attr[NUM_ATTRS];

  for(int i = 0; i < NUM_ATTRS; i++)
  {
      attr[i].mod_op = LDAP_MOD_ADD; 
      attr[i].mod_type = t[i].attr;
      attr[i].mod_values = t[i].vals;
      attrs[i] = &attr[i];
  }
  attrs[NUM_ATTRS] = NULL;

  int msgid;
  TEST( ldap_add_ext(ld, "cn=Barbara Jensen,dc=my-domain,dc=com", attrs, NULL, NULL, &msgid) );*/
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8