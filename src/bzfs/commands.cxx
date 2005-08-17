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

#ifdef _MSC_VER
#pragma warning( 4:4786)
#endif

// interface header
#include "commands.h"

#include "WorldEventManager.h"

// system implementation headers
#include <vector>
#include <string>
#include <sstream>
#ifdef HAVE_CSTDIO
#include <cstdio>
#else
#include <stdio.h>
#endif
#ifdef HAVE_CSTRING
#include <cstring>
#else
#include <string.h>
#endif
#include <time.h>

// common implementation headers
#include "Address.h"
#include "CommandManager.h"
#include "LagInfo.h"
#include "NetHandler.h"
#include "PlayerInfo.h"
#include "TimeKeeper.h"
#include "VotingArbiter.h"
#include "global.h"
#include "version.h"

// local implementation headers
#include "CmdLineOptions.h"
#include "FlagHistory.h"
#include "FlagInfo.h"
#include "PackVars.h"
#include "Permissions.h"
#include "RecordReplay.h"
#include "ServerCommand.h"


#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif

// FIXME -- need to pull communication out of bzfs.cxx...

// extern to initialize permission groups
extern void initGroups();

// externs that poll, veto, vote, and clientquery require
extern void sendMessage(int playerIndex, PlayerId dstPlayer, const char *message);
extern void sendPlayerMessage(GameKeeper::Player *playerData, PlayerId dstPlayer,
			      const char *message);
extern CmdLineOptions *clOptions;
extern uint16_t curMaxPlayers;

// externs that ghost needs
extern void removePlayer(int playerIndex, const char *reason, bool notify=true);
// externs that shutdownserver requires
extern bool done;

// externs that superkill and gameover requires
extern bool gameOver;
extern char *getDirectMessageBuffer();
extern void broadcastMessage(uint16_t code, int len, const void *msg);

#include "PlayerInfo.h"
extern TeamInfo team[NumTeams];
extern void sendTeamUpdate(int playerIndex = -1, int teamIndex1 = -1, int teamIndex2 = -1);
extern int numFlags;
extern void sendFlagUpdate(FlagInfo &flag);
extern void sendDrop(FlagInfo &flag);

// externs that countdown requires
extern bool countdownActive;
extern int countdownDelay;
extern TimeKeeper countdownPauseStart;

// externs that identify and password requires
extern void sendIPUpdate(int targetPlayer, int playerIndex);
extern void sendPlayerInfo(void);

tmCustomSlashCommandMap	customCommands;

class MsgCommand : ServerCommand {
public:
  MsgCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ServerQueryCommand : ServerCommand {
public:
  ServerQueryCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class PartCommand : ServerCommand {
public:
  PartCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class QuitCommand : ServerCommand {
public:
  QuitCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class UpTimeCommand : ServerCommand {
public:
  UpTimeCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class PasswordCommand : ServerCommand {
public:
  PasswordCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class SetCommand : ServerCommand {
public:
  SetCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ResetCommand : ServerCommand {
public:
  ResetCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ShutdownCommand : ServerCommand {
public:
  ShutdownCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class SuperkillCommand : ServerCommand {
public:
  SuperkillCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class GameOverCommand : ServerCommand {
public:
  GameOverCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class CountdownCommand : ServerCommand {
public:
  CountdownCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class FlagCommand : ServerCommand {
public:
  FlagCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class LagWarnCommand : ServerCommand {
public:
  LagWarnCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class LagStatCommand : ServerCommand {
public:
  LagStatCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class IdleStatCommand : ServerCommand {
public:
  IdleStatCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class FlagHistoryCommand : ServerCommand {
public:
  FlagHistoryCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class PlayerListCommand : ServerCommand {
public:
  PlayerListCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ReportCommand : ServerCommand {
public:
  ReportCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class HelpCommand : ServerCommand {
public:
  HelpCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class IdentifyCommand : ServerCommand {
public:
  IdentifyCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class RegisterCommand : ServerCommand {
public:
  RegisterCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class GhostCommand : ServerCommand {
public:
  GhostCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class DeregisterCommand : ServerCommand {
public:
  DeregisterCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class SetPassCommand : ServerCommand {
public:
  SetPassCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class GroupListCommand : ServerCommand {
public:
  GroupListCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ShowGroupCommand : ServerCommand {
public:
  ShowGroupCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class GroupPermsCommand : ServerCommand {
public:
  GroupPermsCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class SetGroupCommand : ServerCommand {
public:
  SetGroupCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class RemoveGroupCommand : ServerCommand {
public:
  RemoveGroupCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ReloadCommand : ServerCommand {
public:
  ReloadCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class PollCommand : ServerCommand {
public:
  PollCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class VoteCommand : ServerCommand {
public:
  VoteCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class VetoCommand : ServerCommand {
public:
  VetoCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ViewReportCommand : ServerCommand {
public:
  ViewReportCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ClientQueryCommand : ServerCommand {
public:
  ClientQueryCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class DateTimeCommand : ServerCommand {
public:
  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
protected:
  DateTimeCommand(std::string _commandName) : ServerCommand(_commandName) {};
};

class DateCommand : DateTimeCommand {
public:
  DateCommand();
};

class TimeCommand : DateTimeCommand {
public:
  TimeCommand();
};

class RecordCommand : ServerCommand {
public:
  RecordCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class ReplayCommand : ServerCommand {
public:
  ReplayCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class SayCommand : ServerCommand {
public:
  SayCommand();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class CmdList : ServerCommand {
public:
  CmdList();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

class CmdHelp : ServerCommand {
public:
  CmdHelp();

  virtual bool operator() (const char         *commandLine,
			   GameKeeper::Player *playerData);
};

static MsgCommand         msgCommand;
static ServerQueryCommand serverQueryCommand;
static PartCommand        partCommand;
static QuitCommand        quitCommand;
static UpTimeCommand      upTimeCommand;
static PasswordCommand    passwordCommand;
static SetCommand         setCommand;
static ResetCommand       resetCommand;
static ShutdownCommand    shutdownCommand;
static SuperkillCommand   superkillCommand;
static GameOverCommand    gameOverCommand;
static CountdownCommand   countdownCommand;
static FlagCommand        flagCommand;
static LagWarnCommand     lagWarnCommand;
static LagStatCommand     lagStatCommand;
static IdleStatCommand    idleStatCommand;
static FlagHistoryCommand flagHistoryCommand;
static PlayerListCommand  playerListCommand;
static ReportCommand      ReportCommand;
static HelpCommand        helpCommand;
static IdentifyCommand    identifyCommand;
static RegisterCommand    registerCommand;
static GhostCommand       ghostCommand;
static DeregisterCommand  deregisterCommand;
static SetPassCommand     setPassCommand;
static GroupListCommand   groupListCommand;
static ShowGroupCommand   showGroupCommand;
static GroupPermsCommand  groupPermsCommand;
static SetGroupCommand    setGroupCommand;
static RemoveGroupCommand removeGroupCommand;
static ReloadCommand      reloadCommand;
static PollCommand        pollCommand;
static VoteCommand        voteCommand;
static VetoCommand        vetoCommand;
static ViewReportCommand  viewReportCommand;
static ClientQueryCommand clientQueryCommand;
static DateCommand        dateCommand;
static TimeCommand        timeCommand;
static RecordCommand      recordCommand;
static ReplayCommand      replayCommand;
static SayCommand         sayCommand;
static CmdList            cmdList;
static CmdHelp            cmdHelp;

CmdHelp::CmdHelp()                       : ServerCommand("") {} // fake entry
CmdList::CmdList()                       : ServerCommand("/?",
  "/? - display the list of server-side commands") {}
MsgCommand::MsgCommand() 		 : ServerCommand("/msg",
  "/msg <nick> text - Send text message to nick") {}
ServerQueryCommand::ServerQueryCommand() : ServerCommand("/serverquery",
  "/serverquery - show the server version") {}
PartCommand::PartCommand()               : ServerCommand("/part",
  "/part message - leave the game with a parting message") {}
QuitCommand::QuitCommand()               : ServerCommand("/quit",
  "/quit - leave the game and close the client") {}
UpTimeCommand::UpTimeCommand()           : ServerCommand("/uptime",
  "/uptime - show the server's uptime") {}
PasswordCommand::PasswordCommand()       : ServerCommand("/password",
  "/password <passwd> - become an administrator with <passwd>") {}
SetCommand::SetCommand()                 : ServerCommand("/set",
  "/set[ <var> <value>] - set BZDB variable to value, or display variables") {}
ResetCommand::ResetCommand()             : ServerCommand("/reset",
  "/reset - reset the BZDB variables") {}
ShutdownCommand::ShutdownCommand()       : ServerCommand("/shutdown",
  "/shutdown - kill the server") {}
SuperkillCommand::SuperkillCommand()     : ServerCommand("/superkill",
  "/superkill - kick all of the players") {}
GameOverCommand::GameOverCommand()       : ServerCommand("/gameover") {}
CountdownCommand::CountdownCommand()     : ServerCommand("/countdown") {}
FlagCommand::FlagCommand()               : ServerCommand("/flag") {}
LagWarnCommand::LagWarnCommand()         : ServerCommand("/lagwarn") {}
LagStatCommand::LagStatCommand()         : ServerCommand("/lagstats") {}
IdleStatCommand::IdleStatCommand()       : ServerCommand("/idlestats") {}
FlagHistoryCommand::FlagHistoryCommand() : ServerCommand("/flaghistory") {}
PlayerListCommand::PlayerListCommand()   : ServerCommand("/playerlist") {}
ReportCommand::ReportCommand()           : ServerCommand("/report") {}
HelpCommand::HelpCommand()               : ServerCommand("/help") {}
IdentifyCommand::IdentifyCommand()       : ServerCommand("/identify") {}
RegisterCommand::RegisterCommand()       : ServerCommand("/register") {}
GhostCommand::GhostCommand()             : ServerCommand("/ghost") {}
DeregisterCommand::DeregisterCommand()   : ServerCommand("/deregister") {}
SetPassCommand::SetPassCommand()         : ServerCommand("/setpass") {}
GroupListCommand::GroupListCommand()     : ServerCommand("/grouplist") {}
ShowGroupCommand::ShowGroupCommand()     : ServerCommand("/showgroup") {}
GroupPermsCommand::GroupPermsCommand()   : ServerCommand("/groupperms") {}
SetGroupCommand::SetGroupCommand()       : ServerCommand("/setgroup") {}
RemoveGroupCommand::RemoveGroupCommand() : ServerCommand("/removegroup") {}
ReloadCommand::ReloadCommand()           : ServerCommand("/reload") {}
PollCommand::PollCommand()               : ServerCommand("/poll") {}
VoteCommand::VoteCommand()               : ServerCommand("/vote") {}
VetoCommand::VetoCommand()               : ServerCommand("/veto") {}
ViewReportCommand::ViewReportCommand()   : ServerCommand("/viewreports") {}
ClientQueryCommand::ClientQueryCommand() : ServerCommand("/clientquery") {}
RecordCommand::RecordCommand()           : ServerCommand("/record") {}
ReplayCommand::ReplayCommand()           : ServerCommand("/replay") {}
SayCommand::SayCommand()                 : ServerCommand("/say") {}
DateCommand::DateCommand()               : DateTimeCommand("/date") {}
TimeCommand::TimeCommand()               : DateTimeCommand("/time") {}


bool CmdList::operator() (const char*, GameKeeper::Player *playerData)
{
  int i;
  const int maxLineLen = 64;
  const int playerId = playerData->getIndex();

  // build a std::vector<> from the std::map<> of command names
  std::vector<const std::string*> commands;
  MapOfCommands::iterator it;
  MapOfCommands &commandMap = *getMapRef();
  for (it = commandMap.begin(); it != commandMap.end(); it++) {
    const std::string& cmd = it->first;
    if (cmd[0] != '/') {
      continue; // ignore any fake entries (ex: CmdHelp)
    } else {
      commands.push_back(&cmd);
    }
  }
  const int cmdCount = (int)commands.size();

  // get the maximum length
  unsigned int maxCmdLen = 0;
  for (i = 0; i < cmdCount; i++) {
    if (commands[i]->size() > maxCmdLen) {
      maxCmdLen = commands[i]->size();
    }
  }
  maxCmdLen += 2; // add some padding

  // message generation variables
  char buffer[MessageLen];
  char* cptr = buffer;
  char format[8];
  snprintf(format, 8, "%%-%is", maxCmdLen);

  // formatting parameters
  const int cols = (maxLineLen / maxCmdLen);
  const int rows = ((cmdCount + (cols - 1)) / cols);
  
  for (int row = 0; row < rows; row++) {
    cptr = buffer;
    for (int col = 0; col < cols; col++) {
      const int index = (col * rows) + row;
      if (index >= cmdCount) {
        break;
      }
      sprintf(cptr, format, commands[index]->c_str());
      cptr += maxCmdLen;
    }
    sendMessage(ServerPlayer, playerId, buffer);
  }
  
  return true;
}


bool CmdHelp::operator() (const char         *message,
			     GameKeeper::Player *playerData)
{
  
  int i;
  for (i = 0; message[i] && !isspace(message[i]); i++);
  if (!i)
    return false;
  i--;
  if (message[i] != '?')
    return false;
  std::string commandToken(message, i);

  bool none = true;
  int t = playerData->getIndex();
  MapOfCommands::iterator it;
  MapOfCommands &commandMap = *getMapRef();
  for (it = commandMap.begin(); it != commandMap.end(); it++) {
    std::string master = it->first;
    master.resize(i);
    if (master == commandToken) {
      sendMessage(ServerPlayer, t, it->second->getHelp().c_str());
      none = false;
    }
  }
  if (none)
    sendMessage(ServerPlayer, t,
		("No command starting with " + commandToken).c_str());
  return true;
}


bool UpTimeCommand::operator() (const char         *,
				GameKeeper::Player *playerData)
{
  float rawTime;
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};

  rawTime = float(TimeKeeper::getCurrent() - TimeKeeper::getStartTime());
  snprintf(reply, MessageLen, "%s.", TimeKeeper::printTime(rawTime).c_str());
  sendMessage(ServerPlayer, t, reply);
  return true;
}

bool ServerQueryCommand::operator() (const char         *,
				     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  DEBUG2("Server query requested by %s [%d]\n",
	 playerData->player.getCallSign(), t);

  sendMessage(ServerPlayer, t,
	      TextUtils::format("BZFS Version: %s", getAppVersion()).c_str());
  return true;
}


bool PartCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  std::string byeStatement = "";

  if (strlen(message) > 5) {
    if (!TextUtils::isWhitespace(*(message+5))) {
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "Unknown command [%s]", message);
      sendMessage(ServerPlayer, playerData->getIndex(), reply);
      return true;
    }
    byeStatement = message + 6;
  }

  if (byeStatement[0] != '\0') {
    std::string message2;
    message2 = TextUtils::format("%s has left (\"%s\") ",
				 playerData->player.getCallSign(),  byeStatement.c_str());

    DEBUG2("%s has quit with the message \"%s\"\n", playerData->player.getCallSign(), byeStatement.c_str());
    sendMessage(ServerPlayer, AllPlayers, message2.c_str());
  }

  // now to kick the player
  int t = playerData->getIndex();
  removePlayer(t, byeStatement.c_str());
  return true;
}


bool QuitCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  std::string byeStatement = "";

  if (strlen(message) > 5) {
    if (!TextUtils::isWhitespace(*(message+5))) {
      char reply[MessageLen] = {0};
      snprintf(reply, MessageLen, "Unknown command [%s]", message);
      sendMessage(ServerPlayer, playerData->getIndex(), reply);
      return true;
    }
    byeStatement = message + 6;
  }

  if (byeStatement[0] != '\0') {
    std::string message2;
    message2 = TextUtils::format("%s has quit (\"%s\") ",
				 playerData->player.getCallSign(),  byeStatement.c_str());

    DEBUG2("%s has quit with the message \"%s\"\n", playerData->player.getCallSign(), byeStatement.c_str());
    sendMessage(ServerPlayer, AllPlayers, message2.c_str());
  }

  // now to kick the player
  int t = playerData->getIndex();
  removePlayer(t, byeStatement.c_str());
  return true;
}


bool MsgCommand::operator() (const char         *message,
			     GameKeeper::Player *playerData)
{
  int from = playerData->getIndex();
  int to= -1;

  std::string message2;
  size_t callsignStart=0, callsignEnd=0, messageStart=0;

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::privateMessage)) {
    char reply[MessageLen] = {0};
    snprintf(reply, MessageLen, "%s, you are not presently authorized to /msg people privately", playerData->player.getCallSign());
    sendMessage(ServerPlayer, from, reply);
    return true;
  }

  // start from after "/msg"
  std::string arguments = &message[4];
  std::string recipient = std::string("");

  // skip any leading whitespace
  callsignStart = 0;
  while ((callsignStart < arguments.size()) &&
	 (isspace(arguments[callsignStart]))) {
    callsignStart++;
  }

  // make sure there was _some_ whitespace after /msg
  if (callsignStart == 0) {
    sendMessage(ServerPlayer, from, "Usage: /msg \"some callsign\" some message");
    return true;
  }

  // find the player name, optionally quoted
  if (arguments[callsignStart] == '"') {
    callsignStart++;

    // find the trailing quote
    bool foundQuote = false;
    callsignEnd = callsignStart;
    while ((callsignEnd + 1 < arguments.size()) &&
	   (!foundQuote)) {
      callsignEnd++;
      if (arguments[callsignEnd] == '"') {
	foundQuote = true;
	messageStart = callsignEnd + 1;
	callsignEnd--;
      }
    }

    // no quote means a mismatch
    if (!foundQuote) {
      sendMessage(ServerPlayer, from, "Quote mismatch?");
      sendMessage(ServerPlayer, from, "Usage: /msg \"some callsign\" some message");
      return true;
    }

  } else {
    // unquoted callsign

    // find the first matching name (not the longest for sake of performance)
    bool foundCallsign = false;
    callsignEnd = callsignStart;
    while ((callsignEnd + 1 < arguments.size()) &&
	   (!foundCallsign)) {
      callsignEnd++;
      if (!isspace(arguments[callsignEnd])) {
	continue;
      }

      // we have a space
      recipient = arguments.substr(callsignStart, callsignEnd - callsignStart);
      messageStart = callsignEnd;

      to = GameKeeper::Player::getPlayerIDByName(recipient);
      if (to < curMaxPlayers) {
	callsignEnd--;
	foundCallsign = true;
      }
    }
  }

  recipient = arguments.substr(callsignStart, callsignEnd - callsignStart + 1);

  if (recipient[0] == '>') {
    // /msg >admin sends on admin channel, /msg >team on team channel
    recipient.erase(0,1);
    if (TextUtils::toupper(recipient) == "ADMIN") 
      to = AdminPlayers;				
    else if (TextUtils::toupper(recipient) == "TEAM") 
      to = 250 - (int)playerData->player.getTeam();
  } else {
    to = GameKeeper::Player::getPlayerIDByName(recipient);

    // valid callsign
    if ((to < 0) || (to >= curMaxPlayers)) {
      message2 = TextUtils::format("\"%s\" is not here.  No such callsign.", recipient.c_str());
      sendMessage(ServerPlayer, from, message2.c_str());
      return true;
    }
  }

  // make sure there is something to send
  if ((messageStart >= arguments.size() - 1) || (messageStart == 0)) {
    // found player, but nothing to send
    message2 = TextUtils::format("No text to send to \"%s\".", recipient.c_str());
    sendMessage(ServerPlayer, from, message2.c_str());
    return true;
  }

  // send the message
  sendPlayerMessage(playerData, to, arguments.c_str() + messageStart + 1);
  return true;
}


bool PasswordCommand::operator() (const char         *message,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (playerData->accessInfo.passwordAttemptsMax()) {
    DEBUG1("\"%s\" (%s) has attempted too many /password tries\n",
	   playerData->player.getCallSign(),
	   playerData->netHandler->getTargetIP());
    sendMessage(ServerPlayer, t, "Too many attempts");
  } else {
    if ((clOptions->password != "") && strncmp(message + 10, clOptions->password.c_str(), clOptions->password.size()) == 0 && clOptions->password.length() == strlen(message + 10)) {
      playerData->accessInfo.setAdmin();
      sendPlayerInfo();
      sendMessage(ServerPlayer, t, "You are now an administrator!");
    } else {
      sendMessage(ServerPlayer, t, "Wrong Password!");
    }
  }
  return true;
}


bool SetCommand::operator() (const char         *message,
			     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  int setvar = playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar) ? 1 : 0;
  int setall = playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll) ? 1 : 0;
  char message2[MessageLen];
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar)
      && !playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the set command");
	DEBUG3("set failed by %s, setvar=%d, setall=%d\n",playerData->player.getCallSign(),setvar,setall);
   return true;
  }
  if (Replay::enabled()) {
    sendMessage(ServerPlayer, t, "You can't /set variables in replay mode");
    return true;
  }
  DEBUG3("set executed by %s, setvar=%d, setall=%d\n",playerData->player.getCallSign(),setvar,setall);
  std::string command = (message + 1);
  // we aren't case sensitive but CMDMGR is
  for (int i = 0; i < 3 /*"set"*/; ++i)
    command[i] = tolower(command[i]);

  bool	cmdError = false;

  std::string cmdReturn = CMDMGR.run(command,&cmdError);
  if(!cmdError)
  {
	  std::string errMsg = "/set failed, reason: ";
	  errMsg += cmdReturn;

	  sendMessage(ServerPlayer, t, errMsg.c_str());
	  return true;
  }

  sendMessage(ServerPlayer, t, cmdReturn.c_str());
  snprintf(message2, MessageLen, "Variable Modification Notice by %s of %s",
	   playerData->player.getCallSign(), command.c_str());
  sendMessage(ServerPlayer, AllPlayers, message2);
  return true;
}


bool ResetCommand::operator() (const char         *message,
			       GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setVar)
      && !playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the reset command");
    return true;
  }
  if (Replay::enabled()) {
    sendMessage(ServerPlayer, t, "You can't /reset variables in replay mode");
    return true;
  }
  std::string command = (message + 1);
  // we aren't case sensitive but CMDMGR is
  for (int i = 0; i < 5 /*"reset"*/; ++i)
    command[i] = tolower(command[i]);
  sendMessage(ServerPlayer, t, CMDMGR.run(command).c_str());
  return true;
}


bool ShutdownCommand::operator() (const char         *,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::shutdownServer)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the shutdown command");
    return true;
  }
  done = true;
  return true;
}


bool SuperkillCommand::operator() (const char         *,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::superKill)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the superkill command");
    return true;
  }
  for (int i = 0; i < curMaxPlayers; i++)
    removePlayer(i, "/superkill");
  gameOver = true;
  if (clOptions->timeManualStart)
    countdownActive = false;
  return true;
}


bool GameOverCommand::operator() (const char         *,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::endGame)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the gameover command");
    return true;
  }

  void *buf, *bufStart = getDirectMessageBuffer();
  buf = nboPackUByte(bufStart, t);
  buf = nboPackUShort(buf, uint16_t(NoTeam));
  broadcastMessage(MsgScoreOver, (char*)buf - (char*)bufStart, bufStart);
  gameOver = true;
  if (clOptions->timeManualStart) {
    countdownActive = false;
    countdownPauseStart = TimeKeeper::getNullTime();
    clOptions->countdownPaused = false;
  }
  return true;
}


bool CountdownCommand::operator() (const char         * message,
				   GameKeeper::Player *playerData)
{
  // /countdown starts timed game, if start is manual, everyone is allowed to
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::countdown)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the countdown command");
    return true;
  } else if (!clOptions->timeManualStart) {
    sendMessage(ServerPlayer, t, "This server was not configured for manual clock countdowns");
    return true;
  } else if (countdownDelay > 0) {
    sendMessage(ServerPlayer, t, "There is a countdown already in progress");
    return true;
  }

  // if the timelimit is not set .. don't countdown
  if (clOptions->timeLimit > 1.0f) {
    std::vector<std::string> parts = TextUtils::tokenize(message, " \t",2);
    
    if (parts.size() > 1) {
      // we have an argument
      
      if (parts[1] == "pause") {
	// pause the countdown
	if (!countdownActive) {
	  sendMessage(ServerPlayer, t, "There is no active game to pause");
	  return true;
	} else if (clOptions->countdownPaused) {
	  sendMessage(ServerPlayer, t, "The game is already paused");
	  return true;
	}
	clOptions->countdownPaused = true;
	sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown paused by %s",playerData->player.getCallSign()).c_str());
	return true;
      } else if (parts[1] == "resume") {
	// resume countdown if it was paused before
	if (!clOptions->countdownPaused) {
	  sendMessage(ServerPlayer, t, "The game is not paused");
	  return true;
	}
	clOptions->countdownPaused = false;
	sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Countdown resumed by %s",playerData->player.getCallSign()).c_str());
	return true;
	      
      } else {
	// so it's the countdown delay? else tell the player how to use /countdown
	std::istringstream timespec(message+10);
	if (!(timespec >> countdownDelay)) {
	  sendMessage(ServerPlayer, t, "Usage: /countdown [<seconds>|pause|resume]");
	  return true;
	}
      }
    } else {
      countdownDelay = 10;
    }
    
    // cancel here if a game is already running
    if (countdownActive) {
      sendMessage(ServerPlayer, t, "A game is already in progress");
      countdownDelay = -1;
      return true;
    }

    // limit/sanity check
    const int max_delay = 120;
    if (countdownDelay > max_delay) {
      sendMessage(ServerPlayer, t, TextUtils::format("Countdown set to %d instead of %d", max_delay, countdownDelay).c_str());
      countdownDelay = max_delay;
    } else if (countdownDelay < 0) {
      sendMessage(ServerPlayer, t, TextUtils::format("Countdown set to 0 instead of %d", countdownDelay).c_str());
      countdownDelay = 0;
    }

    sendMessage(ServerPlayer, AllPlayers, TextUtils::format("Team scores reset, countdown started by %s.",playerData->player.getCallSign()).c_str());

    // let everyone know what's going on
    long int timeArray[4];
    std::string matchBegins;
    if (countdownDelay == 0) {
      matchBegins = "Match begins now!";
    } else {
      TimeKeeper::convertTime(countdownDelay, timeArray);
      std::string countdowntime = TimeKeeper::printTime(timeArray);
      matchBegins = TextUtils::format("Match begins in about %s", countdowntime.c_str());
    }
    sendMessage(ServerPlayer, AllPlayers, matchBegins.c_str());

    TimeKeeper::convertTime(clOptions->timeLimit, timeArray);
    std::string timelimit = TimeKeeper::printTime(timeArray);
    matchBegins = TextUtils::format("Match duration is %s", timelimit.c_str());
    sendMessage(ServerPlayer, AllPlayers, matchBegins.c_str());
		
    // make sure the game always start unpaused
    clOptions->countdownPaused = false;
    countdownPauseStart = TimeKeeper::getNullTime();

  } else {
    sendMessage(ServerPlayer, AllPlayers, "Team scores reset.");
    sendMessage(ServerPlayer, t, "The server is not configured for timed matches.");
  }

  // reset team scores
  for (int i = RedTeam; i <= PurpleTeam; i++) {
    team[i].team.lost = team[i].team.won = 0;
  }
  sendTeamUpdate();

  return true;
}


bool FlagCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::flagMod)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flag command");
    return true;
  }
  if (strncasecmp(message + 6, "reset", 5) == 0) {
    bool onlyUnused = strncasecmp(message + 11, " unused", 7) == 0;
    bz_resetFlags(onlyUnused);
  } else if (strncasecmp(message + 6, "up", 2) == 0) {
    for (int i = 0; i < numFlags; i++) {
      FlagInfo &flag = *FlagInfo::get(i);
      if (flag.flag.type->flagTeam == ::NoTeam) {
	sendDrop(flag);
	flag.flag.status = FlagGoing;
	if (!flag.required)
	  flag.flag.type = Flags::Null;
	sendFlagUpdate(flag);
      }
    }

  } else if (strncasecmp(message + 6, "show", 4) == 0) {
    for (int i = 0; i < numFlags; i++) {
      char showMessage[MessageLen];
      FlagInfo::get(i)->getTextualInfo(showMessage);
      sendMessage(ServerPlayer, t, showMessage);
    }
  } else {
    sendMessage(ServerPlayer, t, "reset|show|up");
  }
  return true;
}


bool LagWarnCommand::operator() (const char         *message,
				 GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::lagwarn)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagwarn command");
    return true;
  }

  char reply[MessageLen] = {0};

  if (message[8] == ' ') {
    const char *maxlag = message + 9;
    clOptions->lagwarnthresh = (float) (atoi(maxlag) / 1000.0);
    snprintf(reply, MessageLen, "lagwarn is now %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
  } else {
    snprintf(reply, MessageLen, "lagwarn is set to %d ms", int(clOptions->lagwarnthresh * 1000 + 0.5));
  }
  sendMessage(ServerPlayer, t, reply);
  return true;
}


bool lagCompare(const GameKeeper::Player *a, const GameKeeper::Player *b)
{
  if (a->player.isObserver() && !b->player.isObserver())
    return true;
  if (!a->player.isObserver() && b->player.isObserver())
    return false;
  return a->lagInfo.getLag() < b->lagInfo.getLag();
}

bool LagStatCommand::operator() (const char         *,
				 GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::lagStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the lagstats command");
    return true;
  }
  // yeah, ok this is ugly but it works - curMaxPlayers is never >255
  GameKeeper::Player *sortedPlayer[256];
  int i = 0, j = 0;
  for (i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(i);
    if (p != NULL) {
      sortedPlayer[j++] = p;
    }
  }
  std::sort(sortedPlayer, sortedPlayer + j, lagCompare);

  char reply[MessageLen];
  for (i = 0; i < j; i++) {
    GameKeeper::Player *p = sortedPlayer[i];
    p->lagInfo.getLagStats(reply);
    if (reply[0])
      sendMessage(ServerPlayer, t, reply);
  }
  return true;
}


bool IdleStatCommand::operator() (const char         *,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::idleStats)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the idlestats command");
    return true;
  }

  GameKeeper::Player *otherData;
  std::string reply;
  for (int i = 0; i < curMaxPlayers; i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (!otherData)
      continue;
    reply = otherData->player.getIdleStat();
    if (reply != "")
      sendMessage(ServerPlayer, t, reply.c_str());
  }
  return true;
}


bool FlagHistoryCommand::operator() (const char         *,
				     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::flagHistory)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the flaghistory command");
    return true;
  }

  char reply[MessageLen];
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData != NULL && otherData->player.isPlaying()
	&& !otherData->player.isObserver()) {
      snprintf(reply, MessageLen, "%-16s : ", otherData->player.getCallSign());
      otherData->flagHistory.get(reply+strlen(reply));
      sendMessage(ServerPlayer, t, reply);
    }
  }
  return true;
}


bool PlayerListCommand::operator() (const char         *,
				    GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::playerList)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the playerlist command");
    return true;
  }

  GameKeeper::Player *otherData;
  char reply[MessageLen] = {0};

  for (int i = 0; i < curMaxPlayers; i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->player.isPlaying()) {
      otherData->netHandler->getPlayerList(reply);
      sendMessage(ServerPlayer, t, reply);
    }
  }
  return true;
}


bool ReportCommand::operator() (const char         *message,
				GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::talk)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the report command");
    return true;
  }

  if (strlen(message + 1) < 8) {
    sendMessage(ServerPlayer, t, "Nothing reported");
  } else {
    time_t now = time(NULL);
    char* timeStr = ctime(&now);
    std::string reportStr;
    reportStr = reportStr + timeStr + "Reported by " +
      playerData->player.getCallSign() + ": " + (message + 8);
    if (clOptions->reportFile.size() > 0) {
      std::ofstream ofs(clOptions->reportFile.c_str(), std::ios::out | std::ios::app);
      ofs << reportStr << std::endl << std::endl;
    }
    if (clOptions->reportPipe.size() > 0) {
      FILE* pipeWrite = popen(clOptions->reportPipe.c_str(), "w");
      if (pipeWrite != NULL) {
	fprintf(pipeWrite, "%s\n\n", reportStr.c_str());
      } else {
	DEBUG1("Couldn't write report to the pipe\n");
      }
      pclose(pipeWrite);
    }
    if (clOptions->reportFile.size() == 0 && clOptions->reportPipe.size() == 0) {
      sendMessage(ServerPlayer, t, "The report command is disabled on this server");
    } else {
      std::string temp = std::string("**\"") + playerData->player.getCallSign() + "\" reports: " +
			 (message + 8);
      if (temp.size() <= (unsigned) MessageLen) {
	sendMessage (ServerPlayer, AdminPlayers, temp.c_str());
	return true;
      }
      const std::vector<std::string> words = TextUtils::tokenize(temp, " \t");
      unsigned int cur = 0;
      const unsigned int wordsize = words.size();
      while (cur != wordsize) {
	std::string temp2;
	while (temp2.size() <= (unsigned) MessageLen &&
	       cur != wordsize &&
	       (temp2.size() + words[cur].size()) <= (unsigned) MessageLen) {
	    temp2 += words[cur] + " ";
	    ++cur;
	}
	sendMessage (ServerPlayer, AdminPlayers, temp2.c_str());
      }
      sendMessage (ServerPlayer, AdminPlayers, message);
      DEBUG1("Player %s [%d] has filed a report (time: %s).\n",
	     playerData->player.getCallSign(), t, timeStr);

      sendMessage(ServerPlayer, t, "Your report has been filed. Thank you.");
    }
  }

  return true;
}


bool HelpCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};

  if (strlen(message + 1) == 4) {
    const std::vector<std::string>& chunks = clOptions->textChunker.getChunkNames();
    sendMessage(ServerPlayer, t, "Available help pages (use /help <page>)");
    for (int i = 0; i < (int) chunks.size(); i++) {
      sendMessage(ServerPlayer, t, chunks[i].c_str());
    }
  } else {
    bool foundChunk = false;
    const std::vector<std::string>& chunks = clOptions->textChunker.getChunkNames();
    for (int i = 0; i < (int)chunks.size() && (!foundChunk); i++) {
      if (chunks[i] == (message +6)){
	const std::vector<std::string>* lines = clOptions->textChunker.getTextChunk((message + 6));
	if (lines != NULL) {
	  for (int j = 0; j < (int)lines->size(); j++) {
	    sendMessage(ServerPlayer, t, (*lines)[j].c_str());
	  }
	  foundChunk = true;
	  break;
	}
      }
    }
    if (!foundChunk) {
      snprintf(reply, MessageLen, "Help command %s not found", message + 6);
      sendMessage(ServerPlayer, t, reply);
    }
  }
  return true;
}


bool IdentifyCommand::operator() (const char         *message,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/identify command disabled");
    return true;
  }
  // player is trying to send an ID
  if (playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You have already identified");
  } else if (playerData->accessInfo.gotAccessFailure()) {
    sendMessage(ServerPlayer, t, "You have attempted to identify too many times");
  } else {
    // get their info
    if (!playerData->accessInfo.isRegistered()) {
      // not in DB, tell them to reg
      sendMessage(ServerPlayer, t, "This callsign is not registered,"
		  " please register it with a /register command");
    } else {
      if (playerData->accessInfo.isPasswordMatching(message + 10)) {
	sendMessage(ServerPlayer, t, "Password Accepted, welcome back.");

	// get their real info
	playerData->accessInfo.setPermissionRights();

	// if they have the PLAYERLIST permission, send the IP list
	sendIPUpdate(t, -1);
	sendPlayerInfo();
      } else {
	playerData->accessInfo.setLoginFail();
	sendMessage(ServerPlayer, t, "Identify Failed, please make sure"
		    " your password was correct");
      }
    }
  }
  return true;
}


bool RegisterCommand::operator() (const char         *message,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/register command disabled");
    return true;
  }
  if (playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You have already registered and"
		" identified this callsign");
  } else {
    if (playerData->accessInfo.isRegistered()) {
      sendMessage(ServerPlayer, t, "This callsign is already registered,"
		  " if it is yours /identify to login");
    } else {
      if (strlen(message) > 12) {
	playerData->accessInfo.storeInfo(message + 10);
	sendMessage(ServerPlayer, t, "Callsign registration confirmed,"
		    " please /identify to login");
      } else {
	sendMessage(ServerPlayer, t, "Your password must be 3 or more characters");
      }
    }
  }
  return true;
}


bool GhostCommand::operator() (const char         *message,
			       GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/ghost command disabled");
    return true;
  }
  char *p1 = (char*)strchr(message + 1, '\"');
  char *p2 = 0;
  if (p1) p2 = strchr(p1 + 1, '\"');
  if (!p2) {
    sendMessage(ServerPlayer, t, "not enough parameters, usage"
		" /ghost \"CALLSIGN\" PASSWORD");
  } else {
    std::string ghostie(p1 + 1, p2 - p1 - 1);
    std::string ghostPass = p2 + 2;

    makeupper(ghostie);

    int user = GameKeeper::Player::getPlayerIDByName(ghostie);
    if (user == -1) {
      sendMessage(ServerPlayer, t, "There is no user logged in by that name");
    } else {
      if (!userExists(ghostie)) {
	sendMessage(ServerPlayer, t, "That callsign is not registered");
      } else {
	if (!verifyUserPassword(ghostie, ghostPass)) {
	  sendMessage(ServerPlayer, t, "Invalid Password");
	} else {
	  sendMessage(ServerPlayer, t, "Ghosting User");
	  char temp[MessageLen];
	  snprintf(temp, MessageLen, "Your Callsign is registered to another user,"
		  " You have been ghosted by %s",
		  playerData->player.getCallSign());
	  sendMessage(ServerPlayer, user, temp);
	  removePlayer(user, "Ghost");
	}
      }
    }
  }
  return true;
}


bool DeregisterCommand::operator() (const char         *message,
				    GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/deregister command disabled");
    return true;
  }
  if (!playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the deregister command");
    return true;
  }

  if (strlen(message) == 11) {
    // removing own callsign
    passwordDatabase.erase(playerData->accessInfo.getName());
    userDatabase.erase(playerData->accessInfo.getName());
    PlayerAccessInfo::updateDatabases();
    sendMessage(ServerPlayer, t, "Your callsign has been deregistered");
  } else if (strlen(message) > 12
	     && playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    char reply[MessageLen];

    // removing someone else's
    std::string name = message + 12;
    makeupper(name);
    if (userExists(name)) {

      // admins can override antiperms
      if (!playerData->accessInfo.isAdmin()) {
	// make sure this player isn't protected
	int v = GameKeeper::Player::getPlayerIDByName(name);
	GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(v);
	if ((p != NULL) && (p->accessInfo.hasPerm(PlayerAccessInfo::antideregister))) {
	  snprintf(reply, MessageLen, "%s is protected from being deregistered.", p->player.getCallSign());
	  sendMessage(ServerPlayer, t, reply);
	  return true;
	}
      }

      passwordDatabase.erase(name);
      userDatabase.erase(name);
      PlayerAccessInfo::updateDatabases();

      snprintf(reply, MessageLen, "%s has been deregistered", name.c_str());
      sendMessage(ServerPlayer, t, reply);
    } else {
      snprintf(reply, MessageLen, "user %s does not exist", name.c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t, "You do not have permission to deregister this user");
  }

  return true;
}


bool SetPassCommand::operator() (const char         *message,
				 GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!passFile.size()){
    sendMessage(ServerPlayer, t, "/setpass command disabled");
    return true;
  }
  if (!playerData->accessInfo.isVerified()) {
    sendMessage(ServerPlayer, t, "You must be registered and verified to run the setpass command");
    return true;
  }

  size_t startPosition = 7;
  /* skip any leading whitespace */
  while (isspace(message[++startPosition]))
    ;
  if (startPosition == strlen(message) || !isspace(message[8])) {
    sendMessage(ServerPlayer, t, "Not enough parameters: usage /setpass PASSWORD");
    return true;
  }
  std::string pass = message + startPosition;
  playerData->accessInfo.setPasswd(pass);
  char text[MessageLen];
  snprintf(text, MessageLen, "Your password is now set to \"%s\"", pass.c_str());
  sendMessage(ServerPlayer, t, text);
  return true;
}


bool GroupListCommand::operator() (const char         *,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  sendMessage(ServerPlayer, t, "Group List:");
  PlayerAccessMap::iterator itr = groupAccess.begin();
  while (itr != groupAccess.end()) {
    sendMessage(ServerPlayer, t, itr->first.c_str());
    itr++;
  }
  return true;
}


bool ShowGroupCommand::operator() (const char         *message,
				   GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  std::string settie;

  if (strlen(message) == 10) {	 // show own groups
    if (playerData->accessInfo.isVerified()) {
      settie = playerData->accessInfo.getName();
    } else {
      sendMessage(ServerPlayer, t, "You are not identified");
    }
  } else if (playerData->accessInfo.hasPerm(PlayerAccessInfo::showOthers)) {
    // show groups for other player
    char *p1 = (char*) strchr(message + 1, '\"');
    char *p2 = 0;
    if (p1) p2 = strchr(p1 + 1, '\"');
    if (p2) {
      settie = std::string(p1 + 1, p2 - p1 - 1);
      makeupper(settie);
    } else {
      sendMessage(ServerPlayer, t, "wrong format, usage"
		  " /showgroup  or  /showgroup \"CALLSIGN\"");
    }
  } else {
    sendMessage(ServerPlayer, t, "No permission!");
  }

  // something is wrong
  if (settie != "") {
    int playerIndex = GameKeeper::Player::getPlayerIDByName(settie);
    // once for global groups
    if (playerIndex < curMaxPlayers) {
      GameKeeper::Player* target = GameKeeper::Player::getPlayerByIndex(playerIndex);
      if (target != NULL) {
	PlayerAccessInfo &info = target->accessInfo;
	// FIXME remove local groups from this list. better yet unify the two.
	std::string line = "Global Groups (only extras) for ";
	line += settie;
	line += ": ";
	std::vector<std::string>::iterator itr = info.groups.begin();
	while (itr != info.groups.end()) {
	  line += *itr;
	  line += " ";
	  itr++;
	}
	while (line.size() > (unsigned int)MessageLen) {
	  sendMessage(ServerPlayer, t, line.substr(0, MessageLen).c_str());
	  line.erase(line.begin(), line.begin() + (MessageLen - 1));
	}
	sendMessage(ServerPlayer, t, line.c_str());
      }
    }
    // once for local groups
    if (userExists(settie)) {
      PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

      std::string line = "Local groups for ";
      line += settie;
      line += ": ";
      std::vector<std::string>::iterator itr = info.groups.begin();
      while (itr != info.groups.end()) {
	line += *itr;
	line += " ";
	itr++;
      }
      while (line.size() > (unsigned int)MessageLen) {
	sendMessage(ServerPlayer, t, line.substr(0, MessageLen).c_str());
	line.erase(line.begin(), line.begin() + (MessageLen - 1));
      }
      sendMessage(ServerPlayer, t, line.c_str());
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return true;
}


bool GroupPermsCommand::operator() (const char         *,
				    GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  sendMessage(ServerPlayer, t, "Group List:");
  PlayerAccessMap::iterator itr = groupAccess.begin();
  std::string line;
  while (itr != groupAccess.end()) {
    line = itr->first + ":   ";
    sendMessage(ServerPlayer, t, line.c_str());

    // allows first
    if (itr->second.explicitAllows.any()) {
      sendMessage(ServerPlayer, t, "  Allows");
      for (int i = 0; i < PlayerAccessInfo::lastPerm; i++) {
     	if (itr->second.explicitAllows.test(i) && !itr->second.explicitDenys.test(i) ) {
	  line = "     ";
	  line += nameFromPerm((PlayerAccessInfo::AccessPerm)i);
	  sendMessage(ServerPlayer, t, line.c_str());
      	}
      }
    }
		
    // same about denys
    if (itr->second.explicitDenys.any()) {
      sendMessage(ServerPlayer, t, "  Denys");
      for (int i = 0; i < PlayerAccessInfo::lastPerm; i++) {
     	if (itr->second.explicitDenys.test(i) ) {
	  line = "     ";
	  line += nameFromPerm((PlayerAccessInfo::AccessPerm)i);
	  sendMessage(ServerPlayer, t, line.c_str());
      	}
      }
    }
			
    itr++;
  }
  return true;
}


bool SetGroupCommand::operator() (const char         *message,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!userDatabaseFile.size()) {
    sendMessage(ServerPlayer, t, "/setgroup command disabled");
    return true;
  }
  char *p1 = (char*)strchr(message + 1, '\"');
  char *p2 = 0;
  if (p1) p2 = strchr(p1 + 1, '\"');
  if (!p2) {
    sendMessage(ServerPlayer, t, "not enough parameters, usage /setgroup \"CALLSIGN\" GROUP");
  } else {
    std::string settie(p1 + 1, p2 - p1 - 1);
    std::string group = p2 + 2;

    makeupper(settie);
    makeupper(group);

    if (userExists(settie)) {
      if (!playerData->accessInfo.canSet(group)) {
	sendMessage(ServerPlayer, t, "You do not have permission to set this group");
      } else {
	PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);

	if (info.addGroup(group)) {
	  sendMessage(ServerPlayer, t, "Group Add successful");
	  int getID = GameKeeper::Player::getPlayerIDByName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    snprintf(temp, MessageLen, "you have been added to the %s group, by %s",
		    group.c_str(), playerData->player.getCallSign());
	    sendMessage(ServerPlayer, getID, temp);
	    GameKeeper::Player::getPlayerByIndex(getID)->accessInfo.
	      addGroup(group);
	  }
	  PlayerAccessInfo::updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "Group Add failed (user may already be in that group)");
	}
      }
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return true;
}


bool RemoveGroupCommand::operator() (const char         *message,
				     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!userDatabaseFile.size()) {
    sendMessage(ServerPlayer, t, "/removegroup command disabled");
    return true;
  }
  char *p1 = (char*)strchr(message + 1, '\"');
  char *p2 = 0;
  if (p1) p2 = strchr(p1 + 1, '\"');
  if (!p2) {
    sendMessage(ServerPlayer, t, "not enough parameters, usage /removegroup \"CALLSIGN\" GROUP");
  } else {
    std::string settie(p1 + 1, p2 - p1 - 1);
    std::string group = p2 + 2;

    makeupper(settie);
    makeupper(group);
    if (userExists(settie)) { 
      if (!playerData->accessInfo.canSet(group)) {
	sendMessage(ServerPlayer, t, "You do not have permission to remove this group");
      } else {
	PlayerAccessInfo &info = PlayerAccessInfo::getUserInfo(settie);
	if (info.removeGroup(group)) {
	  sendMessage(ServerPlayer, t, "Group Remove successful");
	  int getID = GameKeeper::Player::getPlayerIDByName(settie);
	  if (getID != -1) {
	    char temp[MessageLen];
	    snprintf(temp, MessageLen, "You have been removed from the %s group, by %s",
		    group.c_str(), playerData->player.getCallSign());
	    sendMessage(ServerPlayer, getID, temp);
	    GameKeeper::Player::getPlayerByIndex(getID)->accessInfo.
	      removeGroup(group);
	  }
	  PlayerAccessInfo::updateDatabases();
	} else {
	  sendMessage(ServerPlayer, t, "Group Remove failed (user may not have been in group)");
	}
      }
    } else {
      sendMessage(ServerPlayer, t, "There is no user by that name");
    }
  }
  return true;
}


bool ReloadCommand::operator() (const char         *,
				GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::setAll)) {
    sendMessage(ServerPlayer, t,
		"You do not have permission to run the reload command");
    return true;
  }

  // reload the banlist
  clOptions->acl.load();

  groupAccess.clear();
  userDatabase.clear();
  passwordDatabase.clear();
  initGroups();
  if (passFile.size())
    readPassFile(passFile);
  if (userDatabaseFile.size())
    PlayerAccessInfo::readPermsFile(userDatabaseFile);
  GameKeeper::Player::reloadAccessDatabase();
  sendMessage(ServerPlayer, t, "Databases reloaded");

  // Validate all of the current players

  std::string reason;
  char kickmessage[MessageLen];

  // Check host bans
  GameKeeper::Player::setAllNeedHostbanChecked(true);

  // Check IP bans
  for (int i = 0; i < curMaxPlayers; i++) {
    GameKeeper::Player *otherPlayer = GameKeeper::Player::getPlayerByIndex(i);
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
	snprintf(kickmessage, MessageLen, "Reason given: %s", reason.c_str());
	sendMessage(ServerPlayer, i, kickmessage);
      }
      removePlayer(i, "/ban");
    }
  }
  return true;
}


bool VoteCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};
  std::string callsign = std::string(playerData->player.getCallSign());

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::vote)) {
    /* permission denied for /vote */
    snprintf(reply, MessageLen, "%s, you are presently not authorized to run /vote", callsign.c_str());
    sendMessage(ServerPlayer, t, reply);
    return true;
  }

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return true;
  }

  // only need to get this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure that there is a poll to vote upon */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sendMessage(ServerPlayer, t, "A poll is not presently in progress.  There is nothing to vote on");
    return true;
  }

  std::string voteCmd = &message[5];
  std::string answer;

  /* find the start of the vote answer */
  size_t startPosition = 0;
  while ((startPosition < voteCmd.size()) &&
	 (isspace(voteCmd[startPosition]))) {
    startPosition++;
  }

  /* stash the answer ('yes', 'no', etc) in lowercase to simplify comparison */
  for (size_t i = startPosition;  i < voteCmd.size() && !isspace(voteCmd[i]); i++) {
    answer += tolower(voteCmd[i]);
  }

  std::vector<std::string> yesAnswers;
  yesAnswers.push_back("y");
  yesAnswers.push_back("1");
  yesAnswers.push_back("yes");
  yesAnswers.push_back("yea");
  yesAnswers.push_back("si"); // spanish+
  yesAnswers.push_back("ja");  // german
  yesAnswers.push_back("oui"); // french
  yesAnswers.push_back("sim"); // portuguese
  yesAnswers.push_back("tak"); // polish

  std::vector<std::string> noAnswers;
  noAnswers.push_back("n");
  noAnswers.push_back("0");
  noAnswers.push_back("no");
  noAnswers.push_back("nay");
  noAnswers.push_back("nein"); // german
  noAnswers.push_back("nien"); // german misspelled
  noAnswers.push_back("non"); // french
  noAnswers.push_back("nao"); // portuguese
  noAnswers.push_back("nie"); // polish

  // see if the vote response is a valid yes or no answer
  int vote = -1;
  unsigned int maxAnswerCount = noAnswers.size() > yesAnswers.size() ? noAnswers.size() : yesAnswers.size();
  for (unsigned int v = 0; v < maxAnswerCount; v++) {
    if (v < yesAnswers.size()) {
      if (answer == yesAnswers[v]) {
	vote = 1;
	break;
      }
    }
    if (v < noAnswers.size()) {
      if (answer == noAnswers[v]) {
	vote = 0;
	break;
      }
    }
  }

  // cast the vote or complain
  bool cast = false;
  if (vote == 0) {
    if ((cast = arbiter->voteNo(callsign)) == true) {
      /* player voted no */
      snprintf(reply, MessageLen, "%s, your vote in opposition of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else if (vote == 1) {
    if ((cast = arbiter->voteYes(callsign)) == true) {
      /* player voted yes */
      snprintf(reply, MessageLen, "%s, your vote in favor of the %s has been recorded", callsign.c_str(), arbiter->getPollAction().c_str());
      sendMessage(ServerPlayer, t, reply);
    }
  } else {
    if (answer.length() == 0) {
      snprintf(reply, MessageLen, "%s, you did not provide a vote answer", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
    } else {
      snprintf(reply, MessageLen, "%s, you did not vote in favor or in opposition", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
    }
    sendMessage(ServerPlayer, t, "Usage: /vote yes|no|y|n|1|0|yea|nay|si|ja|nein|oui|non|sim|nao");
    return true;
  }

  if (arbiter->hasVoted(callsign)) {
    /* player already voted */
    snprintf(reply, MessageLen, "%s, you have already voted on the poll to %s %s", callsign.c_str(), arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    return true;
  }

  if (!cast){
    /* There was an error while voting, probably could send a less generic message */
    snprintf(reply, MessageLen, "%s, there was an error while voting on the poll to %s %s", callsign.c_str(), arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    return true;
  }

  return true;
}


bool VetoCommand::operator() (const char         *,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::veto)) {
    /* permission denied for /veto */
    sendMessage(ServerPlayer, t,
		TextUtils::format
		("%s, you are presently not authorized to run /veto",
		 playerData->player.getCallSign()).c_str());
    return true;
  }

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return true;
  }

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  /* make sure there is an unexpired poll */
  if ((arbiter != NULL) && !arbiter->knowsPoll()) {
    sendMessage(ServerPlayer, t,
		TextUtils::format
		("%s, there is presently no active poll to veto",
		 playerData->player.getCallSign()).c_str());
    return true;
  }

  sendMessage(ServerPlayer, t,
	      TextUtils::format("%s, you have cancelled the poll to %s %s",
				  playerData->player.getCallSign(),
				  arbiter->getPollAction().c_str(),
				  arbiter->getPollTarget().c_str()).c_str());

  /* poof */
  arbiter->forgetPoll();

  sendMessage(ServerPlayer, AllPlayers,
	      TextUtils::format("The poll was cancelled by %s",
				  playerData->player.getCallSign()).c_str());

  return true;
}


bool PollCommand::operator() (const char         *message,
			      GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  char reply[MessageLen] = {0};
  std::string callsign = std::string(playerData->player.getCallSign());

  DEBUG2("\"%s\" has requested a poll: %s\n", callsign.c_str(), message);

  /* make sure player has permission to request a poll */
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::poll)) {
    snprintf(reply, MessageLen, "%s, you are presently not authorized to run /poll", callsign.c_str());
    sendMessage(ServerPlayer, t, reply);
    return true;
  }

  DEBUG3("Player has permission to run /poll\n");

  /* make sure that there is a poll arbiter */
  if (BZDB.isEmpty("poll")) {
    sendMessage(ServerPlayer, t, "ERROR: the poll arbiter has disappeared (this should never happen)");
    return true;
  }

  DEBUG3("BZDB poll value is not empty\n");

  // only need to do this once
  static VotingArbiter *arbiter = (VotingArbiter *)BZDB.getPointer("poll");

  DEBUG3("Arbiter was acquired with address 0x%p\n", arbiter);

  /* make sure that there is not a poll active already */
  if (arbiter->knowsPoll()) {
    snprintf(reply, MessageLen, "A poll to %s %s is presently in progress", arbiter->getPollAction().c_str(), arbiter->getPollTarget().c_str());
    sendMessage(ServerPlayer, t, reply);
    sendMessage(ServerPlayer, t, "Unable to start a new poll until the current one is over");
    return true;
  }

  DEBUG3("The arbiter says there is not another poll active\n");

  // get available voter count
  unsigned short int available = 0;
  for (int i = 0; i < curMaxPlayers; i++) {
    // any registered/known users on the server (including observers)
    // are eligible to vote
    GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->accessInfo.exists()) {
      available++;
    }
  }

  DEBUG3("There are %d available players for %d votes required\n", available, clOptions->votesRequired);

  /* make sure there are enough players to even make a poll that has a chance
   * of succeeding (not counting the person being acted upon)
   */
  if (available - 1 < clOptions->votesRequired) {
    sendMessage(ServerPlayer, t, "Unable to initiate a new poll.  There are not enough registered players playing.");
    snprintf(reply, MessageLen, "There needs to be at least %d other %s and only %d %s available.",
	    clOptions->votesRequired,
	    clOptions->votesRequired - 1 == 1 ? "player" : "players",
	    available - 1,
	    available - 1 == 1 ? "is" : "are");
    sendMessage(ServerPlayer, t, reply);
    return true;
  }

  std::string arguments = &message[5]; /* skip "/poll" */
  std::string cmd = "";

  DEBUG3("The arguments string is [%s]\n", arguments.c_str());

  /* find the start of the command */
  size_t startPosition = 0;
  while ((startPosition < arguments.size()) &&
	 (isspace(arguments[startPosition]))) {
    startPosition++;
  }

  DEBUG3("Start position is %d\n", (int)startPosition);

  /* find the end of the command */
  size_t endPosition = startPosition + 1;
  while ((endPosition < arguments.size()) &&
	 (!isspace(arguments[endPosition]))) {
    endPosition++;
  }

  DEBUG3("End position is %d\n", (int)endPosition);

  /* stash the command ('kick', etc) in lowercase to simplify comparison */
  if ((startPosition != arguments.size()) &&
      (endPosition > startPosition)) {
    for (size_t i = startPosition; i < endPosition; i++) {
      cmd += tolower(arguments[i]);
    }
  }

  DEBUG3("Command is %s\n", cmd.c_str());

  /* handle subcommands */

  if ((cmd == "ban") || (cmd == "kick") || (cmd == "kill") || (cmd == "set") || (cmd == "flagreset")) {
    std::string target;
    std::string targetIP = "";

    arguments = arguments.substr(endPosition);

    if (arguments.size() == 0) {
      sendMessage(ServerPlayer, t, "/poll: incorrect syntax, argument required.");
      DEBUG3("No command arguments, stopping poll.\n");
      return true;
    }
    
    DEBUG3("Command arguments are [%s]\n", arguments.c_str());

    /* find the start of the target (e.g. player name) */
    startPosition = 0;
    while ((startPosition < arguments.size()) &&
	   (isspace(arguments[startPosition]))) {
      startPosition++;
    }
    // do not include a starting quote, if given
    if (arguments[startPosition] == '"') {
      startPosition++;
    }

    DEBUG3("Start position for target is %d\n", (int)startPosition);

    /* find the end of the target */
    endPosition = arguments.size() - 1;
    while ((endPosition > 0) &&
	   (isspace(arguments[endPosition]))) {
      endPosition--;
    }
    // do not include a trailing quote, if given
    if (arguments[endPosition] == '"') {
      endPosition--;
    }

    DEBUG3("End position for target is %d\n", (int)endPosition);

    target = arguments.substr(startPosition, endPosition - startPosition + 1);

    DEBUG3("Target specified to vote upon is [%s]\n", target.c_str());

    if ((target.length() == 0) && (cmd != "flagreset")) {
      snprintf(reply, MessageLen, "%s, no target was specified for the [%s] vote", callsign.c_str(), cmd.c_str());
      sendMessage(ServerPlayer, t, reply);
      snprintf(reply, MessageLen, "Usage: /poll %s target", cmd.c_str());
      sendMessage(ServerPlayer, t, reply);
      return true;
    }

    // Make sure the specific poll type is allowed
    if ((cmd == "set") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollSet))) {
      snprintf(reply, MessageLen, "%s, you may not /poll set on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll set\n", callsign.c_str());
      return true;
    }
    if ((cmd == "flagreset") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollFlagReset))) {
      snprintf(reply, MessageLen, "%s, you may not /poll flagreset on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll flagreset\n", callsign.c_str());
      return true;
    }
    if ((cmd == "ban") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollBan))) {
      snprintf(reply, MessageLen, "%s, you may not /poll ban on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll ban\n", callsign.c_str());
      return true;
    }
    if ((cmd == "kick") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKick))) {
      snprintf(reply, MessageLen, "%s, you may not /poll kick on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll kick\n", callsign.c_str());
      return true;
    }

    if ((cmd == "kill") && (!playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKill))) {
      snprintf(reply, MessageLen, "%s, you may not /poll kill on this server", callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      DEBUG3("Player %s is not allowed to /poll kill\n", callsign.c_str());
      return true;
    }

    if ((cmd != "set") && (cmd != "flagreset")) {
      // all polls that are not set or flagreset polls take a player name

      /* make sure the requested player is actually here */
      int v = GameKeeper::Player::getPlayerIDByName(target);
      if (v >= curMaxPlayers) {
	/* wrong name? */
	snprintf(reply, MessageLen, 
		"The player specified for a %s vote is not here", cmd.c_str());
	sendMessage(ServerPlayer, t, reply);
	return true;
      }
      GameKeeper::Player* targetData = GameKeeper::Player::getPlayerByIndex(v);
      if (!targetData) {
	/* wrong name? */
	snprintf(reply, MessageLen, "The server has no information on %s.", cmd.c_str());
	sendMessage(ServerPlayer, t, reply);
	return true;
      }
      targetIP = targetData->netHandler->getTargetIP();

      // admins can override antiperms
      if (!playerData->accessInfo.isAdmin()) {
	// otherwise make sure the player is not protected with an antiperm
	GameKeeper::Player *p = GameKeeper::Player::getPlayerByIndex(v);
	if (p != NULL) {
	  if (p->accessInfo.hasPerm(PlayerAccessInfo::antipoll)) {
	    snprintf(reply, MessageLen, "%s is protected from being polled against.", p->player.getCallSign());
	    sendMessage(ServerPlayer, t, reply);
	    return true;
	  }
	  if (cmd == "ban") {
	    if (p->accessInfo.hasPerm(PlayerAccessInfo::antipollban)) {
	      snprintf(reply, MessageLen, "%s is protected from being poll banned.", p->player.getCallSign());
	      sendMessage(ServerPlayer, t, reply);
	      return true;
	    }
	  } else if (cmd == "kick") {
	    if (p->accessInfo.hasPerm(PlayerAccessInfo::antipollkick)) {
	      snprintf(reply, MessageLen, "%s is protected from being poll kicked.", p->player.getCallSign());
	      sendMessage(ServerPlayer, t, reply);
	      return true;
	    }
	  } else if (cmd == "kill") {
	    if (p->accessInfo.hasPerm(PlayerAccessInfo::antipollkill)) {
	      snprintf(reply, MessageLen, "%s is protected from being poll killed.", p->player.getCallSign());
	      sendMessage(ServerPlayer, t, reply);
	      return true;
	    }
	  }
	}
      } // end admin check

    }

    /* create and announce the new poll */
    bool canDo = false;
    if (cmd == "ban") {
      canDo = (arbiter->pollToBan(target, callsign, targetIP));
    } else if (cmd == "kick") {
      canDo = (arbiter->pollToKick(target, callsign, targetIP));
    } else if (cmd == "kill") {
      canDo = (arbiter->pollToKill(target, callsign, targetIP));
    } else if (cmd == "set") {
      canDo = (arbiter->pollToSet(target, callsign));
    } else if (cmd == "flagreset") {
      canDo = (arbiter->pollToResetFlags(callsign));
    }

    if (!canDo) {
      snprintf(reply, MessageLen, "You are not able to request a %s poll right now, %s", cmd.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, t, reply);
      return true;
    } else {
      snprintf(reply, MessageLen, "A poll to %s %s has been requested by %s", cmd.c_str(), target.c_str(), callsign.c_str());
      sendMessage(ServerPlayer, AllPlayers, reply);
    }

    unsigned int necessaryToSucceed = (unsigned int)((clOptions->votePercentage / 100.0) * (double)available);
    snprintf(reply, MessageLen, "%d player%s available, %d additional affirming vote%s required to pass the poll (%f %%)", available, available==1?" is":"s are", necessaryToSucceed, necessaryToSucceed==1?"":"s", clOptions->votePercentage);
    sendMessage(ServerPlayer, AllPlayers, reply);

    // set the number of available voters
    arbiter->setAvailableVoters(available);

    // keep track of who is allowed to vote
    for (int j = 0; j < curMaxPlayers; j++) {
      // any registered/known users on the server (including
      // observers) are eligible to vote
      GameKeeper::Player *otherData = GameKeeper::Player::getPlayerByIndex(j);
      if (otherData && otherData->accessInfo.exists()) {
	arbiter->grantSuffrage(otherData->player.getCallSign());
      }
    }

    // automatically place a vote for the player requesting the poll
    DEBUG3("Attempting to automatically place a vote for [%s]\n", callsign.c_str());

    bool voted = arbiter->voteYes(callsign);
    if (!voted) {
      sendMessage(ServerPlayer, t, "Unable to automatically place your vote for some unknown reason");
      DEBUG3("Unable to automatically place a vote for [%s]\n", callsign.c_str());
    }

  } else if (cmd == "vote") {
    std::string voteCmd = "/vote ";
    voteCmd += arguments;
    voteCommand(voteCmd.c_str(), playerData);
    return true;

  } else if (cmd == "veto") {
    std::string vetoCmd = "/veto ";
    vetoCmd += arguments;
    vetoCommand(vetoCmd.c_str(), playerData);
    return true;

  } else {
    sendMessage(ServerPlayer, t, "Invalid option to the poll command");
    sendMessage(ServerPlayer, t, "Usage: /poll vote yes|no");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollBan))
      sendMessage(ServerPlayer, t, "    or /poll ban playername");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKick))
      sendMessage(ServerPlayer, t, "    or /poll kick playername");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollKill))
      sendMessage(ServerPlayer, t, "    or /poll kill playername");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollSet))
      sendMessage(ServerPlayer, t, "    or /poll set variable value");
    if (playerData->accessInfo.hasPerm(PlayerAccessInfo::pollFlagReset))
      sendMessage(ServerPlayer, t, "    or /poll flagreset");

  } /* end handling of poll subcommands */

  return true;
}


bool ViewReportCommand::operator() (const char         *,
				    GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  std::string line;
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::viewReports)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the viewreports command");
    return true;
  }
  if (clOptions->reportFile.size() == 0 && clOptions->reportPipe.size() == 0) {
    line = "The /report command is disabled on this server or there are no reports filed.";
    sendMessage(ServerPlayer, t, line.c_str());
  }
  std::ifstream ifs(clOptions->reportFile.c_str(), std::ios::in);
  if (ifs.fail()) {
    sendMessage(ServerPlayer, t, "Error reading from report file.");
    return true;
  }
  while (std::getline(ifs, line))
    sendMessage(ServerPlayer, t, line.c_str());
  return true;
}


bool ClientQueryCommand::operator() (const char         *message,
				     GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  DEBUG2("Clientquery requested by %s [%d]\n",
	 playerData->player.getCallSign(), t);
  if (message[12] != '\0') {
    std::string name = message + 13; // assumes there is a space
    while (isspace(name[0]))
      name.erase(name.begin());
    GameKeeper::Player *target;
    int i;
    for (i = 0; i < curMaxPlayers;i++) {
      target = GameKeeper::Player::getPlayerByIndex(i);
      if (target && strcmp(target->player.getCallSign(), name.c_str()) == 0) {
	sendMessage(i, t, TextUtils::format("Version: %s",
		     target->player.getClientVersion()).c_str());
	return true;
      }
    }
    sendMessage(ServerPlayer, t, "Player not found.");
    return true;
  }
  sendMessage(ServerPlayer, AllPlayers, "[Sent version information per request]");
  // send server's own version string just for kicks
  sendMessage(ServerPlayer, t,
	      TextUtils::format("BZFS Version: %s", getAppVersion()).c_str());
  // send all players' version strings
  // is faking a message from the remote client rude?
  // did that so that /clientquery and CLIENTQUERY look about the same.
  GameKeeper::Player *otherData;
  for (int i = 0; i < curMaxPlayers;i++) {
    otherData = GameKeeper::Player::getPlayerByIndex(i);
    if (otherData && otherData->player.isPlaying()) {
      sendMessage(i, t, TextUtils::format
		  ("Version: %s",
		   otherData->player.getClientVersion()).c_str());
    }
  }
  return true;
}


/** /record command
 *
 *  /record start	       # start buffering
 *  /record stop		# stop buffering (or saving to file)
 *  /record size <Mbytes>       # set the buffer size, and truncate
 *  /record rate <secs>	 # set the state capture rate
 *  /record stats	       # display buffer time and memory information
 *  /record file [filename]     # begin capturing straight to file, flush buffer
 *  /record save [filename]     # save buffer to file (or default filename)
 */
bool RecordCommand::operator() (const char         *message,
				GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  const char *buf = message + 8;
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::record)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /record command");
    return true;
  }
  while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

  if (strncasecmp (buf, "start", 5) == 0) {
    Record::start(t);
  }
  else if (strncasecmp (buf, "stop", 4) == 0) {
    Record::stop(t);
  }
  else if (strncasecmp (buf, "size", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
      return true;
    }
    int size = atoi (buf);
    Record::setSize (t, size);
  }
  else if (strncasecmp (buf, "rate", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
      return true;
    }
    int seconds = atoi (buf);
    Record::setRate (t, seconds);
  }
  else if (strncasecmp (buf, "stats", 5) == 0) {
    Record::sendStats(t);
  }
  else if (strncasecmp (buf, "list", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::sendFileList (t, SortNone);   // stolen from '/replay'
    } else if (strncasecmp (buf, "-t", 2) == 0) {
      Replay::sendFileList (t, SortByTime);  
    } else if (strncasecmp (buf, "-n", 2) == 0) {
      Replay::sendFileList (t, SortByName); 
    } else {
      Record::sendHelp (t);
    }
  }
  else if (strncasecmp (buf, "save", 4) == 0) {
    buf = buf + 4;
    char filename[MessageLen];

    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace
    if (*buf == '\0') {
      Record::sendHelp (t);
    }

    // get the filename
    sscanf (buf, "%s", filename);

    // FIXME - do this a little better? use quotations for strings?
    while ((*buf != '\0') && !isspace (*buf)) buf++; // eat filename
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::saveBuffer (t, filename, 0);
    }
    else {
      Record::saveBuffer (t, filename, atoi(buf));
    }
  }
  else if (strncasecmp (buf, "file", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Record::sendHelp (t);
    }
    else {
      Record::saveFile (t, buf);
    }
  }
  else {
    Record::sendHelp (t);
  }

  return true;
}


/** /replay command
 *
 *  /replay list		# list available replay files
 *  /replay load [filename]     # set the replay file (or load the default)
 *  /replay play		# began playing
 *  /replay loop		# began playing in looped mode
 *  /replay stats		# report the current replay state
 *  /replay skip <secs>	 # fast foward or rewind in time
 */
bool ReplayCommand::operator() (const char         *message,
				GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  const char *buf = message + 7;
  while ((*buf != '\0') && isspace (*buf)) { // eat whitespace
    buf++;
  }
  
  // everyone can use the replay stats command
  if (strncasecmp (buf, "stats", 4) == 0) {
    Replay::sendStats (t);
    return true;
  }

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::replay)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /replay command");
    return true;
  }

  if (strncasecmp (buf, "list", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::sendFileList (t, SortNone); 
    } else if (strncasecmp (buf, "-t", 2) == 0) {
      Replay::sendFileList (t, SortByTime);  
    } else if (strncasecmp (buf, "-n", 2) == 0) {
      Replay::sendFileList (t, SortByName); 
    } else {
      Replay::sendHelp (t);
    }
  }
  else if (strncasecmp (buf, "load", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::sendHelp (t);
    }
    else {
      Replay::loadFile (t, buf);
    }
  }
  else if (strncasecmp (buf, "play", 4) == 0) {
    Replay::play (t);
  }
  else if (strncasecmp (buf, "loop", 4) == 0) {
    Replay::loop (t);
  }
  else if (strncasecmp (buf, "skip", 4) == 0) {
    buf = buf + 4;
    while ((*buf != '\0') && isspace (*buf)) buf++; // eat whitespace

    if (*buf == '\0') {
      Replay::skip (t, 0);
    }
    else {
      int skip = atoi (buf);
      Replay::skip (t, skip);
    }
  }
  else if (strncasecmp (buf, "pause", 5) == 0) {
    Replay::pause (t);
  }
  else {
    Replay::sendHelp (t);
  }

  return true;
}


bool SayCommand::operator() (const char         *message,
			     GameKeeper::Player *playerData)
{
  size_t messageStart = 0;
  int t = playerData->getIndex();

  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::say)) {
    char reply[MessageLen] = {0};
    snprintf(reply, MessageLen, "%s, you do not have permission to run the /say command", playerData->player.getCallSign());
    sendMessage(ServerPlayer, t, reply);
    return true;
  }

  std::string messageText = &message[4];

  // skip any leading whitespace
  while ((messageStart < messageText.size()) &&
	 (isspace(messageText[messageStart]))) {
    messageStart++;
  }

  // make sure there was _some_ whitespace after /say
  if (messageStart == 0) {
    sendMessage(ServerPlayer, t, "Usage: /say some message");
    return true;
  }

  // no anonymous messages
  messageText += " (";
  messageText += playerData->player.getCallSign();
  messageText += ")";

  // send the message
  sendMessage(ServerPlayer, AllPlayers, messageText.c_str() + messageStart );
  return true;
}



bool DateTimeCommand::operator() (const char         *,
				  GameKeeper::Player *playerData)
{
  int t = playerData->getIndex();
  if (!playerData->accessInfo.hasPerm(PlayerAccessInfo::date)) {
    sendMessage(ServerPlayer, t, "You do not have permission to run the /date command");
    return true;
  }
  time_t now = time(NULL);
  char* timeStr = ctime(&now);
  timeStr[24] = '\0';
  sendMessage(ServerPlayer, t, timeStr);
  return true;
}


// parse server comands
void parseServerCommand(const char *message, int t)
{
  if (!message) {
    std::cerr << "WARNING: parseCommand was given a null message?!" << std::endl;
    return;
  }

  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(t);
  if (!playerData)
    return;

  if (ServerCommand::execute(message, playerData))
    return;

  if (cmdHelp(message, playerData))
    return;

  {
    // lets see if it is a custom command
    std::vector<std::string> params =
      TextUtils::tokenize(std::string(message+1),std::string(" "));

    if (params.size() == 0)
      return;
    
    tmCustomSlashCommandMap::iterator itr =
      customCommands.find(TextUtils::tolower(params[0]));
  
    bzApiString	command = params[0];
    bzApiString APIMessage;
	bzAPIStringList	APIParams;
	
    for ( unsigned int i = 1; i < params.size(); i++)
      APIParams.push_back(params[i]);

    if ( strlen(message+1) > params[0].size())
      APIMessage = (message+params[0].size()+2);

    // see if we have a registerd custom command and call it
    if (itr != customCommands.end()) {
      // if it handles it, then we are good
      if (itr->second->handle(t, command, APIMessage, &APIParams))
        return;
    }
  
    // lets see if anyone wants to handle the unhandled event
    bz_UnknownSlashCommandEventData commandData;
    commandData.from = t;
    commandData.message = message;
    commandData.time = TimeKeeper::getCurrent().getSeconds();
  
    worldEventManager.callEvents(bz_eUnknownSlashCommand, &commandData);
    if (commandData.handled) // did anyone do it?
      return;

    char reply[MessageLen];
    snprintf(reply, MessageLen, "Unknown command [%s]", message + 1);
    sendMessage(ServerPlayer, t, reply);
  }
}

void registerCustomSlashCommand(std::string command, bz_CustomSlashCommandHandler* handler)
{
  if (handler)
    customCommands[TextUtils::tolower(command)] = handler;
}

void removeCustomSlashCommand(std::string command)
{
  tmCustomSlashCommandMap::iterator itr = customCommands.find(TextUtils::tolower(command));
  if (itr != customCommands.end())
    customCommands.erase(itr);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
