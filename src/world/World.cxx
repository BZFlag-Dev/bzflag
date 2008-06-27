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

namespace BZW
{

  /// Default constructor
  World::World()
  {

  }

  /// Read/Create a world from a stream
  World::World(std::istream& input)
  {
    read(in);
  }

  /// Destructor
  World::~World()
  {

  }

  /// Read a world from stream
  void World::read(std::istream& input)
  {
    Parser parser;

    Parser::Object world(false);

    //One way to do it
    Parser::Parameter name(Parser::Parameter::STRING, 1, false);
    Parser::Parameter size(Parser::Parameter::REAL, 1, false);
    Parser::Parameter flagHeight(Parser::Parameter::REAL, 1, false);
    Parser::Parameter freeCtfSpawns(Parser::Parameter::NOTHING, 0, false);

    world.manage("name", name);
    world.manage("size", size);
    world.manage("flagHeight", flagHeight);
    world.manage("freeCtfSpawns", freeCtfSpawns);

    parser.manageObject("world", world);

    Parser::Object box(true);

    //another way to do it
    Parser::Parameter position(REAL, 3);
    //this is the rotation and size property in one line.
    box.manage("rotation", Parser::Parameter(Parser::Parameter::REAL, 1, false));
    box.manage("size", Parser::Parameter(Parser::Parameter::REAL, 3, false));
    //add some already created parameters
    box.manage("name", name);

    box.manage("position", position);

    parser.manageObject("box", box);

    //actually parse
    parser.parse(in);
    std::multimap<string, Parser::Object> objects = parser.getObjects();
    //Iterate through objects, do stuff here
    //parser.getObjects(); and what have you

  }

  /// Write a world to a stream
  void World::write(std::ostream& output)
  {

  }

}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
