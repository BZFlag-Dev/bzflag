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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

// common - 1st
#include "common.h"

/* common interface headers */
#include "CommandManager.h"


typedef struct CommandListItem {
  const char* name;
  CommandManager::CommandFunction func;
  const char* help;
} CmdListItem;

/* FIXME -- gcc is not liking array type with sizeof() without size
 * for some reason */
#ifdef SNAPPING
extern const CmdListItem commandList[26];
#else
extern const CmdListItem commandList[25];
#endif

#endif /* __COMMANDS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
