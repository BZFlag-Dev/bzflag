/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "commands.h"

// system headers
#include <ctype.h>
#include <string.h>

// common implementation headers
#include "TextUtils.h"
#include "WorldEventManager.h"

// local implementation headers
#include "ServerCommand.h"
#include "MasterBanList.h"
#include "bzfs.h"


class KickCommand : private ServerCommand {
public:
  KickCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class KillCommand : private ServerCommand {
public:
  KillCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class BanListCommand : private ServerCommand {
public:
  BanListCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class CheckIPCommand : private ServerCommand {
public:
  CheckIPCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class HostbanListCommand : private ServerCommand {
public:
  HostbanListCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class IdBanListCommand : private ServerCommand {
public:
  IdBanListCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class BanCommand : private ServerCommand {
public:
  BanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class HostbanCommand : private ServerCommand {
public:
  HostbanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class IdBanCommand : private ServerCommand {
public:
  IdBanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class UnbanCommand : private ServerCommand {
public:
  UnbanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class HostUnbanCommand : private ServerCommand {
public:
  HostUnbanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class IdUnbanCommand : private ServerCommand {
public:
  IdUnbanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class MuteCommand : private ServerCommand {
public:
  MuteCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class UnmuteCommand : private ServerCommand {
public:
  UnmuteCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

class MasterBanCommand : private ServerCommand {
public:
  MasterBanCommand();

  virtual bool operator() (const char	 *commandLine,
			   GameKeeper::Player *playerData);
};

static KickCommand	  kickCommand;
static KillCommand	  killCommand;
static CheckIPCommand	  checkIPCommand;
static BanCommand	  banCommand;
static UnbanCommand	  unbanCommand;
static BanListCommand	  banListCommand;
static HostbanCommand	  hostbanCommand;
static HostUnbanCommand   hostUnbanCommand;
static HostbanListCommand hostbanListCommand;
static IdBanCommand	  idBanCommand;
static IdUnbanCommand     idUnbanCommand;
static IdBanListCommand   idBanListCommand;
static MuteCommand	  muteCommand;
static UnmuteCommand	  unmuteCommand;
static MasterBanCommand   masterBanCommand;

KickCommand::KickCommand()		 : ServerCommand("/kick",
  "<#slot|PlayerName|\"Player Name\"> <reason> - kick the player off the server") {}
KillCommand::KillCommand()		 : ServerCommand("/kill",
  "<#slot|PlayerName|\"Player Name\"> [reason] - kill a player") {}
BanListCommand::BanListCommand()	 : ServerCommand("/banlist",
  "[pattern] - List the IPs currently banned from this server") {}
CheckIPCommand::CheckIPCommand()	 : ServerCommand("/checkip",
  "<ip> - check if IP is banned and print corresponding ban info") {}
BanCommand::BanCommand()		 : ServerCommand("/ban",
  "<#slot|PlayerName|\"Player Name\"|ip> <duration> <reason>  - ban a player, ip or ip range off the server") {}
UnbanCommand::UnbanCommand()		 : ServerCommand("/unban",
  "<ip> - remove a ip pattern from the ban list") {}
HostbanCommand::HostbanCommand()	 : ServerCommand("/hostban",
  "<host pattern> [duration] [reason] - ban using host pattern off the server") {}
HostUnbanCommand::HostUnbanCommand()     : ServerCommand("/hostunban",
  "<host pattern> - remove a host pattern from the host ban list") {}
HostbanListCommand::HostbanListCommand() : ServerCommand("/hostbanlist",
  "[pattern] - List the host patterns currently banned from this server") {}
IdBanCommand::IdBanCommand()		 : ServerCommand("/idban",
  "<#slot|+id|PlayerName|\"Player Name\"> <duration> <reason> - ban using BZID") {}
IdUnbanCommand::IdUnbanCommand()	 : ServerCommand("/idunban",
  "<id> - remove a BZID from the ban list") {}
IdBanListCommand::IdBanListCommand():	 ServerCommand("/idbanlist",
  "[pattern] - List the BZIDs currently banned from this server") {}
MuteCommand::MuteCommand()		 : ServerCommand("/mute",
  "<#slot|PlayerName|\"Player Name\"> - remove the ability for a player to communicate with other players") {}
UnmuteCommand::UnmuteCommand()		 : ServerCommand("/unmute",
  "<#slot|PlayerName|\"Player Name\"> - restore the TALK permission to a previously muted player") {}
MasterBanCommand::MasterBanCommand()	 : ServerCommand("/masterban",
  "<flush|reload|list> - manage the masterban list") {}

bool MuteCommand::operator() (const char	 *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::mute)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the mute command");
    return true;
  }

  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t,
		"Syntax: /mute <#slot | PlayerName | \"Player Name\">");
    return true;
  }

  int i = GameKeeper::Player::getPlayerIDByName(argv[1]);

  char msg[MessageLen];

  // Player not found
  if (i < 0) {
    snprintf(msg, MessageLen, "player \"%s\" not found", argv[1].c_str());
    sendMessage(ServerPlayer, t, msg);
    return true;
  }

  // mute the player
  GameKeeper::Player *muteData = GameKeeper::Player::getPlayerByIndex(i);
  if (muteData) {
    muteData->accessInfo.revokePerm(PlayerAccessInfo::talk);
    snprintf(msg, MessageLen, "You have been muted by %s.",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, msg);
    // confirm player is muted
    snprintf(msg, MessageLen, "player id #%d \"%s\" is now muted.", i,
	     muteData->player.getCallSign());
    sendMessage(ServerPlayer, t, msg);
  }
  return true;
}

bool UnmuteCommand::operator() (const char	 *message,
				GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::mute)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the unmute command");
    return true;
  }

  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t,
		"Syntax: /unmute <#slot | PlayerName | \"Player Name\">");
    return true;
  }

  int i = GameKeeper::Player::getPlayerIDByName(argv[1]);

  char msg[MessageLen];

  // Player not found
  if (i < 0) {
    snprintf(msg, MessageLen, "player \"%s\" not found", argv[1].c_str());
    sendMessage(ServerPlayer, t, msg);
    return true;
  }

  // unmute the player
  GameKeeper::Player *unmuteData = GameKeeper::Player::getPlayerByIndex(i);
  if (unmuteData) {
    unmuteData->accessInfo.grantPerm(PlayerAccessInfo::talk);
    snprintf(msg, MessageLen, "You have been unmuted by %s.",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, i, msg);
    // confirm player is unmuted
    snprintf(msg, MessageLen, "player id #%d \"%s\" is now unmuted.", i,
	     unmuteData->player.getCallSign());
    sendMessage(ServerPlayer, t, msg);
  }
  return true;
}

bool KickCommand::operator() (const char	 *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::kick)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the kick command");
    return true;
  }
  int i;
  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 3) {
    sendMessage(ServerPlayer, t,
		"Syntax: /kick <#slot | PlayerName | \"Player Name\"> "
		"<reason>");
    sendMessage(ServerPlayer, t,
		"	Please keep in mind that reason is displayed to the "
		"user.");
    return true;
  }

  i = GameKeeper::Player::getPlayerIDByName(argv[1]);

  if (i >= 0) {

    // call any plugin events registered for /kick
    bz_KickEventData_V1 kickEvent;
    kickEvent.kickerID = t;
    kickEvent.kickedID = i;
    kickEvent.reason = argv[2].c_str();

    worldEventManager.callEvents(bz_eKickEvent,&kickEvent);

    // need to update playerIndex ?
    if (t != kickEvent.kickerID) {
      playerData = GameKeeper::Player::getPlayerByIndex(kickEvent.kickerID);
      if (!playerData)
	return true;
    }

    char kickmessage[MessageLen];

    GameKeeper::Player *p
      = GameKeeper::Player::getPlayerByIndex(kickEvent.kickedID);

    // operators can override antiperms
    if (!playerData->accessInfo.isOperator()) {
      // otherwise make sure the player is not protected with an antiperm
      if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antikick))) {
	snprintf(kickmessage, MessageLen,
		 "%s is protected from being kicked.",
		 p->player.getCallSign());
	sendMessage(ServerPlayer,kickEvent.kickerID, kickmessage);
	return true;
      }
    }

    snprintf(kickmessage, MessageLen, "You were kicked off the server by %s",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, kickEvent.kickedID, kickmessage);
    if (kickEvent.reason.size() > 0) {
      snprintf(kickmessage, MessageLen, " reason given : %s",
	       kickEvent.reason.c_str());
      sendMessage(ServerPlayer, kickEvent.kickedID, kickmessage);
    }
    snprintf(kickmessage, MessageLen, "%s kicked by %s, reason: %s",
	     p->player.getCallSign(), playerData->player.getCallSign(),
	     kickEvent.reason.c_str());
    sendMessage(ServerPlayer, AdminPlayers, kickmessage);
    removePlayer(kickEvent.kickedID, "/kick");
  } else {
    char errormessage[MessageLen];
    snprintf(errormessage, MessageLen, "player \"%s\" not found",
	     argv[1].c_str());
    sendMessage(ServerPlayer, t, errormessage);
  }
  return true;
}

bool KillCommand::operator() (const char	 *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::kill)) 
  {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the kill command");
    return true;
  }

  int i;

  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) 
  {
    sendMessage(ServerPlayer, t, "Syntax: /kill <#slot | PlayerName | \"Player Name\"> [reason]");
    sendMessage(ServerPlayer, t,"	Please keep in mind that reason is displayed to the user.");
    return true;
  }

  i = GameKeeper::Player::getPlayerIDByName(argv[1]);

  if (i >= 0)
  {
    char killmessage[MessageLen];

    // call any plugin events registered for /kick
    bz_KillEventData_V1 killEvent;
    killEvent.killerID = t;
    killEvent.killedID = i;
    if (argv.size() > 2)
      killEvent.reason = argv[2].c_str();

    bz_AllowKillCommandEventData_V1 allow;
    allow.playerToKill = i;
    allow.playerKilling = t;
    allow.allow = true;

    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);

    // operators can override antiperms
    if (!playerData->accessInfo.isOperator()) 
    {
      // otherwise make sure the player is not protected with an antiperm
      if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antikill))) 
	allow.allow = false;
    }

    // check the API to see if anyone wants to have anything to say about this kill
    worldEventManager.callEvents(bz_eAllowKillCommandEvent,&allow);

    // plug-ins can ovride ANYONE even the gods
    if (p && !allow.allow)
    {
      snprintf(killmessage, MessageLen, "%s is protected from being killed.",
	p->player.getCallSign());
      sendMessage(ServerPlayer, t, killmessage);
      return true;
    }

    snprintf(killmessage, MessageLen, "You were killed by %s",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, killEvent.killedID, killmessage);
    if (killEvent.reason.size() > 0) 
    {
      snprintf(killmessage, MessageLen, " reason given : %s", killEvent.reason.c_str());
      sendMessage(ServerPlayer, killEvent.killedID, killmessage);
    }

    // kill the player
    smitePlayer(killEvent.killedID, GotKilledMsg);

    worldEventManager.callEvents(bz_eKillEvent,&killEvent);
  }
  else 
  {
    char errormessage[MessageLen];
    snprintf(errormessage, MessageLen, "player \"%s\" not found",
	     argv[1].c_str());
    sendMessage(ServerPlayer, t, errormessage);
  }
  return true;
}


bool CheckIPCommand::operator() (const char *message,
				 GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the banlist command");
    return true;
  }

  std::vector<std::string> argv = TextUtils::tokenize(message, " \t");
  if (argv.size() != 2) {
    sendMessage(ServerPlayer, t, "Syntax: /checkip <ip>");
    return true;
  }

  in_addr ip = Address(argv[1]);
  BanInfo baninfo(ip);
  const bool banned = !clOptions->acl.validate(ip, &baninfo);

  if (banned) {
    std::string bannedmsg = argv[1] + " is banned:";
    sendMessage(ServerPlayer, t, bannedmsg.c_str());
    clOptions->acl.sendBan(t, baninfo);
  } else {
    std::string notbannedmsg = argv[1] + " is not banned.";
    sendMessage(ServerPlayer, t, notbannedmsg.c_str());
  }

  return true;
}


bool BanListCommand::operator() (const char* message,
				 GameKeeper::Player* playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the banlist command");
    return true;
  }
  clOptions->acl.sendBans(t, message + commandName.size());
  return true;
}


bool HostbanListCommand::operator() (const char* message,
				     GameKeeper::Player* playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the banlist command");
    return true;
  }
  clOptions->acl.sendHostBans(t, message + commandName.size());
  return true;
}


bool IdBanListCommand::operator() (const char* message,
				   GameKeeper::Player* playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the banlist command");
    return true;
  }
  clOptions->acl.sendIdBans(t, message + commandName.size());
  return true;
}


static void addNamePrefix(std::string& reason, const std::string& name)
{
  // add the name prefix if the name isn't already contained
  if (strstr(reason.c_str(), name.c_str()) == NULL) {
    reason = "(" + name + ") " + reason;
  }
  return;
}


static bool doBanKick(GameKeeper::Player *victim,
		      GameKeeper::Player *banner,
		      const char* reason)
{
  if ((victim == NULL) || (banner == NULL)) {
    return true; // proceed with the ban
  }

  char buffer[MessageLen];
  const int victimID = victim->getIndex();
  const int bannerID = banner->getIndex();

  // make sure this player isn't protected
  if (victim->accessInfo.hasPerm(PlayerAccessInfo::antiban)) {
    snprintf(buffer, MessageLen,
             "%s is protected from being banned (skipped).",
             victim->player.getCallSign());
    sendMessage(ServerPlayer, bannerID, buffer);
    return false; // do not use the ban
  }

  // send a notice to the victim
  snprintf(buffer, MessageLen,
	   "You were banned from this server by %s",
	   banner->player.getCallSign());
  sendMessage(ServerPlayer, victimID, buffer);
  if (strlen(reason) > 0) {
    snprintf(buffer, MessageLen, "Reason given: %s", reason);
    sendMessage(ServerPlayer, victimID, buffer);
  }

  // send a notice to the admins
  snprintf(buffer, MessageLen, "%s banned by %s, reason: %s",
	   victim->player.getCallSign(), banner->player.getCallSign(),
	   reason);
  sendMessage(ServerPlayer, AdminPlayers, buffer);

  // you're outta here
  removePlayer(victimID, "/ban");

  return true;
}


bool BanCommand::operator() (const char	 *message,
			     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) &&
      !playerData->accessInfo.hasPerm(PlayerAccessInfo::shortBan)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the ban command");
    return true;
  }

  std::string msg = message;
  std::vector<std::string> argv = TextUtils::tokenize(msg, " \t", 4, true);

  if (argv.size() != 4) {
    sendMessage(ServerPlayer, t,
		"Syntax: /ban <#slot | PlayerName | \"Player Name\" | ip> "
		"<duration> <reason>");
    sendMessage(ServerPlayer, t,
		"	<duration> can be 'short' or 'default' for the default"
		" ban time ");
    sendMessage(ServerPlayer, t,
		"	or 'forever' or 'max' for infinite bans ");
    sendMessage(ServerPlayer, t,
		"	or a time in the format <weeks>W<days>D<hours>"
		"H<minutes>M ");
    sendMessage(ServerPlayer, t,
		"	or just a number of minutes ");
    sendMessage(ServerPlayer, t,
		"	Please keep in mind that reason is displayed to the "
		"user.");
    return true;
  }

  std::string ip = argv[1];

  int victim = GameKeeper::Player::getPlayerIDByName(argv[1]);
  GameKeeper::Player* victimPlayer = NULL;
  if (victim >= 0) {
    // valid slot or callsign
    victimPlayer = GameKeeper::Player::getPlayerByIndex(victim);
    if (victimPlayer) {
      if (victimPlayer->accessInfo.hasPerm(PlayerAccessInfo::antiban)){
	char buffer[MessageLen];
	snprintf(buffer, MessageLen,
		 "%s is protected from being banned (skipped).",
		 victimPlayer->player.getCallSign());
	      sendMessage(ServerPlayer, t, buffer);
	return true;
      }
      ip = victimPlayer->netHandler->getTargetIP();
    }
  }


  // setup the ban duration
  int durationInt = clOptions->banTime;
  int specifiedDuration;
  if (!TextUtils::parseDuration(argv[2].c_str(), specifiedDuration)) {
    sendMessage(ServerPlayer, t, "Error: invalid ban duration");
    sendMessage(ServerPlayer, t,
		"Duration examples:  30m 1h  1d  1w  and mixing: 1w2d4h "
		"1w2d1m");
    return true;
  }
  if (specifiedDuration >= 0) {
    if ((durationInt > 0) &&
	((specifiedDuration > durationInt) || (specifiedDuration <= 0)) &&
	!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
      sendMessage (ServerPlayer, t, "You only have SHORTBAN privileges,"
		   " using default ban time");
    } else {
      durationInt = specifiedDuration;
    }
  }

  // set the ban reason
  std::string reason;
  reason = argv[3];
  if (victimPlayer) {
    addNamePrefix(reason, victimPlayer->player.getCallSign());
  }

  // call any plugin events registered for /ban
  bz_BanEventData_V1 banEvent;
  banEvent.bannerID = t;
  banEvent.ipAddress = ip.c_str();
  banEvent.reason = reason.c_str();
  banEvent.duration = durationInt;
  // if we know for sure who is to be banned, submit it
  if (victim >= 0) {
    banEvent.banneeID = victim;
  }
  worldEventManager.callEvents(bz_eBanEvent,&banEvent);

  // a plugin might have changed bannerID
  if (t != banEvent.bannerID) {
    playerData = GameKeeper::Player::getPlayerByIndex(banEvent.bannerID);
    if (!playerData)
      return true;
  }

  // handle the case of a single intended victim
  if (victimPlayer) {
    if (!doBanKick(victimPlayer, playerData, banEvent.reason.c_str())) {
      return true; // could not ban, bail
    }
    // add a bzId ban if we can
    const std::string& bzid = victimPlayer->getBzIdentifier();
    if (bzid.size() > 0) {
      clOptions->acl.load();
      clOptions->acl.idBan(bzid, playerData->player.getCallSign(),
			   banEvent.duration, banEvent.reason.c_str());
      clOptions->acl.save();
      sendMessage(ServerPlayer, AdminPlayers, "Pattern added to the BZID banlist");
    }
  }

  // reload the banlist in case anyone else has added
  clOptions->acl.load();

  // add the ban, and kick any that it applies to
  if (clOptions->acl.ban(banEvent.ipAddress.c_str(),
			 playerData->player.getCallSign(),
			 banEvent.duration,
			 banEvent.reason.c_str())) {
    clOptions->acl.save();

    sendMessage(ServerPlayer, AdminPlayers, "Pattern added to the IP banlist");

    for (int i = 0; i < curMaxPlayers; i++) {
      GameKeeper::Player *tmpVictim = GameKeeper::Player::getPlayerByIndex(i);
      if (tmpVictim &&
	  !clOptions->acl.validate(tmpVictim->netHandler->getIPAddress())) {
	// ignore the return code
	doBanKick(tmpVictim, playerData, banEvent.reason.c_str());
      }
    }
  } else {
    char buffer[MessageLen];
    snprintf(buffer, MessageLen,
	     "Malformed address or invalid Player/Slot: %s",
	     argv[1].c_str());
    sendMessage(ServerPlayer, t, buffer);
  }

  return true;
}


bool HostbanCommand::operator() (const char* message,
				 GameKeeper::Player* playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) &&
      !playerData->accessInfo.hasPerm(PlayerAccessInfo::shortBan)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the ban command");
    return true;
  }

  std::string msg = message;
  std::vector<std::string> argv = TextUtils::tokenize(msg, " \t", 4);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t,
		"Syntax: /hostban <host pattern> [duration] [reason]");
    sendMessage(ServerPlayer, t,
		"	Please keep in mind that reason is displayed to the "
		"user.");
    return true;
  }

  std::string hostpat = argv[1];

  // set the ban time
  int durationInt = clOptions->banTime;
  if (argv.size() >= 3) {
    int specifiedDuration;
    // setup the ban duration
    if (!TextUtils::parseDuration(argv[2].c_str(), specifiedDuration)) {
      sendMessage(ServerPlayer, t, "Error: invalid ban duration");
      sendMessage(ServerPlayer, t,
		  "Duration examples:  30m 1h  1d  1w  and mixing: 1w2d4h "
		  "1w2d1m");
      return true;
    }
    if (specifiedDuration >= 0)
      if ((durationInt > 0) &&
	  ((specifiedDuration > durationInt) || (specifiedDuration <= 0)) &&
	  !playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
	sendMessage (ServerPlayer, t, "You only have SHORTBAN privileges,"
		     " using default ban time");
      } else {
	durationInt = specifiedDuration;
      }
  }

  // set the ban reason
  std::string reason;
  if (argv.size() == 4) {
    reason = argv[3];
  }

  // call any plugin events registered for /hostban
  bz_HostBanEventData_V1 hostBanEvent;
  hostBanEvent.bannerID = t;
  hostBanEvent.hostPattern = hostpat.c_str();
  hostBanEvent.reason = reason.c_str();
  hostBanEvent.duration = durationInt;

  worldEventManager.callEvents(bz_eHostBanEvent,&hostBanEvent);

  // a plugin might have changed bannerID
  if (t != hostBanEvent.bannerID) {
    playerData = GameKeeper::Player::getPlayerByIndex(hostBanEvent.bannerID);
    if (!playerData)
      return true;
  }

  // reload the banlist in case anyone else has added
  clOptions->acl.load();

  clOptions->acl.hostBan(hostBanEvent.hostPattern.c_str(),
			 playerData->player.getCallSign(),
			 hostBanEvent.duration,
			 hostBanEvent.reason.c_str());
  clOptions->acl.save();

  GameKeeper::Player::setAllNeedHostbanChecked(true);

  sendMessage(ServerPlayer, AdminPlayers, "Pattern added to the HOSTNAME banlist");

  return true;
}


bool IdBanCommand::operator() (const char* message,
			       GameKeeper::Player* playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) &&
      !playerData->accessInfo.hasPerm(PlayerAccessInfo::shortBan)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the ban command");
    return true;
  }

  std::string msg = message;
  std::vector<std::string> argv = TextUtils::tokenize(msg, " \t", 4, true);

  if (argv.size() != 4) {
    sendMessage(ServerPlayer, t,
		"Syntax: /idban <#slot|+id|PlayerName|\"Player Name\"> <duration> <reason>");
    sendMessage(ServerPlayer, t,
		"  Please keep in mind that reason is displayed to the user.");
    return true;
  }

  // get the ban pattern or victim
  std::string idpat = argv[1];
  int victim = -1;
  GameKeeper::Player *victimPlayer = NULL;
  if (idpat[0] == '+') {
    idpat = idpat.c_str() + 1; // strip the '+' for a raw id pattern
    if (idpat.size() <= 0) {
      sendMessage(ServerPlayer, t, "Error: invalid id pattern");
      return true;
    }
    // check for a player that matches
    for (int i = 0; i < curMaxPlayers; i++) {
      GameKeeper::Player *tmpPlayer = GameKeeper::Player::getPlayerByIndex(i);
      if ((tmpPlayer != NULL) && (tmpPlayer->getBzIdentifier().size() > 0)) {
	const std::string& bzId = tmpPlayer->getBzIdentifier();
	if (idpat == bzId) {
	  victim = i;
	  victimPlayer = tmpPlayer;
	  break;
	}
      }
    }
  } else {
    // looking for a specific player
    victim = GameKeeper::Player::getPlayerIDByName(idpat);
    victimPlayer = GameKeeper::Player::getPlayerByIndex(victim);
    if ((victim < 0) || (victimPlayer == NULL)) {
      char buffer[MessageLen];
      snprintf(buffer, MessageLen, "could not find player (%s)", idpat.c_str());
      sendMessage(ServerPlayer, t, buffer);
      return true;
    }
    // found the player, check for a BZID
    const std::string& bzId = victimPlayer->getBzIdentifier();
    if (bzId.size() <= 0) {
      char buffer[MessageLen];
      snprintf(buffer, MessageLen, "no BZID for player (%s)", idpat.c_str());
      sendMessage(ServerPlayer, t, buffer);
      return true;
    }
    idpat = bzId; // success
  }

  // setup the ban duration
  int durationInt = clOptions->banTime;
  int specifiedDuration;
  if (!TextUtils::parseDuration(argv[2].c_str(), specifiedDuration)) {
    sendMessage(ServerPlayer, t, "Error: invalid ban duration");
    sendMessage(ServerPlayer, t,
		"Duration examples:  30m 1h  1d  1w  and mixing: 1w2d4h "
		"1w2d1m");
    return true;
  }
  if (specifiedDuration >= 0) {
    if ((durationInt > 0) &&
	((specifiedDuration > durationInt) || (specifiedDuration <= 0)) &&
	!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban)) {
      sendMessage (ServerPlayer, t, "You only have SHORTBAN privileges,"
				    " using default ban time");
    } else {
      durationInt = specifiedDuration;
    }
  }

  // set the ban reason
  std::string reason = argv[3];
  if (victimPlayer) {
    addNamePrefix(reason, victimPlayer->player.getCallSign());
  }

  // remove the victim if we have one
  if (victimPlayer) {
    if (!doBanKick(victimPlayer, playerData, reason.c_str())) {
      return true; // could not ban, bail
    }
  }

  //
  // FIXME: add to the plugin system
  //

  // reload the banlist in case anyone else has added
  clOptions->acl.load();

  clOptions->acl.idBan(idpat, playerData->player.getCallSign(),
		       durationInt, reason.c_str());
  clOptions->acl.save();

  sendMessage(ServerPlayer, AdminPlayers, "Pattern added to the BZID banlist");

  return true;
}


bool UnbanCommand::operator() (const char	 *message,
			       GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the unban command");
    return true;
  }

  if (clOptions->acl.unban(message + 7)) {
    sendMessage(ServerPlayer, AdminPlayers, "Removed IP pattern from the ban list");
    clOptions->acl.save();
  } else {
    sendMessage(ServerPlayer, t, "No pattern removed");
  }
  return true;
}


bool HostUnbanCommand::operator() (const char	 *message,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the /hostunban command");
    return true;
  }

  if (clOptions->acl.hostUnban(message + 11)) {
    sendMessage(ServerPlayer, AdminPlayers, "Removed host pattern from the ban list");
    clOptions->acl.save();
  } else {
    sendMessage(ServerPlayer, t, "No pattern removed");
  }
  return true;
}


bool IdUnbanCommand::operator() (const char *message,
				 GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the /idunban command");
    return true;
  }

  if (clOptions->acl.idUnban(message + 9)) {
    sendMessage(ServerPlayer, AdminPlayers, "Removed id from the ban list");
    clOptions->acl.save();
  } else {
    sendMessage(ServerPlayer, t, "No pattern removed");
  }
  return true;
}


/** /masterban command
 *
 * /masterban flush	     # remove all master ban entries from this server
 * /masterban reload	    # reread and reload all master ban entries
 * /masterban list	      # output a list of who is banned
 */
bool MasterBanCommand::operator() (const char	 *message,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  std::string callsign = std::string(playerData->player.getCallSign());

  logDebugMessage(2,"\"%s\" has requested masterban: %s\n", callsign.c_str(), message);

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::masterBan)) {
    sendMessage(ServerPlayer, t,
		TextUtils::format("%s, you are presently not authorized to "
				  "run /masterban", callsign.c_str()).c_str());
    return true;
  }

  logDebugMessage(3,"Player has permission to run /masterban\n");

  if (!clOptions->publicizeServer) {
    sendMessage(ServerPlayer, t,
		"This is not a public server.  Private servers do not use the"
		" master ban list.");
  }
  if (clOptions->suppressMasterBanList) {
    sendMessage(ServerPlayer, t, "The master ban list is disabled on this "
		"server.");
  }

  std::string argument = &message[10]; /* skip "/masterban" */
  std::string cmd = "";

  // allow for arbitrary whitespace
  size_t start = 0;
  while ((start < argument.size()) &&
	 (isspace(argument[start]))) {
    start++;
  }

  size_t end = 0;
  while ((end < argument.size()) &&
	 (!isspace(argument[end]))) {
    end++;
  }

  // make sure the command is lower case for comparison
  // simplicity/insensitivity
  cmd = argument.substr(start, end - start);
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);

  if (cmd == "reload") {
    if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::ban) ||
	!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
      sendMessage(ServerPlayer, t,
		  "You do not have permission to reload the master ban list.");
      sendMessage(ServerPlayer, t,
		  "Permission to ban and unban is required to reload the "
		  "master ban list.");
      return true;
    }

    if (clOptions->publicizeServer && !clOptions->suppressMasterBanList) {
      MasterBanList	banList;

      clOptions->acl.purgeMasters();
      sendMessage(ServerPlayer, t,
		  "Previous master ban list entries have been flushed, reloading in background");

      bz_reloadMasterBans();

    } else {
      sendMessage(ServerPlayer, t, "No action taken.");
    }

  } else if (cmd == "flush") {
    if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
      sendMessage(ServerPlayer, t,
		  "You do not have permission to reload the master ban list.");
      sendMessage(ServerPlayer, t,
		  "Permission to unban is required to flush the master ban "
		  "list.");
      return true;
    }

    clOptions->acl.purgeMasters();
    sendMessage(ServerPlayer, t, "The master ban list has been flushed.");

  } else if (cmd == "list") {
    std::vector<std::pair<std::string,
      std::string> > bans = clOptions->acl.listMasterBans();

    if (bans.size() > 20) {
      sendMessage(ServerPlayer, t,
		  TextUtils::format("There are %d bans, only displaying the "
				    "first 20", (int)bans.size()).c_str());

    } else if (bans.size() == 0) {
      sendMessage(ServerPlayer, t, "There are no master bans loaded.");

    } else {
      // print out a list header
      std::string banmsg = TextUtils::format("Master Bans from %s:",
					     DefaultMasterBanURL);
      sendMessage(ServerPlayer, t, banmsg.c_str());
      for (size_t i = 0; i < banmsg.size(); i++) {
	banmsg[i] = '-';
      }
      sendMessage(ServerPlayer, t, banmsg.c_str());
    }

    // print out the bans
    int counter = 0;
    for (std::vector<std::pair<std::string, std::string> >::const_iterator j
	   = bans.begin(); j != bans.end() && counter < 20; j++, counter++) {
      sendMessage(ServerPlayer, t,
		  TextUtils::format("%s: %s", (j->first).c_str(),
				    (j->second).c_str()).c_str());
    }

  } else {
    if (cmd.size() > 0) {
      sendMessage(ServerPlayer, t,
		  TextUtils::format("Unknown masterban command [%s]",
				    cmd.c_str()).c_str());
    }
    sendMessage(ServerPlayer, t,
		TextUtils::format("Usage: /masterban list|reload|flush")
		.c_str());
  }

  return true;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
