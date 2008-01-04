/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CustomLink.h"

/* system implementation headers */
#include <string.h>
#include <string>


CustomLink::CustomLink()
{
  from = "";
  to = "";
}


bool CustomLink::read(const char *cmd, std::istream& input)
{
  std::string to_string;
  if (strcasecmp(cmd, "from") == 0) {
    input >> from;
  }
  else if (strcasecmp(cmd, "to") == 0) {
    input >> to;
  }
  else {
    return WorldFileObject::read(cmd, input);
  }
  return true;
}


void CustomLink::writeToWorld(WorldInfo *world) const
{
  world->addLink(from, to);
}

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
