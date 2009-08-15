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

/* The Config class defines a singleton used for
 * storing and retreiving configuration values.
 * The values are indexed by an integer key but one can map string keys to them.
 */

#ifndef __BZAUTHD_CONFIG_H__
#define __BZAUTHD_CONFIG_H__

#include <vector>
#include <string>
#include "Thread.h"

enum ConfTypes
{
  CONFIG_LOCALPORT,
  CONFIG_LDAP_MASTER_ADDR,
  CONFIG_LDAP_SUFFIX,
  CONFIG_LDAP_ROOTDN,
  CONFIG_LDAP_ROOTPW,
  CONFIG_TOKEN_EXPIRE_DELAY,
  CONFIG_REGISTER_LOCK_TIME,
  CONFIG_HTTP_PORT,
  CONFIG_WEB_SERVER_NAME,
  CONFIG_WEB_SCRIPT_NAME,
  CONFIG_CALLSIGN_REGEX,
  CONFIG_PASSWORD_REGEX,
  CONFIG_EMAIL_REGEX,
  CONFIG_CHINF_PASS_LOCK_TIME,
  CONFIG_CHINF_MAIL_LOCK_TIME,
  CONFIG_CHINF_NAME_LOCK_TIME,
  CONFIG_GROUP_REGEX,
  CONFIG_ORGANIZATION_REGEX,
  CONFIG_MAX
};

enum ConfValueTypes
{
  CONFIG_TYPE_INTEGER,
  CONFIG_TYPE_STRING,
  CONFIG_TYPE_MAX
};

class Config : public GuardedSingleton<Config>
{
public:
  /** Config manager functions */
  Config();
  ~Config();
  void initialize();
  /** Functions for setting and retrieving config values */
  void setStringValue(uint16_t key, const uint8_t *value);
  const uint8_t * getStringValue(uint16_t key) const;
  uint32_t getIntValue(uint16_t key) const;
  void setIntValue(uint16_t key, uint32_t value);
  /** Mapping functions for string keys and key types */
  void registerKey(std::string stringKey, uint16_t intKey, uint32_t defaultValue);
  void registerKey(std::string stringKey, uint16_t intKey, const char * defaultValue);
  uint16_t lookupKey(std::string stringKey);
  uint8_t lookupType(uint16_t key) const;
protected:
  void parse();

  typedef UNORDERED_MAP<std::string, uint16_t /*key*/> KeyRegisterType;
  typedef std::vector<uint8_t /*type*/> TypeRegisterType;
  typedef std::vector<void *> ValuesType;

  KeyRegisterType keyRegister;
  TypeRegisterType typeRegister;
  ValuesType values;
};

#define sConfig Config::guard().instance()

#endif // __BZAUTHD_CONFIG_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
