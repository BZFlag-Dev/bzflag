/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "CustomLink.h"

/* system implementation headers */
#include <string.h>
#include <string>


CustomLink::CustomLink()
{
  from = 0;
  to = 0;
}


bool CustomLink::read(const char *cmd, std::istream& input)
{
  std::string to_string;
  if (strcmp(cmd, "from") == 0) {
    input >> from;
  }
  else if (strcmp(cmd, "to") == 0) {
    input >> to_string;
    if (strcmp (to_string.c_str(), "random") == 0) {
      to = -1;
    }
    else {
      to = atoi (to_string.c_str());
    }
  }
  else {
    return WorldFileObject::read(cmd, input);
  }
  return true;
}


void CustomLink::write(WorldInfo *world) const
{
  world->addLink(from, to);
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
