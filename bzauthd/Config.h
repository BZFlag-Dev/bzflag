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

#include "Platform.h"
#include <vector>
#include <string>
#include <hash_map>
#include "Singleton.h"

enum ConfTypes
{
  CONFIG_LOCALPORT,
  CONFIG_MAX
};

enum ConfValueTypes
{
  CONFIG_TYPE_INTEGER,
  CONFIG_TYPE_STRING,
  CONFIG_TYPE_MAX
};

class Config : public Singleton<Config>
{
public:
  Config();
  ~Config();
  /* Functions for setting and retrieving config values */
  void setStringValue(uint16 key, const uint8 *value);
  const uint8 * getStringValue(uint16 key);
  uint32 getIntValue(uint16 key);
  void setIntValue(uint16 key, uint32 value);
  /* Mapping functions for string keys and key types */
  void registerKey(std::string stringKey, uint16 intKey, uint8 keyType);
  uint16 lookupKey(std::string stringKey);
  uint8 lookupType(uint16 key);
protected:
  typedef std::hash_map<std::string, uint16 /*key*/> KeyRegisterType;
  typedef std::vector<uint8 /*type*/> TypeRegisterType;
  typedef std::vector<void *> ValuesType;

  KeyRegisterType keyRegister;
  TypeRegisterType typeRegister;
  ValuesType values;
};

#define sConfig Config::instance()

#endif // __BZAUTHD_CONFIG_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8