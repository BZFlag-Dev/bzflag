/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __LOCALCOMMAND_H__
#define __LOCALCOMMAND_H__

// bzflag global header
#include "common.h"

/* system headers */
#include <string>
#include <map>

class LocalCommand
{
public:

    static bool execute(const char *commandToken);

protected:

    LocalCommand(std::string _commandName);
    virtual ~LocalCommand();

    virtual bool operator() (const char *commandToken);

    std::string commandName;

    typedef std::map<std::string, LocalCommand *> MapOfCommands;

    static MapOfCommands *mapOfCommands;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
