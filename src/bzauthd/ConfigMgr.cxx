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
#include <assert.h>
#include "ConfigMgr.h"

INSTANTIATE_SINGLETON(Config);

void Config::initialize()
{
  registerKey("localport", CONFIG_LOCALPORT, 1234);
  registerKey("ldap_master_addr", CONFIG_LDAP_MASTER_ADDR, "ldap://127.0.0.1");
  registerKey("ldap_suffix", CONFIG_LDAP_SUFFIX, "dc=my-domain,dc=com");
  registerKey("ldap_rootdn", CONFIG_LDAP_ROOTDN, "cn=Manager,dc=my-domain,dc=com");
  registerKey("ldap_password", CONFIG_LDAP_ROOTPW, "secret");
  registerKey("token_expire_delay", CONFIG_TOKEN_EXPIRE_DELAY, 5 * 60 * 1000);
}

Config::Config()
{
  typeRegister.resize(CONFIG_MAX);
  values.resize(CONFIG_MAX);
}

Config::~Config()
{
  for(uint8_t i = 0; i < values.size(); i++)
    if(values[i] != NULL) free(values[i]);
}

void Config::setStringValue(uint16_t key, const uint8_t *value)
{
  assert(lookupType(key) == CONFIG_TYPE_STRING);

  values[key] = (void*)strdup((const char*)value);
}

const uint8_t * Config::getStringValue(uint16_t key)
{
  assert(lookupType(key) == CONFIG_TYPE_STRING);

  return (const uint8_t*) values[key];
}

uint32_t Config::getIntValue(uint16_t key)
{
  assert(lookupType(key) == CONFIG_TYPE_INTEGER);

  return *(uint32_t*) values[key];
}

void Config::setIntValue(uint16_t key, uint32_t value)
{
  assert(lookupType(key) == CONFIG_TYPE_INTEGER);

  values[key] = (void*)malloc(4);
  *(uint32_t*)values[key] = value;
}

void Config::registerKey(std::string stringKey, uint16_t intKey, uint32_t defaultValue)
{
  keyRegister[stringKey] = intKey;
  typeRegister[intKey] = CONFIG_TYPE_INTEGER;
  setIntValue(intKey, defaultValue);
} 

void Config::registerKey(std::string stringKey, uint16_t intKey, const char * defaultValue)
{
  keyRegister[stringKey] = intKey;
  typeRegister[intKey] = CONFIG_TYPE_STRING;
  setStringValue(intKey, (const uint8_t*)defaultValue);
}

uint16_t Config::lookupKey(std::string stringKey)
{
  KeyRegisterType::iterator itr = keyRegister.find(stringKey);
  if(itr != keyRegister.end())
    return itr->second;
  else
    return CONFIG_MAX;
}

uint8_t Config::lookupType(uint16_t key)
{
  if(key < typeRegister.size())
    return typeRegister[key];
  else
    return CONFIG_TYPE_MAX;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8