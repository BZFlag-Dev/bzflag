/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CommandsStandard.h"

/* system implementation headers */
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string>
using std::string;

/* common implementation headers */
#include "CommandManager.h"
#include "StateDatabase.h"
#include "KeyManager.h"
#include "TextUtils.h"

typedef CommandManager::ArgList CmdArgList;


static bool quitFlag = false;


bool isValidKey(const string& key)
{
  if (!key.size()) {
    return false;
  }
#ifdef _WIN32
  if (key == "alt+f4") {
    return false;
  }
#endif
  return true;
}


//
// command handlers
//

static string cmdQuit(const string&, const CmdArgList&, bool*)
{
  CommandsStandard::quit();
  return string();
}


static void onHelpCB(const string& name, void* userData)
{
  string& result = *static_cast<string*>(userData);
  result += name;
  result += "\n";
}


static string cmdHelp(const string&, const CmdArgList& args, bool*)
{
  switch (args.size()) {
    case 0: {
      string result;
      CMDMGR.iterate(&onHelpCB, &result);
      return result;
    }
    case 1: {
      return CMDMGR.getHelp(args[0]);
    }
    default: {
      return "usage: help [<command-name>]";
    }
  }
}


static string cmdPrint(const string&, const CmdArgList& args, bool*)
{
  // merge all arguments into one string
  string arg;
  const unsigned int n = (int)args.size();
  if (n > 0)
    arg = args[0];
  for (unsigned int i = 1; i < n; ++i) {
    arg += " ";
    arg += args[i];
  }

  // interpolate variables
  string result;
  const char* scan = arg.c_str();
  while (*scan != '\0') {
    if (*scan != '$') {
      result.append(scan, 1);
      ++scan;
    }
    else {
      // could be a variable
      ++scan;
      if (*scan == '$') {
	// no, it's just $$ which maps to $
	result += "$";
	++scan;
      }
      else if (*scan != '{') {
	// parse variable name
	const char* name = scan;
	scan = TextUtils::skipNonWhitespace(scan);

	// look up variable
	result += BZDB.get(string(name, scan - name));
      }
      else {
	// parse "quoted" variable name
	const char* name = ++scan;
	while (*scan != '\0' && *scan != '}')
	  ++scan;

	if (*scan != '\0') {
	  // look up variable
	  result += BZDB.get(string(name, scan - name));

	  // skip }
	  ++scan;
	}
      }
    }
  }

  std::cout << "printing \"" << result << "\"" << std::endl;
  return result;
}


static void onSetCB(const string& name, void* userData)
{
  // don't show names starting with _
  string& result = *static_cast<string*>(userData);
  if (!name.empty() && name.c_str()[0] != '_') {
    result += name;
    result += " = ";
    result += BZDB.get(name);
    result += "\n";
  }
}


static string cmdSet(const string&, const CmdArgList& args, bool* error)
{
  if (error) {
    *error = false;
  }

  switch (args.size()) {
    case 0: {
	// print out all values that are set
	string result;
	BZDB.iterate(&onSetCB, &result);
	return result;
    }
    case 1: {
      // the string was set to nothing, so just print value
      if (BZDB.isSet(args[0])) {
        return args[0] + " is " + BZDB.get(args[0]);
      } else {
        if(error) *error = true;
        return "variable " + args[0] + " does not exist";
      }
    }
    case 2: {
	// set variable to value
	BZDB.set(args[0], args[1], StateDatabase::User);
	return string();
    }
    default: {
      if (error) {
        *error = true;
      }
      return "usage: set <name> [<value>]";
    }
  }
}


static string cmdUnset(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 1)
    return "usage: unset <name>";
  BZDB.unset(args[0], StateDatabase::User);
  return string();
}


static void onBindCB(const string& name, bool press,
                     const string& cmd, void* userData)
{
  string& result = *static_cast<string*>(userData);
  result += name;
  result += (press ? " down " : " up ");
  result += cmd;
  result += "\n";
}


static string cmdBind(const string&, const CmdArgList& args, bool*)
{
  if (args.size() == 0) {
    string result;
    KEYMGR.iterate(&onBindCB, &result);
    return result;
  }
  else if (args.size() < 3) {
    return "usage: bind <button-name> {up|down} <command> <args>...";
  }

  if (!isValidKey(TextUtils::tolower(args[0]))) {
    return string("bind error: OS can not bind \"") + args[0] + "\"";
  }

  BzfKeyEvent key;
  if (!KEYMGR.stringToKeyEvent(args[0], key)) {
    return string("bind error: unknown and uncreatable button name \"") + args[0] + "\"";
  }

  bool down;
  if (args[1] == "up") {
    down = false;
  }
  else if (args[1] == "down") {
    down = true;
  }
  else {
    return string("bind error: illegal state \"") + args[1] + "\"";
  }

  string cmd = args[2];
  for (unsigned int i = 3; i < args.size(); ++i) {
    cmd += " ";
    cmd += args[i];
  }

  // ignore attempts to modify Esc.  we reserve that for the menu
  if (key.chr != 27) {
    KEYMGR.bind(key, down, cmd);
  }

  return string();
}


static string cmdUnbind(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 2)
    return "usage: unbind <button-name> {up|down}";

  BzfKeyEvent key_event;
  if (!KEYMGR.stringToKeyEvent(args[0], key_event)) {
    return string("bind error: unknown button name \"") + args[0] + "\"";
  }

  bool down;
  if (args[1] == "up") {
    down = false;
  }
  else if (args[1] == "down") {
    down = true;
  }
  else {
    return string("bind error: illegal state \"") + args[1] + "\"";
  }

  KEYMGR.unbind(key_event, down);

  return string();
}


static string cmdToggle(const string&, const CmdArgList& args, bool*)
{
  if ((args.size() < 1) || (args.size() > 3)) {
    return "usage: toggle <name> [first [second]]";
  }
  const string& name = args[0];
  if (args.size() == 1) {
    BZDB.set(name, BZDB.isTrue(name) ? "0" : "1");
  } else if (args.size() == 2) {
    BZDB.set(name, BZDB.isTrue(name) ? "0" : args[1]);
  } else {
    BZDB.set(name, (BZDB.get(name) == args[1]) ? args[2] : args[1]);
  }
  return string();
}


static string cmdMult(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 2) {
    return "usage: mult <name> <value>";
  }
  float value;
  if (sscanf(BZDB.get(args[0]).c_str(), "%f", &value) != 1) {
    value = 0.0;
  }
  float amount;
  if (sscanf(args[1].c_str(), "%f", &amount) != 1) {
    amount = 1.0;
  }
  value *= amount;
  BZDB.set(args[0], TextUtils::format("%f", value), StateDatabase::User);
  return string();
}


static string cmdAdd(const string&, const CmdArgList& args, bool*)
{
  if (args.size() != 2) {
    return "usage: add <name> <value>";
  }
  float value;
  if (sscanf(BZDB.get(args[0]).c_str(), "%f", &value) != 1) {
    value = 0.0f;
  }
  float amount;
  if (sscanf(args[1].c_str(), "%f", &amount) != 1) {
    amount = 0.0f;
  }
  value += amount;
  BZDB.set(args[0], TextUtils::format("%f", value), StateDatabase::User);
  return string();
}


static string cmdCycle(const string&, const CmdArgList& args, bool*)
{
  if (args.size() < 2) {
    return "usage: cycle <name> <value> [value2] [value3] ...";
  }

  const string& key = args[0];
  const string& val = BZDB.get(key);

  size_t index;
  for (index = 1; index < args.size(); index++) {
    if (val == args[index]) {
      break;
    }
  }
  if (index == args.size()) {
    index = 0; // start at the first value when there are no matches
  }
  index = (index % (args.size() - 1)) + 1;

  BZDB.set(key, args[index]);

  return string();
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
  { "cycle",	&cmdCycle,	"cycle name value1 value2 value3 etc...: cycle through a set of values" },
  { "toggle",	&cmdToggle,	"toggle <name> [first [second]]:  toggle value of a variable" },
  { "mult",	&cmdMult,	"mult <name> <value>:  multiply a variable by an amount" },
  { "add",	&cmdAdd,	"add <name> <value>:  add an amount to a variable" },
  { "bind",	&cmdBind,	"bind <button-name> {up|down} <command> <args>...: bind a key" },
  { "unbind",	&cmdUnbind,	"unbind <button-name> {up|down}:  unbind a key" }
};


//
// CommandsStandard
//

void CommandsStandard::add()
{
  unsigned int i;
  for (i = 0; i < countof(commands); ++i) {
    CMDMGR.add(commands[i].name, commands[i].func, commands[i].help);
  }
}


void CommandsStandard::remove()
{
  unsigned int i;
  for (i = 0; i < countof(commands); ++i) {
    CMDMGR.remove(commands[i].name);
  }
}


void CommandsStandard::quit()
{
  quitFlag = true;
}


bool CommandsStandard::isQuit()
{
  return quitFlag;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
