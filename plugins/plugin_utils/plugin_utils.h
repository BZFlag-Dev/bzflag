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

// a series of utilitys for bzfs plugins to use.
#ifndef _PLUGIN_UTILS_H_
#define _PLUGIN_UTILS_H_

#include "bzfsAPI.h"

//common utilities
const char* bzu_GetTeamName ( bz_eTeamType team );
bz_eTeamType bzu_getTeamFromFlag (const char* flagCode);

// text functions
const std::string& tolower(const std::string& s, std::string& dest);
const std::string& toupper(const std::string& s, std::string& dest);
const std::string& tolower(const char* s, std::string& dest);
const std::string& toupper(const char* s, std::string& dest);

const std::string& makelower(std::string& s);
const std::string& makeupper(std::string& s);

inline std::string makelower(const char *s) { std::string t; if (s) tolower(s,t); return t;}
inline std::string makeupper(const char *s) { std::string t; if (s) toupper(s,t); return t;}

std::string format(const char* fmt, ...)_ATTRIBUTE12;
std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens, const bool useQuotes, size_t offset = 0);
std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe);
std::string url_encode(const std::string &text);
std::string url_decode(const std::string &text);

size_t find_first_substr(const std::string &findin, const std::string findwhat, size_t offset = 0);

std::string getStringRange ( const std::string &find, size_t start, size_t end );

void trimLeadingWhitespace ( std::string &text );
std::string trimLeadingWhitespace ( const std::string &text );

std::string no_whitespace(const std::string &s);

std::string printTime ( bz_Time *ts, const char* timezone = "UTC" );
void appendTime ( std::string & text, bz_Time *ts, const char* timezone = "UTC" );

inline int compare_nocase(const std::string& s1,
			  const std::string& s2, size_t maxlength = 4096)
{
  // length check
  if ((s1.size() < maxlength) || (s2.size() < maxlength)) {
    if (s1.size() != s2.size()) {
      return (s1.size() < s2.size()) ? -1 : +1;
    }
    maxlength = s1.size(); // clamp the maxlength
  }

  // check the characters
  for (size_t i = 0; i < maxlength; i++) {
    const std::string::value_type lower1 = ::tolower(s1[i]);
    const std::string::value_type lower2 = ::tolower(s2[i]);
    if (lower1 != lower2) {
      return (lower1 < lower2) ? -1 : +1;
    }
  }

  return 0;
}


inline int compare_nocase(const char* s1,
			  const char* s2, size_t maxlength = 4096)
{
  if (!s1 || !s2) {
    return -1;
  }
  return compare_nocase(std::string(s1), std::string(s2), maxlength);
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
    return true;
  }
  return false;
}

const std::vector<std::string> bzu_standardPerms (void);


// Configuration file parsing functions
#include "plugin_config.h"

#endif //_PLUGIN_UTILS_H_


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
