
#ifndef __PERMISSIONS_H__
#define __PERMISSIONS_H__

#include <bitset>
#include <vector>
#include <map>

#include "TimeKeeper.h"

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
  lastPerm	// just so we know how many rights there
		// are this dosn't do anything really, just
		// make sure it's the last real right
};

struct PlayerAccessInfo
{
  std::bitset<lastPerm>		explicitAllows;
  std::bitset<lastPerm>		explicitDenys;
  std::vector<std::string>	groups;
  bool				verified;
  TimeKeeper			loginTime;
  int				loginAttempts;
};

extern std::map<std::string, PlayerAccessInfo>	groupAccess;
extern std::map<std::string, PlayerAccessInfo>	userDatabase;
extern std::map<std::string, std::string>	passwordDatabase;

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
bool hasPerm(PlayerAccessInfo& info, AccessPerm right);
bool userExists(const std::string &nick);
PlayerAccessInfo& getUserInfo(const std::string &nick);
bool setUserInfo(const std::string &nick, PlayerAccessInfo& info);
bool verifyUserPassword(const std::string &nick, const std::string &pass);
void setUserPassword(const std::string &nick, const std::string &pass);
std::string nameFromPerm(AccessPerm perm);
AccessPerm permFromName(const std::string &name);
void parsePermissionString(const std::string &permissionString, std::bitset<lastPerm> &perms);
bool readPassFile(const std::string &filename);
bool writePassFile(const std::string &filename);
bool readGroupsFile(const std::string &filename);
bool readPermsFile(const std::string &filename);
bool writePermsFile(const std::string &filename);
void updateDatabases();



#endif
