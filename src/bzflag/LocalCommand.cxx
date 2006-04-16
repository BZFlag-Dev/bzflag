/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#include "LocalCommand.h"

// system headers
#include <ctype.h>


LocalCommand::MapOfCommands *LocalCommand::mapOfCommands = NULL;


LocalCommand::LocalCommand(std::string _commandName)
  : commandName(_commandName)
{
  if (!mapOfCommands)
    mapOfCommands = new MapOfCommands;
  (*mapOfCommands)[commandName] = this;
}


LocalCommand::~LocalCommand()
{
  (*mapOfCommands).erase(commandName);
}


bool LocalCommand::execute(const char *commandLine)
{
  if (!mapOfCommands)
    return false;
  int i;
  for (i = 0; commandLine[i] && !isspace(commandLine[i]); i++);
  std::string commandToken(commandLine, i);

  std::map<std::string, LocalCommand *>::iterator it
    = (*mapOfCommands).find(commandToken);
  if (it == (*mapOfCommands).end())
    return false;
  return (*(it->second))(commandLine);
}


bool LocalCommand::operator() (const char *)
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
