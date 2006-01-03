/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_CONFIG_FILE_MANAGER_H
#define BZF_CONFIG_FILE_MANAGER_H

#include <string>
#include "Singleton.h"

#define CFGMGR (ConfigFileManager::instance())

void writeBZDB(const std::string& name, void *stream);
void writeKEYMGR(const std::string& name, bool press, const std::string& command, void* stream);

/**
 Reads in the config file.
 Opens up the file via FileManager, ships lines off to the
 CommandManager and handles default values in BZDB,
*/

class ConfigFileManager : public Singleton<ConfigFileManager> {
public:
  /** Read a configuration file.
   read(filename) uses FileManager to open the stream and returns
   false if the file cannot be opened.  they all call parse().
  */
  bool				read(const std::string& filename);
  /** Read a configuration file.
   read(filename) uses FileManager to open the stream and returns
   false if the file cannot be opened.  they all call parse().
  */
  void				read(std::istream&);

  /** Write out a configuration file.
   Writes to a format that the CommandManager can understand
  */
  bool				write(const std::string& filename);


protected:
  friend class Singleton<ConfigFileManager>;
  ConfigFileManager();
  ~ConfigFileManager();

private:
  // parse a config file
  bool				parse(std::istream&);

};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

