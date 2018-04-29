/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef CREATE_PNG_H
#define CREATE_PNG_H

#include "common.h"

// system interface
#include <string>
#include <vector>


namespace BzPNG {

  struct Chunk {
    Chunk() {}
    Chunk(const std::string& t, const std::string& d) : type(t), data(d) {}
    Chunk(const std::string& t, // for tEXt chunks
	  const std::string& keyword,
	  const std::string& text)
	  : type(t)
	  , data(keyword + std::string(1, 0) + text)
	  {}
    std::string type; // must be 4 bytes
    std::string data;
  };

  /** Create a PNG image, string starts with "ERROR" if there's an error */
  std::string create(const std::vector<Chunk>& extraChunks,
		     size_t sx, size_t sy, size_t channels, const unsigned char* pixels);

  /** Save a PNG image */
  bool save(const std::string& filename,
	    const std::vector<Chunk>& extraChunks,
	    size_t sx, size_t sy, size_t channels, const unsigned char* pixels);
}

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
