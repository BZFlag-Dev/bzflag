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
#include "ServerCommand.h"

ServerCommand::MapOfCommands *ServerCommand::mapOfCommands = NULL;

// Use only lower case command name
ServerCommand::ServerCommand(std::string _commandName)
  : commandName(_commandName)
{
  if (!mapOfCommands)
    mapOfCommands = new MapOfCommands;
  (*mapOfCommands)[commandName] = this;
}

ServerCommand::~ServerCommand()
{
  (*mapOfCommands).erase(commandName);
}

bool ServerCommand::execute(const char         *commandLine,
			    GameKeeper::Player *playerData)
{
  if (!mapOfCommands)
    return false;
  int i;
  char commandName[256];
  for (i = 0; commandLine[i] && !isspace(commandLine[i]); i++)
    commandName[i] = tolower(commandLine[i]);
  commandName[i] = 0;
  
  std::map<std::string, ServerCommand *>::iterator it
    = (*mapOfCommands).find(commandName);
  if (it == (*mapOfCommands).end())
    return false;
  return (*(it->second))(commandLine, playerData);
}

bool ServerCommand::operator() (const char *, GameKeeper::Player *)
{
  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
