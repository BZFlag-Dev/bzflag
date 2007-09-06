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
#include "bzfsAPI.h"

// text functions
std::string tolower(const std::string& s);
std::string format(const char* fmt, ...)_ATTRIBUTE12;
std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens, const bool useQuotes);
std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe);
std::string url_encode(const std::string &text);
std::string url_decode(const std::string &text);

inline int compare_nocase(const std::string& s1, const std::string &s2, int maxlength=4096)
{
  std::string::const_iterator p1 = s1.begin();
  std::string::const_iterator p2 = s2.begin();
  int i=0;
  while (p1 != s1.end() && p2 != s2.end()) {
    if (i >= maxlength) {
      return 0;
    }
    if (::tolower(*p1) != ::tolower(*p2)) {
      return (::tolower(*p1) < ::tolower(*p2)) ? -1 : 1;
    }
    ++p1;
    ++p2;
    ++i;
  }
  return (s2.size() == s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 : 1; // size is unsigned
}

inline bool isAlphabetic(const char c)
{
  if (( c > 64 && c < 91) ||
    ( c > 96 && c < 123)) {
      return true;
    }
    return false;
}

inline bool isNumeric(const char c)
{
  if (( c > 47 && c < 58)) {
    return true;
  }
  return false;
}

inline bool isWhitespace(const char c)
{
  if ((( c >= 9 ) && ( c <= 13 )) ||
    (c == 32)) {
      return true;
    }
    return false;
}

inline bool isPunctuation(const char c)
{
  if (( c > 32 && c < 48) ||
    ( c > 57 && c < 65) ||
    ( c > 90 && c < 97) ||
    ( c > 122 && c < 127)) {
      return true;
    }
    return false;
}

inline bool isAlphanumeric(const char c)
{
  if (isAlphabetic(c) || isNumeric(c)) {
    return true;
  }
  return false;
}

inline bool isVisible(const char c)
{
  if (isAlphanumeric(c) || isPunctuation(c)) {
    return true;
  }
  return false;
}

inline bool isPrintable(const char c)
{
  if (isVisible(c) || isWhitespace(c)) {
    return false;
  }
  return true;
}


// Configuration file parsing functions
#include "plugin_config.h"

#endif //_PLUGIN_UTILS_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
