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

class ConfigFileManager {
public:
  ConfigFileManager();
  ~ConfigFileManager();

  // read a configuration file.  read(filename) uses FileManager
  // to open the stream and returns false if the file cannot be
  // opened.  they all call parse().
  bool				read(std::string filename);
  void				read(std::istream&);

  // get the singleton instance
  static ConfigFileManager*	getInstance();

private:
  // parse a config file
  bool				parse(std::istream&);

  static ConfigFileManager*	mgr;
};

#endif
// ex: shiftwidth=2 tabstop=8
