/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef _MSC_VER
#pragma warning( 4: 4786 )
#endif

#include "WorldFileObstacle.h"

WorldFileObstacle::WorldFileObstacle()
{
  pos[0] = pos[1] = pos[2] = 0.0f;
  rotation = 0.0f;
  size[0] = size[1] = size[2] = 1.0f;
  driveThrough = false;
  shootThrough = false;
  flipZ = false;
}


bool WorldFileObstacle::read(const char *cmd, std::istream& input)
{
  if (strcasecmp(cmd, "position") == 0)
    input >> pos[0] >> pos[1] >> pos[2];
  else if (strcasecmp(cmd, "rotation") == 0) {
    input >> rotation;
    rotation = rotation * M_PI / 180.0f;
  } else if (strcasecmp(cmd, "size") == 0){
    input >> size[0] >> size[1] >> size[2];
    if (size[2] < 0)
      flipZ = true;
    size[0] = fabs(size[0]);	// make sure they are postive, no more tricks
    size[1] = fabs(size[1]);
    size[2] = fabs(size[2]);
  }
  else if (strcasecmp(cmd, "drivethrough") == 0)
    driveThrough = true;
  else if (strcasecmp(cmd, "shootthrough") == 0)
    shootThrough = true;
  else if (strcasecmp(cmd, "passable") == 0)
    driveThrough = shootThrough = true;
  else if (strcasecmp(cmd, "flipz") == 0)
    flipZ = true;
  else
    return WorldFileObject::read(cmd, input);
  return true;
}

// Local variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
