/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _WIN32
#pragma warning(4:4786)
#endif

#include <string>
#include <istream>
#include <fstream>
#include <stdlib.h>
#include "Permissions.h"
#include "MD5.h"

std::map<std::string, PlayerAccessInfo>	groupAccess;
std::map<std::string, PlayerAccessInfo>	userDatabase;
std::map<std::string, std::string>	passwordDatabase;

bool hasGroup(PlayerAccessInfo& info, const std::string &group)
{
  std::string str = group;
  makeupper(str);

  std::vector<std::string>::iterator itr = info.groups.begin();
  while (itr != info.groups.end()) {
    if ((*itr) == str)
      return true;
    itr++;
  }
  return false;
}

bool addGroup(PlayerAccessInfo& info, const std::string &group)
{
  if (hasGroup(info, group))
    return false;

  std::string str = group;
  makeupper(str);

  info.groups.push_back(str);
  return true;
}

bool removeGroup(PlayerAccessInfo& info, const std::string &group)
{
  if (!hasGroup(info, group))
    return false;

  std::string str = group;
  makeupper(str);

  std::vector<std::string>::iterator itr = info.groups.begin();
  while (itr != info.groups.end()) {
    if ((*itr) == str) {
      itr = info.groups.erase(itr);
      return true;
    } else
      itr++;
  }
  return false;
}

bool hasPerm(PlayerAccessInfo& info, AccessPerm right)
{
  if (info.explicitDenys.test(right))
    return false;
  if (info.explicitAllows.test(right))
    return true;
  std::vector<std::string>::iterator itr = info.groups.begin();
  std::map<std::string, PlayerAccessInfo>::iterator group;
  while (itr != info.groups.end()) {
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
  std::map<std::string, PlayerAccessInfo>::iterator itr = userDatabase.find(str);
  if (itr == userDatabase.end())
    return false;
  return true;
}

//FIXME - check for non-existing user (throw?)
PlayerAccessInfo& getUserInfo(const std::string &nick)
{
//  if (!userExists(nick))
//    return false;
  std::string str = nick;
  makeupper(str);
  std::map<std::string, PlayerAccessInfo>::iterator itr = userDatabase.find(str);
//  if (itr == userDatabase.end())
//    return false;
  return itr->second;
}

bool setUserInfo(const std::string &nick, PlayerAccessInfo& info)
{
  std::string str = nick;
  makeupper(str);
  userDatabase[str] = info;
  return true;
}

bool verifyUserPassword(const std::string &nick, const std::string &pass)
{
  std::string str1 = nick;
  makeupper(str1);
  std::map<std::string, std::string>::iterator itr = passwordDatabase.find(str1);
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

std::string nameFromPerm(AccessPerm perm)
{
  switch (perm) {
    case idleStats: return "idleStats";
    case lagStats: return "lagStats";
    case flagMod: return "flagMod";
    case flagHistory: return "flagHistory";
    case lagwarn: return "lagwarn";
    case kick: return "kick";
    case ban: return "ban";
    case banlist: return "banlist";
    case unban: return "unban";
    case countdown: return "countdown";
    case endGame: return "endGame";
    case shutdownServer: return "shutdownServer";
    case superKill: return "superKill";
    case playerList: return "playerList";
    case info: return "info";
    case listPerms: return "listPerms";
    case showOthers: return "showOthers";
    case removePerms: return "removePerms";
    case setPassword: return "setPassword";
    case setPerms: return "setPerms";
    case setAll: return "setAll";
    default: return NULL;
  };
}

AccessPerm permFromName(const std::string &name)
{
  if (name == "IDLESTATS") return idleStats;
  if (name == "LAGSTATS") return lagStats;
  if (name == "FLAGMOD") return flagMod;
  if (name == "FLAGHISTORY") return flagHistory;
  if (name == "LAGWARN") return lagwarn;
  if (name == "KICK") return kick;
  if (name == "BAN") return ban;
  if (name == "BANLIST") return banlist;
  if (name == "UNBAN") return unban;
  if (name == "COUNTDOWN") return countdown;
  if (name == "ENDGAME") return endGame;
  if (name == "SHUTDOWNSERVER") return shutdownServer;
  if (name == "SUPERKILL") return superKill;
  if (name == "PLAYERLIST") return playerList;
  if (name == "INFO") return info;
  if (name == "LISTPERMS") return listPerms;
  if (name == "SHOWOTHERS") return showOthers;
  if (name == "REMOVEPERMS") return removePerms;
  if (name == "SETPASSWORD") return setPassword;
  if (name == "SETPERMS") return setPerms;
  if (name == "SETALL") return setAll;
  return lastPerm;
}

void parsePermissionString(const std::string &permissionString, std::bitset<lastPerm> &perms)
{
  if (permissionString.length() < 1)
    return;
  perms.reset();

  std::istringstream permStream(permissionString);
  std::string word;

  while (permStream >> word) {
    makeupper(word);
    AccessPerm perm = permFromName(word);
    if (perm != lastPerm)
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
    std::string::size_type colonpos = line.find(':');
    if (colonpos != std::string::npos) {
      std::string name = line.substr(0, colonpos);
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
  std::map<std::string, std::string>::iterator itr = passwordDatabase.begin();
  while (itr != passwordDatabase.end()) {
    out << itr->first << ':' << itr->second << std::endl;
    itr++;
  }
  out.close();
  return true;
}

bool readGroupsFile(const std::string &filename)
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

bool readPermsFile(const std::string &filename)
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

bool writePermsFile(const std::string &filename)
{
  int i;
  std::ofstream out(filename.c_str());
  if (!out)
    return false;
  std::map<std::string, PlayerAccessInfo>::iterator itr = userDatabase.begin();
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
    for (i = 0; i < lastPerm; i++)
      if (itr->second.explicitAllows.test(i))
	out << nameFromPerm((AccessPerm) i);
    out << std::endl;
    // denys
    for (i = 0; i < lastPerm; i++)
      if (itr->second.explicitDenys.test(i))
	out << nameFromPerm((AccessPerm) i);
    out << std::endl;
    itr++;
  }
  out.close();
  return true;
}

std::string		groupsFile;
std::string		passFile;
std::string		userDatabaseFile;

void updateDatabases()
{
  if(passFile.size())
    writePassFile(passFile);
  if(userDatabaseFile.size())
    writePermsFile(userDatabaseFile);
}
