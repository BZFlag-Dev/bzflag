/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
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
#include <string>
#include <map>
#include <vector>

#define CMDMGR (CommandManager::getInstance())

class CommandManager {
public:
  // type of function that implements command.  function should return
  // a string with the output of the command (or the empty string if
  // there's no output).
  typedef std::vector<std::string> ArgList;
  typedef std::string (*CommandFunction)(const std::string& name, const ArgList&);
  typedef void (*Callback)(const std::string& name, void* userData);

  ~CommandManager();

  // add/replace a command handler
  void				add(const std::string& name,
  				    CommandFunction, const std::string& help);

  // remove a command handler
  void				remove(const std::string& name);

  // get the help string for a command
  std::string			getHelp(const std::string& name) const;

  // execute a command
  std::string			run(const std::string& name, const ArgList&) const;

  // parse and execute a command
  std::string			run(const std::string& cmd) const;

  // invoke the callback for each registered command
  void				iterate(Callback, void* userData) const;

  // get the singleton instance of the command manager
  static CommandManager* getInstance();

private:
  CommandManager();

  static const char*	readValue(const char* string, std::string* value);
  static const char*	readUnquoted(const char* string, std::string* value);
  static const char*	readQuoted(const char* string, std::string* value);
  static const char*	skipWhitespace(const char* string);

private:
  struct CmdInfo {
  public:
    CommandFunction	func;
    std::string		help;
  };
  typedef std::map<std::string, CmdInfo> Commands;

  Commands			commands;
  static CommandManager* mgr;
};

#endif
// ex: shiftwidth=2 tabstop=8
