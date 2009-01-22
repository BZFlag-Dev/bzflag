/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
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
#include "ServerCommand.h"

// system headers
#include <ctype.h>

// Use only lower case command name
ServerCommand::ServerCommand(std::string _commandName,
			     std::string _oneLineHelp)
  : commandName(_commandName)
{
  (*getMapRef())[commandName] = this;
  oneLineHelp = commandName;
  if (_oneLineHelp != "")
    oneLineHelp += " " + _oneLineHelp;
}

ServerCommand::~ServerCommand()
{
  (*getMapRef()).erase(commandName);
}

bool ServerCommand::execute(const char	 *commandLine,
			    GameKeeper::Player *playerData)
{
  MapOfCommands &commandMap = *getMapRef();
  int i = TextUtils::firstNonvisible(commandLine);
  if (i < 0) i = strlen(commandLine);
  std::string commandToken(commandLine, i);

  MapOfCommands::iterator it
    = commandMap.find(TextUtils::tolower(commandToken));
  if (it == commandMap.end())
    return false;

  /* run the actual command */
  return (*(it->second))(commandLine, playerData);
}

bool ServerCommand::operator() (const char *, GameKeeper::Player *)
{
  return true;
}

ServerCommand::MapOfCommands *ServerCommand::getMapRef()
{
  static MapOfCommands mapOfCommands;

  return &mapOfCommands;
}

std::string ServerCommand::getHelp()
{
  return oneLineHelp;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
