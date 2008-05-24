/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

/* interface header */
#include "ServerCommand.h"

/* system headers */
#include <string>
#include <map>

/* common headers */
#include "bzfsAPI.h"
#include "GameKeeper.h"
#include "TextUtils.h"


// command classes used by the API
class ShutdownCommand : private ServerCommand
{
public:
  ShutdownCommand();

  virtual bool operator() (const char	 *commandLine, GameKeeper::Player *playerData);
};

class SuperkillCommand : private ServerCommand
{
public:
  SuperkillCommand();

  virtual bool operator() (const char	 *commandLine, GameKeeper::Player *playerData);
};

class ReportCommand : private ServerCommand
{
public:
  ReportCommand();
  
  virtual bool operator() (const char *commandLine, GameKeeper::Player *playerData);
};


// parser for the server commands
void parseServerCommand(const char *message, int dstPlayerId);

typedef std::map<std::string, bz_CustomSlashCommandHandler*>	tmCustomSlashCommandMap;

void registerCustomSlashCommand ( std::string command, bz_CustomSlashCommandHandler* handler );
void removeCustomSlashCommand ( std::string command );

extern ShutdownCommand shutdownCommand;
extern SuperkillCommand superkillCommand;
extern ReportCommand reportCommand;

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
