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

/* system interface headers */
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
#include <algorithm>
#include <string>

/* common interface headers */
#include "TimeKeeper.h"


class PlayerAccessInfo
{
public:
  PlayerAccessInfo();

  // player access info
  enum AccessPerm
    {
      actionMessage = 0,
      adminMessages,
      antiban,
      antideregister,
      antikick,
      antipoll,
      antipollban,
      antipollkick,
      ban,
      banlist,
      countdown,
      endGame,
      flagHistory,
      date,
      flagMod,
      idleStats,
      info,
      kick,
      lagStats,
      lagwarn,
      listPerms,
      masterBan,
      playerList,
      poll,
      privateMessage,
      record,
      removePerms,
      replay,
      requireIdentify,
      setAll,
      setPassword,
      setPerms,
      setVar,
      shortBan,
      showOthers,
      shutdownServer,
      superKill,
      unban,
      veto,
      viewReports,
      vote,
      lastPerm	// just so we know how many rights there
		// are this dosn't do anything really, just
		// make sure it's the last real right
    };

  void        setName(const char* callSign);

  std::string getName();

  bool        isAccessVerified() const;
  bool        gotAccessFailure();
  void        setLoginFail();
  bool        passwordAttemptsMax();
  bool        isPasswordMatching(const char* pwd);
  void        setPasswd(const std::string& pwd);
  void        setAdmin();
  bool        isAdmin() const;

  void        setPermissionRights();
  void        reloadInfo();

  bool        hasGroup(const std::string& group);
  bool        addGroup(const std::string &group);
  bool        removeGroup(const std::string& group);
  bool        canSet(const std::string& group);

  bool        hasPerm(AccessPerm right);
  bool        isRegistered() const;
  bool        isIdentifyRequired();
  bool        isAllowedToEnter();
  uint8_t     getPlayerProperties();
  void        storeInfo(const char* pwd);
  bool        exists();
  static PlayerAccessInfo &getUserInfo(const std::string &nick);
  static bool readGroupsFile(const std::string &filename);
  static bool readPermsFile(const std::string &filename);
  static bool writePermsFile(const std::string &filename);
  static void updateDatabases();
  std::bitset<lastPerm>		explicitAllows;
  std::vector<std::string>	groups;
private:
  std::bitset<lastPerm>		explicitDenys;
  bool				verified;
  TimeKeeper			loginTime;
  int				loginAttempts;
  bool                          Admin;

  // number of times they have tried to /password
  int passwordAttempts;
  // player's registration name
  std::string regName;
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
  std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);
}

bool userExists(const std::string &nick);
bool verifyUserPassword(const std::string &nick, const std::string &pass);
std::string nameFromPerm(PlayerAccessInfo::AccessPerm perm);
PlayerAccessInfo::AccessPerm permFromName(const std::string &name);
void parsePermissionString(const std::string &permissionString, std::bitset<PlayerAccessInfo::lastPerm> &perms);
bool readPassFile(const std::string &filename);
bool writePassFile(const std::string &filename);

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
