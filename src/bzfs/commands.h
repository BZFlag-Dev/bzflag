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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

// implementation-specific bzflag headers
#include "GameKeeper.h"

#include "TextUtils.h"
#include <string>
#include <map>
#include "bzfsAPI.h"
#include "ServerCommand.h"


// command classes used by the API
class ShutdownCommand : ServerCommand
{
public:
    ShutdownCommand();

    virtual bool operator() (const char  *commandLine,
                             GameKeeper::Player *playerData);
};

class SuperkillCommand : ServerCommand
{
public:
    SuperkillCommand();

    virtual bool operator() (const char  *commandLine,
                             GameKeeper::Player *playerData);
};

// parser for the server commands
void parseServerCommand(const char *message, int dstPlayerId, int sourceChannel);

typedef std::map<std::string, bz_CustomSlashCommandHandlerV2*>  tmCustomSlashCommandMap;

void registerCustomSlashCommand ( std::string command, bz_CustomSlashCommandHandlerV2* handler );
void removeCustomSlashCommand ( std::string command );

extern ShutdownCommand shutdownCommand;
extern SuperkillCommand superkillCommand;

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
