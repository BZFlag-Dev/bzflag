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

#ifndef BZF_CONFIG_FILE_MANAGER_H
#define BZF_CONFIG_FILE_MANAGER_H

#define CFGMGR (ConfigFileManager::getInstance())

void writeEntry(const std::string& name, void *stream);

/**
 Reads in the config file.
 Opens up the file via FileManager, ships lines off to the
 CommandManager and handles default values in BZDB,
*/

class ConfigFileManager {
public:
  ConfigFileManager();
  ~ConfigFileManager();

  /** Read a configuration file.
   read(filename) uses FileManager to open the stream and returns
   false if the file cannot be opened.  they all call parse().
  */
  bool				read(std::string filename);
  /** Read a configuration file.
   read(filename) uses FileManager to open the stream and returns
   false if the file cannot be opened.  they all call parse().
  */
  void				read(std::istream&);

  /** Write out a configuration file.
   Writes to a format that the CommandManager can understand
  */
  bool				write(std::string filename);

  /** adds default values to BZDB */
  static void			addDefaults();

  /** Get the singleton instance.
   returns the existing singleton if available, if not, creates
   an instance and returns it.
  */
  static ConfigFileManager*	getInstance();

private:
  // parse a config file
  bool				parse(std::istream&);

  static ConfigFileManager*	mgr;
};

#endif
// ex: shiftwidth=2 tabstop=8
