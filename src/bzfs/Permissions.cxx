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

#ifdef _MSC_VER
#pragma warning(4:4786)
#endif

#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include "Permissions.h"
#include "md5.h"

// implementation-specific bzflag headers
#include "bzfio.h"
#include "Protocol.h"
#include "TextUtils.h"

PlayerAccessMap	groupAccess;
PlayerAccessMap	userDatabase;
PasswordMap	passwordDatabase;

void setUserPassword(const std::string &nick, const std::string &pass);

PlayerAccessInfo::PlayerAccessInfo()
  : verified(false), loginTime(TimeKeeper::getCurrent()), loginAttempts (0),
    Admin(false), passwordAttempts(0)
{
  groups.push_back("DEFAULT");
}

void PlayerAccessInfo::setName(const char* callSign) {
  regName = callSign;
  makeupper(regName);
}

bool PlayerAccessInfo::isAccessVerified() const {
  return verified;
}

bool PlayerAccessInfo::gotAccessFailure() {
  bool accessFailure = loginAttempts >= 5;
  if (accessFailure)
    DEBUG1("Too Many Identifys %s\n", getName().c_str());
  return accessFailure;
}

void PlayerAccessInfo::setLoginFail() {
  loginAttempts++;
}

void PlayerAccessInfo::setPermissionRights() {
  verified = true;
  // get their real info
  PlayerAccessInfo &info = getUserInfo(regName);
  explicitAllows = info.explicitAllows;
  explicitDenys = info.explicitDenys;
  groups = info.groups;
  DEBUG1("Identify %s\n", regName.c_str());
}

void PlayerAccessInfo::reloadInfo() {
  if (verified && userExists(regName)) {
    PlayerAccessInfo accessInfo = getUserInfo(regName);
    explicitAllows = accessInfo.explicitAllows;
    explicitDenys  = accessInfo.explicitDenys;
    groups         = accessInfo.groups;
    loginTime      = accessInfo.loginTime;
    loginAttempts  = accessInfo.loginAttempts;
  }
}

void PlayerAccessInfo::setAdmin() {
  passwordAttempts = 0;
  Admin            = true;
}

bool PlayerAccessInfo::passwordAttemptsMax() {
  bool maxAttempts = passwordAttempts >= 5;
  // see how many times they have tried, you only get 5
  if (!maxAttempts) {
    passwordAttempts++;
  }
  return maxAttempts;
}

std::string PlayerAccessInfo::getName() {
  return regName;
}

bool PlayerAccessInfo::isPasswordMatching(const char* pwd) {
  return verifyUserPassword(regName.c_str(), pwd);
}

bool PlayerAccessInfo::isRegistered() const {
  return userExists(regName);
}

bool PlayerAccessInfo::isIdentifyRequired() {
  return getUserInfo(regName).hasPerm(requireIdentify);
}

bool PlayerAccessInfo::isAllowedToEnter() {
  return verified || !isRegistered() || !isIdentifyRequired();
};

void PlayerAccessInfo::storeInfo(const char* pwd) {
  PlayerAccessInfo info;
  info.groups.push_back("DEFAULT");
  info.groups.push_back("REGISTERED");
  std::string pass = pwd;
  setUserPassword(regName.c_str(), pass.c_str());
  userDatabase[regName] = info;
  DEBUG1("Register %s %s\n", regName.c_str(), pwd);
  updateDatabases();
}

void PlayerAccessInfo::setPasswd(const std::string&  pwd) {
  setUserPassword(regName.c_str(), pwd.c_str());
  updateDatabases();
}

uint8_t PlayerAccessInfo::getPlayerProperties() {
  uint8_t result = 0;
  if (isRegistered())
    result |= IsRegistered;
  if (verified)
    result |= IsIdentified;
  if (Admin)
    result |= IsAdmin;
  return result;
}

bool PlayerAccessInfo::exists() {
  return userExists(regName);
}

bool PlayerAccessInfo::hasGroup(const std::string &group)
{
  std::string str = group;
  makeupper(str);

  std::vector<std::string>::iterator itr = groups.begin();
  while (itr != groups.end()) {
    if ((*itr) == str)
      return true;
    itr++;
  }
  return false;
}

bool PlayerAccessInfo::addGroup(const std::string &group)
{
  if (hasGroup(group))
    return false;

  std::string str = group;
  makeupper(str);

  groups.push_back(str);
  return true;
}

bool PlayerAccessInfo::removeGroup(const std::string &group)
{
  if (!hasGroup(group))
    return false;

  std::string str = group;
  makeupper(str);

  std::vector<std::string>::iterator itr = groups.begin();
  while (itr != groups.end()) {
    if ((*itr) == str) {
      itr = groups.erase(itr);
      return true;
    } else
      itr++;
  }
  return false;
}

bool PlayerAccessInfo::canSet(const std::string& group) {
  if (hasPerm(PlayerAccessInfo::setAll))
    return true;
  return hasGroup(group) && hasPerm(PlayerAccessInfo::setPerms);
}

bool PlayerAccessInfo::hasPerm(PlayerAccessInfo::AccessPerm right) {
  if (Admin)
    return true;
  if (explicitDenys.test(right))
    return false;
  if (explicitAllows.test(right))
    return true;
  std::vector<std::string>::iterator itr = groups.begin();
  PlayerAccessMap::iterator group;
  while (itr != groups.end()) {
    group = groupAccess.find(*itr);
    if (group != groupAccess.end())
      if (group->second.explicitAllows.test(right))
	return true;
    itr++;
  }
  return false;
}

bool userExists(const std::string &nick)
{
  std::string str = nick;
  makeupper(str);
  PlayerAccessMap::iterator itr = userDatabase.find(str);
  if (itr == userDatabase.end())
    return false;
  return true;
}

//FIXME - check for non-existing user (throw?)
PlayerAccessInfo &PlayerAccessInfo::getUserInfo(const std::string &nick)
{
//  if (!userExists(nick))
//    return false;
  std::string str = nick;
  makeupper(str);
  PlayerAccessMap::iterator itr = userDatabase.find(str);
//  if (itr == userDatabase.end())
//    return false;
  return itr->second;
}

bool verifyUserPassword(const std::string &nick, const std::string &pass)
{
  std::string str1 = nick;
  makeupper(str1);
  PasswordMap::iterator itr = passwordDatabase.find(str1);
  if (itr == passwordDatabase.end())
    return false;
  return itr->second == MD5(pass).hexdigest();
}

void setUserPassword(const std::string &nick, const std::string &pass)
{
  std::string str1 = nick;
  makeupper(str1);
  // assume it's already a hash when length is 32 (FIXME?)
  passwordDatabase[str1] = pass.size()==32 ? pass : MD5(pass).hexdigest();
}

std::string nameFromPerm(PlayerAccessInfo::AccessPerm perm)
{
  switch (perm) {
    case PlayerAccessInfo::idleStats: return "idleStats";
    case PlayerAccessInfo::lagStats: return "lagStats";
    case PlayerAccessInfo::flagMod: return "flagMod";
    case PlayerAccessInfo::flagHistory: return "flagHistory";
    case PlayerAccessInfo::lagwarn: return "lagwarn";
    case PlayerAccessInfo::kick: return "kick";
    case PlayerAccessInfo::ban: return "ban";
    case PlayerAccessInfo::banlist: return "banlist";
    case PlayerAccessInfo::unban: return "unban";
    case PlayerAccessInfo::countdown: return "countdown";
    case PlayerAccessInfo::endGame: return "endGame";
    case PlayerAccessInfo::shutdownServer: return "shutdownServer";
    case PlayerAccessInfo::superKill: return "superKill";
    case PlayerAccessInfo::playerList: return "playerList";
    case PlayerAccessInfo::info: return "info";
    case PlayerAccessInfo::listPerms: return "listPerms";
    case PlayerAccessInfo::showOthers: return "showOthers";
    case PlayerAccessInfo::removePerms: return "removePerms";
    case PlayerAccessInfo::record: return "record";
    case PlayerAccessInfo::replay: return "replay";
    case PlayerAccessInfo::setPassword: return "setPassword";
    case PlayerAccessInfo::setPerms: return "setPerms";
    case PlayerAccessInfo::setAll: return "setAll";
    case PlayerAccessInfo::setVar: return "setVar";
    case PlayerAccessInfo::poll: return "poll";
    case PlayerAccessInfo::vote: return "vote";
    case PlayerAccessInfo::veto: return "veto";
    case PlayerAccessInfo::requireIdentify: return "requireIdentify";
    case PlayerAccessInfo::viewReports: return "viewReports";
	  case PlayerAccessInfo::adminMessages: return "adminMessages";
  default: return "";
  };
}

PlayerAccessInfo::AccessPerm permFromName(const std::string &name)
{
  if (name == "IDLESTATS") return PlayerAccessInfo::idleStats;
  if (name == "LAGSTATS") return PlayerAccessInfo::lagStats;
  if (name == "FLAGMOD") return PlayerAccessInfo::flagMod;
  if (name == "FLAGHISTORY") return PlayerAccessInfo::flagHistory;
  if (name == "LAGWARN") return PlayerAccessInfo::lagwarn;
  if (name == "KICK") return PlayerAccessInfo::kick;
  if (name == "BAN") return PlayerAccessInfo::ban;
  if (name == "BANLIST") return PlayerAccessInfo::banlist;
  if (name == "UNBAN") return PlayerAccessInfo::unban;
  if (name == "COUNTDOWN") return PlayerAccessInfo::countdown;
  if (name == "ENDGAME") return PlayerAccessInfo::endGame;
  if (name == "SHUTDOWNSERVER") return PlayerAccessInfo::shutdownServer;
  if (name == "SUPERKILL") return PlayerAccessInfo::superKill;
  if (name == "PLAYERLIST") return PlayerAccessInfo::playerList;
  if (name == "INFO") return PlayerAccessInfo::info;
  if (name == "LISTPERMS") return PlayerAccessInfo::listPerms;
  if (name == "SHOWOTHERS") return PlayerAccessInfo::showOthers;
  if (name == "REMOVEPERMS") return PlayerAccessInfo::removePerms;
  if (name == "SETPASSWORD") return PlayerAccessInfo::setPassword;
  if (name == "SETPERMS") return PlayerAccessInfo::setPerms;
  if (name == "SETVAR") return PlayerAccessInfo::setVar;
  if (name == "SETALL") return PlayerAccessInfo::setAll;
  if (name == "POLL") return PlayerAccessInfo::poll;
  if (name == "VOTE") return PlayerAccessInfo::vote;
  if (name == "VETO") return PlayerAccessInfo::veto;
  if (name == "REQUIREIDENTIFY") return PlayerAccessInfo::requireIdentify;
  if (name == "VIEWREPORTS") return PlayerAccessInfo::viewReports;
  if (name == "ADMINMESSAGES") return PlayerAccessInfo::adminMessages;
  return PlayerAccessInfo::lastPerm;
}

void parsePermissionString(const std::string &permissionString, std::bitset<PlayerAccessInfo::lastPerm> &perms)
{
  if (permissionString.length() < 1)
    return;
  perms.reset();

  std::istringstream permStream(permissionString);
  std::string word;

  while (permStream >> word) {
    makeupper(word);
    PlayerAccessInfo::AccessPerm perm = permFromName(word);
    if (perm != PlayerAccessInfo::lastPerm)
      perms.set(perm);
  }
}

bool readPassFile(const std::string &filename)
{
  std::ifstream in(filename.c_str());
  if (!in)
    return false;

  std::string line;
  while (std::getline(in, line)) {
    // Should look at an unescaped ':'
    int colonpos = unescape_lookup(line, '\\', ':');
    if (colonpos == -1)
      continue;
    {
      std::string name = unescape(line.substr(0, colonpos), '\\');
      std::string pass = line.substr(colonpos + 1);
      makeupper(name);
      setUserPassword(name.c_str(), pass.c_str());
    }
  }

  return (passwordDatabase.size() > 0);
}

bool writePassFile(const std::string &filename)
{
  std::ofstream out(filename.c_str());
  if (!out)
    return false;
  PasswordMap::iterator itr = passwordDatabase.begin();
  while (itr != passwordDatabase.end()) {
    out << escape(itr->first, '\\') << ':' << itr->second << std::endl;
    itr++;
  }
  out.close();
  return true;
}

bool PlayerAccessInfo::readGroupsFile(const std::string &filename)
{
  std::ifstream in(filename.c_str());
  if (!in)
    return false;

  std::string line;
  while (std::getline(in, line)) {
    std::string::size_type colonpos = line.find(':');
    if (colonpos != std::string::npos) {
      std::string name = line.substr(0, colonpos);
      std::string perm = line.substr(colonpos + 1);
      makeupper(name);
      PlayerAccessInfo info;
      parsePermissionString(perm, info.explicitAllows);
      info.verified = true;
      groupAccess[name] = info;
    }
  }

  return true;
}

bool PlayerAccessInfo::readPermsFile(const std::string &filename)
{
  std::ifstream in(filename.c_str());
  if (!in)
    return false;

  for (;;) {
    // 1st line - name
    std::string name;
    if (!std::getline(in, name))
      break;

    PlayerAccessInfo info;

    // 2nd line - groups
    std::string groupline;
    std::getline(in, groupline); // FIXME -it's an error when line cannot be read
    std::istringstream groupstream(groupline);
    std::string group;
    while (groupstream >> group) {
      info.groups.push_back(group);
    }

    // 3rd line - allows
    std::string perms;
    std::getline(in, perms);
    parsePermissionString(perms, info.explicitAllows);

    std::getline(in, perms);
    parsePermissionString(perms, info.explicitDenys);

    userDatabase[name] = info;
  }

  return true;
}

bool PlayerAccessInfo::writePermsFile(const std::string &filename)
{
  int i;
  std::ofstream out(filename.c_str());
  if (!out)
    return false;
  PlayerAccessMap::iterator itr = userDatabase.begin();
  std::vector<std::string>::iterator group;
  while (itr != userDatabase.end()) {
    out << itr->first << std::endl;
    group = itr->second.groups.begin();
    while (group != itr->second.groups.end()) {
      out << (*group) << ' ';
      group++;
    }
    out << std::endl;
    // allows
    for (i = 0; i < PlayerAccessInfo::lastPerm; i++)
      if (itr->second.explicitAllows.test(i))
	out << nameFromPerm((PlayerAccessInfo::AccessPerm) i);
    out << std::endl;
    // denys
    for (i = 0; i < PlayerAccessInfo::lastPerm; i++)
      if (itr->second.explicitDenys.test(i))
	out << nameFromPerm((PlayerAccessInfo::AccessPerm) i);
    out << std::endl;
    itr++;
  }
  out.close();
  return true;
}

std::string		groupsFile;
std::string		passFile;
std::string		userDatabaseFile;

void PlayerAccessInfo::updateDatabases()
{
  if(passFile.size())
    writePassFile(passFile);
  if(userDatabaseFile.size())
    writePermsFile(userDatabaseFile);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

