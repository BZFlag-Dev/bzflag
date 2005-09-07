/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __SERVERCOMMAND_H__
#define __SERVERCOMMAND_H__

// bzflag global header
#include "common.h"

/* system headers */
#include <string>
#include <map>

// local implementation headers
#include "GameKeeper.h"

class ServerCommand {
 public:

  static bool execute(const char         *commandToken,
		      GameKeeper::Player *playerData);

  std::string getHelp();

  virtual bool operator () (const char         *commandLine,
			    GameKeeper::Player *playerData);


 protected:

  ServerCommand(std::string _commandName, std::string _oneLineHelp = "");
  virtual ~ServerCommand();

  std::string commandName;
  std::string oneLineHelp;

  typedef std::map<std::string, ServerCommand *> MapOfCommands;

  static MapOfCommands *getMapRef();
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
