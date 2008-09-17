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
#include "BZW/WorldObjects.h"
#include "Parser.h"

namespace BZW
{

  /// Default constructor
  World::World()
  {
  }

  /// Destructor
  World::~World()
  {

  }

  /// Read a world from stream
  void World::read(std::istream& input)
  {
    Parser p;
    /* add custom objects first */
    for(std::map<std::string, WorldObjectFactory>::iterator i = custom_objects.begin(); i != custom_objects.end(); i++)
    {
      p.addWorldObjectFactory(i->first, i->second);
    }

    /* add default objects */
    p.addWorldObjectFactory("box", World::addBox);

    /* parse */
    p.parse(input);
  }

  void World::write(std::ostream& output)
  {

  }

  bool World::registerObjectCallback(const std::string& tag, WorldObjectFactory factory)
  {
    return (custom_objects.insert(std::make_pair(tag, factory))).second;
  }

  bool World::insertWorldObject(const std::string& tag, WorldObject* wobj)
  {
    typedef std::map<std::string, std::vector<WorldObject*> >::iterator world_objects_iter;
    std::pair<world_objects_iter, bool> result;
    world_objects_iter i = world_objects.find(tag);

    if(i == world_objects.end())
    {
      result = world_objects.insert(std::make_pair(tag, std::vector<WorldObject*>()));
      if(result.second)
        i = result.first;
      else
        return false;
    }

    i->second.push_back(wobj);

    return true;
  }

  // World Objects
  static WorldObject* World::addBox()
  {
    Box* new_box = new Box();
    if(insertWorldObject("box", new_box))
      return new_box;
    else
      return NULL;
  }
}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
