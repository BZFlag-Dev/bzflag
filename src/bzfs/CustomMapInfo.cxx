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

#include "common.h"

/* interface header */
#include "CustomMapInfo.h"

/* system headers */
#include <iostream>
#include <math.h>
#include <sstream>
#include <vector>
#include <map>

/* common implementation headers */
#include "MapInfo.h"


CustomMapInfo::CustomMapInfo()
{
  return;
}


CustomMapInfo::~CustomMapInfo()
{
  return;
}


bool CustomMapInfo::read(const char* cmd, std::istream& input)
{
  std::string line;
  std::getline(input, line);
  lines.push_back(cmd + line);
  input.putback('\n');
  return true;
}


void CustomMapInfo::writeToWorld(WorldInfo* world) const
{
  world->addMapInfo(lines);
}


// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
