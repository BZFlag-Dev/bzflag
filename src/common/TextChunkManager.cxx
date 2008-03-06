/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
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
#pragma warning(4: 4786)
#endif

/* interface header */
#include "TextChunkManager.h"

/* system implementation headers */
#include <fstream>

/* common headers */
#include "TextUtils.h"

/******************************************************************************/

TextChunk::TextChunk() :
  fileName(""),
  maxLines(-1),
  maxLineLength(-1)
{
  // for the <map>[] operator
}


TextChunk::TextChunk(const std::string& _fileName, int _maxLines, int _maxLineLength)
{
  fileName = _fileName;
  maxLines = _maxLines;
  maxLineLength = _maxLineLength < 0 ? (int)std::string::npos : _maxLineLength;
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
  std::ifstream in(fileName.c_str());

  if (!in) {
    strings.push_back(TextUtils::format("WARNING: unable to open %s", fileName.c_str()));
    return strings;
  }

  // read at most maxLines lines
  int long_lines_encountered = 0;
  int linecount = 0;
  std::string line;

  while (getline(in, line)) {

    // read at most maxLines lines (-1 = no limit)
    if (maxLines > 0 && linecount == maxLines) {
      strings.push_back(TextUtils::format("WARNING: %s has more than %d lines, truncated.", fileName.c_str(), maxLines));
      break;
    }

    // truncate long lines
    if ((int)line.size() > maxLineLength) {
      line.erase(maxLineLength);
      long_lines_encountered++;
    }

    strings.push_back(line);
    linecount++;
  }

  // warn about long lines
  if (long_lines_encountered)
    strings.push_back(TextUtils::format("WARNING: truncated %d long line%s from %s (limit of %d characters)", long_lines_encountered, long_lines_encountered == 1? "" : "s", fileName.c_str(), maxLineLength));

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
				 const std::string &chunkName,
				 const int maxLines,
				 const int maxLineLength)
{
  TextChunk textChunk(fileName, maxLines, maxLineLength);

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

