/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include "plugin_utils.h"

const char* bzu_GetTeamName(bz_eTeamType team)
{
  switch (team) {

  case eRedTeam:
    return "Red";

  case eGreenTeam:
    return "Green";

  case eBlueTeam:
    return "Blue";

  case ePurpleTeam:
    return "Purple";

  case eRogueTeam:
    return "Rogue";

  case eObservers:
    return "Observer";

  case eRabbitTeam:
    return "Rabbit";

  case eHunterTeam:
    return "Hunter";

  default:
    break;
  }

  return "Unknown";
}

std::string printTime(bz_Time *ts, const char* _timezone)
{
  std::string time;
  appendTime(time,ts,_timezone);
  return time;
}

//Date: Mon, 23 Jun 2008 17:50:22 GMT

void appendTime(std::string & text, bz_Time *ts, const char* _timezone)
{
  switch(ts->dayofweek) {
  case 1:
    text += "Mon";
    break;
  case 2:
    text += "Tue";
    break;
  case 3:
    text += "Wed";
    break;
  case 4:
    text += "Thu";
    break;
  case 5:
    text += "Fri";
    break;
  case 6:
    text += "Sat";
    break;
  case 0:
    text += "Sun";
    break;
  }

  text += format(", %d ",ts->day);

  switch(ts->month) {
  case 0:
    text += "Jan";
    break;
  case 1:
    text += "Feb";
    break;
  case 2:
    text += "Mar";
    break;
  case 3:
    text += "Apr";
    break;
  case 4:
    text += "May";
    break;
  case 5:
    text += "Jun";
    break;
  case 6:
    text += "Jul";
    break;
  case 7:
    text += "Aug";
    break;
  case 8:
    text += "Sep";
    break;
  case 9:
    text += "Oct";
    break;
  case 10:
    text += "Nov";
    break;
  case 11:
    text += "Dec";
    break;
  }

  text += format(" %d %d:%d:%d ",ts->year,ts->hour,ts->minute,ts->second);
  if (_timezone)
    text += _timezone;
  else
    text += "GMT";
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

const std::string& tolower(const std::string& s, std::string& dest)
{
  for (std::string::const_iterator i=s.begin(), end=s.end(); i!=end; ++i)
    dest += ::tolower(*i);

  return dest;
}

const std::string& toupper(const std::string& s, std::string& dest)
{
  for (std::string::const_iterator i=s.begin(), end=s.end(); i!=end; ++i)
    dest += ::toupper(*i);

  return dest;
}

const std::string& tolower(const char* s, std::string& dest)
{
  if (!s)
    return dest;

  for (size_t i =0,end = strlen(s); i < end; i++)
    dest += ::tolower(s[i]);

  return dest;
}

const std::string& toupper(const char* s, std::string& dest)
{
  if (!s)
    return dest;

  for (size_t i =0,end = strlen(s); i < end; i++)
    dest += ::toupper(s[i]);

  return dest;
}

const std::string& makelower(std::string& s)
{
  for (std::string::iterator i=s.begin(), end=s.end(); i!=end; ++i)
    *i = ::tolower(*i);

  return s;
}

const std::string& makeupper(std::string& s)
{
  for (std::string::iterator i=s.begin(), end=s.end(); i!=end; ++i)
    *i = ::toupper(*i);

  return s;
}

std::string format(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  char temp[2048];
  vsprintf(temp,fmt, args);
  std::string result = temp;
  va_end(args);
  return result;
}

std::vector<std::string> tokenize(const std::string& in, const std::string &delims,
				  const int maxTokens, const bool useQuotes, size_t offset)
{
  std::vector<std::string> tokens;
  int numTokens = 0;
  bool inQuote = false;

  std::ostringstream currentToken;

  std::string::size_type pos = in.find_first_not_of(delims,offset);
  int currentChar  = (pos == std::string::npos) ? -1 : in[pos];
  bool enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));

  while (pos != std::string::npos && !enoughTokens) {

    // get next token
    bool tokenDone = false;
    bool foundSlash = false;

    currentChar = (pos < in.size()) ? in[pos] : -1;
    while ((currentChar != -1) && !tokenDone) {

      tokenDone = false;

      if (delims.find(currentChar) != std::string::npos && !inQuote) { // currentChar is a delim
	pos ++;
	break; // breaks out of while loop
      }

      if (!useQuotes) {
	currentToken << char(currentChar);
      } else {

	switch (currentChar) {
	case '\\' : // found a backslash
	  if (foundSlash) {
	    currentToken << char(currentChar);
	    foundSlash = false;
	  } else {
	    foundSlash = true;
	  }
	  break;
	case '\"' : // found a quote
	  if (foundSlash) { // found \"
	    currentToken << char(currentChar);
	    foundSlash = false;
	  } else { // found unescaped "
	    if (inQuote) { // exiting a quote
	      // finish off current token
	      tokenDone = true;
	      inQuote = false;
	      //slurp off one additional delimeter if possible
	      if (pos+1 < in.size() &&
		  delims.find(in[pos+1]) != std::string::npos) {
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
	  if (foundSlash) { // don't care about slashes except for above cases
	    currentToken << '\\';
	    foundSlash = false;
	  }
	  currentToken << char(currentChar);
	  break;
	}
      }

      pos++;
      currentChar = (pos < in.size()) ? in[pos] : -1;
    } // end of getting a Token

    if (currentToken.str().size() > 0) { // if the token is something add to list
      tokens.push_back(currentToken.str());
      currentToken.str("");
      numTokens ++;
    }

    enoughTokens = (maxTokens && (numTokens >= (maxTokens-1)));
    if (enoughTokens)
      break;
    else
      pos = in.find_first_not_of(delims,pos);

  } // end of getting all tokens -- either EOL or max tokens reached

  if (enoughTokens && pos != std::string::npos) {
    std::string lastToken = in.substr(pos);
    if (lastToken.size() > 0)
      tokens.push_back(lastToken);
  }

  return tokens;
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

std::string url_encode(const std::string &text)
{
  char hex[5];
  std::string destination;
  for (int i=0;  i < (int) text.size(); i++) {
    char c = text[i];
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

std::string url_decode(const std::string &text)
{
  std::string destination;

  std::string::const_iterator itr = text.begin();
  while (itr != text.end()) {
    if (*itr != '%' && *itr != '+')
      destination += *itr++;
    else if (*itr == '+') {
      destination += " ";
      itr++;
    }
    else {
      char hex[5] = "0x00";

      itr++;
      if (itr == text.end())
	return destination;

      hex[2] = *itr;

      itr++;
      if (itr == text.end())
	return destination;

      hex[3] = *itr;

      unsigned int val = 0;
      sscanf(hex,"%x",&val);
      if (val != 0)
	destination += (char)val;
      itr++;
    }
  }
  return destination;
}

size_t find_first_substr(const std::string &findin, const std::string findwhat, size_t offset)
{
  if (findwhat.size()) {
    for (size_t f = offset; f < findin.size(); f++) {
      if (findin[f] == findwhat[0]) {
	size_t start = f;
	for (size_t w = 1; w < findwhat.size(); w++) {
	  if (f+w > findin.size())
	    return std::string::npos;
	  if (findin[f+w] != findwhat[w]) {
	    f+=w;
	    w = findwhat.size();
	  }
	}
	if (start == f)
	  return f;
      }
    }
  }
  return std::string::npos;
}

std::string getStringRange ( const std::string &find, size_t start, size_t end )
{
  std::string ret;

  if (end <= start || start > find.size() || end > find.size())
    return ret;

  for ( size_t p = start; p <= end; p++)
    ret += find[p];

  return ret;
}


void trimLeadingWhitespace(std::string &text)
{
  for(size_t s = 0; s < text.size(); s++) {
    if (!isWhitespace(text[s])) {
      if (s)
	text.erase(text.begin()+s-1);
      return;
    }
  }
}

std::string trimLeadingWhitespace(const std::string &text)
{
  std::string s = text;
  trimLeadingWhitespace(s);
  return s;
}

std::vector<std::string> perms;

const std::vector<std::string> bzu_standardPerms (void)
{
  if (perms.empty()){
    perms.push_back("actionMessage");
    perms.push_back("adminMessageReceive");
    perms.push_back("adminMessageSend");
    perms.push_back("antiban");
    perms.push_back("antikick");
    perms.push_back("antikill");
    perms.push_back("antipoll");
    perms.push_back("antipollban");
    perms.push_back("antipollkick");
    perms.push_back("antipollkill");
    perms.push_back("ban");
    perms.push_back("banlist");
    perms.push_back("countdown");
    perms.push_back("date");
    perms.push_back("endGame");
    perms.push_back("flagHistory");
    perms.push_back("flagMaster");
    perms.push_back("flagMod");
    perms.push_back("hideAdmin");
    perms.push_back("idleStats");
    perms.push_back("info");
    perms.push_back("jitter_warn");
    perms.push_back("kick");
    perms.push_back("kill");
    perms.push_back("lagStats");
    perms.push_back("lagwarn");
    perms.push_back("listPlugins");
    perms.push_back("listPerms");
    perms.push_back("masterBan");
    perms.push_back("modCount");
    perms.push_back("mute");
    perms.push_back("packetlosswarn");
    perms.push_back("playerList");
    perms.push_back("plugins");
    perms.push_back("poll");
    perms.push_back("pollBan");
    perms.push_back("pollKick");
    perms.push_back("pollKill");
    perms.push_back("pollSet");
    perms.push_back("pollFlagReset");
    perms.push_back("privateMessage");
    perms.push_back("record");
    perms.push_back("rejoin");
    perms.push_back("removePerms");
    perms.push_back("replay");
    perms.push_back("report");
    perms.push_back("say");
    perms.push_back("sendHelp");
    perms.push_back("setAll");
    perms.push_back("setPerms");
    perms.push_back("setVar");
    perms.push_back("showAdmin");
    perms.push_back("showOthers");
    perms.push_back("shortBan");
    perms.push_back("shutdownServer");
    perms.push_back("spawn");
    perms.push_back("superKill");
    perms.push_back("talk");
    perms.push_back("unban");
    perms.push_back("unmute");
    perms.push_back("veto");
    perms.push_back("viewReports");
    perms.push_back("vote");
  }
  return perms;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
