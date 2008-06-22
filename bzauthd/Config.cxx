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

#include "common.h"
#include "Config.h"

INSTANTIATE_SINGLETON(Config);

Config::Config()
{
  typeRegister.resize(CONFIG_MAX);
  values.resize(CONFIG_MAX);

  registerKey("localport", CONFIG_LOCALPORT, CONFIG_TYPE_INTEGER);
}

Config::~Config()
{
  for(uint8 i = 0; i < values.size(); i++)
    if(values[i] != NULL) free(values[i]);
}

void Config::setStringValue(uint16 key, const uint8 *value)
{
  assert(key < values.size());
  assert(typeRegister[key] == CONFIG_TYPE_STRING);

  values[key] = (void*)strdup((const char*)value);
}

const uint8 * Config::getStringValue(uint16 key)
{
  if(key >= values.size())
    return NULL;

  assert(typeRegister[key] == CONFIG_TYPE_STRING);

  return (const uint8*) values[key];
}

uint32 Config::getIntValue(uint16 key)
{
  if(key >= values.size())
    return NULL;

  assert(typeRegister[key] == CONFIG_TYPE_INTEGER);

  return *(uint32*) values[key];
}

void Config::setIntValue(uint16 key, uint32 value)
{
  assert(key < values.size());
  assert(typeRegister[key] == CONFIG_TYPE_INTEGER);

  values[key] = (void*)malloc(4);
  *(uint32*)values[key] = value;
}

void Config::registerKey(std::string stringKey, uint16 intKey, uint8 keyType)
{
  keyRegister[stringKey] = intKey;
  typeRegister[intKey] = keyType;
}

uint16 Config::lookupKey(std::string stringKey)
{
  KeyRegisterType::iterator itr = keyRegister.find(stringKey);
  if(itr != keyRegister.end())
    return itr->second;
  else
    return CONFIG_MAX;
}

uint8 Config::lookupType(uint16 key)
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