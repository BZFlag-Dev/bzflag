/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "ServerCommand.h"

// system headers
#include <ctype.h>

ServerCommand::MapOfCommands *ServerCommand::mapOfCommands = NULL;

// Use only lower case command name
ServerCommand::ServerCommand(std::string _commandName,
                             std::string _oneLineHelp)
    : commandName(_commandName)
{
    if (!mapOfCommands)
        mapOfCommands = new MapOfCommands;
    (*mapOfCommands)[commandName] = this;
    oneLineHelp = commandName;
    if (_oneLineHelp != "")
        oneLineHelp += " " + _oneLineHelp;
}

ServerCommand::~ServerCommand()
{
    if (mapOfCommands)
    {
        mapOfCommands->erase(commandName);
        if (mapOfCommands->empty())
        {
            delete mapOfCommands;
            mapOfCommands = NULL;
        }
    }
}

bool ServerCommand::execute(const char   *commandLine,
                            GameKeeper::Player *playerData)
{
    if (!mapOfCommands)
        return false;
    int i;
    for (i = 0; commandLine[i] && !isspace(commandLine[i]); i++)
        ;
    std::string commandToken(commandLine, i);

    MapOfCommands::iterator it
        = mapOfCommands->find(TextUtils::tolower(commandToken));
    if (it == mapOfCommands->end())
        return false;
    return (*(it->second))(commandLine, playerData);
}

bool ServerCommand::operator() (const char *, GameKeeper::Player *)
{
    return true;
}

std::string ServerCommand::getHelp()
{
    return oneLineHelp;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
