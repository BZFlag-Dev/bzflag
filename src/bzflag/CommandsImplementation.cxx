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

// common implementation headers
#include "BZDBCache.h"
#include "AnsiCodes.h"
#include "TextUtils.h"
#include "CommandsStandard.h"

// local implementation headers
#include "LocalCommand.h"
#include "Player.h"
#include "Roster.h"
#include "playing.h"

class SilenceCommand : LocalCommand {
public:
  SilenceCommand();

  virtual bool operator() (const char *commandLine);
};

class UnsilenceCommand : LocalCommand {
public:
  UnsilenceCommand();

  virtual bool operator() (const char *commandLine);
};

class DumpCommand : LocalCommand {
public:
  DumpCommand();

  virtual bool operator() (const char *commandLine);
};

class ClientQueryCommand : LocalCommand {
public:
  ClientQueryCommand();

  virtual bool operator() (const char *commandLine);
};

class HighlightCommand : LocalCommand {
public:
  HighlightCommand();

  virtual bool operator() (const char *commandLine);
};

class SetCommand : LocalCommand {
public:
  SetCommand();

  virtual bool operator() (const char *commandLine);
};

class DiffCommand : LocalCommand {
public:
  DiffCommand();

  virtual bool operator() (const char *commandLine);
};

#ifdef DEBUG
class LocalSetCommand : LocalCommand {
public:
  LocalSetCommand();

  virtual bool operator() (const char *commandLine);
};
#endif

class QuitCommand : LocalCommand {
public:
  QuitCommand();

  virtual bool operator() (const char *commandLine);
};

static SilenceCommand     silenceCommand;
static UnsilenceCommand   unsilenceCommand;
static DumpCommand        dumpCommand;
static ClientQueryCommand clientQueryCommand;
static HighlightCommand   highlightCommand;
static SetCommand         setCommand;
static DiffCommand        diffCommand;
#ifdef DEBUG
static LocalSetCommand    localSetCommand;
#endif
static QuitCommand        quitCommand;

SilenceCommand::SilenceCommand() : LocalCommand("SILENCE")
{
}

bool SilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 8);
  if (loudmouth) {
    silencePlayers.push_back(commandLine + 8);
    std::string silenceMessage = "Silenced ";
    silenceMessage += (commandLine + 8);
    addMessage(NULL, silenceMessage);
  }
  return true;
}

UnsilenceCommand::UnsilenceCommand() : LocalCommand("UNSILENCE")
{
}

bool UnsilenceCommand::operator() (const char *commandLine)
{
  Player *loudmouth = getPlayerByName(commandLine + 10);
  if (loudmouth) {
    std::vector<std::string>::iterator it = silencePlayers.begin();
    for (; it != silencePlayers.end(); it++) {
      if (*it == commandLine + 10) {
	silencePlayers.erase(it);
	std::string unsilenceMessage = "Unsilenced ";
	unsilenceMessage += (commandLine + 10);
	addMessage(NULL, unsilenceMessage);
	break;
      }
    }
  }
  return true;
}

void printout(const std::string& name, void*)
{
  std::cout << name << " = " << BZDB.get(name) << std::endl;
}

DumpCommand::DumpCommand() : LocalCommand("DUMP")
{
}

bool DumpCommand::operator() (const char *)
{
  BZDB.iterate(printout, NULL);
  return true;
}

ClientQueryCommand::ClientQueryCommand() : LocalCommand("CLIENTQUERY")
{
}

bool ClientQueryCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 11)
    return false;
  char messageBuffer[MessageLen];
  memset(messageBuffer, 0, MessageLen);
  strncpy(messageBuffer, "/clientquery", MessageLen);
  nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
  serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);

  return true;
}

HighlightCommand::HighlightCommand() : LocalCommand("/highlight")
{
}

bool HighlightCommand::operator() (const char *commandLine)
{
  const char* c = commandLine + 10;
  while ((*c != '\0') && isspace(*c)) c++; // skip leading white
  BZDB.set("highlightPattern", std::string(c));
  return true;
}

static bool foundVarDiff = false;

static bool varIsEqual(const std::string& name)
{
  // avoid "poll"
  if (name[0] != '_') {
    return true;
  }

  // get the parameters
  const std::string exp = BZDB.get(name);
  const std::string defexp = BZDB.getDefault(name);
  const float val = BZDB.eval(name);
  BZDB.set("tmp", defexp); // BZDB.eval() can't take expressions directly
  BZDB.setPersistent("tmp", false);
  const float defval = BZDB.eval("tmp");
  const bool valNaN = !(val == val);
  const bool defNaN = !(defval == defval);

  if (valNaN != defNaN) {
    return false;
  }

  if (valNaN) {
    return (exp == defexp);
  } else {
    return (val == defval);
  }
}

static void listSetVars(const std::string& name, void* boolPtr)
{
  bool& diff = *((bool*)boolPtr);

  if (diff) {
    if (varIsEqual(name)) {
      return;
    } else {
      foundVarDiff = true;
    }
  }

  char message[MessageLen];
  if (BZDB.getPermission(name) == StateDatabase::Locked) {
    if (BZDBCache::colorful) {
      sprintf(message, "/set %s%s %s%f %s%s",
	      ColorStrings[RedColor].c_str(), name.c_str(),
	      ColorStrings[GreenColor].c_str(), BZDB.eval(name),
	      ColorStrings[BlueColor].c_str(), BZDB.get(name).c_str());
    } else {
      sprintf(message, "/set %s <%f> %s", name.c_str(),
	      BZDB.eval(name), BZDB.get(name).c_str());
    }
    addMessage(LocalPlayer::getMyTank(), message, 2);
  }
}


SetCommand::SetCommand() : LocalCommand("/set")
{
}

bool SetCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 4)
    return false;
  bool diff = false;
  BZDB.iterate(listSetVars, &diff);
  return true;
}

DiffCommand::DiffCommand() : LocalCommand("/diff")
{
}

bool DiffCommand::operator() (const char *commandLine)
{
  if (strlen(commandLine) != 5)
    return false;
  bool diff = true;
  foundVarDiff = false;
  BZDB.iterate(listSetVars, &diff);
  if (!foundVarDiff) {
    addMessage(LocalPlayer::getMyTank(), "all variables are at defaults", 2);
  }
  return true;
}

LocalSetCommand::LocalSetCommand() : LocalCommand("/localset")
{
}

bool LocalSetCommand::operator() (const char *commandLine)
{
  std::string params              = commandLine + 9;
  std::vector<std::string> tokens = TextUtils::tokenize(params, " ", 2);
  if (tokens.size() == 2) {
    if (!(BZDB.getPermission(tokens[0]) == StateDatabase::Server)) {
      BZDB.setPersistent(tokens[0], BZDB.isPersistent(tokens[0]));
      BZDB.set(tokens[0], tokens[1]);
      std::string msg = "/localset " + tokens[0] + " " + tokens[1];
      addMessage(NULL, msg);
    } else {
      addMessage
	(NULL,
	 "This is a server-defined variable.  Use /set instead of /localset.");
    }
  } else {
    addMessage(NULL, "usage: /localset <variable> <value>");
  }
  return true;
}

QuitCommand::QuitCommand() : LocalCommand("/quit")
{
}

bool QuitCommand::operator() (const char *commandLine)
{
  char messageBuffer[MessageLen]; // send message
  memset(messageBuffer, 0, MessageLen);
  strncpy(messageBuffer, commandLine, MessageLen);
  nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
  serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);
  CommandsStandard::quit(); // kill client
  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
