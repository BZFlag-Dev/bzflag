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

/*
 * common definitions
 */

#ifndef __SIMPLETEXTUTILS_H__
#define	__SIMPLETEXTUTILS_H__

/* system interface headers */
#include <string>
#include <stdarg.h>
#include <vector>

/** This namespace provides basic functionality to parse and
 * format strings
 */
namespace SimpleTextUtils {
  std::string vformat(const char* fmt, va_list args);
  std::string format(const char* fmt, ...);

  /** returns a string converted to lowercase
   */
  inline std::string tolower(const std::string& s)
  {
    std::string trans = s;

    for (std::string::iterator i=trans.begin(), end=trans.end(); i!=end; ++i)
      *i = ::tolower(*i);
    return trans;
  }

  /** returns a string converted to uppercase
   */
  inline std::string toupper(const std::string& s)
  {
    std::string trans = s;
    for (std::string::iterator i=trans.begin(), end=trans.end(); i!=end; ++i)
      *i = ::toupper(*i);
    return trans;
  }

  /** replace all of in in replaceMe with withMe
   */
  std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe);

  /**
   * Get a vector of strings from a string, using all of chars of thedelims
   * string as separators. If maxTokens > 0, then the last 'token' maycontain delimiters
   * as it just returns the rest of the line
   * if you specify use quotes then tokens can be grouped in quotes and delimeters
   * inside quotes are ignored.
   * Hence /ban "Mr Blahblah" isajerk parses to "/ban", "Mr Blahlah" and "isajerk"
   * but so does "Mr Blahblah" "isajerk", so if you want 3 tokens and a delimeter
   * is in one of the tokens, by puting quotes around it you can get the correct
   * parsing. When use quotes is enabled, \'s and "'s should\can be escaped with \
   * escaping is not currently done for any other character.
   * Should not have " as a delimeter if you want to use quotes
   */
  std::vector<std::string> tokenize(const std::string& in, const std::string &delims, const int maxTokens = 0, const bool useQuotes = false);
}


#endif // __SIMPLETEXTUTILS_H__


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
