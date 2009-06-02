/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// BZFlag common header
#include "common.h"

// interface header
#include "TextUtils.h"

// system headers
#include <string.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdarg.h>
#include <vector>
#include <stdio.h>
#include <functional>
#include <locale>

#ifndef _TEXT_UTIL_NO_REGEX_
// common headers
#include "bzregex.h"
#include "AnsiCodes.h"
#endif //_TEXT_UTIL_NO_REGEX_

namespace TextUtils
{
  std::string vformat(const char* fmt, va_list args) {
    const int fixedbs = 8192;
    char buffer[fixedbs] = {0};

    if (!fmt)
      return buffer;

    const int bs = vsnprintf(buffer, fixedbs, fmt, args) + 1;
    if (bs > fixedbs) {
      char *bufp = new char[bs];
      vsnprintf(bufp, bs, fmt, args);
      std::string ret = bufp;
      delete[] bufp;
      return ret;
    }

    return buffer;
  }


  std::string format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::string result = vformat(fmt, args);
    va_end(args);
    return result;
  }


  std::wstring convert_to_wide(const std::string& string)
  {
#ifdef _WIN32    // Get the required size for the new array and allocate the memory for it
    int neededSize = MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, 0, 0);
    wchar_t* wideCharString = new wchar_t[neededSize];

    MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, wideCharString, neededSize);

    std::wstring wideString(wideCharString);
    delete[] wideCharString;
    return wideString;
#else
    std::wstring out;
    wchar_t* buf = new wchar_t[string.size() + 1];
    mbstowcs(buf, string.c_str(), string.size());
    buf[string.size()] = 0;
    out = buf;
    delete[] buf;
    return out;
#endif // _WIN32
  }

  std::string replace_all(const std::string& in, const std::string& replaceMe, const std::string& withMe)
  {
    std::string result;
    std::string::size_type beginPos = 0;
    std::string::size_type endPos = 0;
    std::ostringstream tempStream;

    endPos = in.find(replaceMe);
    if (endPos == std::string::npos)
      return in; // can't find anything to replace
    if (replaceMe.empty()) return in; // can't replace nothing with something -- can do reverse

    while (endPos != std::string::npos) {
      // push the  part up to
      tempStream << in.substr(beginPos,endPos-beginPos);
      tempStream << withMe;
      beginPos = endPos + replaceMe.size();
      endPos = in.find(replaceMe,beginPos);
    }
    tempStream << in.substr(beginPos);
    return tempStream.str();
  }


  std::string no_whitespace(const std::string &s)
  {
    const int sourcesize = (int)s.size();

    int count = 0, i = 0, j = 0;
    for (i = 0; i < sourcesize; i++)
      if (!isWhitespace(s[i]))
	count++;

    // create result string of correct size
    std::string result(count, ' ');

    for (i = 0, j = 0; i < sourcesize; i++)
      if (!isWhitespace(s[i]))
	result[j++] = s[i];

    return result;
  }


  std::vector<std::string> tokenize(const std::string& in,
                                    const std::string &delims,
                                    const int maxTokens,
                                    const bool useQuotes,
                                    const bool useEscapes)
  {
    std::vector<std::string> tokens;
    int numTokens = 0;
    bool inQuote = false;

    std::ostringstream currentToken;

    const std::string::size_type len = in.size();
    std::string::size_type pos = in.find_first_not_of(delims);

    int currentChar  = (pos == std::string::npos) ? -1 : in[pos];
    bool enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));

    while (pos < len && pos != std::string::npos && !enoughTokens) {

      // get next token
      bool tokenDone = false;
      bool foundSlash = false;

      currentChar = (pos < len) ? in[pos] : -1;
      while ((currentChar != -1) && !tokenDone){

	tokenDone = false;

        if (delims.find(currentChar) != std::string::npos) {
          // currentChar is a delim
          if (foundSlash && useEscapes) {
            currentToken << char(currentChar);
            foundSlash = false;
            pos++;
            currentChar = (pos < len) ? in[pos] : -1;
            continue;
          }
          else if (!inQuote) {
            pos++;
            break; // breaks out of inner while loop
          }
        }

	if (!useQuotes){
	  currentToken << char(currentChar);
	}
	else {
	  switch (currentChar){
	    case '\\': { // found a backslash
	      if (foundSlash){
		currentToken << char(currentChar);
		foundSlash = false;
	      } else {
		foundSlash = true;
	      }
	      break;
            }
	    case '\"': { // found a quote
	      if (foundSlash){ // found \"
		currentToken << char(currentChar);
		foundSlash = false;
	      }
	      else { // found unescaped "
		if (inQuote){ // exiting a quote
		  // finish off current token
		  tokenDone = true;
		  inQuote = false;
		  //slurp off one additional delimeter if possible
		  if (pos+1 < len &&
		      delims.find(in[pos+1]) != std::string::npos) {
		    pos++;
		  }
		}
		else { // entering a quote
		  // finish off current token
		  tokenDone = true;
		  inQuote = true;
		}
	      }
	      break;
            }
	    default: {
	      if (foundSlash) {
	        if (!useEscapes) {
	          currentToken << '\\';
                }
		foundSlash = false;
	      }
	      currentToken << char(currentChar);
	      break;
            }
	  }
	}

	pos++;
	currentChar = (pos < len) ? in[pos] : -1;
      } // end of getting a Token

      if (currentToken.str().size() > 0){ // if the token is something add to list
	tokens.push_back(currentToken.str());
	currentToken.str("");
	numTokens ++;
      }

      enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));
      if ((pos < len) && (pos != std::string::npos)) {
	pos = in.find_first_not_of(delims,pos);
      }

    } // end of getting all tokens -- either EOL or max tokens reached

    if (enoughTokens && pos != std::string::npos) {
      std::string lastToken = in.substr(pos);
      if (lastToken.size() > 0)
	tokens.push_back(lastToken);
    }

    return tokens;
  }

  bool parseDuration(const char *duration, int &durationInt)
  {
#ifndef _TEXT_UTIL_NO_REGEX_
    if (strcasecmp(duration,"short") == 0
	|| strcasecmp(duration,"default") == 0) {
      durationInt = -1;
      return true;
    }
    if (strcasecmp(duration,"forever") == 0
	|| strcasecmp(duration,"max") == 0) {
      durationInt = 0;
      return true;
    }

    regex_t preg;
    int res = regcomp(&preg, "^([[:digit:]]+[hwdm]?)+$",
		      REG_ICASE | REG_NOSUB | REG_EXTENDED);
    res = regexec(&preg, duration, 0, NULL, 0);
    regfree(&preg);
    if (res == REG_NOMATCH)
      return false;

    durationInt = 0;
    int t = 0;
    int len = (int)strlen(duration);
    for (int i = 0; i < len; i++) {
      if (isdigit(duration[i])) {
	t = t * 10 + (duration[i] - '0');
      } else if(duration[i] == 'h' || duration[i] == 'H') {
	durationInt += (t * 60);
	t = 0;
      } else if(duration[i] == 'd' || duration[i] == 'D') {
	durationInt += (t * 1440);
	t = 0;
      } else if(duration[i] == 'w' || duration[i] == 'W') {
	durationInt += (t * 10080);
	t = 0;
      } else if(duration[i] == 'm' || duration[i] == 'M') {
	durationInt += (t);
	t = 0;
      }
    }
    durationInt += t;
    return true;
#else
    return false;
#endif //_TEXT_UTIL_NO_REGEX_
  }

  std::string url_encode(const std::string &text)
  {
    char hex[5];
    std::string destination;
    for (size_t i=0;  i < text.size(); ++i) {
      unsigned char c = text[i];
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


  std::string escape(const std::string &text, char escaper)
  {
    std::string destination;
    for (int i = 0; i < (int) text.size(); i++) {
      char c = text[i];
      if (!isAlphanumeric(c))
	destination += escaper;
      destination += c;
    }
    return destination;
  }

  std::string unescape(const std::string &text, char escaper)
  {
    const int len = (int) text.size();
    std::string destination;
    for (int i = 0; i < len; i++) {
      char c = text[i];
      if (c == escaper) {
	if (i < len - 1)
	  destination += text[++i];
	// Otherwise should print an error
      } else {
	destination += c;
      }
    }
    return destination;
  }

  int unescape_lookup(const std::string &text, char escaper, char sep)
  {
    int position = -1;
    for (int i = 0; i < (int) text.size(); i++) {
      char c = text[i];
      if (c == sep) {
	position = i;
	break;
      }
      if (c == escaper)
	i++;
    }
    return position;
  }

  static int expandEscName(const char* source, std::string& outLine)
  {
    const char* c = source;
    while ((*c != 0) && (*c != ')')) { c++; }
    if (*c != ')') {
      return 0;
    }
    const std::string key(source, c - source);
    const int len = key.size() + 2; // 2 for the () chars
    if (key == "backslash")   { outLine.push_back('\\');   return len; }
    if (key == "newline")     { outLine.push_back('\n');   return len; }
    if (key == "escape")      { outLine.push_back('\033'); return len; }
    if (key == "space")       { outLine.push_back(' ');    return len; }
    if (key == "red")         { outLine += ANSI_STR_FG_RED;       return len; }
    if (key == "green")       { outLine += ANSI_STR_FG_GREEN;     return len; }
    if (key == "blue")        { outLine += ANSI_STR_FG_BLUE;      return len; }
    if (key == "yellow")      { outLine += ANSI_STR_FG_YELLOW;    return len; }
    if (key == "purple")      { outLine += ANSI_STR_FG_MAGENTA;   return len; }
    if (key == "cyan")        { outLine += ANSI_STR_FG_CYAN;      return len; }
    if (key == "orange")      { outLine += ANSI_STR_FG_ORANGE;    return len; }
    if (key == "white")       { outLine += ANSI_STR_FG_WHITE;     return len; }
    if (key == "black")       { outLine += ANSI_STR_FG_BLACK;     return len; }
    if (key == "bright")      { outLine += ANSI_STR_BRIGHT;       return len; }
    if (key == "dim")         { outLine += ANSI_STR_DIM;          return len; }
    if (key == "blink")       { outLine += ANSI_STR_PULSATING;    return len; }
    if (key == "blinkOff")    { outLine += ANSI_STR_NO_PULSATE;   return len; }
    if (key == "under")       { outLine += ANSI_STR_UNDERLINE;    return len; }
    if (key == "underOff")    { outLine += ANSI_STR_NO_UNDERLINE; return len; }
    if (key == "reverse")     { outLine += ANSI_STR_REVERSE;      return len; }
    if (key == "reverseOff")  { outLine += ANSI_STR_NO_REVERSE;   return len; }
    if (key == "reset")       { outLine += ANSI_STR_RESET_FINAL;  return len; }
    if (key == "resetBright") { outLine += ANSI_STR_RESET;        return len; }

    outLine.push_back('\\');

    return 0;
  }

  std::string unescape_colors(const std::string& source)
  {
    // looking for:
    //  \\ - backslash
    //  \n - newline
    //  \e - escape character
    std::string out;
    for (const char* c = source.c_str(); *c != 0; c++) {
      if (*c != '\\') {
        out.push_back(*c);
      }
      else {
        switch (*(c + 1)) {
          case '\\': { out.push_back('\\');   c++; break; }
          case 'n':  { out.push_back('\n');   c++; break; }
          case 'e':  { out.push_back('\033'); c++; break; }
          case 's':  { out.push_back(' ');    c++; break; }
          case 'r':  { out += ANSI_STR_FG_RED;       c++; break; } // not carriage return
          case 'g':  { out += ANSI_STR_FG_GREEN;     c++; break; }
          case 'b':  { out += ANSI_STR_FG_BLUE;      c++; break; } // not backspace
          case 'y':  { out += ANSI_STR_FG_YELLOW;    c++; break; }
          case 'p':  { out += ANSI_STR_FG_MAGENTA;   c++; break; }
          case 'c':  { out += ANSI_STR_FG_CYAN;      c++; break; }
          case 'o':  { out += ANSI_STR_FG_ORANGE;    c++; break; }
          case 'w':  { out += ANSI_STR_FG_WHITE;     c++; break; }
          case 'd':  { out += ANSI_STR_FG_BLACK;     c++; break; }
          case '+':  { out += ANSI_STR_BRIGHT;       c++; break; }
          case '-':  { out += ANSI_STR_DIM;          c++; break; }
          case '*':  { out += ANSI_STR_PULSATING;    c++; break; }
          case '/':  { out += ANSI_STR_NO_PULSATE;   c++; break; }
          case '_':  { out += ANSI_STR_UNDERLINE;    c++; break; }
          case '~':  { out += ANSI_STR_NO_UNDERLINE; c++; break; }
          case '[':  { out += ANSI_STR_REVERSE;      c++; break; }
          case ']':  { out += ANSI_STR_NO_REVERSE;   c++; break; }
          case '!':  { out += ANSI_STR_RESET_FINAL;  c++; break; }
          case '#':  { out += ANSI_STR_RESET;        c++; break; }
          case '(':  {
            c += expandEscName(c + 2, out);
            break;
          }
          default: {
            out.push_back('\\');
          }
        }
      }
    }

    return out;
  }

  // return a copy of a string, truncated to specified length,
  //    make last char a '`' if truncation took place
  std::string str_trunc_continued (const std::string &text, int len)
  {
    std::string retstr = std::string (text, 0, len);
    if ( retstr.size() == (unsigned int)len  )
      retstr[len-1] = '~';
    return retstr;
  }

}


std::string TextUtils::ltrim(const std::string& s)
{

  std::string::size_type i;
  for (i = 0; i < s.size(); i++) {
    if (!isWhitespace(s[i])) {
      return s.substr(i);
    }
  }
  return s;
}


std::string TextUtils::rtrim(const std::string& s)
{
  std::string::size_type i;
  for (i = s.size(); i != 0; i--) {
    if (!isWhitespace(s[i - 1])) {
      return s.substr(0, i);
    }
  }
  return s;
}


std::string TextUtils::trim(const std::string& s)
{
  return ltrim(rtrim(s));
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
