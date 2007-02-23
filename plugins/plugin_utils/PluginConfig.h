/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// PluginUtils.h - Collection of useful utility functions for plugins

#include <map>
#include <string>

using namespace std;

/*
 * PluginConfig - INI style configuration file parser class
 */

class PluginConfig {
public:
  PluginConfig(string filename);
  ~PluginConfig() {};
  string item(string section, string key);
  unsigned int errors;
private:
  string whitespace;
  void parse(void);
  map<string, map<string, string> > sections;
  string configFilename;
};

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
