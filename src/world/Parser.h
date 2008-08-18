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

#ifndef __BZW_PARSER_H__
#define __BZW_PARSER_H__

#include <string>
#include <map>
#include <vector>
#include <iostream>

/* boost headers */
#include <boost/spirit/core.hpp>
/* BZW */
#include "BZW/World.h"
#include "BZW/WorldObject.h"
#include "Grammar.h"

namespace BZW
{
  class Parser
  {
    public:
      /// Constructor
      Parser();
      /// Destructor
      ~Parser();
      /// Parse a BZW file from an istream
      void Parse(std::istream& input);
      /// Register a WorldObjectFactory callback
      bool addWorldObjectFactory(const std::string& tag, WorldObjectFactory factory);
    private:
      std::map<std::string, World::WorldObjectFactory> factories;
      WorldObject* current_object;

  };
}

#endif // __BZW_PARSER_H__

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

