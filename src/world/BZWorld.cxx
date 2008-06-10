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

#include "BZWorld.h"

// Read a world in from a file
World::World(const std::string &filename) : cURLManager(),
			      location(filename),
			      input(NULL),
			      fromBlob(false)
{
  
}

// Read a world in from a blob (cURL)
World::World(std::istream &in) : cURLManager(),
			      location("blob"),
			      input(&in),
			      fromBlob(true)
{
  //TODO: add some error handling here
  if(input->peek() == EOF) {
    //throw a fatal error "Could not find bzflag world file"
  }
}

World::~World()
{

}

// Local Variables: ***
// Mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
