/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "CommandsStandard.h"
#include "CommandManager.h"
#include "KeyManager.h"
#include "MenuManager.h"
#include "StateDatabase.h"
#include <stdio.h>
#include <ctype.h>

static bool				quitFlag = false;

//
// command handlers
//

static BzfString		cmdQuit(const BzfString&,
								const CommandManager::ArgList&)
{
	CommandsStandard::quit();
	return BzfString();
}

static void				onHelpCB(const BzfString& name,
								void* userData)
{
	BzfString& result = *reinterpret_cast<BzfString*>(userData);
	result += name;
	result += "\n";
}

static BzfString		cmdHelp(const BzfString&,
								const CommandManager::ArgList& args)
{
	switch (args.size()) {
		case 0: {
			BzfString result;
			CMDMGR->iterate(&onHelpCB, &result);
			return result;
		}

		case 1:
			return CMDMGR->getHelp(args[0]);

		default:
			return "usage: help [<command-name>]";
	}
}

static BzfString		cmdPrint(const BzfString&,
								const CommandManager::ArgList& args)
{
	// merge all arguments into one string
	BzfString arg;
	const unsigned int n = args.size();
	if (n > 0)
		arg = args[0];
	for (unsigned int i = 1; i < n; ++i) {
		arg += " ";
		arg += args[i];
	}

	// interpolate variables
	BzfString result;
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
				while (*scan != '\0' && !isspace(*scan))
					++scan;

				// look up variable
				result += BZDB->get(BzfString(name, scan - name));
			}
			else {
				// parse "quoted" variable name
				const char* name = ++scan;
				while (*scan != '\0' && *scan != '}')
					++scan;

				if (*scan != '\0') {
					// look up variable
					result += BZDB->get(BzfString(name, scan - name));

					// skip }
					++scan;
				}
			}
		}
	}

	return result;
}

static void				onSetCB(const BzfString& name,
								void* userData)
{
	// don't show names starting with _
	BzfString& result = *reinterpret_cast<BzfString*>(userData);
	if (!name.empty() && name.c_str()[0] != '_') {
		result += name;
		result += " = ";
		result += BZDB->get(name);
		result += "\n";
	}
}

static BzfString		cmdSet(const BzfString&,
								const CommandManager::ArgList& args)
{
	switch (args.size()) {
		case 0: {
			BzfString result;
			BZDB->iterate(&onSetCB, &result);
			return result;
		}

		case 2:
			BZDB->set(args[0], args[1], StateDatabase::User);
			return BzfString();

		default:
			return "usage: set <name> <value>";
	}
}

static BzfString		cmdUnset(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 1)
		return "usage: unset <name>";
	BZDB->unset(args[0], StateDatabase::User);
	return BzfString();
}

static BzfString		cmdToggle(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 1)
		return "usage: toggle <name>";
	const BzfString& name = args[0];
	if (BZDB->isTrue(name))
		BZDB->set(name, "0", StateDatabase::User);
	else
		BZDB->set(name, "1", StateDatabase::User);
	return BzfString();
}

static BzfString		cmdAdd(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 2)
		return "usage: add <name> <value>";

	// get value
	double value;
	if (sscanf(BZDB->get(args[0]).c_str(), "%lf", &value) != 1)
		value = 0.0;

	// add argument
	double amount;
	if (sscanf(args[1].c_str(), "%lf", &amount) != 1)
		amount = 0.0;
	value += amount;

	// set new value
	BZDB->set(args[0], BzfString::format("%f", value), StateDatabase::User);

	return BzfString();
}

static void				onBindCB(const BzfString& name, bool press,
								const BzfString& cmd, void* userData)
{
	BzfString& result = *reinterpret_cast<BzfString*>(userData);
	result += name;
	result += (press ? " down " : " up ");
	result += cmd;
	result += "\n";
}

static BzfString		cmdBind(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() == 0) {
		BzfString result;
		KEYMGR->iterate(&onBindCB, &result);
		return result;
	}

	if (args.size() < 3)
		return "usage: bind <button-name> {up|down} <command> <args>...";

	// lookup key/button by name
	BzfKeyEvent key;
	if (!KEYMGR->stringToKeyEvent(args[0], key))
		return BzfString::format("bind error:  unknown button name \"%s\"",
								args[0].c_str());

	// get up/down
	bool down;
	if (args[1] == "up")
		down = false;
	else if (args[1] == "down")
		down = true;
	else
		return BzfString::format("bind error:  illegal state \"%s\"",
								args[1].c_str());

	// assemble command
	BzfString cmd = args[2];
	for (unsigned int i = 3; i < args.size(); ++i) {
		cmd += " ";
		cmd += args[i];
	}

	// ignore attempts to modify Esc.  we reserve that for the menu.
	if (key.ascii == 27)
		return BzfString();

	// bind key
	KEYMGR->bind(key, down, cmd);

	return BzfString();
}

static BzfString		cmdUnbind(const BzfString&,
								const CommandManager::ArgList& args)
{
	if (args.size() != 2)
		return "usage: unbind <button-name> {up|down}";

	// lookup key/button by name
	BzfKeyEvent key;
	if (!KEYMGR->stringToKeyEvent(args[0], key))
		return BzfString::format("bind error:  unknown button name \"%s\"",
								args[0].c_str());

	// get up/down
	bool down;
	if (args[1] == "up")
		down = false;
	else if (args[1] == "down")
		down = true;
	else
		return BzfString::format("bind error:  illegal state \"%s\"",
								args[1].c_str());

	// ignore attempts to modify Esc.  we reserve that for the menu.
	if (key.ascii == 27)
		return BzfString();

	// unbind key
	KEYMGR->unbind(key, down);

	return BzfString();
}

static BzfString		cmdMenu(const BzfString&,
								const CommandManager::ArgList& args)
{
	switch (args.size()) {
		case 2:
			if (args[0] != "push")
				break;
			if (!MENUMGR->push(args[1]))
				return BzfString::format("unknown menu: %s", args[1].c_str());
			else
				return BzfString();

		case 1:
			if (args[0] != "pop")
				break;
			if (MENUMGR->top() != NULL)
				MENUMGR->pop();
			return BzfString();
	}

	return "usage: menu {push <name>|pop}";
}


//
// command name to function mapping
//

struct CommandListItem {
public:
	const char*			name;
	CommandManager::CommandFunction func;
	const char*			help;
};
static const CommandListItem commandList[] = {
	{ "quit",	&cmdQuit,	"quit:  quit the program" },
	{ "help",	&cmdHelp,	"help [<command-name>]:  get help on a command or a list of commands" },
	{ "print",	&cmdPrint,	"print ...:  print arguments; $name is replaced by value of variable \"name\"" },
	{ "set",	&cmdSet,	"set [<name> <value>]:  set a variable or print all set variables" },
	{ "unset",	&cmdUnset,	"unset <name>:  unset a variable" },
	{ "toggle",	&cmdToggle,	"toggle <name>:  toggle truth value of a variable" },
	{ "add",	&cmdAdd,	"add <name> <value>:  add an amount to a variable" },
	{ "bind",	&cmdBind,	"bind [<button-name> {up|down} <command> <args>...]:  bind a key/button to a command or print all bindings" },
	{ "unbind",	&cmdUnbind,	"unbind <button-name> {up|down}:  unbind a key/button from a command" },
	{ "menu",	&cmdMenu,	"menu {push <name>|pop}:  push a menu onto the menu stack or pop the top menu" }
};
// FIXME -- may want a cmd to cycle through a list


//
// CommandsStandard
//

void					CommandsStandard::add()
{
	unsigned int i;
	for (i = 0; i < countof(commandList); ++i)
		CMDMGR->add(commandList[i].name,
							commandList[i].func, commandList[i].help);
}

void					CommandsStandard::remove()
{
	unsigned int i;
	for (i = 0; i < countof(commandList); ++i)
		CMDMGR->remove(commandList[i].name);
}

void					CommandsStandard::quit()
{
	quitFlag = true;
}

bool					CommandsStandard::isQuit()
{
	return quitFlag;
}
