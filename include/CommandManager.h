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

#ifndef BZF_COMMAND_MANAGER_H
#define BZF_COMMAND_MANAGER_H

#include "common.h"
#include "BzfString.h"
#include <map>
#include <vector>

#define CMDMGR (CommandManager::getInstance())

class CommandManager {
public:
	// type of function that implements command.  function should return
	// a string with the output of the command (or the empty string if
	// there's no output).
	typedef std::vector<BzfString> ArgList;
	typedef BzfString (*CommandFunction)(const BzfString& name, const ArgList&);
	typedef void (*Callback)(const BzfString& name, void* userData);

	~CommandManager();

	// add/replace a command handler
	void				add(const BzfString& name,
								CommandFunction, const BzfString& help);

	// remove a command handler
	void				remove(const BzfString& name);

	// get the help string for a command
	BzfString			getHelp(const BzfString& name) const;

	// execute a command
	BzfString			run(const BzfString& name, const ArgList&) const;

	// parse and execute a command
	BzfString			run(const BzfString& cmd) const;

	// invoke the callback for each registered command
	void				iterate(Callback, void* userData) const;

	// get the singleton instance of the command manager
	static CommandManager* getInstance();

private:
	CommandManager();

	static const char*	readValue(const char* string, BzfString* value);
	static const char*	readUnquoted(const char* string, BzfString* value);
	static const char*	readQuoted(const char* string, BzfString* value);
	static const char*	skipWhitespace(const char* string);

private:
	struct CmdInfo {
	public:
		CommandFunction	func;
		BzfString		help;
	};
	typedef std::map<BzfString, CmdInfo> Commands;

	Commands			commands;
	static CommandManager* mgr;
};

#endif
