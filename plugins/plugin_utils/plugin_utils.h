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

// a series of utilitys for bzfs plugins to use.
#ifndef _PLUGIN_UTILS_H_
#define _PLUGIN_UTILS_H_

#include <string>
#include <vector>

// text functions
std::string tolower(const std::string& s);
std::string format(const char* fmt, ...)
  __attribute__ ((__format__ (__printf__, 1, 2)));
std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens, const bool useQuotes);

// Configuration file parsing functions
#include "PluginConfig.h"

#endif //_PLUGIN_UTILS_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
