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

#ifndef __BZAUTHD_CONFIG_H__
#define __BZAUTHD_CONFIG_H__

#include "Platform.h"
#include <vector>
#include <string>
#include "Singleton.h"

enum ConfTypes
{
  CONFIG_LOCALPORT,
  CONFIG_TYPE_MAX
};

class Config : public Singleton<Config>
{
public:
  Config();
  ~Config();
  void setStringValue(uint16 key, const uint8 *value);
  const uint8 * getStringValue(uint16 key);
  uint32 getIntValue(uint16 key);
  void setIntValue(uint16 key, uint32 value);
protected:
  std::vector<void *> values;
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