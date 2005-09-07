/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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


// maintains a list of lists of strings, more or less a bunch
// of files that can be read into and managed by this class.
// chunkname is the name that is used to index into this list.
// note that there is no delete function as of yet.
class TextChunkManager
{
protected:

  // wrapper to avoid compile issues on VC++
  class StringVector
  {
  public:
    size_t size() const
    {
      return theVector.size();
    }
    void push_back(const std::string &x)
    {
      theVector.push_back(x);
    }
    const std::vector<std::string>& getVector() const
    {
      // get the real vector
      return theVector;
    }
  private:
    std::vector<std::string> theVector;
  };

public:

  // load the file fileName into the chunk specified by chunkname
  // if the chunkname is already taken it will *not* be replaced
  bool parseFile(const std::string &fileName, const std::string &chunkName);

  // get a chunk given a name of the chunk returns null if it
  // can't find it
  const std::vector<std::string>* getTextChunk(const std::string &chunkName) const;

  const std::vector<std::string>& getChunkNames() const
  {
    return chunkNames;
  }

private:
  typedef std::map<std::string, StringVector> StringChunkMap;
  StringChunkMap theChunks; // a mapping of names of chunks to chunks
  std::vector<std::string> chunkNames; // vector of all the names of the chunks
};

#endif /* __TEXTCHUNKMANAGER_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

