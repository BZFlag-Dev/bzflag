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

#include "BZW/Box.h"

namespace BZW
{
  /// Default constructor
  World::World()
  {
    Register<GenericWorldObject>("generic");
    Register<Box>("box");
  }

  /// Destructor
  World::~World()
  {

  }

  /// Read a world from stream
  void World::read(std::istream& input)
  {
    Parser p(&this);
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

  WorldObject* World::newObject ( const std::string &name )
  {
    WorldObject *obj = NULL;

    if (IsRegistered(name))
      obj = Create(name);
    else
    {
      obj = Create("generic");
      ((GenericWorldObject*)obj)->className = name;

      CallbackMap::iterator itr = customCallbacks.find(name);
      if (itr != customCallbacks.end())
        ((GenericWorldObject*)obj)->callbacks = itr->second;
    }

    worldObjects.push_back(obj);
  }

  void World::addCallback ( const std::string &object, CustomObjectCallback *callback )
  {
    CallbackMap::iterator itr = customCallbacks.find(object);

    if (itr != customCallbacks.end())
      itr->second.push_back(callback);
    else
    {
      std::list<CustomObjectCallback*> tmp;
      customCallbacks[object] = tmp;
      customCallbacks[object].push_back(callback);
    }
  }

  void World::removeCallback ( const std::string &object, CustomObjectCallback *callback )
  {
    CallbackMap::iterator itr = customCallbacks.find(object);

    if (itr != customCallbacks.end())
    {
      std::list<CustomObjectCallback*>::iterator listItr = itr->second.begin();
      while (listItr != itr->second.end())
      {
        if (*listItr == callback)
        {
          itr->second.erase(listItr);
          return;
        }
      }
    }
  }
}
// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
