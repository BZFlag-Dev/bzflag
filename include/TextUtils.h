/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
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

#include <algorithm>
#include <common.h>

/** The string utility class provides basic functionality to parse and
 * format strings
 */
class string_util {
public:
  inline static std::string vformat(const char* fmt, va_list args) {
    // FIXME -- should prevent buffer overflow in all cases
    // not all platforms support vsnprintf so we'll use vsprintf and a
    // big temporary buffer and hope for the best.
    char buffer[8192];
    vsnprintf(buffer, 8192, fmt, args);
    return std::string(buffer);
  }
  inline static std::string format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string result = vformat(fmt, args);
    va_end(args);
    return result;
  }

  /** returns a string converted to lowercase
   */
  inline static std::string tolower(const std::string& s)
  {
    std::string trans = s;

    std::transform(trans.begin(), trans.end(), // source
               trans.begin(),    // destination
               ::tolower);
    return trans;
  }

  /** returns a string converted to uppercase
   */
  inline static std::string toupper(const std::string& s)
  {
    std::string trans = s;
    std::transform (trans.begin(), trans.end(), // source
               trans.begin(),    // destination
               ::toupper);
    return trans;
  }

  /** replace all of in in replaceMe with withMe
   */
  static std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe)
  {
    std::string result;
    unsigned int beginPos = 0;
    unsigned int endPos = 0;
    std::ostringstream tempStream;


    endPos = (unsigned int)in.find(replaceMe);
    if (endPos == std::string::npos) return in; // can't find anything to replace
    if (replaceMe == "") return in; // can't replace nothing with something -- can do reverse

    while (endPos != std::string::npos){
      // push the  part up to 
      tempStream << in.substr(beginPos,endPos-beginPos); 
      tempStream << withMe;
      beginPos = endPos + replaceMe.size();
      endPos = (unsigned int)in.find(replaceMe,beginPos);
    } 
    tempStream << in.substr(beginPos);
    return tempStream.str();
  }


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
  static std::vector<std::string> tokenize(const std::string& in, const std::string delims, const int maxTokens = 0, const bool useQuotes = false){
    std::vector<std::string> tokens;
    int numTokens = 0;
    bool inQuote = false;
    bool enoughTokens;

    unsigned int pos = 0; 
    int currentChar;
    std::ostringstream currentToken;
	
    pos = (unsigned int)in.find_first_not_of(delims);
    currentChar = (pos == std::string::npos)? -1: in[pos];
    enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));

    while (pos != std::string::npos && !enoughTokens){
	
      // get next token
      bool tokenDone = false;
      bool foundSlash = false;

      currentChar = (pos < in.size()) ? in[pos] : -1;
      while ((currentChar != -1) && !tokenDone){

	tokenDone = false;

	if (delims.find(currentChar) != std::string::npos && !inQuote) { // currentChar is a delim
	  pos ++;
	  break; // breaks out of while look
	}
			
	if (!useQuotes){
	  currentToken << char(currentChar);
	} else {

	  switch (currentChar){
	    case '\\' : // found a slash
	      if (foundSlash){
		currentToken << char(currentChar);
		foundSlash = false;
	      } else {
		foundSlash = true;
	      }
	      break;
	    case '\"' : // found a quote
	      if (foundSlash){ // found \"
		currentToken << char(currentChar);
		foundSlash = false;
	      } else { // found unescaped "
		if (inQuote){ // exiting a quote
		  // finish off current token
		  tokenDone = true;
		  inQuote = false;
		  //slurp off one additional delimeter if possible
		  if ( (pos+1 < in.size()) && (delims.find(in[pos+1]) != std::string::npos)){
		    pos++;
		  }

		} else { // entering a quote
		  // finish off current token
		  tokenDone = true;
		  inQuote = true;
		}
	      }
	      break;
	    default:
	      if (foundSlash){ // don't care about slashes except for above cases
		currentToken << '\\';
		foundSlash = false;
	      }
	      currentToken << char(currentChar);
	      break;
	  }
	}

	pos ++;
	currentChar = (pos < in.size() )? in[pos] : -1;
      } // end of getting a Token
		
      if (currentToken.str().size() > 0){ // if the token is something add to list
	tokens.push_back(currentToken.str());
	currentToken.str("");
	numTokens ++;
      }

      enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));
      if (enoughTokens){
	break;
      } else{
	pos = (unsigned int)in.find_first_not_of(delims,pos);
      }

    } // end of getting all tokens -- either EOL or max tokens reached

    if (enoughTokens && pos != std::string::npos){
      std::string lastToken = in.substr(pos);
      if (lastToken.size() > 0)
	tokens.push_back(lastToken); 
    }
 
    return tokens;
  } 

  static int parseDuration(std::string &duration)
  {
    int durationInt = 0;
    int t = 0;

    int len = (int)duration.length();
    for(int i=0; i < len; i++) {
      if( isdigit(duration[i]) ) {
	t = t * 10 + (duration[i] - '0');
      }
      else if(duration[i] == 'h' || duration[i] == 'H') {
	durationInt += (t * 60);
	t = 0;
      }
      else if(duration[i] == 'd' || duration[i] == 'D') {
	durationInt += (t * 1440);
	t = 0;
      }
      else if(duration[i] == 'w' || duration[i] == 'w') {
	durationInt += (t * 10080);
	t = 0;
      }
    }
    durationInt += t;
    return durationInt;
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
      ( c > 57 && c < 65) ||
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

/** url-encodes a string
 */
inline std::string url_encode(std::string text)
{
  char c;
  char hex[5];
  int i;
  std::string destination;
  for (i=0;  i < (int) text.size(); i++) {
    c = text[i];
    if (isAlphanumeric(c)) {
      destination+=c;
    } else if (isWhitespace(c)) {
      destination+='+';
    } else {
      destination+='%';
      sprintf(hex, "%-2.2X", c);
      destination.append(hex);
    }
  }
  return destination;
}


/** escape a string
 */
inline std::string escape(std::string text, char escaper)
{
  char c;
  int  i;
  std::string destination;
  for (i = 0; i < (int) text.size(); i++) {
    c = text[i];
    if (!isAlphanumeric(c))
      destination += escaper;
    destination += c;
  }
  return destination;
}

/** un-escape a string
 */
inline std::string unescape(std::string text, char escaper)
{
  char c;
  int  i;
  int  len = (int) text.size();
  std::string destination;
  for (i = 0; i < len; i++) {
    c = text[i];
    if (c == escaper) {
      if (i < len - 1)
	destination += text[++i];
      else
	// Should print an error 
	;
    } else {
      destination += c;
    }
  }
  return destination;
}

/** lookup for an un-escaped separator
 */
inline int unescape_lookup(std::string text, char escaper, char sep)
{
  char c;
  int  i;
  int  position = -1;
  for (i = 0; i < (int) text.size(); i++) {
    c = text[i];
    if (c == sep) {
      position = i;
      break;
    }
    if (c == escaper)
      i++;
  }
  return position;
}


// so we can keep a map with strings as the key
inline  bool operator < (const std::string &s1,const std::string &s2) { return (s1.compare(s2)<0);}


#endif // __TEXTUTILS_H__


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

