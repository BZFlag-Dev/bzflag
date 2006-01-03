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

#ifndef __COMMANDSSTANDARD_H__
#define __COMMANDSSTANDARD_H__

/* common interface headers */
#include "CommandManager.h"


struct CommandsItem {
public:
  const char* name;
  CommandManager::CommandFunction func;
  const char* help;
};
extern const struct CommandsItem commands[];


/** standard commands
 */
class CommandsStandard {
public:
  static void		add();
  static void		remove();
  static void		quit();
  static bool		isQuit();
};


#endif /* __COMMANDSSTANDARD_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
