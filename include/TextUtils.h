/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * common definitions
 */

#ifndef __TEXTUTILS_H__
#define	__TEXTUTILS_H__

#if (_WIN32)
// turn off bogus `this used in base member initialization list'
	#pragma warning(disable: 4786)
	#pragma warning(disable: 4503)
	#pragma warning(disable: 4355)
#endif

#include <config.h>

#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdarg.h>

/** The string utility class provides basic functionality to parse and
 * format strings
 */
class string_util {
  public:
    static std::string string_util::vformat(const char* fmt, va_list args) {
      // FIXME -- should prevent buffer overflow in all cases
      // not all platforms support vsnprintf so we'll use vsprintf and a
      // big temporary buffer and hope for the best.
      char buffer[8192];
      vsprintf(buffer, fmt, args);
      return std::string(buffer);
    }
    static std::string string_util::format(const char* fmt, ...) {
      va_list args;
      va_start(args, fmt);
      std::string result = vformat(fmt, args);
      va_end(args);
      return result;
    }
    // get a vector of strings from a string, using all of chars of the delims
    // string as separators
    static std::vector<std::string> string_util::tokenize(std::string& in, std::string delims){
      std::vector<std::string> out;

      unsigned int startPos = in.find_first_not_of(delims);
      while (startPos != std::string::npos){
	unsigned int endPos = in.find_first_of(delims,startPos);
	if (endPos == std::string::npos) {
	  out.push_back(in.substr(startPos));
	  break;
	}
	else
	  out.push_back(in.substr(startPos,endPos-startPos));

	startPos = in.find_first_not_of(delims,endPos);
      }
      return out;
    }
};


// C h a r a c t e r  c o m p a r i s o n

/** compare_nocase is strait from Stroustrup.  This implementation uses
 * strings instead of char arrays and includes a maxlength bounds check.
 * It compares two strings and returns 0 if equal, <0 if s1 is less than
 * s2, and >0 if s1 is greater than s2. 
 */
inline static int compare_nocase(const std::string& s1, const std::string &s2, int maxlength=4096)
{
std::string::const_iterator p1 = s1.begin();
std::string::const_iterator p2 = s2.begin();
  int i=0;
  while (p1 != s1.end() && p2 != s2.end()) {
    if (i >= maxlength) {
      return 0;
    }
    if (tolower(*p1) != tolower(*p2)) {
      return (tolower(*p1) < tolower(*p2)) ? -1 : 1;
    }
    ++p1;
    ++p2;
    ++i;
  }
  return (s2.size() == s1.size()) ? 0 : (s1.size() < s2.size()) ? -1 : 1; // size is unsigned
}



/** utility function returns truthfully whether
 * given character is a letter.
 */
inline bool isAlphabetic(const char c)
{
  if (( c > 64 && c < 91) ||
      ( c > 96 && c < 123)) {
    return true;
  }
  return false;
}


/** utility function returns truthfully whether
 * given character is a number.
 */
inline bool isNumeric(const char c)
{
  if (( c > 47 && c < 58)) {
    return true;
  }
  return false;
}


/** utility function returns truthfully whether
 * a given character is printable whitespace.
 * this includes newline, carriage returns, tabs
 * and spaces.
 */
inline bool isWhitespace(const char c)
{
  if ((( c >= 9 ) && ( c <= 13 )) ||
      (c == 32)) {
    return true;
  }
  return false;
}


/** utility function returns truthfully whether
 * a given character is punctuation.
 */
inline bool isPunctuation(const char c)
{
  if (( c > 32 && c < 48) ||
      ( c > 90 && c < 97) ||
      ( c > 122 && c < 127)) {
    return true;
  }
  return false;
}


/** utility function returns truthfully whether
 * given character is an alphanumeric.  this is
 * strictly letters and numbers.
 */
inline bool isAlphanumeric(const char c)
{
  if ( isAlphabetic(c) || isNumeric(c) ) {
    return true;
  }
  return false;
}


/** utility function returns truthfully whether
* given character is printable.  this includes
* letters, numbers, and punctuation.
* (but NOT whitespace)
*/
inline bool isVisible(const char c)
{
  if ( isAlphanumeric(c) || isPunctuation(c) ) {
    return true;
  }
  return false;
}


/** utility function returns truthfully whether
 * given character is printable.  this includes
 * letters, numbers, punctuation, and whitespace
 */
inline bool isPrintable(const char c)
{
  if ( isVisible(c) || isWhitespace(c) ) {
    return false;
  }
  return true;
}



// S t r i n g  i t e r a t i o n

/** utility method that returns the position of the
 * first alphanumeric character from a string
 */
inline int firstAlphanumeric(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;

  }

  int i = 0;
  while (!isAlphanumeric(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


/** utility method that returns the position of the
 * first non-alphanumeric character from a string
 */
inline int firstNonalphanumeric(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;
  }

  int i = 0;
  while (isAlphanumeric(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


/** utility method that returns the position of the
 * first printable character from a string
 */
inline int firstPrintable(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;
  }

  int i = 0;
  while (!isPrintable(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


/** utility method that returns the position of the
 * first non-printable character from a string
 */
inline int firstNonprintable(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;
  }

  int i = 0;
  while (isPrintable(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


/** utility method that returns the position of the
 * first visible character from a string
 */
inline int firstVisible(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;
  }

  int i = 0;
  while (isVisible(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


/** utility method that returns the position of the
 * first non visible character from a string (control
 * codes or whitespace
 */
inline int firstNonvisible(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;
  }

  int i = 0;
  while (!isVisible(input[i]) && (i < max)) {
    i++;
  }
	 return i;
}


/** utility method that returns the position of the
* first alphabetic character from a string
*/
inline int firstAlphabetic(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;

  }

  int i = 0;
  while (!isAlphabetic(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


/** utility method that returns the position of the
* first printable character from a string
*/
inline int firstNonalphabetic(const std::string &input, unsigned short int max=4096)
{
  if (input.size() == 0) {
    return -1;
  }

  int i = 0;
  while (isAlphabetic(input[i]) && (i < max)) {
    i++;
  }
  return i;
}


#endif // __TEXTUTILS_H__

// ex: shiftwidth=2 tabstop=8
