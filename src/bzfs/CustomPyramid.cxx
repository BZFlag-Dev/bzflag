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

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

// class-interface header
#include "CustomPyramid.h"

// common-interface headers
#include "StateDatabase.h"

// implementation-specific headers
#include <math.h>

CustomPyramid::CustomPyramid()
{
  size[0] = size[1] = BZDB.eval(StateDatabase::BZDB_PYRBASE);
  size[2] = BZDB.eval(StateDatabase::BZDB_PYRHEIGHT);
  flipZ = false;
}

bool CustomPyramid::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "flipz") == 0)
    flipZ = true;
  else 
    return WorldFileObstacle::read(cmd, input);
  return true;
}


void CustomPyramid::write(WorldInfo *world) const
{
  bool flipit = flipZ;
  if (size[2] < 0.0f)
    flipit = true;
  world->addPyramid(pos[0], pos[1], pos[2], rotation, fabs(size[0]), fabs(size[1]), fabs(size[2]),driveThrough,shootThrough,flipit);
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
