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

#ifdef _MSC_VER
#pragma warning(4:4786)
#endif

// interface header
#include "Bundle.h"

// system headers
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>

// local implementation headers
#include "StateDatabase.h"
#include "AnsiCodes.h"

Bundle::Bundle(const Bundle *pBundle)
{
  if (pBundle == NULL)
    return;

  mappings = pBundle->mappings;
}

void Bundle::load(const std::string &path)
{
  std::string untranslated;
  std::string translated;
  char buffer[1024];

  std::ifstream poStrm(path.c_str());
  if (!poStrm.good())
    return;

  poStrm.getline(buffer,1024);
  while (poStrm.good()) {
    std::string line = buffer;
    std::string data;
    TLineType type = parseLine(line, data);
    if (type == tMSGID) {
      if (untranslated.length() > 0) {
	mappings.erase(untranslated);
	ensureNormalText(translated);
	mappings.insert(std::pair<std::string,std::string>(untranslated, translated));
      }
      untranslated = data;
      translated.resize(0);
    }
    else if (type == tMSGSTR) {
      if (untranslated.length() > 0)
	translated = data;
    }
    else if (type == tAPPEND) {
      if (untranslated.length() > 0)
	translated += data;
    }
    else if (type == tERROR) {

    }


    poStrm.getline(buffer,1024);
  }

  if ((untranslated.length() > 0) && (translated.length() > 0)) {
    mappings.erase(untranslated);
    ensureNormalText(translated);
    mappings.insert(std::pair<std::string,std::string>(untranslated, translated));
  }
}

Bundle::TLineType Bundle::parseLine(const std::string &line, std::string &data) const
{
  int startPos, endPos;
  TLineType type;

  data.resize(0);
  startPos = line.find_first_not_of("\t \r\n");

  if ((startPos < 0) || (line.at(startPos) == '#'))
    return tCOMMENT;

  else if (line.at(startPos) == '"') {
    endPos = line.find_first_of('"', startPos+1);
    if (endPos < 0)
      endPos = line.length();
    data = line.substr(startPos+1, endPos-startPos-1);
    return tAPPEND;
  }

  endPos = line.find_first_of("\t \r\n\"");
  if (endPos < 0)
    endPos = line.length();
  std::string key = line.substr(startPos, endPos-startPos);
  if (key == "msgid")
    type = tMSGID;
  else if (key == "msgstr")
    type = tMSGSTR;
  else
    return tERROR;

  startPos = line.find_first_of('"', endPos + 1);
  if (startPos >= 0) {
    startPos++;
    endPos = line.find_first_of('"', startPos);
    if (endPos < 0)
      endPos = line.length();
    data = line.substr( startPos, endPos-startPos);
  }
  return type;
}

#include <set>
static std::set<std::string> unmapped;

std::string Bundle::getLocalString(const std::string &key) const
{
  if (key == "") return key;
  BundleStringMap::const_iterator it = mappings.find(key);
  if (it != mappings.end()) {
    return it->second;
  } else {
    if (BZDB.getDebug()) {
      if (unmapped.find( key ) == unmapped.end( )) {
        unmapped.insert( key );
    std::string stripped = stripAnsiCodes (key);
	std::string debugStr = "Unmapped Locale String: " + stripped + "\n";
	DEBUG1("%s", debugStr.c_str());
      }
    }
    return key;
  }
}

void Bundle::ensureNormalText(std::string &msg)
{
// This is an ugly hack. If you don't like it fix it.
// BZFlag's font bitmaps don't contain letters with accents, so strip them here
// Would be nice if some kind sole added them.

  for (std::string::size_type i = 0; i < msg.length(); i++) {
    char c = msg.at(i);
    switch (c) {
      case 'â':
      case 'à':
      case 'á':
      case 'ã':
      case 'Œ':
      case 'Š':
	msg[i] = 'a';
      break;
      case 'å':
	msg[i] = 'a';
	i++;
	msg.insert(i, 1, 'a');
      break;
      case 'ä':
      case 'æ':
	msg[i] = 'a';
	i++;
	msg.insert(i, 1, 'e');
      break;
      case 'Â':
      case '':
      case '€':
	msg[i] = 'A';
      break;
      case 'Ä':
      case 'Æ':
	msg[i] = 'A';
	i++;
	msg.insert(i, 1, 'e');
      break;
      case 'Å':
	msg[i] = 'A';
	i++;
	msg.insert(i, 1, 'a');
      break;
      case 'ç':
	msg[i] = 'c';
      break;
      case 'é':
      case 'è':
      case 'ê':
      case 'ë':
	msg[i] = 'e';
      break;
      case 'î':
      case 'ï':
      case 'í':
      case '†':
	msg[i] = 'i';
      break;
      case 'ô':
      case 'ó':
      case 'õ':
      case 'š':
	msg[i] = 'o';
      break;
      case 'ö':
      case 'ø':
	msg[i] = 'o';
	i++;
	msg.insert(i, 1, 'e');
      break;
      case '…':
	msg[i] = 'O';
      break;
      case 'Ö':
      case 'Ø':
	msg[i] = 'O';
	i++;
	msg.insert(i, 1, 'e');
      break;
      case 'û':
      case 'ù':
      case 'ú':
	msg[i] = 'u';
      break;
      case 'ü':
	msg[i] = 'u';
	i++;
	msg.insert(i, 1, 'e');
      break;
      case 'Ü':
	msg[i] = 'U';
	i++;
	msg.insert(i, 1, 'e');
      break;
      case 'ñ':
	msg[i] = 'n';
      break;
      case 'Ÿ':
	msg[i] = 'Y';
      break;
      case 'ß':
	msg[i] = 's';
	i++;
	msg.insert(i, 1, 's');
      break;
      case '¿':
      case '¡':
	msg[i] = ' ';
      break;

      default: // A temporary patch, to catch untranslated chars.. To be removed eventually
	if (((c >= 'A') && (c <= 'Z'))
	    || ((c >= 'a') && (c <= 'z'))
	    || ((c >= '0') && (c <= '9'))
	    || (c == '}') || (c == '{') || (c == ' ')
	    || (c == ':') || (c == '/') || (c == '-')
	    || (c == ',') || (c == '&') || (c == '?')
	    || (c == '<') || (c == '>') || (c == '.')
	    || (c == '(') || (c == ')') || (c == '%')
	    || (c == '!') || (c == '+') || (c == '-')
	    || (c == '$') || (c == ';') || (c == '@')
	    || (c == '[') || (c == ']')
	    || (c == '=') || (c == '\''))
	;
	else {
		msg = std::string("unsupported char:") + c;
		return;
	}
      break;

    }
  }
}


std::string Bundle::formatMessage(const std::string &key, const std::vector<std::string> *parms) const
{
  std::string messageIn = getLocalString(key);
  std::string messageOut;

  if (!parms || (parms->size() == 0))
    return messageIn;

  int parmCnt = parms->size();
  int startPos = 0;
  int lCurlyPos = messageIn.find_first_of("{");

  while (lCurlyPos >= 0) {
    messageOut += messageIn.substr( startPos, lCurlyPos - startPos);
    int rCurlyPos = messageIn.find_first_of("}", lCurlyPos++);
    if (rCurlyPos < 0) {
      messageOut += messageIn.substr(lCurlyPos);
      return messageOut;
    }
    std::string numStr = messageIn.substr(lCurlyPos, rCurlyPos-lCurlyPos);
    int num;
    if (sscanf(numStr.c_str(), "%d", &num) != 1)
      messageOut += messageIn.substr(lCurlyPos, rCurlyPos-lCurlyPos);
    else {
      num--;
      if ((num >= 0) && (num < parmCnt))
	messageOut += (*parms)[num];
    }
    startPos = rCurlyPos+1;
    lCurlyPos = messageIn.find_first_of("{", startPos);
  }
  messageOut += messageIn.substr(startPos);
  return messageOut;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

