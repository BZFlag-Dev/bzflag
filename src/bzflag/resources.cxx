/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#if defined(_WIN32)
	#pragma warning(disable: 4786)
#endif

#include "resources.h"
#include "BzfDisplay.h"
#include "network.h"
#include "ErrorHandler.h"
#include "bzfio.h"
#include <ctype.h>

//
// ResourceDatabase
//

static const int	MaximumResourceLineLength = 1024;

ResourceDatabase::ResourceDatabase()
{
  // do nothing
}

ResourceDatabase::~ResourceDatabase()
{
  // do nothing
}

bool			ResourceDatabase::hasValue(const std::string& name) const
{
  return getNameIndex(name) != -1;
}

std::string		ResourceDatabase::getValue(const std::string& name) const
{
  const int index = getNameIndex(name);
  if (index == -1) return std::string();
  return values[index];
}

void			ResourceDatabase::addValue(
				const std::string& name, const std::string& value)
{
  const int index = getNameIndex(name);
  if (index == -1) {
    names.push_back(name);
    values.push_back(value);
  }
  else {
    values[index] = value;
  }
}

void			ResourceDatabase::removeValue(const std::string& name)
{
  const int index = getNameIndex(name);
  if (index == -1) return;
  std::vector<std::string>::iterator it;
  {
    it = names.begin();
    for(int i = 0; i < index; i++) it++;
    names.erase(it);
  }
  {
    it = values.begin();
    for(int i = 0; i < index; i++) it++;
    values.erase(it);
  }
}

int			ResourceDatabase::getNameIndex(
				const std::string& name) const
{
  const int count = names.size();
  for (int i = 0; i < count; i++)
    if (names[i] == name)
      return i;
  return -1;
}

istream&		operator>>(istream& input, ResourceDatabase& db)
{
  char lineBuffer[MaximumResourceLineLength + 1];
  int lineNumber = 0;
  std::vector<std::string> args;
  char buf[50];

  while (input) {
    // get next line
    lineNumber++;
    input.getline(lineBuffer, MaximumResourceLineLength);

    // check for errors
    if (input.eof()) continue;
    if (input.fail()) {
      if (input.bad()) {
	sprintf(buf, "%d", lineNumber);
	args.push_back(buf);
	printError("Configuration file:  Error on line {1}: Stream failure", &args);
      }
      else {
	sprintf(buf, "%d", lineNumber);
	args.push_back(buf);
	printError("Configuration file:  Error on line {1}: Line too long", &args);
      }
      continue;
    }

    // see if there's a keyword
    char* name = lineBuffer;
    while (*name && isspace(*name)) name++;
    if (!*name || *name == '#') continue;

    // put a NUL at end of name
    char* scan = name;
    while (*scan && !isspace(*scan)) scan++;
    if (*scan) *scan++ = '\0';

    // find the value
    char* value = scan;
    while (*value && isspace(*value)) value++;

    // add the name/value pair (value can be empty)
    db.addValue(name, value);
  }

  return input;
}

ostream&		operator<<(ostream& s, const ResourceDatabase& db)
{
  const int count = db.names.size();
  for (int i = 0; i < count; i++) {
    int length = db.names[i].length();
    s << db.names[i].c_str() << "\t";
    for (; length < 24; length += 8) s << "\t";
    s << db.values[i].c_str() << endl;
  }
  return s;
}
// ex: shiftwidth=2 tabstop=8
