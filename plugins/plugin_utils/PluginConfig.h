/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
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
 *
 * Reads a configuration file like this:
 *
 * -- myconfig.txt --
 * [Section]
 *    SomeKey = Some value goes here
 *    key2    =    Some other value
 * [ AnotherSection ]
 *    Key3 = 4
 * ------------------
 * Then you parse it as follows:
 *  config = PluginConfig("myconfig.txt");
 *  config.item("Section", "SomeKey") returns "Some value goes here"
 *  config.item("Section", "key2")    returns "Some other value"
 *  config.item("anotherSection", "Key3") returns "4"
 *  config.item("missingSection", "Anything") returns ""
 *
 * Section and Key strings are not case sensitive but value strings are.
 * Sections, keys, and values have leading and trailing whitespace stripped.
 * Nonexistent keys return the empty string
 *
 * Debug level 4 (-dddd) provides information about the parsing process
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
