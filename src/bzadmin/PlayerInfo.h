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

#ifndef PLAYERINFO_H
#define PLAYERINFO_H

#include <map>

#include "Address.h"
#include "global.h"


/** This struct stores information about a player that is relevant to
    bzadmin. */
struct PlayerInfo {
  PlayerInfo() {}
  PlayerInfo(const std::string& n) : name(n) { }
  std::string name;
  std::string ip;
  TeamColor team;
  int wins;
  int losses;
  int tks;
  bool isRegistered;
  bool isVerified;
  bool isAdmin;
};


typedef std::map<PlayerId, PlayerInfo> PlayerIdMap;


#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
