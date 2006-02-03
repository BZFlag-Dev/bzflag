/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
#pragma warning( 4: 4786)
#endif

/* interface header */
#include "TextChunkManager.h"

/* system implementation headers */
#include <fstream>

/* common implementation headers */
#include "global.h"


/******************************************************************************/

TextChunk::TextChunk()
{
  // for the <map>[] operator
}


TextChunk::TextChunk(const std::string& _fileName)
{
  fileName = _fileName;
  theVector = parse();
  return;
}


TextChunk::TextChunk(const TextChunk& tc)
{
  fileName = tc.fileName;
  theVector = tc.theVector;
  return;
}


StringVector TextChunk::parse()
{
  StringVector strings;
  char buffer[MessageLen];
  std::ifstream in(fileName.c_str());
  if (in) {
    for(int i = 0; (i < 50) && in.good(); i++) {
      in.getline(buffer, MessageLen);
      if(!in.fail()){ // really read something
	strings.push_back(buffer);
      }
    }
  }
  return strings;
}


bool TextChunk::reload()
{
  StringVector newVec = parse();
  if (newVec.size() > 0) {
    theVector = newVec;
  }
  return (theVector.size() > 0);
}


size_t TextChunk::size() const
{
  return theVector.size();
}


const StringVector& TextChunk::getVector() const
{
  return theVector;
}


/******************************************************************************/

bool TextChunkManager::parseFile(const std::string &fileName,
				 const std::string &chunkName)
{
  TextChunk textChunk(fileName);

  if (textChunk.size() <= 0) {
    return false;
  }

  // add a new chunk name if it isn't already listed
  if (theChunks.find(chunkName) == theChunks.end()) {
    chunkNames.push_back(chunkName);
  }

  // add or replace the chunk
  theChunks[chunkName] = textChunk;

  return true;
}


const StringVector* TextChunkManager::getTextChunk(const std::string &chunkName) const
{
  TextChunkMap::const_iterator it;
  it = theChunks.find(chunkName);
  if (it != theChunks.end()){
    return &it->second.getVector();
  } else {
    return NULL;
  }
}


const StringVector& TextChunkManager::getChunkNames() const
{
  return chunkNames;
}


void TextChunkManager::reload()
{
  TextChunkMap::iterator it;
  for (it = theChunks.begin(); it != theChunks.end(); it++) {
    it->second.reload();
  }
  return;
}


/******************************************************************************/


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

