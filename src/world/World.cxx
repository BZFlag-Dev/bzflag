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

#include "BZW/World.h"
#include "WorldObjects.h"
#include "Parser.h"

namespace BZW
{

  /// Default constructor
  World::World()
  {
  }

  /// Read/Create a world from a stream
  World::World(std::istream& input)
  {
    /* TODO: decide if this should parse immediately or store the stream
     * untul a parse method is called, to allow for registration of custom
     * objects, or whatever.
     */
  }

  /// Destructor
  World::~World()
  {

  }

  /// Read a world from stream
  void World::read(std::istream& input)
  {
    Parser p;
    //TODO register objects with parser
  }

  void World::write(std::ostream& output)
  {

  }

  void World::registerObject(std::string tag, WorldObjectFactory factory)
  {
    customObjects.insert(std::make_pair(tag, factory));
  }

  // World Objects
  Box* World::addBox()
  {
    Box* new_box = new Box();
    world_objects.push_back(new_box);
    return new_box;
  }
}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
