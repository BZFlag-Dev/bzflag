/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __PERMISSIONS_H__
#define __PERMISSIONS_H__

// work around an ugly STL bug in BeOS
// FIXME someone test whether it is still needed
#ifdef __BEOS__
#define private public
#endif
#include <bitset>
#ifdef __BEOS__
#undef private
#endif
#include <vector>
#include <map>

#include "TimeKeeper.h"

struct PlayerAccessInfo
{
  // player access info
  enum AccessPerm
    {
      idleStats = 0,
      lagStats,
      flagMod,
      flagHistory,
      lagwarn,
      kick,
      ban,
      banlist,
      unban,
      countdown,
      endGame,
      shutdownServer,
      superKill,
      playerList,
      info,
      listPerms,
      showOthers,
      removePerms,
      setPassword,
      setPerms,
      setAll,
      setVar,
      poll,
      vote,
      veto,
      requireIdentify,
      viewReports,
			adminMessages,
      lastPerm	// just so we know how many rights there
		// are this dosn't do anything really, just
		// make sure it's the last real right
    };

  std::bitset<lastPerm>		explicitAllows;
  std::bitset<lastPerm>		explicitDenys;
  std::vector<std::string>	groups;
  bool				verified;
  TimeKeeper			loginTime;
  int				loginAttempts;

};

typedef std::map<std::string, PlayerAccessInfo> PlayerAccessMap;
typedef std::map<std::string, std::string> PasswordMap;

extern PlayerAccessMap	groupAccess;
extern PlayerAccessMap	userDatabase;
extern PasswordMap	passwordDatabase;

extern std::string		groupsFile;
extern std::string		passFile;
extern std::string		userDatabaseFile;

inline void makeupper(std::string& str)
{
  for (unsigned int i = 0; i < str.length(); i++)
    str[i] = toupper(str[i]);
}

bool hasGroup(PlayerAccessInfo& info, const std::string &group);
bool addGroup(PlayerAccessInfo& info, const std::string &group);
bool removeGroup(PlayerAccessInfo& info, const std::string &group);
bool hasPerm(PlayerAccessInfo& info, PlayerAccessInfo::AccessPerm right);
bool userExists(const std::string &nick);
PlayerAccessInfo& getUserInfo(const std::string &nick);
bool setUserInfo(const std::string &nick, PlayerAccessInfo& info);
bool verifyUserPassword(const std::string &nick, const std::string &pass);
void setUserPassword(const std::string &nick, const std::string &pass);
std::string nameFromPerm(PlayerAccessInfo::AccessPerm perm);
PlayerAccessInfo::AccessPerm permFromName(const std::string &name);
void parsePermissionString(const std::string &permissionString, std::bitset<PlayerAccessInfo::lastPerm> &perms);
bool readPassFile(const std::string &filename);
bool writePassFile(const std::string &filename);
bool readGroupsFile(const std::string &filename);
bool readPermsFile(const std::string &filename);
bool writePermsFile(const std::string &filename);
void updateDatabases();

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

