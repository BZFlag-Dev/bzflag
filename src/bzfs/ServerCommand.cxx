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

// Use only lower case command name
ServerCommand::ServerCommand(std::string _commandName,
			     std::string _oneLineHelp)
  : commandName(_commandName), oneLineHelp(_oneLineHelp)
{
  (*getMapRef())[commandName] = this;
  if (oneLineHelp == "")
    oneLineHelp = commandName;
}

ServerCommand::~ServerCommand()
{
  (*getMapRef()).erase(commandName);
}

bool ServerCommand::execute(const char         *commandLine,
			    GameKeeper::Player *playerData)
{
  MapOfCommands &commandMap = *getMapRef();
  int i;
  for (i = 0; commandLine[i] && !isspace(commandLine[i]); i++);
  std::string commandToken(commandLine, i);
  
  MapOfCommands::iterator it
    = commandMap.find(TextUtils::tolower(commandToken));
  if (it == commandMap.end())
    return false;
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
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
