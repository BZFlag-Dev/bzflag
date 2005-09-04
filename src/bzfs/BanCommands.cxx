/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "commands.h"

// common implementation headers
#include "WorldEventManager.h"

// local implementation headers
#include "ServerCommand.h"
#include "MasterBanList.h"

extern void sendMessage(int playerIndex, PlayerId dstPlayer,
			const char *message);

// externs that ghost needs
extern void removePlayer(int playerIndex, const char *reason,
			 bool notify=true);
extern void playerKilled(int victimIndex, int killerIndex, int reason,
			 int16_t shotIndex, const FlagType* flagType,
			 int phydrv, bool respawnOnBase = false);
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;

class NoDigit {
public:
  bool operator() (char c) {return !isdigit(c);}
};



class KickCommand : ServerCommand {
public:
  KickCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class KillCommand : ServerCommand {
public:
  KillCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class BanListCommand : ServerCommand {
public:
  BanListCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class HostbanListCommand : ServerCommand {
public:
  HostbanListCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class BanCommand : ServerCommand {
public:
  BanCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class HostbanCommand : ServerCommand {
public:
  HostbanCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class UnbanCommand : ServerCommand {
public:
  UnbanCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class HostUnbanCommand : ServerCommand {
public:
  HostUnbanCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class MuteCommand : ServerCommand {
public:
  MuteCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class UnmuteCommand : ServerCommand {
public:
  UnmuteCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class MasterBanCommand : ServerCommand {
public:
  MasterBanCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

static KickCommand        kickCommand;
static KillCommand        killCommand;
static BanListCommand     banListCommand;
static HostbanListCommand hostbanListCommand;
static BanCommand         banCommand;
static HostbanCommand     hostbanCommand;
static UnbanCommand       unbanCommand;
static HostUnbanCommand   hostUnbanCommand;
static MuteCommand        muteCommand;
static UnmuteCommand      unmuteCommand;
static MasterBanCommand   masterBanCommand;

KickCommand::KickCommand()               : ServerCommand("/kick",
  "<#slot|PlayerName|\"Player Name\"> <reason> - kick the player off the server") {}
KillCommand::KillCommand()               : ServerCommand("/kill", 
  "<#slot|PlayerName|\"Player Name\"> [reason] - kill a player") {}
BanListCommand::BanListCommand()         : ServerCommand("/banlist",
  "- List all of the IPs currently banned from this server") {}
HostbanListCommand::HostbanListCommand() : ServerCommand("/hostbanlist",
  "- List all of the host patterns currently banned from this server") {}
BanCommand::BanCommand()                 : ServerCommand("/ban", 
  "<#slot|PlayerName|\"Player Name\"|ip> <duration> <reason>  - ban a player, ip or ip range off the server") {}
HostbanCommand::HostbanCommand()         : ServerCommand("/hostban", 
  "<host pattern> [duration] [reason] - ban using host pattern off the server") {}
UnbanCommand::UnbanCommand()             : ServerCommand("/unban", 
  "[ip] - remove a ip pattern from the ban list") {}
HostUnbanCommand::HostUnbanCommand()     : ServerCommand("/hostunban", 
  "<host pattern> - remove a host pattern from the host ban list") {}
MuteCommand::MuteCommand()               : ServerCommand("/mute", 
  "<#slot|PlayerName|\"Player Name\"> - remove the ability for a player to communicate with other players") {}
UnmuteCommand::UnmuteCommand()           : ServerCommand("/unmute", 
  "<#slot|PlayerName|\"Player Name\"> - restore the TALK permission to a previously muted player") {}
MasterBanCommand::MasterBanCommand()     : ServerCommand("/masterban",
  "<flush|reload|list> - manage the masterban list") {}

bool MuteCommand::operator() (const char         *message,
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

bool UnmuteCommand::operator() (const char         *message,
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

bool KickCommand::operator() (const char         *message,
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
    bz_KickEventData kickEvent;
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

    // admins can override antiperms
    if (!playerData->accessInfo.isAdmin()) {
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

bool KillCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::kill)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the kill command");
    return true;
  }
  int i;
  std::vector<std::string> argv = TextUtils::tokenize(message, " \t", 3, true);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t,
		"Syntax: /kill <#slot | PlayerName | \"Player Name\">  "
		"[reason]");
    sendMessage(ServerPlayer, t,
		"	Please keep in mind that reason is displayed to the "
		"user.");
    return true;
  }

  i = GameKeeper::Player::getPlayerIDByName(argv[1]);

  if (i >= 0) {
    // call any plugin events registered for /kick
    bz_KillEventData killEvent;
    killEvent.killerID = t;
    killEvent.killedID = i;
    if (argv.size() > 2)
      killEvent.reason = argv[2].c_str();

    worldEventManager.callEvents(bz_eKillEvent,&killEvent);

    // need to update playerIndex ?
    if (t != killEvent.killerID) {
      playerData = GameKeeper::Player::getPlayerByIndex(killEvent.killerID);
      if (!playerData)
	return true;
    }

    char killmessage[MessageLen];

    // admins can override antiperms
    if (!playerData->accessInfo.isAdmin()) {
      // otherwise make sure the player is not protected with an antiperm
      GameKeeper::Player *p
	= GameKeeper::Player::getPlayerByIndex(killEvent.killedID);
      if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antikill))) {
	snprintf(killmessage, MessageLen, "%s is protected from being killed.",
		 p->player.getCallSign());
	sendMessage(ServerPlayer, i, killmessage);
	return true;
      }
    }

    snprintf(killmessage, MessageLen, "You were killed by %s",
	     playerData->player.getCallSign());
    sendMessage(ServerPlayer, killEvent.killedID, killmessage);
    if (killEvent.reason.size() > 0) {
      snprintf(killmessage, MessageLen, " reason given : %s",
	       killEvent.reason.c_str());
      sendMessage(ServerPlayer, killEvent.killedID, killmessage);
    }
    // kill the player
    playerKilled(killEvent.killedID, ServerPlayer, 0, -1, Flags::Null, -1);

  } else {
    char errormessage[MessageLen];
    snprintf(errormessage, MessageLen, "player \"%s\" not found",
	     argv[1].c_str());
    sendMessage(ServerPlayer, t, errormessage);
  }
  return true;
}

bool BanListCommand::operator() (const char         *,
				 GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the banlist command");
    return true;
  }
  clOptions->acl.sendBans(t);
  return true;
}


bool HostbanListCommand::operator() (const char         *,
				     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::banlist)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the banlist command");
    return true;
  }
  clOptions->acl.sendHostBans(t);
  return true;
}


bool BanCommand::operator() (const char         *message,
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

  if (argv.size() < 4) {
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
  std::string reason;
  int durationInt = clOptions->banTime;

  int victim = GameKeeper::Player::getPlayerIDByName(argv[1]);

  if (victim >= 0) {
    // valid slot or callsign
    GameKeeper::Player *playerBannedData
      = GameKeeper::Player::getPlayerByIndex(victim);
    if (playerBannedData)
      ip = playerBannedData->netHandler->getTargetIP();
  }

  int specifiedDuration = 0;

  // check the ban duration
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
  {
    // set the ban reason
    reason = argv[3];

    // call any plugin events registered for /ban
    bz_BanEventData banEvent;
    banEvent.bannerID = t;
    banEvent.ipAddress = ip.c_str();
    banEvent.reason = reason.c_str();
    banEvent.duration = durationInt;
    // if we know for sure who is to be banned, submit it	
    if (victim >= 0)
      banEvent.banneeID = victim;

    worldEventManager.callEvents(bz_eBanEvent,&banEvent);

    // a plugin might have changed bannerID
    if (t != banEvent.bannerID) {
      playerData = GameKeeper::Player::getPlayerByIndex(banEvent.bannerID);
      if (!playerData)
	return true;
    }


    // reload the banlist in case anyone else has added
    clOptions->acl.load();

    if (clOptions->acl.ban(banEvent.ipAddress.c_str(), 
			   playerData->player.getCallSign(), banEvent.duration,
			   banEvent.reason.c_str())) {
      clOptions->acl.save();

      sendMessage(ServerPlayer, t, "IP pattern added to banlist");

      char kickmessage[MessageLen];
      GameKeeper::Player *otherPlayer;
      for (int i = 0; i < curMaxPlayers; i++) {
	otherPlayer = GameKeeper::Player::getPlayerByIndex(i);
	if (otherPlayer && !clOptions->acl.validate
	    (otherPlayer->netHandler->getIPAddress())) {

	  // admins can override antiperms
	  if (!playerData->accessInfo.isAdmin()) {
	    // make sure this player isn't protected
	    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
	    if ((p != NULL)
		&& (p->accessInfo.hasPerm(PlayerAccessInfo::antiban))) {
	      snprintf(kickmessage, MessageLen, 
		       "%s is protected from being banned (skipped).",
		       p->player.getCallSign());
	      sendMessage(ServerPlayer, t, kickmessage);
	      continue;
	    }
	  }

	  snprintf(kickmessage, MessageLen,
		   "You were banned from this server by %s",
		   playerData->player.getCallSign());
	  sendMessage(ServerPlayer, i, kickmessage);
	  if (reason.length() > 0) {
	    snprintf(kickmessage, MessageLen, "Reason given: %s",
		     banEvent.reason.c_str());
	    sendMessage(ServerPlayer, i, kickmessage);
	  }
	  if (otherPlayer) {
	    snprintf(kickmessage, MessageLen,
		     "%s banned by %s, reason: %s", 
		     otherPlayer->player.getCallSign(),
		     playerData->player.getCallSign(),
		     banEvent.reason.c_str());
	    sendMessage(ServerPlayer, AdminPlayers, kickmessage);
	  }
	  removePlayer(i, "/ban");
	}
      }
    } else {
      char errormessage[MessageLen];
      snprintf(errormessage, MessageLen,
	       "Malformed address or invalid Player/Slot: %s",
	       argv[1].c_str());
      sendMessage(ServerPlayer, t, errormessage);
    }
  }
  return true;
}


bool HostbanCommand::operator() (const char         *message,
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
  std::vector<std::string> argv = TextUtils::tokenize(msg, " \t", 4);

  if (argv.size() < 2) {
    sendMessage(ServerPlayer, t,
		"Syntax: /hostban <host pattern> [duration] [reason]");
    sendMessage(ServerPlayer, t,
		"	Please keep in mind that reason is displayed to the "
		"user.");
  } else {
    std::string hostpat = argv[1];
    std::string reason;
    int durationInt = clOptions->banTime;

    // set the ban time
    if (argv.size() >= 3) {
      int specifiedDuration;
      // check the ban duration
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
    if (argv.size() == 4) {
      reason = argv[3];
    }

    // call any plugin events registered for /hostban
    bz_HostBanEventData hostBanEvent;
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

    clOptions->acl.hostBan(hostBanEvent.hostPattern.c_str(), 
			   playerData->player.getCallSign(),
			   hostBanEvent.duration,
			   hostBanEvent.reason.c_str());
    clOptions->acl.save();

    GameKeeper::Player::setAllNeedHostbanChecked(true);

    sendMessage(ServerPlayer, t, "Host pattern added to banlist");
  }
  return true;
}


bool UnbanCommand::operator() (const char         *message,
			       GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the unban command");
    return true;
  }

  if (clOptions->acl.unban(message + 7)) {
    sendMessage(ServerPlayer, t, "Removed IP pattern from the ban list");
    clOptions->acl.save();
  } else {
    sendMessage(ServerPlayer, t, "No pattern removed");
  }
  return true;
}

bool HostUnbanCommand::operator() (const char         *message,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::unban)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the unban command");
    return true;
  }

  if (clOptions->acl.hostUnban(message + 11)) {
    sendMessage(ServerPlayer, t, "Removed host pattern from the ban list");
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
bool MasterBanCommand::operator() (const char         *message,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  std::string callsign = std::string(playerData->player.getCallSign());

  DEBUG2("\"%s\" has requested masterban: %s\n", callsign.c_str(), message);

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::masterBan)) {
    sendMessage(ServerPlayer, t,
		TextUtils::format("%s, you are presently not authorized to "
				  "run /masterban", callsign.c_str()).c_str());
    return true;
  }

  DEBUG3("Player has permission to run /masterban\n");

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
      int               banCount;

      clOptions->acl.purgeMasters();
      sendMessage(ServerPlayer, t,
		  "Previous master ban list entries have been flushed.");

      for (std::vector<std::string>::const_iterator i
	     = clOptions->masterBanListURL.begin();
	   i != clOptions->masterBanListURL.end(); i++) {
	banCount = clOptions->acl.merge(banList.get(i->c_str()));
	std::string reloadmsg
	  = TextUtils::format("Loaded %d master bans from %s", banCount,
			      i->c_str());
	DEBUG1("%s\n", reloadmsg.c_str());
	sendMessage(ServerPlayer, t, reloadmsg.c_str());
      }

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
				    "first 20", bans.size()).c_str());

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
