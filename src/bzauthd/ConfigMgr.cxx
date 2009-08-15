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

#include <assert.h>
#include <string.h>
#include "ConfigMgr.h"
#include <plugin_config.h>

INSTANTIATE_GUARDED_SINGLETON(Config)

void Config::initialize()
{
  registerKey("localport", CONFIG_LOCALPORT, 1234);
  registerKey("ldap_master_addr", CONFIG_LDAP_MASTER_ADDR, "ldap://127.0.0.1");
  registerKey("ldap_suffix", CONFIG_LDAP_SUFFIX, "dc=my-domain,dc=com");
  registerKey("ldap_rootdn", CONFIG_LDAP_ROOTDN, "cn=Manager,dc=my-domain,dc=com");
  registerKey("ldap_password", CONFIG_LDAP_ROOTPW, "secret");
  registerKey("token_expire_delay", CONFIG_TOKEN_EXPIRE_DELAY, 5 * 60 * 1000);
  registerKey("register_lock_time", CONFIG_REGISTER_LOCK_TIME, 5 * 60);
  registerKey("chinf_pass_lock_time", CONFIG_CHINF_PASS_LOCK_TIME, 5 * 60);
  registerKey("chinf_mail_lock_time", CONFIG_CHINF_MAIL_LOCK_TIME, 5 * 60);
  registerKey("chinf_name_lock_time", CONFIG_CHINF_NAME_LOCK_TIME, 10 * 60);
  registerKey("web_server_name", CONFIG_WEB_SERVER_NAME, "my.bzflag.org");
  registerKey("web_script_name", CONFIG_WEB_SCRIPT_NAME, "/db");
  registerKey("http_port", CONFIG_HTTP_PORT, "88");
  registerKey("callsign_regex", CONFIG_CALLSIGN_REGEX, "^[][[:alnum:] +_-]{2,20}$");
  registerKey("password_regex", CONFIG_PASSWORD_REGEX, "^[[:print:]]{3,255}$");
  registerKey("email_regex", CONFIG_EMAIL_REGEX, "^[[:alnum:]._%+-]+@[[:alnum:].-]+\\.[[:alpha:]]{2,4}$");
  registerKey("group_regex", CONFIG_GROUP_REGEX, "^[[:alnum:]]{3,40}$");
  registerKey("organization_regex", CONFIG_ORGANIZATION_REGEX, "^[[:alnum:]]{3,40}$");

  parse();
}

void Config::parse()
{
  PluginConfig config("bzauthd.conf");
  for(KeyRegisterType::iterator itr = keyRegister.begin(); itr != keyRegister.end(); ++itr) {
    std::string value = config.item("global", itr->first);
    if(value != "")
      setStringValue(itr->second, (const uint8_t*)value.c_str());
  }
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
  if(lookupType(key) == CONFIG_TYPE_STRING) {
    values[key] = (void*)strdup((const char*)value);
  } else if(lookupType(key) == CONFIG_TYPE_INTEGER) {
    uint32_t intValue;
    sscanf((const char*)value, "%d", (int*)&intValue);
    setIntValue(key, intValue); 
  } else
    assert(false);
}

const uint8_t * Config::getStringValue(uint16_t key) const
{
  assert(lookupType(key) == CONFIG_TYPE_STRING);

  return (const uint8_t*) values[key];
}

uint32_t Config::getIntValue(uint16_t key) const
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

uint8_t Config::lookupType(uint16_t key) const
{
  if(key < typeRegister.size())
    return typeRegister[key];
  else
    return CONFIG_TYPE_MAX;
}

// hack to be able to reuse plugin_config
// normally these are defined in bzfsAPI.cxx and plugin_utils.cxx

#ifdef _WIN32
#  define BZF_API __declspec(dllexport)
#else
#  define BZF_API
#endif

BZF_API void bz_debugMessagef(int /*_level*/, const char * /*fmt*/, ...)
{
}

const std::string& makelower(std::string& s)
{
  for (std::string::iterator i=s.begin(), end=s.end(); i!=end; ++i)
    *i = ::tolower(*i);

  return s;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
