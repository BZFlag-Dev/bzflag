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

#include "common.h"
#include "ConfigFileManager.h"
#include "FileManager.h"
#include "CommandManager.h"
#include "StateDatabase.h"
#include "KeyManager.h"

ConfigFileManager* ConfigFileManager::mgr = NULL;
static const int        MaximumLineLength = 1024;

void writeBZDB(const std::string& name, void *stream)
{
  std::ostream& s = *reinterpret_cast<std::ostream*>(stream);
  std::string value = BZDB->get(name);
  // quotify anything with a space
  if (value.find(' ') != value.npos)
    value = std::string("\"") + value + "\"";
  s << "set " << name << ' ' << value << std::endl;
}

void writeKEYMGR(const std::string& name, bool press, const std::string& command, void* stream)
{
  std::ostream& s = *reinterpret_cast<std::ostream*>(stream);
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
  if (mgr == this)
    mgr = NULL;
}

bool				ConfigFileManager::parse(std::istream& stream)
{
  char buffer[MaximumLineLength];
  while (!stream.eof()) {
    stream.getline(buffer, MaximumLineLength);
    CMDMGR->run(buffer);
  }
  return true;
}

bool				ConfigFileManager::read(std::string filename)
{
  std::istream* stream = FILEMGR->createDataInStream(filename);
  if (stream == NULL)
    return false;
  bool ret = parse(*stream);
  delete stream;
  return ret;
}

void				ConfigFileManager::read(std::istream& stream)
{
  parse(stream);
}

bool				ConfigFileManager::write(std::string filename)
{
  std::ostream* stream = FILEMGR->createDataOutStream(filename);
  if (stream == NULL)
    return false;
  BZDB->write(writeBZDB, stream);
  KEYMGR->iterate(writeKEYMGR, stream);
  delete stream;
  return true;
}

ConfigFileManager*		ConfigFileManager::getInstance()
{
  if (mgr == NULL)
    mgr = new ConfigFileManager;
  return mgr;
}
