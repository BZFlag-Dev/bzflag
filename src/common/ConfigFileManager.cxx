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

ConfigFileManager* ConfigFileManager::mgr = NULL;
static const int        MaximumLineLength = 1024;

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
  // FIXME - temporarily add '19' on the end of the file name
  filename.append("19");
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

ConfigFileManager*	ConfigFileManager::getInstance()
{
  if (mgr == NULL)
    mgr = new ConfigFileManager;
  return mgr;
}
