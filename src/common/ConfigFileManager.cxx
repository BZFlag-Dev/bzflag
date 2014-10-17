/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "ConfigFileManager.h"
#include "FileManager.h"
#include "CommandManager.h"
#include "StateDatabase.h"
#include "KeyManager.h"

static const int	MaximumLineLength = 1024;

// initialize the singleton
template <>
ConfigFileManager* Singleton<ConfigFileManager>::_instance = (ConfigFileManager*)0;

void writeBZDB(const std::string& name, void *stream)
{
  std::ostream& s = *static_cast<std::ostream*>(stream);
  std::string value = BZDB.get(name);
  std::string defaultVal = BZDB.getDefault(name);
  std::string newkey;
  bool commentOut = (value == defaultVal);

  // quotify anything with a space and empty strings
  if ((value.find(' ') != value.npos) || (value.size() == 0)) {
    value = std::string("\"") + value + "\"";
  }

  // quotify the key if there's a space
  if (name.find(' ') != name.npos)
    newkey = std::string("\"") + name + "\"";
  else
    newkey = name;

  s << (commentOut ? "#set " : "set ") << newkey << ' ' << value << std::endl;
}

void writeKEYMGR(const std::string& name, bool press, const std::string& command, void* stream)
{
  std::ostream& s = *static_cast<std::ostream*>(stream);
  // quotify anything with a space
  std::string value = name;
  if (value.find(' ') != value.npos)
    value = std::string("\"") + value + "\"";
  s << "bind " << value << ' ' << (press ? "down " : "up ");
  value = command;
  if (value.find(' ') != value.npos)
    value = std::string("\"") + value + "\"";
  s << value << std::endl;
}

ConfigFileManager::ConfigFileManager()
{
  // do nothing
}

ConfigFileManager::~ConfigFileManager()
{
}

bool				ConfigFileManager::parse(std::istream& stream)
{
  char buffer[MaximumLineLength];
  while (stream.good()) {
    stream.getline(buffer, MaximumLineLength);
    CMDMGR.run(buffer);
  }
  return true;
}

bool				ConfigFileManager::read(const std::string& filename)
{
  std::istream* stream = FILEMGR.createDataInStream(filename);
  if (stream == NULL) {
    return false;
  }
  bool ret = parse(*stream);
  delete stream;
  return ret;
}

void				ConfigFileManager::read(std::istream& stream)
{
  parse(stream);
}

bool				ConfigFileManager::write(const std::string& filename)
{
  std::ostream* stream = FILEMGR.createDataOutStream(filename);
  if (stream == NULL) {
    return false;
  }
  BZDB.write(writeBZDB, stream);
  KEYMGR.iterate(writeKEYMGR, stream);
  delete stream;
  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
