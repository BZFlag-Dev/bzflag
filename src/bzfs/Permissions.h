/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __PERMISSIONS_H__
#define __PERMISSIONS_H__

#include "common.h"

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
#include <cctype>

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
      adminMessageReceive,
      adminMessageSend,
      antiban,
      antikick,
      antikill,
      antipoll,
      antipollban,
      antipollkick,
      antipollkill,
      ban,
      banlist,
      clientQuery,
      countdown,
      date,
      endGame,
      flagHistory,
      flagMaster,
      flagMod,
      hideAdmin,
      idleStats,
      info,
      kick,
      kill,
      lagStats,
      lagwarn,
      jitterwarn,
      listPerms,
      masterBan,
      modCount,
      mute,
      packetlosswarn,
      playerList,
      plugins,
      poll,
      pollBan,
      pollKick,
      pollKill,
      pollSet,
      pollFlagReset,
      privateMessage,
      record,
      rejoin,
      removePerms,
      replay,
      report,
      say,
      sendHelp,
      setAll,
      setPassword,
      setPerms,
      setVar,
      shortBan,
      showAdmin,
      showOthers,
      shutdownServer,
      spawn,
      superKill,
      talk,
      unban,
      unmute,
      veto,
      viewReports,
      vote,
      // just so we know how many rights there
      // are this dosn't do anything really, just
      // make sure it's the last real right
      lastPerm
    };

  enum GroupStates
	{
	  isGroup,  // we can check if this is a group or a player
	  isDefault,   // mark default groups
	  lastState
	};

  void	setName(const char* callSign);

  std::string getName();

  bool	gotAccessFailure();
  void	setLoginFail();
  bool	passwordAttemptsMax();
  bool	isPasswordMatching(const char* pwd);
  bool	hasRealPassword();
  void	setPasswd(const std::string& pwd);

  /** have successfully provided server password */
  void  setOperator();
  bool	isOperator() const;

  /** have ability to ban */
  bool	isAdmin() const;

  /** are not marked as hidden admins */
  bool	showAsAdmin() const;

  void	setPermissionRights();
  void	reloadInfo();

  bool	hasGroup(const std::string& group);
  bool	addGroup(const std::string &group);
  bool	removeGroup(const std::string& group);
  bool	canSet(const std::string& group);

  bool	hasPerm(AccessPerm right) const;
  void	grantPerm(AccessPerm right);
  void	revokePerm(AccessPerm right);

	bool	hasCustomPerm(const char* right) const;

  bool	isRegistered() const;
  bool	isAllowedToEnter();
  bool	isVerified() const;
  uint8_t     getPlayerProperties();
  void	storeInfo(const char* pwd);
  bool	exists();
  static PlayerAccessInfo &getUserInfo(const std::string &nick);
  static bool readGroupsFile(const std::string &filename);
  static bool readPermsFile(const std::string &filename);
  static bool writePermsFile(const std::string &filename);
  static void updateDatabases();
  std::bitset<lastPerm>		explicitAllows;
  std::bitset<lastPerm>		explicitDenys;
  std::bitset<lastState>	groupState;
  std::vector<std::string>	groups;

  bool				hasALLPerm;

  std::vector<std::string> customPerms;
private:
  bool				verified;
  TimeKeeper			loginTime;
  int				loginAttempts;

  /** server operator that has provided the server password */
  bool				serverop;

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
extern std::string		userDatabaseFile;

inline void makeupper(std::string& str)
{
  std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);
}

bool userExists(const std::string &nick);
bool checkPasswordExistence(const std::string &nick);
bool verifyUserPassword(const std::string &nick, const std::string &pass);
std::string nameFromPerm(PlayerAccessInfo::AccessPerm perm);
PlayerAccessInfo::AccessPerm permFromName(const std::string &name);
bool parsePermissionString(const std::string &permissionString, std::bitset<PlayerAccessInfo::lastPerm> &perms);

uint8_t GetPlayerProperties( bool registered, bool verified, bool admin );

#endif


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
