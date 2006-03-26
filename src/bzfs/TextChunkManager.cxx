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


TextChunk::TextChunk(const std::string& _fileName, const int _max_lines)
{
  fileName = _fileName;
  maxLines = _max_lines;
  theVector = parse(_max_lines);
  return;
}


TextChunk::TextChunk(const TextChunk& tc)
{
  fileName = tc.fileName;
  theVector = tc.theVector;
  return;
}

#ifndef BZ_PARSE_BUFSIZE
#    define BZ_PARSE_BUFSIZE MessageLen
#endif
#if BZ_PARSE_BUFSIZE < MessageLen
#  error "BZ_PARSE_BUFSIZE is shorter than MessageLen"
#endif
 
StringVector TextChunk::parse(const int _max_lines)
{
  StringVector strings;
  char buffer[BUFSIZ] = {0};
  std::ifstream in;
  int long_lines_encountered = 0;

  in.open(fileName.c_str());
  if (!in || !in.is_open()) {
    snprintf(buffer, MessageLen, "WARNING: unable to open %s", fileName.c_str());
    strings.push_back(buffer);
    return strings;
  }

  for(int i = 0; in.good(); i++) {
    
    // pull a line from the file
    in.getline(buffer, BZ_PARSE_BUFSIZE);
    
    if (in.fail()) {
      snprintf(buffer, MessageLen, "WARNING: there was a problem reading %s", fileName.c_str());
      strings.push_back(buffer);
      break;
    }

    if (strlen(buffer) > (size_t) MessageLen - 1) {
      // line is too long, say something when we're done
      long_lines_encountered++;
    }
    buffer[MessageLen - 1] = '\0';  // terminate/clamp all lines with/to MessageLen 
    strings.push_back(buffer);
    
    if ((_max_lines > 0) && (i >= _max_lines)) {
      snprintf(buffer, MessageLen, "WARNING: %s has more than %d lines, truncated.", fileName.c_str(), _max_lines);
      strings.push_back(buffer);
      break;
    }
  } // end parsing for loop

  if (long_lines_encountered) {
    // at least one long line was encountered
    snprintf(buffer, MessageLen, "WARNING: truncated %d long line%s from %s (limit of %d characters)", long_lines_encountered, long_lines_encountered == 1? "" : "s", fileName.c_str(), MessageLen - 1);
    strings.push_back(buffer);
  }

  return strings;
}

bool TextChunk::reload()
{
  StringVector newVec = parse(maxLines);
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
				 const std::string &chunkName,
				 const int maxLines)
{
  TextChunk textChunk(fileName, maxLines);

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

