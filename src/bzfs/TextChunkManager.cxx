/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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
#pragma warning( 4: 4786)
#endif

/* interface header */
#include "TextChunkManager.h"

/* system implementation headers */
#include <fstream>

/* common implementation headers */
#include "global.h"


bool TextChunkManager::parseFile(const std::string &fileName, const std::string &chunkName)
{
  char buffer[MessageLen];
  std::ifstream in(fileName.c_str());
  if (!in) return false;

  StringVector strings;
  for(int i = 0; i < 20 && in.good(); i++) {
    in.getline(buffer,MessageLen);
    if(!in.fail()){ // really read something
      strings.push_back(buffer);
    }
  }

  if (strings.size() != 0) {
    theChunks.insert(StringChunkMap::value_type(chunkName,strings));
    chunkNames.push_back(chunkName);
  }
  return true;
}

const std::vector<std::string>* TextChunkManager::getTextChunk(const std::string &chunkName) const
{
  StringChunkMap::const_iterator it;
  it =theChunks.find(chunkName);
  if (it != theChunks.end()){
    return &it->second.getVector();
  } else {
    return NULL;
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

