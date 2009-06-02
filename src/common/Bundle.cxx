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

#ifdef _MSC_VER
#pragma warning(4:4786)
#endif

// interface header
#include "Bundle.h"

// system headers
#include <iostream>
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

  poStrm.getline(buffer, 1024);
  while (poStrm.good()) {
    std::string line = buffer;
    std::string data;
    TLineType type = parseLine(line, data);
    if (type == tMSGID) {
      if (untranslated.length() > 0) {
	mappings.erase(untranslated);
	mappings.insert(std::pair<std::string, std::string>(untranslated, translated));
      }
      untranslated = data;
      translated.resize(0);
    } else if (type == tMSGSTR) {
      if (untranslated.length() > 0)
	translated = data;
    } else if (type == tAPPEND) {
      if (untranslated.length() > 0)
	translated += data;
    } else if (type == tERROR) {

    }


    poStrm.getline(buffer, 1024);
  }

  if ((untranslated.length() > 0) && (translated.length() > 0)) {
    mappings.erase(untranslated);
    mappings.insert(std::pair<std::string, std::string>(untranslated, translated));
  }
}

Bundle::TLineType Bundle::parseLine(const std::string &line, std::string &data) const
{
  int startPos, endPos;
  TLineType type;

  data.resize(0);
  startPos = line.find_first_not_of("\t \r\n");

  if ((startPos < 0) || (line.at(startPos) == '#')) {
    return tCOMMENT;
  } else if (line.at(startPos) == '"') {
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
    data = line.substr(startPos, endPos-startPos);
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
      if (unmapped.find(key) == unmapped.end()) {
	unmapped.insert(key);
	logDebugMessage(1, "Unmapped Locale String: %s\n", stripAnsiCodes(key.c_str()));
      }
    }
    return key;
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
    messageOut += messageIn.substr(startPos, lCurlyPos - startPos);
    int rCurlyPos = messageIn.find_first_of("}", lCurlyPos++);
    if (rCurlyPos < 0) {
      messageOut += messageIn.substr(lCurlyPos);
      return messageOut;
    }
    std::string numStr = messageIn.substr(lCurlyPos, rCurlyPos-lCurlyPos);
    int num;
    if (sscanf(numStr.c_str(), "%d", &num) != 1) {
      messageOut += messageIn.substr(lCurlyPos, rCurlyPos-lCurlyPos);
    } else {
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
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
