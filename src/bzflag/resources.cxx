/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "resources.h"
#include "BzfString.h"
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

boolean			ResourceDatabase::hasValue(const BzfString& name) const
{
  return getNameIndex(name) != -1;
}

BzfString		ResourceDatabase::getValue(const BzfString& name) const
{
  const int index = getNameIndex(name);
  if (index == -1) return BzfString();
  return values[index];
}

void			ResourceDatabase::addValue(
				const BzfString& name, const BzfString& value)
{
  const int index = getNameIndex(name);
  if (index == -1) {
    names.append(name);
    values.append(value);
  }
  else {
    values[index] = value;
  }
}

void			ResourceDatabase::removeValue(const BzfString& name)
{
  const int index = getNameIndex(name);
  if (index == -1) return;
  names.remove(index);
  values.remove(index);
}

int			ResourceDatabase::getNameIndex(
				const BzfString& name) const
{
  const int count = names.getLength();
  for (int i = 0; i < count; i++)
    if (names[i] == name)
      return i;
  return -1;
}

istream&		operator>>(istream& input, ResourceDatabase& db)
{
  char lineBuffer[MaximumResourceLineLength + 1];
  int lineNumber = 0;

  while (input) {
    // get next line
    lineNumber++;
    input.getline(lineBuffer, MaximumResourceLineLength);

    // check for errors
    if (input.eof()) continue;
    if (input.fail()) {
      if (input.bad())
	printError("Configuration file:  "
		"Error on line %d: Stream failure", lineNumber);
      else
	printError("Configuration file:  "
		"Error on line %d: Line too long", lineNumber);
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
  const int count = db.names.getLength();
  for (int i = 0; i < count; i++) {
    int length = db.names[i].getLength();
    s << db.names[i] << "\t";
    for (; length < 24; length += 8) s << "\t";
    s << db.values[i] << endl;
  }
  return s;
}
