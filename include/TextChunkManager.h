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

#ifndef __TEXTCHUNKMANAGER_H__
#define __TEXTCHUNKMANAGER_H__

#include "common.h"

/* system interface headers */
#include <vector>
#include <map>
#include <string>


typedef std::vector<std::string> StringVector;


// holds a vector of strings loaded from a file
class TextChunk {
public:
  TextChunk();
  TextChunk(const TextChunk& tc);
  TextChunk(const std::string& fileName, const int _maxLines = -1, const int _maxLineLength = -1);

  size_t size() const;
  const StringVector& getVector() const;
  bool reload();

private:
  StringVector parse();

private:
  std::string fileName;
  int maxLines;
  std::string::size_type maxLineLength;
  StringVector theVector;
};

// maintains a list of lists of strings, more or less a bunch
// of files that can be read into and managed by this class.
// chunkname is the name that is used to index into this list.
// note that there is no delete function as of yet.
class TextChunkManager {
  public:
    // load the file fileName into the chunk specified by chunkname
    // if the chunkname is already taken it will *not* be replaced
    bool parseFile(const std::string &fileName, const std::string &chunkName, const int maxLines = -1, const int maxLineLength = -1);

    // get a chunk given a name of the chunk returns null if it can't find it
    const StringVector* getTextChunk(const std::string &chunkName) const;

    // get the list of current stored chunk names
    const StringVector& getChunkNames() const;

    // reload all of the text chunks from their source files
    // (if a file's reload fails, we keep the old data)
    void reload();

  private:
    typedef std::map<std::string, TextChunk> TextChunkMap;
    TextChunkMap theChunks; // a mapping of names of chunks to chunks
    StringVector chunkNames; // vector of all the names of the chunks
};

#endif /* __TEXTCHUNKMANAGER_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

