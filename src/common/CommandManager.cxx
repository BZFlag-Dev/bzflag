/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#include "CommandManager.h"

/* system implementation headers */
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string>

/* common implementation headers */
#include "TextUtils.h"


// initialize the singleton
template <>
CommandManager* Singleton<CommandManager>::_instance = (CommandManager*)0;

CommandManager::CommandManager()
{
  // do nothing
}

CommandManager::~CommandManager()
{
  // do nothing
}

void				CommandManager::add(const std::string& name,
						    CommandFunction func,
						    const std::string& help)
{
  commands.erase(name);
  CmdInfo info;
  info.func = func;
  info.help = help;
  commands.insert(std::make_pair(name, info));
}

void				CommandManager::remove(const std::string& name)
{
  commands.erase(name);
}

std::string			CommandManager::getHelp(const std::string& name) const
{
  // look up command
  Commands::const_iterator index = commands.find(name);
  if (index == commands.end())
    return "";

  // return help string
  return index->second.help;
}

std::string			CommandManager::run(const std::string& name,
							    const ArgList& args, bool* ret) const
{
  // look up command
  Commands::const_iterator index = commands.find(name);
  if (index == commands.end())
  {
    return TextUtils::format("Command %s not found", name.c_str());
	if (ret)
		*ret = false;
  }
  if (ret)
	  *ret = true;
  // run it
  return (*index->second.func)(name, args,ret);
}

std::string			CommandManager::run(const std::string& cmd,bool *ret) const
{
  std::string result;
  const char* scan = cmd.c_str();

  scan = skipWhitespace(scan);
  while (scan != NULL && *scan != '\0') {
    std::string name;
    ArgList args;

    // parse command name
    scan = skipWhitespace(scan);
    scan = readValue(scan, &name);
    if (scan != NULL)
      scan = skipWhitespace(scan);

    // parse arguments
    while (scan != NULL && *scan != '\0' && *scan != ';') {
      std::string value;
      scan = readValue(scan, &value);
      if (scan != NULL) {
	scan = skipWhitespace(scan);
	args.push_back(value);
      }
    }

    // run it or report error
    if (scan == NULL)
	{
		if (ret)
			*ret = false;
      return std::string("Error parsing command");
	}
    else if (name[0] != '#')
      result = run(name, args,ret);

    // discard ; and empty commands
    while (scan != NULL && *scan == ';') {
      ++scan;
      scan = skipWhitespace(scan);
    }
  }

  // return result of last command only
  return result;
}

void				CommandManager::iterate(Callback callback,
							void* userData) const
{
  assert(callback != NULL);

  for (Commands::const_iterator index = commands.begin();
       index != commands.end(); ++index)
    (*callback)(index->first, userData);
}


const char*			CommandManager::readValue(const char* string,
							  std::string* value)
{
  if (*string == '\"')
    return readQuoted(string + 1, value);
  else if (*string != '\0')
    return readUnquoted(string, value);
  else
    return string;
}

const char*			CommandManager::readUnquoted(const char* string,
							     std::string* value)
{
  // read up to next whitespace.  escapes are not interpreted.
  const char* start = string;
  while (*string != '\0' && !isspace(*string) && *string != ';')
    ++string;
  *value = std::string(start, string - start);
  return string;
}

const char*			CommandManager::readQuoted(const char* string,
							   std::string* value)
{
  *value = "";
  bool escaped = false;
  for (; *string != '\0'; ++string) {
    if (escaped) {
      switch (*string) {
	case 't': value->append("\t", 1); break;
	case 'n': value->append("\n", 1); break;
	case 'r': value->append("\r", 1); break;
	case '\\': value->append("\\", 1); break;
	case '\"': value->append("\"", 1); break;
	default: value->append(string, 1); break;
      }
      escaped = false;
    } else if (*string == '\\') {
      escaped = true;
    } else if (*string == '\"') {
      return string + 1;
    } else {
      value->append(string, 1);
    }
  }
  // closing quote is missing.  if escaped is true the called may have
  // wanted to continue the line but we don't allow that.
  return NULL;
}

const char*			CommandManager::skipWhitespace(const char* string)
{
  while (*string != '\0' && isspace(*string))
    ++string;
  return string;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
