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
#include "CustomGate.h"

/* system implementation headers */
#include <math.h>

/* common interface headers */
#include "StateDatabase.h"


CustomGate::CustomGate()
{
  size[0] = 0.5f * BZDB.eval(StateDatabase::BZDB_TELEWIDTH);
  size[1] = BZDB.eval(StateDatabase::BZDB_TELEBREADTH);
  size[2] = 2.0f * BZDB.eval(StateDatabase::BZDB_TELEHEIGHT);
  border = size[0] * 2.0f;
  horizontal = false;
}


bool CustomGate::read(const char *cmd, std::istream& input)
{
  if (strcmp(cmd, "border") == 0)
    input >> border;
  else if (strcmp(cmd, "horizontal") == 0)
    horizontal = true;
  else
    return WorldFileObstacle::read(cmd, input);
  return true;
}


void CustomGate::write(WorldInfo *world) const
{
  world->addTeleporter(pos[0], pos[1], pos[2], rotation, fabs(size[0]), fabs(size[1]), fabs(size[2]), border,horizontal, driveThrough,shootThrough);
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
