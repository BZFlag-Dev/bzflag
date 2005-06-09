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

// local implementation headers
#include "LocalCommand.h"
#include "Player.h"
#include "Roster.h"
#include "playing.h"
#include "World.h"

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

class SaveWorldCommand : LocalCommand {
public:
  SaveWorldCommand();

  virtual bool operator() (const char *commandLine);
};

static SilenceCommand     silenceCommand;
static UnsilenceCommand   unsilenceCommand;
static DumpCommand        dumpCommand;
static ClientQueryCommand clientQueryCommand;
static SaveWorldCommand   saveWorldCommand;

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

bool ClientQueryCommand::operator() (const char *)
{
  char messageBuffer[MessageLen];
  memset(messageBuffer, 0, MessageLen);
  strncpy(messageBuffer, "/clientquery", MessageLen);
  nboPackString(messageMessage + PlayerIdPLen, messageBuffer, MessageLen);
  serverLink->send(MsgMessage, sizeof(messageMessage), messageMessage);

  return true;
}

SaveWorldCommand::SaveWorldCommand() : LocalCommand("SAVEWORLD")
{
}

bool SaveWorldCommand::operator() (const char *commandLine)
{
  std::string path = commandLine + 10;
  if (World::getWorld()->writeWorld(path)) {
    addMessage(NULL, "World Saved");
  } else {
    addMessage(NULL, "Invalid file name specified");
  }
  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
