/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "common.h"

/* system interface headers */
#include <string>

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
extern const CmdListItem commandList[17];
#else
extern const CmdListItem commandList[16];
#endif

/** jump
 */
std::string cmdJump(const std::string&, const CommandManager::ArgList& args);

/** fire weapon
 */
std::string cmdFire(const std::string&, const CommandManager::ArgList& args);

/** drop a flag
 */
std::string cmdDrop(const std::string&, const CommandManager::ArgList& args);

/** identify to a server
 */
std::string cmdIdentify(const std::string&, const CommandManager::ArgList& args);

/** restart/respawn
 */
std::string cmdRestart(const std::string&, const CommandManager::ArgList& args);

/** self-destruct
 */
std::string cmdDestruct(const std::string&, const CommandManager::ArgList& args);

/** pause
 */
std::string cmdPause(const std::string&, const CommandManager::ArgList& args);

/** toggle auto-pilot
 */
std::string cmdAutoPilot(const std::string&, const CommandManager::ArgList& args);

/** send
 */
std::string cmdSend(const std::string&, const CommandManager::ArgList& args);

#ifdef SNAPPING
/** capture a screenshot
 */
std::string cmdScreenshot(const std::string&, const CommandManager::ArgList& args);
#endif

/** time
 */
std::string cmdTime(const std::string&, const CommandManager::ArgList& args);

/** roam
 */
std::string cmdRoam(const std::string&, const CommandManager::ArgList& args);

/** silence another player
 */
std::string cmdSilence(const std::string&, const CommandManager::ArgList& args);

/** perform a server command
 */
std::string cmdServerCommand(const std::string&, const CommandManager::ArgList& args);

/** scroll the chat panel
 */
std::string cmdScrollPanel(const std::string&, const CommandManager::ArgList& args);

/** hunt another player
 */
std::string cmdHunt(const std::string&, const CommandManager::ArgList& args);

/** iconify window
 */
std::string cmdIconify(const std::string&,
		       const CommandManager::ArgList& args);


#endif /* __COMMANDS_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
