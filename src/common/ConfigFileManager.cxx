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
  std::string value = command;
  if (value.find(' ') != value.npos)
    value = std::string("\"") + value + "\"";
  s << "bind " << name << ' ' << (press ? "down" : "up") << ' ' << value << std::endl;
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

bool				ConfigFileManager::write(std::string filename)
{
  // FIXME - temporarily add '19' on the end of the file name
  filename.append("19");
  std::ostream* stream = FILEMGR->createDataOutStream(filename);
  if (stream == NULL)
    return false;
  BZDB->write(writeBZDB, stream);
  KEYMGR->iterate(writeKEYMGR, stream);
  delete stream;
  return true;
}

void				ConfigFileManager::addDefaults()
{
  BZDB->setDefault("udpnet", "1");
  BZDB->set("udpnet", "1");
  BZDB->setDefault("team", "Rogue");
  BZDB->set("team", "Rogue");
  BZDB->setDefault("list", "http://BZFlag.SourceForge.net/list-server.txt");
  BZDB->set("list", "http://BZFlag.SourceForge.net/list-server.txt");
  BZDB->setDefault("volume", "10");
  BZDB->set("volume", "10");
  BZDB->setDefault("latitude", "37.5");
  BZDB->set("latitude", "37.5");
  BZDB->setDefault("longitude", "122");
  BZDB->set("longitude", "122");
  // FIXME: keys?
  BZDB->setDefault("joystick", "0");
  BZDB->set("joystick", "0");
  BZDB->setDefault("enhancedRadar", "1");
  BZDB->set("enhancedRadar", "1");
  BZDB->setDefault("coloredradarshots", "1");
  BZDB->set("coloredradarshots", "1");
  BZDB->setDefault("linedradarshots", "0");
  BZDB->set("linedradarshots", "0");
  BZDB->setDefault("panelopacity", "0.3");
  BZDB->set("panelopacity", "0.3");
  BZDB->setDefault("radarsize", "4");
  BZDB->set("radarsize", "4");
  BZDB->setDefault("mouseboxsize", "5");
  BZDB->set("mouseboxsize", "5");
  BZDB->setDefault("bigfont", "0");
  BZDB->set("bigfont", "0");
  BZDB->setDefault("colorful", "1");
  BZDB->set("colorful", "1");
  BZDB->setDefault("underline", "0");
  BZDB->set("underline", "0");
  BZDB->setDefault("killerhighlight", "0");
  BZDB->set("killerhighlight", "0");
  BZDB->setDefault("serverCacheAge", "0");
  BZDB->set("serverCacheAge", "0");
}

ConfigFileManager*		ConfigFileManager::getInstance()
{
  if (mgr == NULL)
    mgr = new ConfigFileManager;
  return mgr;
}
