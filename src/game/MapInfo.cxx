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

#include "common.h"

#include "MapInfo.h"

#include <ctype.h>
#include <string>
#include <vector>
using std::string;
using std::string;

#include "Pack.h"


MapInfo::MapInfo()
{
}


MapInfo::MapInfo(const InfoVec& lines)
{
  infoVec = lines;
  finalize();
}


MapInfo::~MapInfo()
{
}


void MapInfo::clear()
{
  infoVec.clear();  
  infoMap.clear();  
}


void MapInfo::setLines(const InfoVec& lines)
{
  clear();
  infoVec = lines;
  finalize();
}


static bool ParseKeyValue(const string& line, string& key, string& value)
{
  const char* c = line.c_str();

  // find the start
  while ((*c != 0) && isspace(*c)) { c++; }
  const char* keyStart = c;

  // find the ':' at the end of the key  
  while ((*c != 0) && (*c != ':') && !isspace(*c)) { c++; }
  if (*c != ':') {
    return false;
  }
  const char* keyEnd = c;

  c++; // skip the ':'
  while ((*c != 0) && isspace(*c)) { c++; }
  const char* valueStart = c;
  const char* valueEnd = c;

  // discard trailing whitespace
  while (*c != 0) {
    if (!isspace(*c)) {
      valueEnd = c;
    }
    c++;
  }
  valueEnd++;

  // assign the strings
  key.assign(keyStart,     keyEnd - keyStart);
  value.assign(valueStart, valueEnd - valueStart);  

  if (key.empty() || value.empty()) {
    key.clear();
    value.clear();
    return false;
  }  

  return true;
}


void MapInfo::finalize()
{
  infoMap.clear();
  for (size_t i = 0; i < infoVec.size(); i++) {
    string key, value;
    if (ParseKeyValue(infoVec[i], key, value)) {
      infoMap[key].push_back(value);
    }
  }
}


const std::vector<std::string>* MapInfo::getValue(const std::string& key) const
{
  InfoMap::const_iterator it = infoMap.find(key);
  if (it == infoMap.end()) {
    return NULL;
  }
  return &(it->second);
}


int MapInfo::packSize() const
{
  int fullSize = 0;
  fullSize += sizeof(uint32_t);
  for (uint32_t i = 0; i < infoVec.size(); i++) {
    fullSize += nboStdStringPackSize(infoVec[i]);
  }
  return fullSize;
}


void* MapInfo::pack(void* buf) const
{
  uint32_t count = (uint32_t)infoVec.size();
  buf = nboPackUInt(buf, count);
  for (uint32_t i = 0; i < count; i++) {
    buf = nboPackStdString(buf, infoVec[i]);
  }
  return buf;
}


void* MapInfo::unpack(void* buf)
{
  infoVec.clear();
  uint32_t count;
  buf = nboUnpackUInt(buf, count);
  for (uint32_t i = 0; i < count; i++) {
    std::string line;
    buf = nboUnpackStdString(buf, line);
    infoVec.push_back(line);
  }
  finalize();
  return buf;
}


void MapInfo::print(std::ostream& out, const std::string& indent) const
{
  if (infoVec.empty()) {
    return;
  }
  out << indent << "info" << std::endl;
  for (unsigned i = 0; i < infoVec.size(); i++) {
    out << indent << infoVec[i] << std::endl;
  }
  out << indent << "end" << std::endl << std::endl;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
