/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
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

#include "bzfsAPI.h"
#include "plugin_config.h"
#include "plugin_utils.h"
#include <iostream>
#include <fstream>

/*
 * INI style configuration file parser class
 *
 * Comments start with # or ;
 * Ignores whitespace around sections, keys, and values
 * Default section if none is specified is [global]
 */

PluginConfig::PluginConfig()
{
}

PluginConfig::PluginConfig(const std::string &filename) : errors(0), whitespace(" \t\r"), configFilename(filename)
{
  parse();
}

void PluginConfig::read(const char* filename)
{
  read(std::string(filename));
}

void PluginConfig::read(const std::string &filename)
{
  sections.clear();
  whitespace = " \t\r";
  errors = 0;
  configFilename = filename;

  parse();
}


std::string PluginConfig::item(const char *section, const  char *key)
{
  return item(std::string(section),std::string(key));
}

std::string PluginConfig::item(const std::string &section, const std::string &key)
{
  std::string s = section,k = key;
  makelower(s);
  makelower(k);

  return sections[s][k];
}

void PluginConfig::parse(void)
{
  std::string line;
  std::string section;
  std::string key;
  std::string value;
  std::ifstream iniFile;
  size_t start = 0, end = 0;
  size_t equalPos = 0;

  /*
   * Parse key,value pairs for sections out of the INI type
   * configuration file specified in 'filename'
   *
   */
  iniFile.open(configFilename.c_str(), std::ios::in);

  if (!iniFile.is_open()) {
    bz_debugMessagef(1, "PluginConfig: Can't open configuration file: %s", configFilename.c_str());
    errors++;
    return;
  }

  section = "global";

  while (!iniFile.eof()) {
    getline(iniFile, line);
    start = line.find_first_not_of(whitespace);

    /*
     * Look for comments and skip
     */
    if (line[start] == '#') {
      continue;
    }

    /*
     * Look for a section tag
     */
    if (line[start] == '[') {
      start = line.find_first_not_of(whitespace, start + 1);

      /* Check if the last non whitespace character is a close bracket */
      end = line.find_last_not_of(whitespace);
      if (line[end] == ']') {
	end = line.find_last_not_of(whitespace, end - 1);
	/* Got a section header - save it */
	section = line.substr(start, end - start + 1);
	bz_debugMessagef(4, "PluginConfig: Found section [%s]", section.c_str());
	continue;
      }
      /*
       * We either got a valid section or we have a
       * Malformed line - '[' but not matching close ']' which we ignore.
       */
      bz_debugMessagef(1, "PluginConfig: Malformed line ignored: %s", line.c_str());
      continue;
    }

    /*
     * No section tag, look for 'key = value' pairs
     *
     * Split the line into 'key = value' pairs
     */
    equalPos = line.find("=", start);

    /* If there is no '=' sign then ignore the line - treated as a comment */
    if (equalPos == std::string::npos) {
      if (line.find_first_not_of(whitespace) != std::string::npos)
	bz_debugMessagef(1, "PluginConfig: Malformed line ignored: %s", line.c_str());
      continue;
    }

    /* Extract the key */
    end = line.find_last_not_of(whitespace, equalPos - 1);
    key = line.substr(start, end - start + 1);

    /* Extract the value */
    start = line.find_first_not_of(whitespace, equalPos + 1);
    end = line.find_last_not_of(whitespace);
    if (start == std::string::npos || end == std::string::npos)
      value = "";
    else
      value = line.substr(start, end - start + 1);

    makelower(key);
    makelower(section);

    /* Save the section, key and value in the std::map for later retrieval */
    sections[section][key] = value;
    bz_debugMessagef(4, "PluginConfig: Found key [%s].%s = %s", section.c_str(), key.c_str(), value.c_str());
  }

  iniFile.close();
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
