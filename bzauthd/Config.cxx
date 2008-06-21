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
}

Config::~Config()
{
  for(uint8 i = 0; i < values.size(); i++)
    if(values[i] != NULL) free(values[i]);
}

void Config::setStringValue(uint16 key, const uint8 *value)
{
  if(key >= values.size())
    values.resize(2*key);
  values[key] = (void*)strdup((const char*)value);
}

const uint8 * Config::getStringValue(uint16 key)
{
  if(key >= values.size())
    return NULL;
  return (const uint8*) values[key];
}

uint32 Config::getIntValue(uint16 key)
{
  if(key >= values.size())
    return NULL;
  return *(uint32*) values[key];
}

void Config::setIntValue(uint16 key, uint32 value)
{
  if(key >= values.size())
    values.resize(key+1);
  values[key] = (void*)malloc(4);
  *(uint32*)values[key] = value;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8