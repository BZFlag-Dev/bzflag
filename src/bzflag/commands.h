/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
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

#include "common.h"

/* common headers */
#include "CommandManager.h"

// system headers
#include <vector>


struct CommandListItem {
  CommandListItem(const char* n,
                  CommandManager::CommandFunction f,
                  const char* h)
    : name(n)
    , func(f)
    , help(h)
  {}
  const char* name;
  CommandManager::CommandFunction  func;
  const char* help;
};


extern const std::vector<CommandListItem>& getCommandList();


#endif /* __COMMANDS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
