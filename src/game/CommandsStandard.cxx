/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CommandsStandard.h"

/* system implementation headers */
#include <stdio.h>
#include <ctype.h>
#include <string>

/* common implementation headers */
#include "CommandManager.h"
#include "StateDatabase.h"
#include "KeyManager.h"
#include "TextUtils.h"

static bool			quitFlag = false;

//
// command handlers
//

static std::string		cmdQuit(const std::string&,
					const CommandManager::ArgList&)
{
  CommandsStandard::quit();
  return std::string();
}

static void			onHelpCB(const std::string& name,
					 void* userData)
{
  std::string& result = *reinterpret_cast<std::string*>(userData);
  result += name;
  result += "\n";
}

static std::string		cmdHelp(const std::string&,
					const CommandManager::ArgList& args)
{
  switch (args.size()) {
    case 0: {
      std::string result;
      CMDMGR.iterate(&onHelpCB, &result);
      return result;
    }

    case 1:
      return CMDMGR.getHelp(args[0]);

    default:
      return "usage: help [<command-name>]";
  }
}

static std::string		cmdPrint(const std::string&,
					 const CommandManager::ArgList& args)
{
  // merge all arguments into one string
  std::string arg;
  const unsigned int n = (int)args.size();
  if (n > 0)
    arg = args[0];
  for (unsigned int i = 1; i < n; ++i) {
    arg += " ";
    arg += args[i];
  }

  // interpolate variables
  std::string result;
  const char* scan = arg.c_str();
  while (*scan != '\0') {
    if (*scan != '$') {
      result.append(scan, 1);
      ++scan;
    } else {
      // could be a variable
      ++scan;
      if (*scan == '$') {
	// no, it's just $$ which maps to $
	result += "$";
	++scan;
      } else if (*scan != '{') {
	// parse variable name
	const char* name = scan;
	while (*scan != '\0' && !isspace(*scan))
	  ++scan;

	// look up variable
	result += BZDB.get(std::string(name, scan - name));
      } else {
	// parse "quoted" variable name
	const char* name = ++scan;
	while (*scan != '\0' && *scan != '}')
	  ++scan;

	if (*scan != '\0') {
	  // look up variable
	  result += BZDB.get(std::string(name, scan - name));

	  // skip }
	  ++scan;
	}
      }
    }
  }

  std::cout << "printing \"" << result << "\"" << std::endl;
  return result;
}

static void			onSetCB(const std::string& name,
					void* userData)
{
  // don't show names starting with _
  std::string& result = *reinterpret_cast<std::string*>(userData);
  if (!name.empty() && name.c_str()[0] != '_') {
    result += name;
    result += " = ";
    result += BZDB.get(name);
    result += "\n";
  }
}

static std::string		cmdSet(const std::string&,
				       const CommandManager::ArgList& args)
{
  switch (args.size()) {

    case 0: 
      {
	// print out all values that are set
	std::string result;
	BZDB.iterate(&onSetCB, &result);
	return result;
      }
    case 1:
      {
	// the string was set to nothing, so just print value
        if (BZDB.isSet(args[0])) {
	  return args[0] + " is " + BZDB.get(args[0]);
	} else {
	  return "variable " + args[0] + " does not exist";
	}
      }
    case 2:
      {
	// set variable to value
	BZDB.set(args[0], args[1], StateDatabase::User);
	return std::string();
      }
    default:
      {
	return "usage: set <name> [<value>]";
      }
  }
}

static std::string		cmdUnset(const std::string&,
					 const CommandManager::ArgList& args)
{
  if (args.size() != 1)
    return "usage: unset <name>";
  BZDB.unset(args[0], StateDatabase::User);
  return std::string();
}


static void			onBindCB(const std::string& name, bool press,
					 const std::string& cmd, void* userData)
{
  std::string& result = *reinterpret_cast<std::string*>(userData);
  result += name;
  result += (press ? " down " : " up ");
  result += cmd;
  result += "\n";
}

static std::string		cmdBind(const std::string&,
					const CommandManager::ArgList& args)
{
  if (args.size() == 0) {
    std::string result;
    KEYMGR.iterate(&onBindCB, &result);
    return result;
  } else if (args.size() < 3) {
    return "usage: bind <button-name> {up|down} <command> <args>...";
  }

  BzfKeyEvent key;
  if (!KEYMGR.stringToKeyEvent(args[0], key))
    return std::string("bind error: unknown button name \"") + args[0] + "\"";

  bool down;
  if (args[1] == "up")
    down = false;
  else if (args[1] == "down")
    down = true;
  else
    return std::string("bind error: illegal state \"") + args[1] + "\"";

  std::string cmd = args[2];
  for (unsigned int i = 3; i < args.size(); ++i) {
    cmd += " ";
    cmd += args[i];
  }

  // ignore attempts to modify Esc.  we reserve that for the menu
  if (key.ascii != 27)
    KEYMGR.bind(key, down, cmd);

  return std::string();
}

static std::string		cmdUnbind(const std::string&,
					  const CommandManager::ArgList& args)
{
  if (args.size() != 2)
    return "usage: unbind <button-name> {up|down}";

  BzfKeyEvent key_event;
  if (!KEYMGR.stringToKeyEvent(args[0], key_event))
    return std::string("bind error: unknown button name \"") + args[0] + "\"";

  bool down;
  if (args[1] == "up")
    down = false;
  else if (args[1] == "down")
    down = true;
  else
    return std::string("bind error: illegal state \"") + args[1] + "\"";

  if (key_event.ascii != 27)
    KEYMGR.unbind(key_event, down);

  return std::string();
}

static std::string		cmdToggle(const std::string&,
					  const CommandManager::ArgList& args)
{
  if (args.size() != 1)
    return "usage: toggle <name>";
  const std::string& name = args[0];
  if (BZDB.isTrue(name))
    BZDB.set(name, "0", StateDatabase::User);
  else
    BZDB.set(name, "1", StateDatabase::User);
  return std::string();
}

static std::string cmdMult(const std::string&, const CommandManager::ArgList& args)
{
  if (args.size() != 2)
    return "usage: mult <name> <value>";
  float value;
  if (sscanf(BZDB.get(args[0]).c_str(), "%f", &value) != 1)
    value = 0.0;
  float amount;
  if (sscanf(args[1].c_str(), "%f", &amount) != 1)
    amount = 1.0;
  value *= amount;
  BZDB.set(args[0], string_util::format("%f", value), StateDatabase::User);
  return std::string();
}

//
// command name to function mapping
//

const struct CommandsItem commands[] = {
  { "quit",	&cmdQuit,	"quit:  quit the program" },
  { "help",	&cmdHelp,	"help [<command-name>]:  get help on a command or a list of commands" },
  { "print",	&cmdPrint,	"print ...:  print arguments; $name is replaced by value of variable \"name\"" },
  { "set",	&cmdSet,	"set [<name> <value>]:  set a variable or print all set variables" },
  { "unset",	&cmdUnset,	"unset <name>:  unset a variable" },
  { "bind",	&cmdBind,	"bind <button-name> {up|down} <command> <args>...: bind a key" },
  { "unbind",	&cmdUnbind,	"unbind <button-name> {up|down}:  unbind a key" },
  { "toggle",	&cmdToggle,	"toggle <name>:  toggle truth value of a variable" },
  { "mult",	&cmdMult,	"mult <name> <value>:  multiply a variable by an amount" }
};
// FIXME -- may want a cmd to cycle through a list


//
// CommandsStandard
//

void				CommandsStandard::add()
{
  unsigned int i;
  for (i = 0; i < countof(commands); ++i)
    CMDMGR.add(commands[i].name, commands[i].func, commands[i].help);
}

void				CommandsStandard::remove()
{
  unsigned int i;
  for (i = 0; i < countof(commands); ++i)
    CMDMGR.remove(commands[i].name);
}

void				CommandsStandard::quit()
{
  quitFlag = true;
}

bool				CommandsStandard::isQuit()
{
  return quitFlag;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

